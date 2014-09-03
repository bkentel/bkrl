#pragma once
#include "bsp_layout.hpp"
#include "generate.hpp"
#include "random.hpp"

namespace bkrl { namespace detail {

//==============================================================================
//! Implementation for bkrl::bsp_layout
//==============================================================================
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
    //! split nodes_[index] along the y (vertical) axis, and add the result
    //! as children of nodes_[index].
    //--------------------------------------------------------------------------
    void split_y(index_t const index, grid_index const where);

    //--------------------------------------------------------------------------
    //! split nodes_[index] along the x (vertical) axis, and add the result
    //! as children of nodes_[index].
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
    //! Decide if a room should be generated at @p node.
    //--------------------------------------------------------------------------
    bool can_generate_room(random::generator& gen, node_t_ const& node) const;

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

    range_t connect(random::generator& gen, index_t index);

    void gen_rooms(random::generator& gen, index_t index = 0);

    //--------------------------------------------------------------------------
    //! If @p region is not 0 in size, pre-split such that @p region is a
    //! region in the final layout.
    //--------------------------------------------------------------------------
    void reserve_region(grid_region const& region);
private:
    std::vector<node_t_> nodes_;
    room_callback        on_room_gen_;
    split_callback       on_split_;
    params_t             params_;
    unsigned             next_room_id_ = 0;
    std::vector<index_t> connected_nodes_;
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
    //TODO verify params
    auto const& p = params_;
    auto const  n = (p.height * p.width) / (p.min_region_w * p.min_region_h);

    nodes_.reserve(n);
    nodes_.emplace_back(grid_region {0u, 0u, p.width, p.height});
        
    reserve_region(reserve);
    split(gen);
    gen_rooms(gen);

