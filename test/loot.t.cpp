#include "catch/catch.hpp"

#include <memory>
#include <vector>
#include <functional>

#include "loot_table.hpp"
#include "items.hpp"
#include "json.hpp"
#include "random.hpp"

#include <map>

////////////////////////////////////////////////////////////////////////////////
// tests
////////////////////////////////////////////////////////////////////////////////

struct test_data {
    struct result_t {
        result_t()
          : count {0}
          , min_n {std::numeric_limits<int>::max()}
          , max_n {std::numeric_limits<int>::min()}
        {
        }

        void increment(int const n) {
            ++count;

            min_n = std::min(n, min_n);
            max_n = std::max(n, max_n);
        }

        bool check_with_tolerance(
            double const expected
          , double const tolerance
        ) const {
            auto const delta = std::abs(count - expected);
            return delta <= expected * tolerance;
        }

        bool check_min_max(int const lo, int const hi) const {
            return min_n == lo && max_n == hi;
        }

        int count;
        int min_n;
        int max_n;
    };

    bkrl::loot_table_definitions defs;
    
    bkrl::random_t gen {900010};
    
    std::map<bkrl::hash_t, result_t> results;

    auto make_table(char const* data, bkrl::string_ref const root = "") {    
        bkrl::loot_table_parser parser;

        auto json = bkrl::json::common::from_memory(data);
        return root.empty()
            ? parser.parse(json)
            : parser.parse(json[bkrl::string_ref("loot")]);
    }

    void roll_sink(bkrl::item_def_id const id, uint16_t const n) {
        results[bkrl::id_to_value(id)].increment(n);
    }

    void roll(bkrl::loot_table const& table, size_t const n) {
        auto const f = [&](bkrl::item_def_id const id, uint16_t const n) {
            roll_sink(id, n);
        };

        for (size_t i = 0; i < n; ++i) {
            table.generate(gen, defs, f);
        }
    }
};

//==============================================================================
TEST_CASE("Check a roll_all table with a single percentage entry", "[loot_table]") {
    constexpr auto iterations = 1000;

    char const table[] = R"(
    { "type": "roll_all"
    , "rules": [
        [10, "TEST"]
      ]
    }
    )";

    test_data test;
    auto const test_table = test.make_table(table);
    test.roll(test_table, iterations);

    REQUIRE(test.results.size() == 1);
    
    auto const& result = test.results.begin()->second;
    
    REQUIRE(result.check_min_max(1, 1));
    REQUIRE(result.check_with_tolerance(iterations * (10.0 / 100.0), 0.1));
}

//==============================================================================
TEST_CASE("Check a roll_all table with a single fractional entry", "[loot_table]") {
    constexpr auto iterations = 1000;

    char const table[] = R"(
    { "type": "roll_all"
    , "rules": [
        [[2, 3], "TEST"]
      ]
    }
    )";

    test_data test;
    auto const test_table = test.make_table(table);
    test.roll(test_table, iterations);

    REQUIRE(test.results.size() == 1);
    
    auto const& result = test.results.begin()->second;
    
    REQUIRE(result.check_min_max(1, 1));
    REQUIRE(result.check_with_tolerance(iterations * (2.0 / 3.0), 0.1));
}

//==============================================================================
TEST_CASE("Check a roll_all table with a single guarenteed entry", "[loot_table]") {
    constexpr auto iterations = 1000;

    char const table[] = R"(
    { "type": "roll_all"
    , "rules": [
        [100, "TEST"]
      ]
    }
    )";

    test_data test;
    auto const test_table = test.make_table(table);
    test.roll(test_table, iterations);

    REQUIRE(test.results.size() == 1);
    
    auto const& result = test.results.begin()->second;
    REQUIRE(result.count == iterations);
    REQUIRE(result.check_min_max(1, 1));
}

//==============================================================================
TEST_CASE("Check uniform quantity with 0", "[loot_table]") {   
    constexpr auto iterations = 100;

    char const table[] = R"(
    { "type": "choose_one"
    , "rules": [
        [1, "TEST", [0, 10]]
      ]
    }
    )";

    test_data test;
    auto const test_table = test.make_table(table);
    test.roll(test_table, iterations);

    REQUIRE(test.results.size() == 1);
    
    auto const& result = test.results.begin()->second;
    
    //A result of 0 quantity means that loot is never generated
    REQUIRE(result.count <= iterations);
    REQUIRE(result.check_min_max(1, 10));
}

//==============================================================================
TEST_CASE("Check a choose_one table with a single entry", "[loot_table]") {
    constexpr auto iterations = 100;

    char const table[] = R"(
    { "type": "choose_one"
    , "rules": [
        [1, "TEST"]
      ]
    }
    )";

    test_data test;
    auto const test_table = test.make_table(table);
    test.roll(test_table, iterations);

    REQUIRE(test.results.size() == 1);
    
    auto const& result = test.results.begin()->second;
    REQUIRE(result.count == iterations);
    REQUIRE(result.check_min_max(1, 1));
}

//==============================================================================
TEST_CASE("Check a choose_one table with unevenly weighted entries", "[loot_table]") {
    constexpr auto iterations = 1000;

    char const table[] = R"(
    { "type": "choose_one"
    , "rules": [
        [2,  "TEST0"]
      , [10, "TEST1"]
      ]
    }
    )";

    test_data test;
    auto const test_table = test.make_table(table);
    test.roll(test_table, iterations);

    REQUIRE(test.results.size() == 2);
    
    auto it = test.results.begin();

    auto const& result0 = it->second;
    auto const& result1 = (++it)->second;

    REQUIRE((result0.count + result1.count) == iterations);

    REQUIRE(result0.check_min_max(1, 1));
    REQUIRE(result1.check_min_max(1, 1));

    auto const expected0 = iterations * ( 2.0 / 12.0);
    auto const expected1 = iterations * (10.0 / 12.0);

    REQUIRE(result0.check_with_tolerance(expected0, 0.1));
    REQUIRE(result1.check_with_tolerance(expected1, 0.1));
}

//==============================================================================
TEST_CASE("Simple check loot_table_definitions", "[loot_table]") {
    char const table_defs[] = R"(
    { "file_type": "LOOT"
    , "definitions": [
        { "id": "ID0"
        , "type": "choose_one"
        , "rules": [[1, "TEST"]]
        }
      , { "id": "ID1"
        , "type": "roll_all"
        , "rules": [[1, "TEST"]]
        }
      , { "id": "ID2"
        , "rules": [[1, "TEST"]]
        }
      ]
    }
    )";

    bkrl::loot_table_definitions defs;
    auto json = bkrl::json::common::from_memory(table_defs);
    defs.load_definitions(json);

    auto const& t0 = defs[bkrl::loot_table_def_id {bkrl::slash_hash32("ID0")}];
    auto const& t1 = defs[bkrl::loot_table_def_id {bkrl::slash_hash32("ID1")}];
    auto const& t2 = defs[bkrl::loot_table_def_id {bkrl::slash_hash32("ID2")}];

    REQUIRE(t0.type() == bkrl::loot_table::roll_t::choose_one);
    REQUIRE(t1.type() == bkrl::loot_table::roll_t::roll_all);
    REQUIRE(t2.type() == bkrl::loot_table::roll_t::roll_all);
}

//==============================================================================
TEST_CASE("Check a string table", "[loot_table]") {
    constexpr auto iterations = 1000;

    char const table[] = R"(
    { "loot": "table_id" }
    )";

    test_data test;
    auto const test_table = test.make_table(table, "loot");

    //TODO
}
