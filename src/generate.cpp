#include "generate.hpp"
#include "random.hpp"

using bkrl::generate::simple_room;
using bkrl::generate::circle_room;

namespace random = bkrl::random;
using bkrl::room;
using bkrl::grid_region;

////////////////////////////////////////////////////////////////////////////////
// simple_room
////////////////////////////////////////////////////////////////////////////////
room
simple_room::generate(
    random::generator&       gen
  , grid_region        const bounds
  , bkrl::room_id      const id
) {
    random::uniform_int dist;

    //TODO temp
    auto const w = dist.generate(gen, 4u, bounds.width());
    auto const h = dist.generate(gen, 4u, bounds.height());

    auto const slack_w = bounds.width()  - w;
    auto const slack_h = bounds.height() - h;

    auto const left   = dist.generate(gen, 0u, slack_w);
    auto const top    = dist.generate(gen, 0u, slack_h);
    auto const right  = left + w;
    auto const bottom = top + h;

    room result {
        bounds
      , grid_point {
            bounds.left + left + (right - left) / 2
          , bounds.top  + top  + (bottom - top) / 2
        }
      , id
    };

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
            case 2 :
                result.set(attribute::tile_type, x, y, tile_type::wall);
                break;
            }

            result.set(attribute::room_id, x, y, id);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
// circle_room
////////////////////////////////////////////////////////////////////////////////
room
circle_room::generate(
    random::generator&
  , grid_region         bounds
  , bkrl::room_id const id
) {
    auto const w = bounds.width();
    auto const h = bounds.height();
    auto const x0 = w / 2;
    auto const y0 = h / 2;
        
    auto const r = std::min(w, h) / 2.0;
    auto const r2 = static_cast<unsigned>(std::floor(r*r));
    auto const rr = static_cast<unsigned>(std::floor((r - 1.25)*(r - 1.25))); //hack

    room result {bounds, bounds.center(), id};

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