    on_room_gen_  = room_callback  {};
    on_split_     = split_callback {};
}
//--------------------------------------------------------------------------
void
bsp_layout_impl::reserve_region(
    grid_region const& region
) {
    BK_PRECONDITION(nodes_.size() == 1);
    BK_PRECONDITION(region.right  <= params_.width);
    BK_PRECONDITION(region.bottom <= params_.height);

    if (region.area() == 0) {
        return;
    }

    split_y(0, region.right);
    split_x(1, region.bottom);
    split_y(3, region.left);
    split_x(6, region.top);

    nodes_[8].child_index = node_t_::index_reserved;
}

//--------------------------------------------------------------------------
//TODO unify split_x and split_y
void
bsp_layout_impl::split_y(
    index_t    const index
  , grid_index const where
) {
    BK_PRECONDITION(index < nodes_.size());

    auto& node = nodes_[index];
    BK_PRECONDITION(node.is_leaf() && !node.is_reserved());

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
    BK_PRECONDITION(node.is_leaf() && !node.is_reserved());

    auto const split = bkrl::split_x(node.region, where);
    add_children(index, split.first, split.second);
}

//--------------------------------------------------------------------------
void
bsp_layout_impl::add_children(
    index_t     const  parent_index
  , grid_region const& child0
  , grid_region const& child1
) {
    BK_PRECONDITION(parent_index < nodes_.size());

    auto& parent = nodes_[parent_index];

    BK_PRECONDITION(parent.is_leaf() && !parent.is_reserved());

    node_t_ n0 {child0, parent_index};
    node_t_ n1 {child1, parent_index};

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

    //
    // continue to split as long as new nodes have been added.
    //
    auto beg = 0;
    auto end = nodes_.size();

    for (auto size = end - beg; size != 0; size = end - beg) {
        auto const new_end = split_all(beg, end);

        beg = end;
        end = new_end;
    }
}

//--------------------------------------------------------------------------
bool
bsp_layout_impl::can_generate_room(
    random::generator& gen
  , node_t_ const&     node
) const {
    if (!node.is_leaf()) {
        return false;
    } else if (node.is_reserved()) {
        return true;
    }

    auto const roll = random::uniform_range(gen, 0u, 100u);
    return roll < params_.room_gen_chance;
}

//--------------------------------------------------------------------------
void
bsp_layout_impl::gen_rooms(
    random::generator& gen
  , index_t const      index
) {
    BK_PRECONDITION(index < nodes_.size());
    auto& node = nodes_[index];

    //
    // base case
    //
    if (can_generate_room(gen, node)) {
        std::cout << "room generated at index = " << index << std::endl;
        node.child_index = node_t_::index_reserved;
        on_room_gen_(node.region, ++next_room_id_); //increment

        return;
    }
    
    // possible to gen, but rejected
    if (node.is_leaf()) {
        return;
    }
   
    //
    // recursive case
    //
    auto const lhs = node.child_index + 0;
    auto const rhs = node.child_index + 1;

    BK_ASSERT(lhs != node_t_::index_none);

    auto const& left  = nodes_[lhs];
    auto const& right = nodes_[rhs];

    //only one child
    BK_ASSERT(left.parent_index == right.parent_index);
    //if (left.parent_index != right.parent_index) {
    //    gen_rooms(gen, lhs);
    //    return;
    //}

    gen_rooms(gen, lhs);
    gen_rooms(gen, rhs);
}

//--------------------------------------------------------------------------
bsp_layout_impl::split_type 
bsp_layout_impl::choose_split_type(
    random::generator& gen
  , node_t_ const&     node
) const {
    BK_PRECONDITION(node.is_leaf());
    BK_PRECONDITION(!node.is_reserved());

    auto const& p = params_;
    auto const  w = node.region.width();
    auto const  h = node.region.height();

    //
    // if one or more dimensions are too small
    //
    auto const ok_x = h >= p.min_region_h * 2;
    auto const ok_y = w >= p.min_region_w * 2;

    if (ok_x && !ok_y) {
        return split_type::split_x;
    } else if (!ok_x && ok_y) {
        return split_type::split_y;
    } else if (!ok_x && !ok_y) {
        return split_type::split_none;
    }

    //
    // if the max aspect ration is exceeded
    //
    auto const aspect = (w >= h)
        ? (static_cast<float>(w) / h)
        : (static_cast<float>(h) / w);

    if (aspect > p.max_aspect_ratio) {
        return (w >= h) ? split_type::split_y : split_type::split_x;
    }

    //
    // if one dimension is too big
    //
    auto const big_x = h > p.max_region_w;
    auto const big_y = w > p.max_region_h;

    if (big_x && !big_y) {
        return split_type::split_x;
    } else if (big_y && !big_x) {
        return split_type::split_y;
    } else if (!big_x && !big_y) {
        //
        // reject based on params_t::split_chance
        //
        auto const roll = random::uniform_range(gen, 0u, 100u);
        if (roll >= p.split_chance) {
            return split_type::split_none;
        }
    }

    //
    // otherwise, split randomly 50/50
    //
    auto const roll = random::uniform_range(gen, 0u, 100u);
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
    
    //
    // don't split if reserved or an internal node
    //
    if (!node.is_leaf()) {
        return false;
    } else if (node.is_reserved()) {
        return false;
    }

    auto const split_dir = choose_split_type(gen, node);
    
    // no split required / possible
    if (split_dir == split_type::split_none) {
        node.child_index = node_t_::index_none;
        return false;
    }
    
    //
    // chose a split point
    //
    auto const& p = params_;
    auto const& r = node.region;
    auto const  w = r.width();
    auto const  h = r.height();

    auto distribution = [&](unsigned const lo, unsigned const hi) {
        return random::uniform_range(gen, lo, hi);
    };

    if (split_dir == split_type::split_x) {
        auto const where = distribution(p.min_region_h, h - p.min_region_h);
        split_x(index, r.top + where);
    } else if (split_dir == split_type::split_y) {
        auto const where = distribution(p.min_region_w, w - p.min_region_w);
        split_y(index, r.left + where);
    } else {
        // ought to be impossible
        BK_TODO_FAIL();
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
  , index_t const      index
) {
    BK_PRECONDITION(index < nodes_.size());
    auto const& node = nodes_[index];

    //
    // base case
    //
    if (node.is_leaf()) {
        auto const size = connected_nodes_.size();

        if (node.is_reserved()) {
            connected_nodes_.emplace_back(index);
        }

        return range_t {size, connected_nodes_.size()};
    }

    //
    // recursive case
    //
    auto const i0 = node.child_index + 0;
    auto const i1 = node.child_index + 1;

    BK_ASSERT(i0 != node_t_::index_none);

    auto const& left  = nodes_[i0];
    auto const& right = nodes_[i1];

    //only one child
    BK_ASSERT(left.parent_index == right.parent_index);
    //if (left.parent_index != right.parent_index) {
    //    return connect(gen, i0);
    //}

    auto const lhs = connect(gen, i0);
    auto const rhs = connect(gen, i1);

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
