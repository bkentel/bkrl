#include "catch/catch.hpp"

#include <memory>
#include <vector>
#include <functional>
#include <boost/container/flat_map.hpp>
#include "items.hpp"
#include "json.hpp"
#include "random.hpp"

#include <map> //temp

namespace bkrl {

template <typename F, void (F::*)(size_t) const = &F::operator()>
inline void repeat_n(size_t const n, F&& f) {
    for (size_t i = 0; i < n; ++i) {
        f(i);
    }
}

template <typename F, void (F::*)() const = &F::operator()>
inline void repeat_n(size_t const n, F&& f) {
    for (size_t i = 0; i < n; ++i) {
        f();
    }
}

template <typename SrcCont, typename DstCont>
inline void append_to(SrcCont const& src, DstCont& dst) {
    using std::begin;
    using std::end;

    dst.insert(end(dst), begin(src), end(src));
}

// ROOT -> {FILE_TYPE, DEFINITIONS}
//
// FILE_TYPE -> "file_type": "LOOT"
//
// DEFINITIONS -> "definitions": [DEFINITION*]
//
// DEFINITION -> LOOT_TABLE

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

class loot_table_definitions;
class loot_table_definitions_impl;
class loot_table_;

//==============================================================================
//! data defining the result of a successful roll.
//==============================================================================
struct loot_rule_data_t {
    enum class id_t : uint8_t {
        item_ref  //!< the rule refers to an item definition.
      , table_ref //!< the rule refers to a loot table definition.
    };

    using string_t = std::array<char, 31>;

    hash_t   id;              //!< id of the resulting item or table.
    uint16_t count_lo;        //!< the lower bound on the quantity generated.
    uint16_t count_hi;        //!< the upper bound on the quantity generated.
    string_t id_debug_string; //!< snipped of the string used to generate the id hash.
    id_t     id_type;         //!< indicated either an item or table result.
};

//==============================================================================
//! collection of named loot tables.
//==============================================================================
class loot_table_definitions {
public:
    ~loot_table_definitions();

    loot_table_definitions();
    loot_table_definitions(loot_table_definitions&&);
    loot_table_definitions& operator=(loot_table_definitions&&);

    //--------------------------------------------------------------------------
    void load_definitions(json::cref data);

    //--------------------------------------------------------------------------
    loot_table_ const& operator[](loot_table_def_id id) const;
private:
    std::unique_ptr<loot_table_definitions_impl> impl_;
};

//==============================================================================
//! the definition for a single loot table.
//==============================================================================
class loot_table_ {
public:
    enum class roll_t {
        roll_all   //!< all entries are rolled for success / failure.
      , choose_one //!< one weighted entry is chosen.
    };

    using defs_t = loot_table_definitions const&;
    using rule_t = loot_rule_data_t const&;

    using write_t = std::function<void (item_def_id, int)>;

    //--------------------------------------------------------------------------
    loot_table_(loot_table_&&)                 = default;
    loot_table_& operator=(loot_table_&&)      = default;
    loot_table_(loot_table_ const&)            = delete;
    loot_table_& operator=(loot_table_ const&) = delete;

    //--------------------------------------------------------------------------
    loot_table_()
      : type_ {roll_t::roll_all}
    {
    }

    //--------------------------------------------------------------------------
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

        BK_ASSERT_DBG(
            (type == roll_t::roll_all)   && (size_data == size_rules * 2)
         || (type == roll_t::choose_one) && (size_data == size_rules * 1)
        );

        append_to(data,  roll_data_);
        append_to(rules, rules_);

        tranform_data_();
    }

    //--------------------------------------------------------------------------
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

    //--------------------------------------------------------------------------
    void set_id(string_ref const id_str, loot_table_def_id const id) {
        id_string_.assign(id_str.data(), id_str.size());
        id_ = id;
    }

