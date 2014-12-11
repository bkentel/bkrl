#include "loot_table.hpp"

#include <boost/container/flat_map.hpp>

#include "hash.hpp"
#include "json.hpp"

namespace bkrl {

namespace json {

template <typename F>
inline void for_each(cref array, F&& f) {
    for (cref e : array.array_items()) {
        f(e);
    }
}

template <typename F>
inline void for_each(cref value, string_ref const key, F&& f) {
    for (cref e : require_array(value[key]).array_items()) {
        f(e);
    }
}

template <typename Result, typename Check>
inline Result require_int(cref value, Check&& check) {
    auto const result = require_int<Result>(value);
    if (!check(result)) {
        BK_TODO_FAIL();
    }

    return result;
}

template <typename Result, typename Check>
inline Result require_int(cref value, string_ref const key, Check&& check) {
    return require_int<Result>(value[key], std::forward<Check>(check));
}

template <typename Result, typename Check>
inline Result require_int(cref value, size_t const key, Check&& check) {
    return require_int<Result>(value[key], std::forward<Check>(check));
}

inline auto require_string_id(cref value) {
    auto const str = json::require_string(value);
    if (str.empty()) {
        BK_TODO_FAIL();
    }

    auto const hash = slash_hash32(str);

    return std::make_pair(str, hash);
}

inline auto require_string_id(cref value, string_ref const key) {
    return require_string_id(value[key]);
}

template <typename T, size_t N>
inline T find_or_fail(std::pair<hash_t, T> const (&mappings)[N], hash_t const value) {
    for (auto const& m : mappings) {
        if (m.first == value) {
            return m.second;
        }
    }

    BK_TODO_FAIL();
}

} //namespace json

inline void assign(string_ref const src, utf8string& dst) {
    dst.assign(std::begin(src), std::end(src));
}

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

//==============================================================================
//! loot_table_parser_impl
//==============================================================================
class detail::loot_table_parser_impl {
public:
    using id_t    = loot_rule_data_t::id_t;
    using roll_t  = loot_table::roll_t;
    using count_t = std::pair<uint16_t, uint16_t>;
    using data_t  = std::pair<uint16_t, uint16_t>;

    enum : size_t {
        index_weight = 0
      , index_chance = 0
      , index_id     = 1
      , index_count  = 2
      , index_type   = 3
    };

    //--------------------------------------------------------------------------
    static auto make_rule(
        string_ref const id
      , count_t    const count
      , id_t       const type
      , data_t     const data
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
        string_ref const id
      , count_t    const count
      , id_t       const type
      , uint16_t   const data
    ) {
        return make_rule(id, count, type, std::make_pair(data, uint16_t {}));
    }

    //--------------------------------------------------------------------------
    auto rule_weight(json::cref value) const {
        return json::require_int<uint16_t>(value, index_weight, [](auto const w) {
            return w > 0;
        });
    }

    //--------------------------------------------------------------------------
    auto rule_id(json::cref value) const {
        auto const& id = value[index_id];
        return json::require_string(id);
    }

    //--------------------------------------------------------------------------
    auto rule_count_1(json::cref value) const {
        auto const count = json::require_int<uint16_t>(value, [](auto const n) {
            return n > 0;
        });

        return std::make_pair(count, count);
    }

    //--------------------------------------------------------------------------
    auto rule_count_2(json::cref value) const {
        json::require_array(value, 2, 2);

        auto const lo = json::require_int<uint16_t>(value[0]);
        auto const hi = json::require_int<uint16_t>(value, 1, [&](auto const n) {
            return n && n >= lo;
        });

        return std::make_pair(lo, hi);
    }

    //--------------------------------------------------------------------------
    auto rule_count(json::cref value) const {
        auto const& count = value[index_count];

        if (count.is_array()) {
            return rule_count_2(count);
        }

        return rule_count_1(count);
    }

    //--------------------------------------------------------------------------
    auto rule_chance_1(json::cref value) const {
        auto const chance = json::require_int<uint16_t>(value, [](auto const n) {
            return n > 0 && n <= 100;
        });

        return std::make_pair(chance, uint16_t {100});
    }

    //--------------------------------------------------------------------------
    auto rule_chance_2(json::cref value) const {
        json::require_array(value, 2, 2);

        auto const num = json::require_int<uint16_t>(value[0]);
        auto const den = json::require_int<uint16_t>(value, 1, [&](auto const n) {
            return num && n && num < n;
        });

        return std::make_pair(num, den);
    }

    //--------------------------------------------------------------------------
    auto rule_chance(json::cref value) const {
        auto const& chance = value[index_chance];

        if (chance.is_array()) {
            return rule_chance_2(chance);
        }

        return rule_chance_1(chance);
    }

