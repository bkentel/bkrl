#include "catch/catch.hpp"
#include "spatial_map.hpp"

#include "types.hpp"

TEST_CASE("spatial_map basics", "[spatial_map]") {
    using bkrl::ipoint2;

    auto const p = ipoint2 {0, 5};

    bkrl::spatial_map<int> map;
   
    map.emplace(p, 0);
    map.emplace(ipoint2 {1, 5}, 1);
    map.emplace(p, 2);
    map.emplace(ipoint2 {1, 5}, 3);
    map.emplace(ipoint2 {0, 1}, 4);
    map.sort();

    std::array<int, 5> result {0};

    map.find(p, [&](int const i) {
        result[i]++;
    });

    map.remove(p);

    map.find(p, [&](int const i) {
        result[i]++;
    });

    REQUIRE(result[0] == 1);
    REQUIRE(result[1] == 0);
    REQUIRE(result[2] == 1);
    REQUIRE(result[3] == 0);
    REQUIRE(result[4] == 0);

}
