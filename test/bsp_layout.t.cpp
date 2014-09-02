#include "catch/catch.hpp"
#include "bsp_layout.hpp"
#include "generate.hpp"

TEST_CASE("bsp_layout obeys params", "[generate][layout]") {
    using namespace bkrl;

    random::generator gen {100};
       
    bsp_layout::params_t const params {};

    auto const on_split = [](grid_region const) {
        return true;
    };

    auto const on_room_gen = [&params](grid_region const bounds, unsigned const id) {
        REQUIRE(id != 0);
        
        auto const w = bounds.width();
        auto const h = bounds.height();
        
        REQUIRE(w >= params.min_region_w);
        REQUIRE(h >= params.min_region_h);
        REQUIRE(w <= params.max_region_w);
        REQUIRE(h <= params.max_region_h);

        auto const aspect = (w >= h)
            ? (static_cast<float>(w) / h)
            : (static_cast<float>(h) / w);

        REQUIRE(aspect <= params.max_aspect_ratio);
    };

    auto layout = bsp_layout::generate(gen, on_split, on_room_gen, params);
}
