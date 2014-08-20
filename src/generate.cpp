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
                result.set(attribute::tile_type, x, y, tile_type::wall);
            } else {
                result.set(attribute::tile_type, x, y, tile_type::floor);
            }
        }
    }

    //tmp
    //result.set(attribute::tile_type, left + 2, top + 2, tile_type::wall);

    return result;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

//split_result bkrl::generate::split_y(
//    random::range_generator const& gen
//  , grid_region const              region
//  , unsigned const                 minimum 
//) {
//    auto const w = region.width();
//
//    if (w <= minimum * 2) {
//        return std::make_tuple(false, region, region);
//    }
//
//    auto const slack = w - minimum * 2;
//    auto const split = gen(0, slack);
//    
//    auto const l0 = region.left;
//    auto const t0 = region.top;
//    auto const r0 = l0 + minimum + split;
//    auto const b0 = region.bottom;
//
//    auto const l1 = r0;
//    auto const t1 = region.top;
//    auto const r1 = region.right;
//    auto const b1 = region.bottom;
//
//    return std::make_tuple(
//        true
//      , bkrl::grid_region {l0, t0, r0, b0}
//      , bkrl::grid_region {l1, t1, r1, b1}
//    );
//}
//
//split_result bkrl::generate::split_x(
//    random::range_generator const& gen
//  , grid_region const              region
//  , unsigned const                 minimum
//) {
//    auto const h = region.height();
//
//    if (h <= minimum * 2) {
//        return std::make_tuple(false, region, region);
//    }
//
//    auto const slack = h - minimum * 2;
//    auto const split = gen(0, slack);
//
//    auto const l0 = region.left;
//    auto const t0 = region.top;
//    auto const r0 = region.right;
//    auto const b0 = t0 + minimum + split;
//
//    auto const l1 = region.left;
//    auto const t1 = b0;
//    auto const r1 = region.right;
//    auto const b1 = region.bottom;
//
//    return std::make_tuple(
//        true
//        , bkrl::grid_region {l0, t0, r0, b0}
//        , bkrl::grid_region {l1, t1, r1, b1}
//    );
//}


////////////////////////////////////////////////////////////////////////////////
// bsp_layout
////////////////////////////////////////////////////////////////////////////////
bsp_layout::bsp_layout(split_callback on_split, params_t const& params)
  : params_ {params}
  , on_split_ {on_split}
{
}

void bsp_layout::generate(random::generator& gen) {
    nodes_.clear();
    nodes_.emplace_back(bkrl::grid_region {0, 0, 100, 100});

    split(gen);
}

void bsp_layout::generate(random::generator& gen, grid_region const reserve) {
    nodes_.clear();
    nodes_.emplace_back(bkrl::grid_region {0, 0, 100, 100});
        
    split_y(0, reserve.right);
    split_x(1, reserve.bottom);
    split_y(3, reserve.left);
    split_x(6, reserve.top);

    nodes_[8].child_index = 0;

    split(gen);
}

void bsp_layout::split(random::generator& gen) {
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

    while (true) {
        auto const new_end = split_all(beg, end);

        if (new_end - end == 0) {
            break;
        }

        beg = end;
        end = new_end;
    }
}

bool bsp_layout::split(random::generator& gen, index_t const index) {
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
