#include "generate.hpp"
#include "random.hpp"

using bkrl::generate::bsp_layout;
using bkrl::generate::simple_room;
using bkrl::generate::circle_room;

namespace random = bkrl::random;
using bkrl::room;
using bkrl::grid_region;

////////////////////////////////////////////////////////////////////////////////
// simple_room
////////////////////////////////////////////////////////////////////////////////
room simple_room::generate(random::generator& gen, grid_region const bounds) {
    room result {bounds};

    random::uniform_int dist;

    auto const w = bounds.width(); //dist.generate(gen, 4u, bounds.width());
    auto const h = bounds.height(); //dist.generate(gen, 4u, bounds.height());

    auto const slack_w = bounds.width()  - w;
    auto const slack_h = bounds.height() - h;

    auto const left   = dist.generate(gen, 0u, slack_w);
    auto const top    = dist.generate(gen, 0u, slack_h);
    auto const right  = left + w;
    auto const bottom = top + h;

    auto const edge_count = [w, h](grid_index const x, grid_index const y) {
        return ((x == 0) || (x == w - 1) ? 1 : 0u)
             + ((y == 0) || (y == h - 1) ? 1 : 0u);
    };

    for (grid_index yi = 0; yi < h; ++yi) {
        for (grid_index xi = 0; xi < w; ++xi) {
            auto const x = xi + left;
            auto const y = yi + top;

            switch (edge_count(xi, yi)) {
            case 0 :
                result.set(attribute::tile_type, x, y, tile_type::floor);
                break;
            case 1 :
                if (xi + yi == w + h - 4) {
                    result.set(attribute::tile_type, x, y, tile_type::door);
                    break;
                }
            case 2 :
                result.set(attribute::tile_type, x, y, tile_type::wall);
                break;
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
// circle_room
////////////////////////////////////////////////////////////////////////////////
room circle_room::generate(random::generator&, grid_region bounds) {
    auto const w = bounds.width();
    auto const h = bounds.height();
    auto const x0 = w / 2;
    auto const y0 = h / 2;
        
    auto const r = std::min(w, h) / 2.0;
    auto const r2 = static_cast<unsigned>(std::floor(r*r));
    auto const rr = static_cast<unsigned>(std::floor((r - 1.25)*(r - 1.25))); //hack

    room result {bounds};

    for_each_xy(result, [&](unsigned x, unsigned y) {
        auto const xx = static_cast<unsigned>(x - x0);
        auto const yy = static_cast<unsigned>(y - y0);

        if (xx*xx + yy*yy <= r2) {

            if (xx*xx + yy*yy >= rr) {
                result.set(attribute::tile_type, x, y, tile_type::wall);
            } else {
                result.set(attribute::tile_type, x, y, tile_type::floor);
            }

        } 
    });
    
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// bsp_layout
////////////////////////////////////////////////////////////////////////////////

//! implementation
class bsp_layout::impl_t {
public:
    using index_t = unsigned;

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

        bool is_leaf()     const noexcept { return child_index == index_none; }
        bool is_reserved() const noexcept { return child_index == index_reserved; }

        grid_region region;
        index_t     child_index  = index_none;
        index_t     parent_index = index_none;
    };

    impl_t(
        random::generator&    gen
      , params_t       const& params
      , split_callback const& on_split
      , room_callback  const& on_room_gen
      , grid_region    const& reserve
    )
      : params_ {params}
      , on_room_gen_ {on_room_gen}
      , on_split_ {on_split}
    {
        auto const& p = params_;
        nodes_.emplace_back(grid_region {0, 0, p.width, p.height});
        
        if (reserve.width() && reserve.height()) {
            if (reserve.width() > p.width || reserve.height() > p.height) {
                BK_TODO_FAIL();
            }

            split_y(0, reserve.right);
            split_x(1, reserve.bottom);
            split_y(3, reserve.left);
            split_x(6, reserve.top);

            nodes_[8].child_index = 0;
        }

        split(gen);
    }

    //--------------------------------------------------------------------------
    //! split along the y (vertical) axis.
    //--------------------------------------------------------------------------
    void split_y(index_t const index, grid_index const where) {
        BK_PRECONDITION(index < nodes_.size());

        auto& node = nodes_[index];
        add_children(index, generate::split_y(node.region, where));
    }

    //--------------------------------------------------------------------------
    //! split along the x (horizontal) axis.
    //--------------------------------------------------------------------------
    void split_x(index_t const index, grid_index const where) {
        BK_PRECONDITION(index < nodes_.size());

        auto& node = nodes_[index];
        add_children(index, generate::split_x(node.region, where));
    }

    //--------------------------------------------------------------------------
    //! Add @children to the node at @index.
    //--------------------------------------------------------------------------
    void add_children(index_t const index, region_pair const children) {
        BK_PRECONDITION(index < nodes_.size());

        auto& parent = nodes_[index];

        BK_PRECONDITION(parent.is_leaf());

        node_t_ n0 {std::get<0>(children), index};
        node_t_ n1 {std::get<1>(children), index};

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
    //! Top-level split function. Recursively divide breadth-first.
    //--------------------------------------------------------------------------
    void split(random::generator& gen) {
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

        for (auto const& node : nodes_) {
            if (node.is_leaf()) {
                auto const roll = random::uniform_range(gen, 0, 100);
                if (roll < params_.room_gen_change) {
                    on_room_gen_(node.region);
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    //! Decide if node can be split.
    //! @returns true if @p node can be split, false otherwise.
    //--------------------------------------------------------------------------
    bool can_split(node_t_ const& node) const {
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
    //! split axis.
    //--------------------------------------------------------------------------
    enum class split_type {
        split_none, split_x, split_y
    };

    //--------------------------------------------------------------------------
    //! Decide which axis to split @node along.
    //! @pre can_split(gen, node)
    //--------------------------------------------------------------------------
    split_type choose_split_type(random::generator& gen, node_t_ const& node) const {
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
            auto const roll = random::uniform_range(gen, 0, 100);
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
    //! Split one node into two nodes.
    //! @returns true if the node was split, false otherwise.
    //! @pre index < nodes_.size()
    //--------------------------------------------------------------------------
    bool split(random::generator& gen, index_t const index) {
        BK_PRECONDITION(index < nodes_.size());

        auto const& node = nodes_[index];
    
        if (!can_split(node)) {
            return false;
        }

        auto const& p = params_;
        auto const  w = node.region.width();
        auto const  h = node.region.height();

        auto const split_dir = choose_split_type(gen, node);

        auto distribution = [&](unsigned const lo, unsigned const hi) {
            return random::uniform_range(gen, lo, hi);
        };

        if (split_dir == split_type::split_x) {
            auto const where = distribution(p.min_region_h, h - p.min_region_h);
            split_x(index, node.region.top + where);
        } else if (split_dir == split_type::split_y) {
            auto const where = distribution(p.min_region_w, w - p.min_region_w);
            split_y(index, node.region.left + where);
        } else if (split_dir == split_type::split_none) {
            return false;
        }

        return true;
    }

    std::vector<index_t> connect(random::generator& gen, index_t const i) {
        BK_PRECONDITION(i < nodes_.size());

        auto const& node = nodes_[i];

        if (node.is_leaf()) {
            return {i};
        }

        auto const i0 = node.child_index + 0;
        auto const i1 = node.child_index + 1;

        return connect(gen, i0, i1);
    }

    std::vector<index_t> connect(random::generator& gen, index_t const left, index_t const right) {
        auto const lhs = connect(gen, left);
        auto const rhs = connect(gen, right);

        auto const i_left  = random::uniform_range(gen, 0, lhs.size() - 1);
        auto const i_right = random::uniform_range(gen, 0, rhs.size() - 1);

    }

private:
    std::vector<node_t_> nodes_;
    room_callback        on_room_gen_;
    split_callback       on_split_;
    params_t             params_;
};

bsp_layout::~bsp_layout() = default;

bsp_layout::bsp_layout(bsp_layout&& other) 
  : impl_ {std::move(other.impl_)}
{
}

bsp_layout::bsp_layout(
    random::generator&    gen
  , params_t       const& params
  , split_callback const& on_split
  , room_callback  const& on_room_gen
  , grid_region    const& reserve
)
  : impl_ {std::make_unique<impl_t>(gen, params, on_split, on_room_gen, reserve)}
{
}

bsp_layout bsp_layout::generate(
    random::generator&    gen
  , params_t       const& params
  , split_callback const& on_split
  , room_callback  const& on_room_gen
  , grid_region    const& reserve
) {
    return bsp_layout {gen, params, on_split, on_room_gen, reserve};
}


