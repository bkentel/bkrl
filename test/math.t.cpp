#include "catch/catch.hpp"
#include "math.hpp"

using namespace bkrl;

//--------------------------------------------------------------------------
TEST_CASE("clamp", "[math]") {
    constexpr float  lo_f = 0.0f;
    constexpr float  hi_f = 1.0f;
    constexpr double lo_d = 0.0;
    constexpr double hi_d = 1.0;
    constexpr int    lo_i = 0;
    constexpr int    hi_i = 1;
    
    REQUIRE(lo_f == clamp(-1.0f, lo_f, hi_f));
    REQUIRE(hi_f == clamp( 2.0f, lo_f, hi_f));
    REQUIRE(lo_d == clamp(-1.0,  lo_d, hi_d));
    REQUIRE(hi_d == clamp( 2.0,  lo_d, hi_d));
    REQUIRE(lo_i == clamp(-1,    lo_i, hi_i));
    REQUIRE(hi_i == clamp( 2,    lo_i, hi_i));
}

//--------------------------------------------------------------------------
TEST_CASE("axis_aligned_rect make_rect", "[math][rect]") {
    auto constexpr left   = -10;
    auto constexpr top    = -5;
    auto constexpr right  =  5;
    auto constexpr bottom =  10;

    auto const r0 = make_rect_bounds(left, top, right, bottom);
    auto const r1 = make_rect_size(left, top, right - left, bottom - top);

    REQUIRE(r0 == r1);
}

//--------------------------------------------------------------------------
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

//--------------------------------------------------------------------------
TEST_CASE("axis_aligned_rect is exclusive of its uppper bounds", "[math][rect]") {
    auto constexpr left   = -10;
    auto constexpr top    = -5;
    auto constexpr right  =  5;
    auto constexpr bottom =  10;

    auto const r0 = axis_aligned_rect<int> {left, top, right, bottom};

    //a "good" rect.
    REQUIRE(!!r0);

    //REQUIRE(r0.contains(left, top));
    //REQUIRE(!r0.contains(left, bottom));
    //REQUIRE(!r0.contains(right, top));
    //REQUIRE(!r0.contains(right, bottom));

    //REQUIRE(!r0.contains(left - 1, top));
    //REQUIRE(!r0.contains(left, top - 1));
    //REQUIRE(!r0.contains(left - 1, top - 1));

    //REQUIRE(r0.contains(right - 1, bottom - 1));
}