    //--------------------------------------------------------------------------
    loot_table_def_id id() const noexcept {
        return id_;
    }
private:
    //--------------------------------------------------------------------------
    void tranform_data_() {
        if (type_ != roll_t::choose_one) {
            return;
        }

        constexpr auto max = min_max_value<uint16_t>::max;
        
        uint32_t sum = 0;

        for (auto& w : roll_data_) {
            auto const new_w = w + sum;

            if (new_w > max) {
                BK_TODO_FAIL();
            }

            w   = static_cast<uint16_t>(new_w);
            sum = w;
        }
    }

    //--------------------------------------------------------------------------
    template <typename Write>
    void roll_all_(random_t& gen, defs_t defs, Write&& write) const {
        repeat_n(rules_.size(), [&](size_t const i) {
            auto const& rule = rules_[i];

            auto const num = roll_data_[i*2 + 0];
            auto const den = roll_data_[i*2 + 1];

            auto const lo = static_cast<uint16_t>(0);
            auto const hi = static_cast<uint16_t>(den - 1);

            auto const roll = random::uniform_range(gen, lo, hi);
            if (roll < num) {
                roll_one_(gen, defs, rule, std::forward<Write>(write));
            }
        });
    }
    
    //--------------------------------------------------------------------------
    template <typename Write>
    void choose_one_(random_t& gen, defs_t defs, Write&& write) const {
        auto const find_index = [&](uint32_t const n) {
            int i = 0;
            for (auto const sum : roll_data_) {
                if (n < sum) { return i; }
                ++i;
            }

            BK_TODO_FAIL();
        };

        auto const  sum  = roll_data_.back();
        auto const  roll = random::uniform_range<uint16_t>(gen, uint16_t {0}, sum - 1);
        auto const  i    = find_index(roll);
        auto const& rule = rules_[i];

        roll_one_(gen, defs, rule, std::forward<Write>(write));
    }

    //--------------------------------------------------------------------------
    template <typename Write>
    void roll_one_(random_t& gen, defs_t defs, rule_t rule, Write&& write) const {
        using id_t = loot_rule_data_t::id_t;
        
        if (rule.id_type == id_t::item_ref) {
            roll_one_item_(gen, rule, std::forward<Write>(write));
        } else if (rule.id_type == id_t::table_ref) {
            roll_one_table_(gen, defs, rule, std::forward<Write>(write));
        } else {
            BK_TODO_FAIL();
        }
    }

    //--------------------------------------------------------------------------
    template <typename Write>
    void roll_one_item_(random_t& gen, rule_t rule, Write&& write) const {
        BK_ASSERT_DBG(rule.id_type == loot_rule_data_t::id_t::item_ref);

        auto const id = item_def_id {rule.id};
        auto const n  = random::uniform_range(gen, rule.count_lo, rule.count_hi);

        if (n > 0) {
            write(id, n);
        }
    }

    //--------------------------------------------------------------------------
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

    //--------------------------------------------------------------------------
    std::vector<uint16_t>         roll_data_;
    std::vector<loot_rule_data_t> rules_;
    utf8string                    id_string_;
    loot_table_def_id             id_;
    roll_t                        type_;
};

} //namespace bkrl

////////////////////////////////////////////////////////////////////////////////
// .cpp
////////////////////////////////////////////////////////////////////////////////

namespace bkrl {

//==============================================================================
//! loot_table_parser
//==============================================================================
class loot_table_parser {
public:
    enum : size_t {
        index_weight = 0
      , index_chance = 0
      , index_id     = 1
      , index_count  = 2
      , index_type   = 3
    };

    //--------------------------------------------------------------------------
    static auto make_rule(
        string_ref                    const id
      , std::pair<uint16_t, uint16_t> const count
      , loot_rule_data_t::id_t        const type
      , std::pair<uint16_t, uint16_t> const data
    ) {
        loot_rule_data_t rule;

        rule.id       = slash_hash32(id);
        rule.count_lo = count.first;
        rule.count_hi = count.second;
        rule.id_type  = type;

        auto const size = std::min(id.size(), rule.id_debug_string.size());

        std::copy_n(
            std::cbegin(id)
          , size
          , std::begin(rule.id_debug_string)
        );

        auto const last = std::min(size, rule.id_debug_string.size() - 1);
        rule.id_debug_string[last] = 0;

        return std::make_pair(std::move(rule), data);
    }

