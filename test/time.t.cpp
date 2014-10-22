#include "catch/catch.hpp"
#include "time.hpp"

using bkrl::timer;

TEST_CASE("timer basics", "[time]") {
    using namespace std::chrono_literals;
    
    using durarion = timer::duration_t;
    using timer_id = timer::id_t;

    std::array<int, 3> counts {};

    auto const dur0 = 100ms;
    auto const dur1 = 200ms;
    auto const dur2 = 1000ms;

    timer_id id0 {};
    timer_id id1 {};
    timer_id id2 {};

    auto const t0 = [&](timer_id id, durarion dur) {
        REQUIRE(id  == id0);
        REQUIRE(dur >= dur0);

        counts[0]++;
        return dur0;
    };

    auto const t1 = [&](timer_id id, durarion dur) {
        REQUIRE(id  == id1);
        REQUIRE(dur >= dur1);

        counts[1]++;
        return dur1;
    };

    auto const t2 = [&](timer_id id, durarion dur) {
        REQUIRE(id  == id2);
        REQUIRE(dur >= dur2);

        counts[2]++;
        return dur2;
    };

    timer timers;

    id0 = timers.add_timer(dur0, t0);
    id1 = timers.add_timer(dur1, t1);
    id2 = timers.add_timer(dur2, t2);

    REQUIRE(id0.value == 1);
    REQUIRE(id1.value == 2);
    REQUIRE(id2.value == 3);

    while (counts[0] != 20) {
        timers.update();
    }

    REQUIRE(counts[0] == 20);
    REQUIRE(counts[1] == 10);
    REQUIRE(counts[2] == 2);
}
