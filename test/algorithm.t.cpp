#include "catch/catch.hpp"
#include "algorithm.hpp"
#include <vector>

TEST_CASE("Sort works for vectors", "[algorithm][sort]") {

    std::vector<int> data_random {10, 1, 4, 3, 8, 9, 5, 6, 2, 7, 0};
    std::vector<int> data_sorted {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    bkrl::sort(data_random);

    REQUIRE(std::equal(std::begin(data_random), std::end(data_random), std::begin(data_sorted)) == true);
}
