#include "catch/catch.hpp"
#include "keyboard.hpp"

using km = bkrl::key_modifier_type;

TEST_CASE("key_modifier set, test, clear one", "[keyboard][key_modifier]") {
    using namespace bkrl;

    key_modifier mod;

    auto const do_test = [&](km const m) {
        REQUIRE_FALSE(mod.test(m));
        mod.set(m);
        REQUIRE(mod.test(m));
        mod.clear(m);
        REQUIRE_FALSE(mod.test(m));
    };

    do_test(km::alt);
    do_test(km::ctrl);
    do_test(km::shift);

    do_test(km::alt_left);
    do_test(km::alt_right);
    do_test(km::ctrl_left);
    do_test(km::ctrl_right);
    do_test(km::shift_left);
    do_test(km::shift_right);
}

TEST_CASE("key_modifier set, test, clear both", "[keyboard][key_modifier]") {
    using namespace bkrl;

    key_modifier mod;

    auto const do_test = [&](km const m0, km const m1, km const m2) {
        REQUIRE_FALSE(mod.test(m0));
        REQUIRE_FALSE(mod.test(m1));
        REQUIRE_FALSE(mod.test(m2));

        mod.set(m0);

        REQUIRE(mod.test(m0));
        REQUIRE(mod.test(m1));
        REQUIRE(mod.test(m2));

        mod.clear(m0);

        REQUIRE_FALSE(mod.test(m0));
        REQUIRE_FALSE(mod.test(m1));
        REQUIRE_FALSE(mod.test(m2));
    };

    do_test(km::alt,   km::alt_left,   km::alt_right);
    do_test(km::ctrl,  km::ctrl_left,  km::ctrl_right);
    do_test(km::shift, km::shift_left, km::shift_right);
}