    //--------------------------------------------------------------------------
    static auto make_rule(
        string_ref                    const id
      , std::pair<uint16_t, uint16_t> const count
      , loot_rule_data_t::id_t        const type
      , uint16_t                      const data
    ) {
        return make_rule(id, count, type, std::make_pair(data, uint16_t {}));
    }

    //--------------------------------------------------------------------------
    auto rule_weight(json::cref value) const {
        auto const& weight = value[index_weight];

        auto const w = json::require_int<uint16_t>(weight);
        if (w <= 0) {
            BK_TODO_FAIL();
        }

        return w;
    }

    //--------------------------------------------------------------------------
    auto rule_id(json::cref value) const {
        auto const& id = value[index_id];
        return json::require_string(id);
    }

    //--------------------------------------------------------------------------
    auto rule_count(json::cref value) const {
        auto const& count = value[index_count];

        if (count.is_array()) {
            auto result = std::make_pair(
                json::require_int<uint16_t>(count[0])
              , json::require_int<uint16_t>(count[1])
            );

            if (result.second == 0 || result.first >= result.second) {
                BK_TODO_FAIL();
            }

            return result;
        }

        auto const n = json::require_int<uint16_t>(count);
        if (n <= 0) {
            BK_TODO_FAIL();
        }

        return std::make_pair(n, n);
    }

    //--------------------------------------------------------------------------
    auto rule_chance(json::cref value) const {
        auto const& chance = value[index_chance];

        if (!chance.is_array()) {
            auto const n = json::require_int<uint16_t>(chance);
            if (n > 100) {
                BK_TODO_FAIL();
            }

            return std::make_pair(n, uint16_t {100});
        }

        json::require_array(chance, 2, 2);

        auto const num = json::require_int<uint16_t>(chance[0]);
        auto const den = json::require_int<uint16_t>(chance[1]);

        if (num == 0 || den == 0 || num > den) {
            BK_TODO_FAIL();
        }

        return std::make_pair(num, den);
    }

