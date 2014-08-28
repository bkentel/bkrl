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

    auto const w = dist.generate(gen, 4u, bounds.width());
    auto const h = dist.generate(gen, 4u, bounds.height());

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

    //vertical
    void split_y(index_t const index, grid_index const where) {
        BK_PRECONDITION(index < nodes_.size());

        auto& node = nodes_[index];
        split(node, index, generate::split_y(node.region, where));
    }

    //horizontal
    void split_x(index_t const index, grid_index const where) {
        BK_PRECONDITION(index < nodes_.size());

        auto& node = nodes_[index];
        split(node, index, generate::split_x(node.region, where));
    }

    void split(
        node_t_&          parent
      , index_t     const index
      , region_pair const children
    ) {
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

    void split(random::generator& gen) {
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

    bool split(random::generator& gen, index_t const index) {
        auto const& node = nodes_[index];
    
        if (!node.is_leaf()) {
            return false;
        } else if (node.is_reserved()) {
            return false;
        }

        auto const& region = node.region;
        auto const  w      = region.width();
        auto const  h      = region.height();

        auto const& p = params_;
        //--------------------------------------------------------------------------
        //if small enough, decide randomly whether to split
        if ((w < p.max_region_w) && (h < p.max_region_h)) {
            auto const split_chance = params_.split_chance;
            auto const do_split     = random::uniform_range(gen, 0, 100) < split_chance;

            if (!do_split) {
                return false;
            }
        }

        //--------------------------------------------------------------------------
        // choose a split direction for this node
        enum {
            split_none, split_x_axis, split_y_axis
        };

        auto const get_split_type = [&] {
            auto const ok_x = (h >= p.min_region_h * 2);
            auto const ok_y = (w >= p.min_region_w * 2);

            if (
                (w > h) && (ok_y)
             && (static_cast<float>(w) / h > p.max_aspect_ratio)
            ) {
                return split_y_axis;
            } else if (
                (h > w) && (ok_x)
             && (static_cast<float>(h) / w > p.max_aspect_ratio)
            ) {
                return split_x_axis;
            } else if (ok_x && ok_y) {
                return (random::uniform_range(gen, 0, 100) < 50)
                  ? split_x_axis
                  : split_y_axis;
            } else if (ok_x) {
                return split_x_axis;
            } else if (ok_y) {
                return split_y_axis;
            }

            return split_none; 
        };
        //--------------------------------------------------------------------------

        auto distribution = [&](unsigned const lo, unsigned const hi) {
            return random::uniform_range(gen, lo, hi);
        };

        auto const do_split_x = [&] {
            split_x(index, region.top + distribution(p.min_region_h, h - p.min_region_h));
        };

        auto const do_split_y = [&] {
            split_y(index, region.left + distribution(p.min_region_w, w - p.min_region_w));
        };

        switch (get_split_type()) {
        case split_x_axis : do_split_x(); return true;
        case split_y_axis : do_split_y(); return true;
        case split_none   : //fallthrough
        default           : break;
        }

        return false;
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


