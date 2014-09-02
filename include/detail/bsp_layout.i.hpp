#pragma once
#include "bsp_layout.hpp"
#include "generate.hpp"
#include "random.hpp"

namespace bkrl { namespace detail {

class bsp_layout_impl : public bsp_layout_base {
public:
    using index_t        = unsigned;

    struct node_t_ {
        enum : index_t {
            index_reserved = 0
          , index_none = static_cast<unsigned>(-1) 
        };

        explicit node_t_(
            grid_region const region
          , index_t     const parent_index = index_none
          , index_t     const child_index  = index_none
        )
          : region {region}
          , child_index {child_index}
          , parent_index {parent_index}
        {
        }

        bool is_leaf()     const noexcept { return is_reserved() || child_index == index_none; }
        bool is_reserved() const noexcept { return child_index == index_reserved; }

        grid_region region;
        index_t     child_index  = index_none;
        index_t     parent_index = index_none;
    };

    bsp_layout_impl(
        random::generator&    gen
      , params_t       const& params
      , split_callback const& on_split
      , room_callback  const& on_room_gen
      , grid_region    const& reserve
    );

    //--------------------------------------------------------------------------
    //! split along the y (vertical) axis.
    //--------------------------------------------------------------------------
    void split_y(index_t const index, grid_index const where);

    //--------------------------------------------------------------------------
    //! split along the x (horizontal) axis.
    //--------------------------------------------------------------------------
    void split_x(index_t const index, grid_index const where);

    //--------------------------------------------------------------------------
    //! Add @children to the node at @index.
    //--------------------------------------------------------------------------
    void add_children(index_t const index, grid_region const& child0, grid_region const& child1);

    //--------------------------------------------------------------------------
    //! Top-level split function. Recursively divide breadth-first.
    //--------------------------------------------------------------------------
    void split(random::generator& gen);

    //--------------------------------------------------------------------------
    //! Decide if node can be split.
    //! @returns true if @p node can be split, false otherwise.
    //--------------------------------------------------------------------------
    bool can_split(node_t_ const& node) const;

    //--------------------------------------------------------------------------
    //! split axis.
    //--------------------------------------------------------------------------
    enum class split_type {
        split_none, split_x, split_y
    };

    //--------------------------------------------------------------------------
    //! Decide which axis to split @node along.
    //! @pre can_split(gen, node)
    //--------------------------------------------------------------------------
    split_type choose_split_type(random::generator& gen, node_t_ const& node) const;

    //--------------------------------------------------------------------------
    //! Split one node into two nodes.
    //! @returns true if the node was split, false otherwise.
    //! @pre index < nodes_.size()
    //--------------------------------------------------------------------------
    bool split(random::generator& gen, index_t const index);

    using range_t = range<unsigned>;

    void connect(random::generator& gen, connect_callback on_connect);

    range_t connect(random::generator& gen, index_t i, unsigned n = 0);