    //--------------------------------------------------------------------------   
    auto rule_type(json::cref value) const {
        static std::pair<hash_t, id_t> const map[] = {
            {slash_hash32("item"),  id_t::item_ref}
          , {slash_hash32("table"), id_t::table_ref}
        };

        auto const& type = value[index_type];
        if (type.is_null()) {
            return id_t::item_ref;
        }

        auto const id = json::require_string_id(type);
        return json::find_or_fail(map, id.second);
    }

    //--------------------------------------------------------------------------
    auto rule_choose_one_rule(json::cref value) {
        json::require_array(value, 2, 4);

        auto const size = value.array_items().size();
        
        return make_rule(
            rule_id(value)
          , (size > 2) ? rule_count(value) : count_t {1, 1}
          , (size > 3) ? rule_type(value)  : id_t::item_ref
          , rule_weight(value)
        );
    }

    //--------------------------------------------------------------------------
    auto rule_roll_all_rule(json::cref value) {
        json::require_array(value, 2, 4);

        auto const size = value.array_items().size();
        
        return make_rule(
            rule_id(value)
          , (size > 2) ? rule_count(value) : count_t {1, 1}
          , (size > 3) ? rule_type(value)  : id_t::item_ref
          , rule_chance(value)
        );
    }

    //--------------------------------------------------------------------------
    void rule_rules_roll_all(json::cref value) {
        json::for_each(value, [&](json::cref rule) {
            std::tie(cur_rule_, cur_data_) = rule_roll_all_rule(rule);
                
            rules_.push_back(cur_rule_);
                
            rule_data_.push_back(cur_data_.first);
            rule_data_.push_back(cur_data_.second);
        });
    }

    //--------------------------------------------------------------------------
    void rule_rules_choose_one(json::cref value) {
        json::for_each(value, [&](json::cref rule) {
            std::tie(cur_rule_, cur_data_) = rule_choose_one_rule(rule);

            rules_.push_back(cur_rule_);

            rule_data_.push_back(cur_data_.first);
        });
    }

    //--------------------------------------------------------------------------
    void rule_rules(json::cref value) {
        json::require_array(value, 1);

        auto const size = value.array_items().size();

        rules_.reserve(size);

        if (table_type_ == roll_t::roll_all) {
            rule_data_.reserve(size * 2);
            rule_rules_roll_all(value);
        } else if (table_type_ == roll_t::choose_one) {
            rule_data_.reserve(size);
            rule_rules_choose_one(value);
        } else {
            BK_TODO_FAIL();
        }
    }

    //--------------------------------------------------------------------------
    void rule_named_table_id(json::cref value) {
        auto const& id = value[json::common::field_id];
        if (id.is_null()) {
            return;
        }

        auto const string_id = json::require_string_id(id);

        assign(string_id.first, table_string_);
        table_id_ = loot_table_def_id {string_id.second};
    }
    
    //--------------------------------------------------------------------------
    void rule_named_table_type(json::cref value) {
        static std::pair<hash_t, roll_t> const map[] = {
            {slash_hash32("choose_one"), roll_t::choose_one}
          , {slash_hash32("roll_all"),   roll_t::roll_all}
        };

        auto const& type = value[string_ref{"type"}];

        table_type_ = type.is_null()
            ? roll_t::roll_all
            : json::find_or_fail(map, json::require_string_id(type).second);
    }

    //--------------------------------------------------------------------------
    void rule_named_table(json::cref value) {
        rule_named_table_id(value);      
        rule_named_table_type(value);
        rule_rules(value[string_ref{"rules"}]);
    }

    //--------------------------------------------------------------------------
    void rule_simple_table(json::cref value) {
        table_type_ = roll_t::roll_all;
        rule_rules(value);
    }

    //--------------------------------------------------------------------------
    loot_table rule_root(json::cref value) {
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

        return loot_table {table_string_, table_type_, rule_data_, rules_};
    }
private:
    loot_rule_data_t              cur_rule_;
    std::pair<uint16_t, uint16_t> cur_data_;

    std::vector<uint16_t>         rule_data_;
    std::vector<loot_rule_data_t> rules_;
    loot_table::roll_t            table_type_;
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

        if (table.id() == loot_table_def_id {0}) {
            BK_TODO_FAIL();
        }

        auto const result = tables_.emplace(id, std::move(table));
        if (!result.second) {
            BK_TODO_FAIL();
        }
    }
    
    //--------------------------------------------------------------------------
    void rule_definitions(json::cref value) {
        json::for_each(value, json::common::field_definitions, [&](json::cref def) {
            rule_definition(def);
        });
    }
    
    //--------------------------------------------------------------------------
    void rule_file_type(json::cref value) const {
        json::common::get_filetype(value, string_ref{"LOOT"}); //TODO
    }
    