    //--------------------------------------------------------------------------
    auto rule_type(json::cref value) const {
        auto const& type = value[index_type];

        static auto const item_hash  = slash_hash32("ITEM");
        static auto const table_hash = slash_hash32("TABLE");
        
        if (type.is_null()) {
            return loot_rule_data_t::id_t::item_ref;
        }

        auto const str  = json::require_string(type);
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

    //--------------------------------------------------------------------------
    auto rule_choose_one_rule(json::cref value) {
        json::require_array(value, 2, 4);

        auto const size = value.array_items().size();
        
        return make_rule(
            rule_id(value)
          , (size > 2) ? rule_count(value) : std::make_pair(1, 1)
          , (size > 3) ? rule_type(value)  : loot_rule_data_t::id_t::item_ref
          , rule_weight(value)
        );
    }

    //--------------------------------------------------------------------------
    auto rule_roll_all_rule(json::cref value) {
        json::require_array(value, 2, 4);

        auto const size = value.array_items().size();
        
        return make_rule(
            rule_id(value)
          , (size > 2) ? rule_count(value) : std::make_pair(1, 1)
          , (size > 3) ? rule_type(value)  : loot_rule_data_t::id_t::item_ref
          , rule_chance(value)
        );
    }

    //--------------------------------------------------------------------------
    void rule_rules(json::cref value) {
        json::require_array(value, 1);

        auto const size = value.array_items().size();

        rules_.reserve(size);
        rule_data_.reserve(size);

        if (table_type_ == loot_table_::roll_t::roll_all) {
            for(auto const& rule : value.array_items()) {
                std::tie(cur_rule_, cur_data_) = rule_roll_all_rule(rule);
                rules_.push_back(cur_rule_);
                rule_data_.push_back(cur_data_.first);
                rule_data_.push_back(cur_data_.second);
            }
        } else if (table_type_ == loot_table_::roll_t::choose_one) {
            for(auto const& rule : value.array_items()) {
                std::tie(cur_rule_, cur_data_) = rule_choose_one_rule(rule);
                rules_.push_back(cur_rule_);
                rule_data_.push_back(cur_data_.first);
            }
        }

    }

    //--------------------------------------------------------------------------
    void rule_named_table_id(json::cref value) {
        table_string_ = json::common::get_id_string(value).to_string();
        table_id_     = loot_table_def_id {slash_hash32(table_string_)};
    }
    
    //--------------------------------------------------------------------------
    void rule_named_table_type(json::cref value) {
        static auto const hash_choose_one = slash_hash32("choose_one");
        static auto const hash_roll_all   = slash_hash32("roll_all");

        auto const& type = value[string_ref{"type"}];

        if (type.is_null()) {
            table_type_ = loot_table_::roll_t::roll_all;
            return;
        }

        auto const str  = json::require_string(type);
        auto const hash = slash_hash32(str);

        if (hash == hash_choose_one) {
            table_type_ = loot_table_::roll_t::choose_one;
        } else if (hash == hash_roll_all) {
            table_type_ = loot_table_::roll_t::roll_all;
        } else {
            BK_TODO_FAIL();
        }
    }

    //--------------------------------------------------------------------------
    void rule_named_table(json::cref value) {
        rule_named_table_id(value);
        rule_named_table_type(value);
        rule_rules(value[string_ref{"rules"}]);
    }

    //--------------------------------------------------------------------------
    void rule_simple_table(json::cref value) {
        table_type_ = loot_table_::roll_t::roll_all;
        rule_rules(value);
    }

    //--------------------------------------------------------------------------
    loot_table_ rule_root(json::cref value) {
        rule_data_.clear();
        rules_.clear();
        table_string_.clear();
        table_id_ = loot_table_def_id {0};

        if (value.is_object()) {
            rule_named_table(value);
        } else if (value.is_array()) {
            rule_simple_table(value);
        } else {
            BK_TODO_FAIL();
        }

        auto result = loot_table_ {table_type_, rule_data_, rules_};
        result.set_id(table_string_, table_id_);
        
        return result;
    }
private:
    loot_rule_data_t              cur_rule_;
    std::pair<uint16_t, uint16_t> cur_data_;

    std::vector<uint16_t>         rule_data_;
    std::vector<loot_rule_data_t> rules_;
    loot_table_::roll_t           table_type_;
    utf8string                    table_string_;
    loot_table_def_id             table_id_;
};

//==============================================================================
//! loot_table_definitions_parser
//==============================================================================
class loot_table_definitions_parser {
public:
    //--------------------------------------------------------------------------
    void rule_definition(json::cref value) {
        auto table = table_parser_.rule_root(value);
        auto const id = table.id();

        auto const result = tables_.emplace(id, std::move(table));
        if (!result.second) {
            BK_TODO_FAIL();
        }
    }
    
    //--------------------------------------------------------------------------
    void rule_definitions(json::cref value) {
        auto const& defs = json::require_array(value[string_ref{"definitions"}], 1);

        for (auto const& def : defs.array_items()) {
            rule_definition(def);
        }
    }
    
    //--------------------------------------------------------------------------
    void rule_file_type(json::cref value) const {
        static auto const hash_loot = slash_hash32("LOOT");

        auto const str  = json::require_string(value[string_ref{"file_type"}]);
        auto const hash = slash_hash32(str);

        if (hash != hash_loot) {
            BK_TODO_FAIL();
        }
    }
    
    //--------------------------------------------------------------------------
    void rule_root(json::cref value) {
        auto const& definitions = json::require_object(value);
        rule_file_type(definitions);
        rule_definitions(definitions);
    }
protected:
    loot_table_parser table_parser_;

    template <typename K, typename V>
    using map_t = boost::container::flat_map<K, V, std::less<>>;

    map_t<loot_table_def_id, loot_table_> tables_;
};

} //namespace bkrl

//==============================================================================
//! loot_table_definitions_impl
//==============================================================================
class bkrl::loot_table_definitions_impl : loot_table_definitions_parser {
public:
    //--------------------------------------------------------------------------
    void load_definitions(json::cref data) {
        rule_root(data);
    }