    void gen_rooms(random::generator& gen, index_t i);
private:
    std::vector<node_t_> nodes_;
    std::vector<index_t> connected_nodes_;
    room_callback        on_room_gen_;
    split_callback       on_split_;
    params_t             params_;
    unsigned             next_room_id_ = 0;
    connect_callback     on_connect_;
};

//--------------------------------------------------------------------------
bsp_layout_impl::bsp_layout_impl(
    random::generator&    gen
  , params_t       const& params
  , split_callback const& on_split
  , room_callback  const& on_room_gen
  , grid_region    const& reserve
)
  : params_      {params}
  , on_room_gen_ {on_room_gen}
  , on_split_    {on_split}
{
    auto const& p = params_;
    nodes_.reserve((p.height * p.width) / (p.max_region_w * p.min_region_h));
    nodes_.emplace_back(grid_region {0, 0, p.width, p.height});
        
    if (reserve.width() && reserve.height()) {
        if (reserve.width() > p.width || reserve.height() > p.height) {
            BK_TODO_FAIL();
        }

        split_y(0, reserve.right);
        split_x(1, reserve.bottom);
        split_y(3, reserve.left);
        split_x(6, reserve.top);

        nodes_[8].child_index = node_t_::index_reserved;
    }

    split(gen);

    on_room_gen_  = room_callback  {};
    on_split_     = split_callback {};
}

//--------------------------------------------------------------------------
void
bsp_layout_impl::split_y(
    index_t    const index
  , grid_index const where
) {
    BK_PRECONDITION(index < nodes_.size());

    auto& node = nodes_[index];
    auto const split = bkrl::split_y(node.region, where);
    add_children(index, split.first, split.second);
}

//--------------------------------------------------------------------------
void
bsp_layout_impl::split_x(
    index_t    const index
  , grid_index const where
) {
    BK_PRECONDITION(index < nodes_.size());

    auto& node = nodes_[index];
    auto const split = bkrl::split_x(node.region, where);
    add_children(index, split.first, split.second);
}

//--------------------------------------------------------------------------
void
bsp_layout_impl::add_children(
    index_t     const  index
  , grid_region const& child0
  , grid_region const& child1
) {
    BK_PRECONDITION(index < nodes_.size());

    auto& parent = nodes_[index];

    BK_PRECONDITION(parent.is_leaf());

    node_t_ n0 {child0, index};
    node_t_ n1 {child1, index};

    auto const ok0 = on_split_(n0.region);
    auto const ok1 = on_split_(n1.region);

    if (!ok0) {
        n0.child_index = node_t_::index_reserved;
    }

    if (!ok1) {
        n1.child_index = node_t_::index_reserved;
    }

    parent.child_index = nodes_.size();

    nodes_.push_back(n0);
    nodes_.push_back(n1);
}

//--------------------------------------------------------------------------
void
bsp_layout_impl::split(
    random::generator& gen
) {
    //
    // split all nodes from [beg, end).
    //
    auto const split_all = [&](size_t const beg, size_t const end) {
        for (auto i = beg; i < end; ++i) {
            auto const did_split = split(gen, i);
        }

        return nodes_.size();
    };

    // TODO could improve this for clarity?
    // split while there are unvisited nodes
    auto beg = 0;
    auto end = nodes_.size();

    for (;;) {
        auto const new_end = split_all(beg, end);

        if (new_end - end == 0) {
            break;
        }

        beg = end;
        end = new_end;
    }

    gen_rooms(gen, 0);
}

void
bsp_layout_impl::gen_rooms(
    random::generator& gen
  , index_t const i
) {
    BK_PRECONDITION(i < nodes_.size());
    auto& node = nodes_[i];

    if (node.is_leaf()) {
        auto const is_reserved = node.is_reserved();
        auto const roll = random::uniform_range(gen, 0u, 100u);

        if (is_reserved || roll < params_.room_gen_chance) {
            std::cout << "room generated at index = " << i << std::endl;
            node.child_index = node_t_::index_reserved;
            on_room_gen_(node.region, ++next_room_id_);
        }

        return;
    }

    auto const lhs = node.child_index + 0;
    auto const rhs = node.child_index + 1;

    BK_ASSERT(lhs != node_t_::index_none);

    auto const& left  = nodes_[lhs];
    auto const& right = nodes_[rhs];

    if (left.parent_index != right.parent_index) {
        gen_rooms(gen, lhs);
        return;
    }

    gen_rooms(gen, lhs);
    gen_rooms(gen, rhs);
}

//--------------------------------------------------------------------------
bool
bsp_layout_impl::can_split(
    node_t_ const& node
) const {
    if (!node.is_leaf()) {
        return false;
    } else if (node.is_reserved()) {
        return false;
    }

    auto const  w = node.region.width();
    auto const  h = node.region.height();
    auto const& p = params_;

    if (w < p.min_region_w * 2 && h < p.min_region_h * 2) {
        //too small - can't split
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------
bsp_layout_impl::split_type 
bsp_layout_impl::choose_split_type(
    random::generator& gen
  , node_t_ const&     node
) const {
    auto const& p = params_;
    auto const  w = node.region.width();
    auto const  h = node.region.height();

    auto const ok_x = h >= p.min_region_h * 2;
    auto const ok_y = w >= p.min_region_w * 2;

    BK_PRECONDITION(ok_x || ok_y);

    //TODO could be simplified?
    if (ok_x && !ok_y) {
        return split_type::split_x;
    } else if (!ok_x && ok_y) {
        return split_type::split_y;
    }

    //
    // if the max aspect ration is exceeded, split
    //
    auto const aspect = (w >= h)
        ? (static_cast<float>(w) / h)
        : (static_cast<float>(h) / w);

    if (aspect > p.max_aspect_ratio) {
        return (w >= h) ? split_type::split_y : split_type::split_x;
    }

    //
    // else, if small enough, reject split based on params_t::split_chance
    //
    if (w <= p.max_region_w && h <= p.max_region_h) {
        auto const roll = random::uniform_range(gen, 0u, 100u);
        if (roll < p.split_chance) {
            return split_type::split_none;
        }
    }

    //
    // otherwise, split randomly 50/50
    //
    auto const roll = random::uniform_range(gen, 0, 100);
    return roll < 50 ? split_type::split_y : split_type::split_x;
}

//--------------------------------------------------------------------------
bool
bsp_layout_impl::split(
    random::generator& gen
  , index_t const      index
) {
    BK_PRECONDITION(index < nodes_.size());

    auto& node = nodes_[index];
    
    if (!can_split(node)) {
        return false;
    }

    auto const& p = params_;
    auto const& r = node.region;
    auto const  w = r.width();
    auto const  h = r.height();

    auto const split_dir = choose_split_type(gen, node);

    auto distribution = [&](unsigned const lo, unsigned const hi) {
        return random::uniform_range(gen, lo, hi);
    };

    if (split_dir == split_type::split_x) {
        auto const where = distribution(p.min_region_h, h - p.min_region_h);
        split_x(index, r.top + where);
    } else if (split_dir == split_type::split_y) {
        auto const where = distribution(p.min_region_w, w - p.min_region_w);
        split_y(index, r.left + where);
    } else if (split_dir == split_type::split_none) {
        node.child_index = node_t_::index_none;
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------
void
bsp_layout_impl::connect(
    random::generator& gen
  , connect_callback   on_connect
) {
    BK_ASSERT(connected_nodes_.empty());
    connected_nodes_.reserve(next_room_id_);

    on_connect_ = on_connect;
    connect(gen, 0);
    on_connect_ = connect_callback {};
}

//--------------------------------------------------------------------------
bsp_layout_impl::range_t
bsp_layout_impl::connect(
    random::generator& gen
  , index_t const      i
  , unsigned const n
) {
    //std::cout << std::string(n*4, '-');
    //std::cout << "connect " << i << " ";

    BK_PRECONDITION(i < nodes_.size());
    auto const& node = nodes_[i];

    auto const size = connected_nodes_.size();

    if (node.is_leaf()) {
        if (node.is_reserved()) {
            //std::cout << "leaf -> room" << std::endl;
            connected_nodes_.emplace_back(i);
            return range_t{size, size + 1};
        } else {
            //std::cout << "leaf -> empty" << std::endl;
            return range_t{size, size};
        }
    }

    auto const i0 = node.child_index + 0;
    auto const i1 = node.child_index + 1;

    BK_ASSERT(i0 != node_t_::index_none);

    auto const& left  = nodes_[i0];
    auto const& right = nodes_[i1];

    if (left.parent_index != right.parent_index) {
        //std::cout << "parent -> 1 child" << std::endl;
        return connect(gen, i0, n+1);
    }

    //std::cout << "parent -> 2 child" << std::endl;

    auto const lhs = connect(gen, i0, n+1);
    auto const rhs = connect(gen, i1, n+1);

    //std::cout << std::string(n*4, '-');
    //std::cout << "connected" << std::endl;

    if (lhs.size() == 0) {
        return rhs;
    } else if (rhs.size() == 0) {
        return lhs;
    }

    auto const which0 = random::uniform_range(gen, lhs.lo, lhs.hi - 1);
    auto const which1 = random::uniform_range(gen, rhs.lo, rhs.hi - 1);

    on_connect_(node.region, which0 + 1, which1 + 1);

    BK_ASSERT(lhs.hi == rhs.lo);

    return range_t {lhs.lo, rhs.hi};
}

}} //namespace bkrl::detail
