#include "generate.hpp"
#include "random.hpp"

using bkrl::generate::bsp_layout;
using bkrl::generate::simple_room;
namespace random = bkrl::random;
using bkrl::generate::split_result;
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

    for (grid_index yi = 0; yi < h; ++yi) {
        for (grid_index xi = 0; xi < w; ++xi) {
            auto const x = xi + left;
            auto const y = yi + top;

            if (xi == 0 || yi == 0 || xi == w - 1 || yi == h - 1) {
                result.set(attribute::type, x, y, tile_type::wall);
            } else {
                result.set(attribute::type, x, y, tile_type::floor);
            }
        }
    }

    //tmp
    result.set(attribute::type, left + 2, top + 2, tile_type::wall);

    return result;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

split_result bkrl::generate::split_vertical(
    random::range_generator const& gen
  , grid_region const              region
  , unsigned const                 minimum 
) {
    auto const w = region.width();

    if (w <= minimum * 2) {
        return std::make_tuple(false, region, region);
    }

    auto const slack = w - minimum * 2;
    auto const split = gen(0, slack);
    
    auto const l0 = region.left;
    auto const t0 = region.top;
    auto const r0 = l0 + minimum + split;
    auto const b0 = region.bottom;

    auto const l1 = r0;
    auto const t1 = region.top;
    auto const r1 = region.right;
    auto const b1 = region.bottom;

    return std::make_tuple(
        true
      , bkrl::grid_region {l0, t0, r0, b0}
      , bkrl::grid_region {l1, t1, r1, b1}
    );
}

split_result bkrl::generate::split_horizontal(
    random::range_generator const& gen
  , grid_region const              region
  , unsigned const                 minimum
) {
    auto const h = region.height();

    if (h <= minimum * 2) {
        return std::make_tuple(false, region, region);
    }

    auto const slack = h - minimum * 2;
    auto const split = gen(0, slack);

    auto const l0 = region.left;
    auto const t0 = region.top;
    auto const r0 = region.right;
    auto const b0 = t0 + minimum + split;

    auto const l1 = region.left;
    auto const t1 = b0;
    auto const r1 = region.right;
    auto const b1 = region.bottom;

    return std::make_tuple(
        true
        , bkrl::grid_region {l0, t0, r0, b0}
        , bkrl::grid_region {l1, t1, r1, b1}
    );
}


////////////////////////////////////////////////////////////////////////////////
// bsp_layout
////////////////////////////////////////////////////////////////////////////////
bsp_layout::bsp_layout(params_t const& params)
  : params_ {params}
{
}
  
void bsp_layout::generate(random::generator& gen) {
    nodes_.clear();
    nodes_.emplace_back(bkrl::grid_region {0, 0, 100, 100});
    split(gen);
}

void bsp_layout::split(random::generator& gen) {
    auto const split_all = [&](size_t const beg, size_t const end) {
        for (auto i = beg; i < end; ++i) {
            auto const did_split = split(gen, nodes_[i]);
            if (did_split) {
                nodes_[i].child_index = nodes_.size() - 2; //added 2 elements
            }
        }

        return nodes_.size();
    };

    // TODO could improve this for clarity?
    // split while there are unvisited nodes
    auto beg = 0;
    auto end = nodes_.size();

    while (true) {
        auto const new_end = split_all(beg, end);

        if (new_end - end == 0) {
            break;
        }

        beg = end;
        end = new_end;
    }
}

bool bsp_layout::split(random::generator& gen, node_t const node) {
    auto const w = node.region.width();
    auto const h = node.region.height();

    if (w < 20 && h < 20) {
        auto const do_split = random::uniform_range(gen, 0, 100) < 50;
        if (!do_split) {
            return false;
        }
    }

    if (node.room) {
        BK_TODO_FAIL();
    }

    auto distribution = [&](unsigned const lo, unsigned const hi) {
        return random::uniform_range(gen, lo, hi);
    };

    //TODO clean up
    auto const ratio = (w > h)
        ? (static_cast<float>(w) / h)
        : (static_cast<float>(h) / w);

    auto const which = (ratio > 1.6f)
        ? (w > h)
        : (random::uniform_range(gen, 0, 100) < 50);

    auto const children = which
        ? split_vertical(distribution, node.region, 4)
        : split_horizontal(distribution, node.region, 4);

    if (!std::get<0>(children)) {
        //BK_TODO_FAIL();
        return false;
    }

    nodes_.emplace_back(std::get<1>(children));
    nodes_.emplace_back(std::get<2>(children));

    return true;
}