    //--------------------------------------------------------------------------
    loot_table_ const& operator[](loot_table_def_id const id) const {
        auto const result = tables_.find(id);
        
        if (result == std::cend(tables_)) {
            BK_TODO_FAIL();
        }

        return result->second;
    }
};

////////////////////////////////////////////////////////////////////////////////
// loot_table_definitions
////////////////////////////////////////////////////////////////////////////////
bkrl::loot_table_definitions::~loot_table_definitions() = default;

bkrl::loot_table_definitions::loot_table_definitions(loot_table_definitions&&) = default;
bkrl::loot_table_definitions&

bkrl::loot_table_definitions::operator=(loot_table_definitions&&) = default;

bkrl::loot_table_definitions::loot_table_definitions()
  : impl_ {std::make_unique<loot_table_definitions_impl>()}
{
}

void bkrl::loot_table_definitions::load_definitions(json::cref data) {
    impl_->load_definitions(data);
}

bkrl::loot_table_ const&
bkrl::loot_table_definitions::operator[](loot_table_def_id const id) const {
    return (*impl_)[id];
}

////////////////////////////////////////////////////////////////////////////////
// tests
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Loot test simple", "[loot]") {
    char const table[] = R"(
    { "loot": [
        [10,  "WEAPON_SWORD_SHORT"]
      , [10,  "WEAPON_LONG_SWORD"]
      , [20,  "WEAPON_HAMMER"]
      , [10,  "WEAPON_STAFF"]
      , [10,  "WEAPON_DAGGER"]
      , [10,  "WEAPON_AXE"]
      , [100, "TROPHY"]
      , [[1, 250], "POTION_MINOR", [1, 3]]
      ]
    }
    )";

    bkrl::loot_table_definitions defs;
    bkrl::loot_table_parser      parser;
    bkrl::random_t               rand {900010};
    std::map<bkrl::hash_t, int>  rolls;

    auto json = bkrl::json::common::from_memory(table);
    auto const result = parser.rule_root(json[bkrl::string_ref("loot")]);

    for (int i = 0; i < 10000; ++i) {
        result.generate(rand, defs, [&](bkrl::item_def_id const id, int const n) {
            BK_ASSERT(n > 0);

            auto& sum = rolls[bkrl::id_to_value(id)];
            sum++;
        });
    }
}

TEST_CASE("Loot test named", "[loot]") {
char const table[] = R"(
    { "id": "BASIC_WEAPON"
    , "type": "choose_one"
    , "rules": [
        [10, "WEAPON_SWORD_SHORT"]
      , [10, "WEAPON_LONG_SWORD"]
      , [1,  "TROPHY"]
      , [10, "WEAPON_HAMMER"]
      , [10, "WEAPON_STAFF"]
      , [10, "WEAPON_DAGGER"]
      , [2,  "POTION_MINOR", [1, 5]]
      , [10, "WEAPON_AXE"]
      ]
    }
    )";

    bkrl::loot_table_definitions defs;
    bkrl::loot_table_parser      parser;
    bkrl::random_t               rand {900010};
    std::map<bkrl::hash_t, int>  rolls;

    auto json = bkrl::json::common::from_memory(table);
    auto const result = parser.rule_root(json);

    for (int i = 0; i < 6300; ++i) {
        result.generate(rand, defs, [&](bkrl::item_def_id const id, int const n) {
            BK_ASSERT(n > 0);

            auto& sum = rolls[bkrl::id_to_value(id)];
            sum++;
        });
    }
}

TEST_CASE("Loot test definitions", "[loot]") {
    char const table_defs[] = R"(
    { "file_type": "LOOT"
    , "definitions": [
        { "id": "BASIC_WEAPON"
        , "type": "choose_one"
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
        , "rules": [
            [10, "BASIC_WEAPON", 1, "TABLE"]
          , [10, "BASIC_POTION", 1, "TABLE"]
          , [50, "WEAPON_SHORT_BOW", 1]
          ]
        }
      ]
    }
    )";

    bkrl::loot_table_definitions defs;

    auto json = bkrl::json::common::from_memory(table_defs);

    defs.load_definitions(json);
}
