#include "catch/catch.hpp"

#include <vector>
#include "items.hpp"
#include "json.hpp"
#include "random.hpp"

#include <map> //temp

namespace bkrl {

class loot_table_definitions;
class loot_table_;
class loot_table_parser;

//==============================================================================
//!
//==============================================================================
struct loot_rule_data_t {
    enum class id_t {
        item_ref  //!< the rule refers to an item definition
      , table_ref //!< the rule refers to a loot table definition
    };

    hash_t   id;
    uint16_t count_lo;
    uint16_t count_hi;
    id_t     id_type;
};

//==============================================================================
//!
//==============================================================================
class loot_table_definitions {
public:
    ~loot_table_definitions() = default;

    loot_table_definitions() = default;
    loot_table_definitions(loot_table_definitions&&) = default;
    loot_table_definitions& operator=(loot_table_definitions&&) = default;

    loot_table_ const& operator[](loot_table_def_id id) const;
private:
};

template <typename F>
inline void repeat_n(size_t const n, F&& f) {
    while (n > 0) { f(); }
}

//==============================================================================
//!
//==============================================================================
class loot_table_ {
public:
    enum class roll_t {
        roll_all   //!< all entries are rolled for success / failure.
      , choose_one //!< one weighted entry is chosen.
    };

    using defs_t = loot_table_definitions const&;
    using rule_t = loot_rule_data_t const&;

    loot_table_()
      : type_ {roll_t::roll_all}
    {
    }

    template <typename Data, typename Rules>
    loot_table_(
        roll_t const  type
      , Data   const& data
      , Rules  const& rules
    )
      : type_ {type}
    {
        auto const size_data  = data.size();
        auto const size_rules = rules.size();

        if (type == roll_t::roll_all) {
            BK_ASSERT_DBG(size_rules == size_data * 2);
        } else if (type == roll_t::choose_one) {
            BK_ASSERT_DBG(size_rules == size_data);
        }

        using std::begin;
        using std::end;

        roll_data_.reserve(size_data);
        std::copy(begin(data), end(data), std::back_inserter(roll_data_));

        rules_.reserve(size_rules);
        std::copy(begin(rules), end(rules), std::back_inserter(rules_));

        if (type == roll_t::choose_one) {
            uint16_t sum = 0;
            for (auto& w : roll_data_) {
                auto const n = w;
                w = sum;
                sum += n;
            }
        }
    }

    template <typename Write>
    void generate(random_t& gen, defs_t defs, Write&& write) const {
        if (type_ == roll_t::roll_all) {
            roll_all_(gen, defs, std::forward<Write>(write));
        } else if (type_ == roll_t::choose_one) {
            choose_one_(gen, defs, std::forward<Write>(write));
        } else {
            BK_TODO_FAIL();
        }
    }
private:
    template <typename Write>
    void roll_all_(random_t& gen, defs_t defs, Write&& write) const {
    }
    
    template <typename Write>
    void choose_one_(random_t& gen, defs_t defs, Write&& write) const {
        auto const find_index = [&](uint32_t const n) {
            int i = 0;
            for (auto const sum : roll_data_) {
                if (n <= sum) { return i; }
                ++i;
            }

            BK_TODO_FAIL();
        };

        auto const  sum  = roll_data_.back();
        auto const  roll = random::uniform_range<uint16_t>(gen, 0, sum - 1);
        auto const  i    = find_index(roll);
        auto const& rule = rules_[i];

        roll_one_(gen, defs, rule, std::forward<Write>(write));
    }

    template <typename Write>
    void roll_one_(random_t& gen, defs_t defs, rule_t rule, Write&& write) const {
        using id_t = loot_rule_data_t::id_t;
        
        if (rule.id_type == id_t::item_ref) {
            roll_one_item_(gen, defs, rule, std::forward<Write>(write));
        } else if (rule.id_type == id_t::table_ref) {
            roll_one_table_(gen, defs, rule, std::forward<Write>(write));
        } else {
            BK_TODO_FAIL();
        }
    }

    template <typename Write>
    void roll_one_item_(random_t& gen, defs_t defs, rule_t rule, Write&& write) const {
        BK_ASSERT_DBG(rule.id_type == loot_rule_data_t::id_t::item_ref);

        auto const id = item_def_id {rule.id};
        auto const n  = random::uniform_range(gen, rule.count_lo, rule.count_hi);

        if (n > 0) {
            write(id, n);
        }
    }