    //--------------------------------------------------------------------------
    void rule_root(json::cref value) {
        auto const& definitions = json::require_object(value);
        rule_file_type(definitions);
        rule_definitions(definitions);
    }
protected:
    detail::loot_table_parser_impl table_parser_;

    template <typename K, typename V>
    using map_t = boost::container::flat_map<K, V, std::less<>>;

    map_t<loot_table_def_id, loot_table> tables_;
};

} //namespace bkrl

////////////////////////////////////////////////////////////////////////////////
// loot_table
////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------
bkrl::loot_table::loot_table()
  : type_ {roll_t::roll_all}
{
}

//--------------------------------------------------------------------------
void bkrl::loot_table::generate(random_t& gen, defs_t defs, write_t const& write) const {
    if (type_ == roll_t::roll_all) {
        roll_all_(gen, defs, write);
    } else if (type_ == roll_t::choose_one) {
        choose_one_(gen, defs, write);
    } else {
        BK_TODO_FAIL();
    }
}

//--------------------------------------------------------------------------
void bkrl::loot_table::set_id(string_ref const id_str, loot_table_def_id const id) {
    id_string_.assign(id_str.data(), id_str.size());
    id_ = id;
}

//--------------------------------------------------------------------------
void bkrl::loot_table::tranform_data_() {
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
void bkrl::loot_table::roll_all_(random_t& gen, defs_t defs, write_t const& write) const {
    repeat_n(rules_.size(), [&](size_t const i) {
        auto const& rule = rules_[i];

        auto const num = roll_data_[i*2 + 0];
        auto const den = roll_data_[i*2 + 1];

        auto const lo = static_cast<uint16_t>(0);
        auto const hi = static_cast<uint16_t>(den - 1);

        auto const roll = random::uniform_range(gen, lo, hi);
        if (roll < num) {
            roll_one_(gen, defs, rule, write);
        }
    });
}
    
//--------------------------------------------------------------------------
void bkrl::loot_table::choose_one_(random_t& gen, defs_t defs, write_t const& write) const {
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

    roll_one_(gen, defs, rule, write);
}

//--------------------------------------------------------------------------
void bkrl::loot_table::roll_one_(random_t& gen, defs_t defs, rule_t rule, write_t const& write) const {
    using id_t = loot_rule_data_t::id_t;
        
    if (rule.id_type == id_t::item_ref) {
        roll_one_item_(gen, rule, write);
    } else if (rule.id_type == id_t::table_ref) {
        roll_one_table_(gen, defs, rule, write);
    } else {
        BK_TODO_FAIL();
    }
}

//--------------------------------------------------------------------------
void bkrl::loot_table::roll_one_item_(random_t& gen, rule_t rule, write_t const& write) const {
    BK_ASSERT_DBG(rule.id_type == loot_rule_data_t::id_t::item_ref);

    auto const id = item_def_id {rule.id};
    auto const n  = random::uniform_range(gen, rule.count_lo, rule.count_hi);

    if (n > 0) {
        write(id, n);
    }
}

//--------------------------------------------------------------------------
void bkrl::loot_table::roll_one_table_(random_t& gen, defs_t defs, rule_t rule, write_t const& write) const {
    BK_ASSERT_DBG(rule.id_type == loot_rule_data_t::id_t::table_ref);

    auto const id = loot_table_def_id {rule.id};
    auto const n  = random::uniform_range(gen, rule.count_lo, rule.count_hi);

    auto const& table = defs[id];

    repeat_n(n, [&] {
        table.generate(gen, defs, write);
    });
}

//==============================================================================
//! loot_table_definitions_impl
//==============================================================================
class bkrl::detail::loot_table_definitions_impl : loot_table_definitions_parser {
public:
    //--------------------------------------------------------------------------
    void load_definitions(json::cref data) {
        rule_root(data);
    }

    //--------------------------------------------------------------------------
    loot_table const& operator[](loot_table_def_id const id) const {
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
  : impl_ {std::make_unique<detail::loot_table_definitions_impl>()}
{
}

void bkrl::loot_table_definitions::load_definitions(json::cref data) {
    impl_->load_definitions(data);
}

bkrl::loot_table const&
bkrl::loot_table_definitions::operator[](loot_table_def_id const id) const {
    return (*impl_)[id];
}

////////////////////////////////////////////////////////////////////////////////
// loot_table_parser
////////////////////////////////////////////////////////////////////////////////

bkrl::loot_table_parser::~loot_table_parser() = default;

bkrl::loot_table_parser::loot_table_parser()
  : impl_ {std::make_unique<detail::loot_table_parser_impl>()}
{
}

bkrl::loot_table bkrl::loot_table_parser::parse(json::cref data) {
    return impl_->rule_root(data);
}
