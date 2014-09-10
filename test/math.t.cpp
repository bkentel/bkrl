#include "catch/catch.hpp"
#include "math.hpp"

using namespace bkrl;

TEST_CASE("axis_aligned_rect sanity checks", "[math][rect]") {
    auto constexpr left   = -10;
    auto constexpr top    = -5;
    auto constexpr right  =  5;
    auto constexpr bottom =  10;

    auto const r0 = axis_aligned_rect<int> {left, top, right, bottom};

    //fields
    REQUIRE(r0.left == left);
    REQUIRE(r0.top == top);
    REQUIRE(r0.right == right);
    REQUIRE(r0.bottom == bottom);

    //a "good" rect.
    REQUIRE(!!r0);

    //expected dimensions
    REQUIRE(r0.width() == (r0.right - r0.left));
    REQUIRE(r0.height() == (r0.bottom - r0.top));

    //area
    REQUIRE(r0.area() == r0.width() * r0.height());

    //equality and assignment
    auto const r1 = r0;
    REQUIRE(r0 == r1);

    //inequality
    auto const r2 = axis_aligned_rect<int> {0, 0, 1, 1};
    REQUIRE(r0 != r2);
}

TEST_CASE("axis_aligned_rect is exclusive of its uppper bounds", "[math][rect]") {
    auto constexpr left   = -10;
    auto constexpr top    = -5;
    auto constexpr right  =  5;
    auto constexpr bottom =  10;

    auto const r0 = axis_aligned_rect<int> {left, top, right, bottom};

    //a "good" rect.
    REQUIRE(!!r0);

    REQUIRE(r0.contains(left, top));
    REQUIRE(!r0.contains(left, bottom));
    REQUIRE(!r0.contains(right, top));
    REQUIRE(!r0.contains(right, bottom));

    REQUIRE(!r0.contains(left - 1, top));
    REQUIRE(!r0.contains(left, top - 1));
    REQUIRE(!r0.contains(left - 1, top - 1));

    REQUIRE(r0.contains(right - 1, bottom - 1));
}