    template <typename Write>
    void roll_one_table_(random_t& gen, defs_t defs, rule_t rule, Write&& write) const {
        BK_ASSERT_DBG(rule.id_type == loot_rule_data_t::id_t::table_ref);

        auto const id = loot_table_def_id {rule.id};
        auto const n  = random::uniform_range(gen, rule.count_lo, rule.count_hi);

        auto const& table = defs[id];

        repeat_n(n, [&] {
            table.generate(gen, defs, std::forward<Write>(write));
        });
    }

    std::vector<uint16_t>         roll_data_;
    std::vector<loot_rule_data_t> rules_;
    roll_t type_;
};


// LOOT_TABLE -> NAMED_TABLE | SIMPLE_TABLE
//
// SIMPLE_TABLE -> [RULE*]
//
// NAMED_TABLE ->
//
// RULE ->
//  CHOOSE_ONE_RULE2 | CHOOSE_ONE_RULE3 | CHOOSE_ONE_RULE4
//  ROLL_ALL_RULE2   | ROLL_ALL_RULE3   | ROLL_ALL_RULE4
//
// CHOOSE_ONE_RULE2 -> [WEIGHT, ID]
// CHOOSE_ONE_RULE3 -> [WEIGHT, ID, COUNT]
// CHOOSE_ONE_RULE4 -> [WEIGHT, ID, COUNT, TYPE]
//
// ROLL_ALL_RULE2 -> [CHANCE, ID]
// ROLL_ALL_RULE3 -> [CHANCE, ID, COUNT]
// ROLL_ALL_RULE4 -> [CHANCE, ID, COUNT, TYPE]
//
// WEIGHT -> int
//
// CHANCE -> int | [int, int]
//
// ID -> string
//
// COUNT -> int | [int, int]
//
// TYPE -> null | "ITEM" | "TABLE"
class loot_table_parser {
public:
    enum : size_t {
        index_weight = 0
      , index_id     = 1
      , index_count  = 2
      , index_type   = 3
    };

    std::pair<loot_rule_data_t, std::pair<uint16_t, uint16_t>>
    make_rule(
        hash_t                        const id
      , std::pair<uint16_t, uint16_t> const count
      , loot_rule_data_t::id_t        const type
      , uint16_t                      const data0
      , uint16_t                      const data1
    ) {
        return std::make_pair(
            loot_rule_data_t {id, count.first, count.second, type}
          , std::make_pair(data0, data1)
        ); 
    }

    auto rule_weight(json::cref value) {
        auto const w = json::require_int<uint16_t>(value);
        if (w <= 0) {
            BK_TODO_FAIL();
        }

        return w;
    }

    auto rule_id(json::cref value) {
        auto const str = json::require_string(value);
        return slash_hash32(str);
    }

    auto rule_count(json::cref value) {
        if (value.is_array()) {
            auto result = std::make_pair(
                json::require_int<uint16_t>(value[0])
              , json::require_int<uint16_t>(value[1])
            );

            if (result.second == 0 || result.first >= result.second) {
                BK_TODO_FAIL();
            }

            return result;
        }

        auto const n = json::require_int<uint16_t>(value);
        if (n <= 0) {
            BK_TODO_FAIL();
        }

        return std::make_pair(n, n);
    }

    auto rule_chance(json::cref value) {
        json::require_array(value, 1, 2);

        auto const size = value.array_items().size();

        auto const num = json::require_int<uint16_t>(value[0]);
        auto const den = (size == 1) ? 100 : json::require_int<uint16_t>(value[1]);

        if (num == 0 || den == 0) {
            BK_TODO_FAIL();
        }

        return std::make_pair(num, den);
    }

    auto rule_type(json::cref value) {
        static auto const item_hash  = slash_hash32("ITEM");
        static auto const table_hash = slash_hash32("TABLE");
        
        if (value.is_null()) {
            return loot_rule_data_t::id_t::item_ref;
        }

        auto const str  = json::require_string(value);
        auto const hash = slash_hash32(str);

        if (hash == item_hash) {
            return loot_rule_data_t::id_t::item_ref;
        } else if (hash == table_hash) {
            //return loot_rule_data_t::id_t::table_ref;
        } else {
            BK_TODO_FAIL();
        }

        return loot_rule_data_t::id_t::table_ref;
    }

    auto rule_choose_one_2(json::cref value) {
        return make_rule(
            rule_id(value[index_id])
          , std::make_pair(1, 1)
          , loot_rule_data_t::id_t::item_ref
          , rule_weight(value[index_weight])
          , 0
        );
    }
    
    auto rule_choose_one_3(json::cref value) {
        return make_rule(
            rule_id(value[index_id])
          , rule_count(value[index_count])
          , loot_rule_data_t::id_t::item_ref
          , rule_weight(value[index_weight])
          , 0
        );
    }

