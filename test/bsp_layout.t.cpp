#include "catch/catch.hpp"
#include "bsp_layout.hpp"
#include "generate.hpp"

TEST_CASE("bsp_layout", "[algorithm][sort]") {
    using namespace bkrl;

    random::generator gen {100};

    generate::simple_room room_gen {};
        
    auto const on_split = [](grid_region const) {
        return true;
    };

    auto const on_room_gen = [](grid_region const bounds, unsigned const id) {
    };

    auto layout = bsp_layout::generate(gen, on_split, on_room_gen);
    layout.connect(gen);

}