    auto rule_choose_one_4(json::cref value) {
        return make_rule(
            rule_id(value[index_id])
          , rule_count(value[index_count])
          , rule_type(value[index_type])
          , rule_weight(value[index_weight])
          , 0
        );
    }

    auto rule_rule(json::cref value) {
        json::require_array(value, 2, 4);

        auto const size = value.array_items().size();
        if (size == 2) {
            return rule_choose_one_2(value);
        } else if (size == 3) {
            return rule_choose_one_3(value);
        }

        if (size != 4) {
            BK_TODO_FAIL();
        }

        return rule_choose_one_4(value);
    }

    void rule_named_table(json::cref value) {
    
        //table_type_ = loot_table_::roll_t::roll_all;
    }

    void rule_simple_table(json::cref value) {
        json::require_array(value, 1);

        auto const size = value.array_items().size();

        rules_.reserve(size);
        rule_data_.reserve(size);

        for(auto const& rule : value.array_items()) {
            std::tie(cur_rule_, cur_data_) = rule_rule(rule);
            rules_.push_back(cur_rule_);
            rule_data_.push_back(cur_data_.first);
        }

        table_type_ = loot_table_::roll_t::choose_one;
    }

    loot_table_ rule_root(json::cref value) {
        rule_data_.clear();
        rules_.clear();

        if (value.is_object()) {
            rule_named_table(value);
        } else if (value.is_array()) {
            rule_simple_table(value);
        } else {
            BK_TODO_FAIL();
        }

        return loot_table_ {table_type_, rule_data_, rules_};
    }
   
    loot_rule_data_t              cur_rule_;
    std::pair<uint16_t, uint16_t> cur_data_;

    std::vector<uint16_t>         rule_data_;
    std::vector<loot_rule_data_t> rules_;
    loot_table_::roll_t           table_type_;
};

loot_table_ const& loot_table_definitions::operator[](loot_table_def_id id) const {
    static loot_table_ table;
    return table;
}

} //namespace bkrl

namespace {

char const entity_string[] = R"(
{ "file_type": "ENTITY"
, "file_name": "./data/entities.bmp"
, "tile_size": [18, 18]
, "player" : [13, 13]
, "definitions": [
    { "id": "SKELETON"
    , "tile": [5, 9]
    , "color": [85, 85, 85]
    , "health": ["dice", 3, 4]
    , "items": [
        [10, "BASIC_WEAPON", 1, "TABLE"]
      , [10, "BASIC_POTION", 1, "TABLE"]
      , [50, "WEAPON_SHORT_BOW", 1, "ITEM"]
      ]
    }
  ]
}
)";



char const tables_string[] = R"(
{ "file_type": "LOOT"
, "definitions": [
    { "id": "BASIC_WEAPON"
    , "type": "CHOOSE_ONE"
    , "rules": [
        [1, "WEAPON_SWORD_SHORT", 1]
      , [1, "WEAPON_LONG_SWORD", 1]
      , [1, "WEAPON_HAMMER", 1]
      , [1, "WEAPON_STAFF", 1]
      , [1, "WEAPON_DAGGER", 1]
      , [1, "WEAPON_AXE", 1]
      ]
    }
  , { "id": "TEST"
    , "type": "DEFAULT"
    , "rules": [
        [10, "BASIC_WEAPON", 1, "TABLE"]
      , [10, "BASIC_POTION", 1, "TABLE"]
      , [50, "WEAPON_SHORT_BOW", 1]
      ]
    }
  ]
}
)";

char const tables_simple[] = R"(
{ "loot": [
    [1, "WEAPON_SWORD_SHORT"]
  , [1, "WEAPON_LONG_SWORD"]
  , [10, "WEAPON_HAMMER"]
  , [1, "WEAPON_STAFF"]
  , [1, "WEAPON_DAGGER"]
  , [1, "WEAPON_AXE"]
  , [1, "POTION_MINOR", [0, 3]]
  ]
}
)";

} //namespace


TEST_CASE("Loot test", "[llot]") {
    bkrl::loot_table_parser parser;

    auto json = bkrl::json::common::from_memory(tables_simple);

    auto const result = parser.rule_root(json[bkrl::string_ref("loot")]);

    bkrl::random_t rand {10010};

    bkrl::loot_table_definitions defs;

    std::map<bkrl::hash_t, int> rolls;

    for (int i = 0; i < 1000000; ++i) {

    result.generate(rand, defs, [&](bkrl::item_def_id const id, int const n) {
        BK_ASSERT(n > 0);

        auto& sum = rolls[bkrl::id_to_value(id)];
        sum++;

        //printf("rolled %u of %x\n", n, bkrl::id_to_value(id));
    });

    }
}
