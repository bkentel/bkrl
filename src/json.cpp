#include "json.hpp"

#if BOOST_OS_WINDOWS
#   include <utf8.h>
#endif

namespace json = bkrl::json;
namespace jc   = bkrl::json::common;

using bkrl::string_ref;
using bkrl::json::cref;

//------------------------------------------------------------------------------
string_ref const jc::field_filetype    {"file_type"};
string_ref const jc::field_stringtype  {"string_type"};
string_ref const jc::field_language    {"language"};
string_ref const jc::field_definitions {"definitions"};
string_ref const jc::field_id          {"id"};
string_ref const jc::field_name        {"name"};
string_ref const jc::field_text        {"text"};
string_ref const jc::field_sort        {"sort"};
string_ref const jc::field_mappings    {"mappings"};
string_ref const jc::field_filename    {"file_name"};
string_ref const jc::field_tile_size   {"tile_size"};
string_ref const jc::field_stack       {"stack"};
string_ref const jc::field_damage_min  {"damage_min"};
string_ref const jc::field_damage_max  {"damage_max"};
//------------------------------------------------------------------------------
string_ref const jc::filetype_config  {"CONFIG"};
string_ref const jc::filetype_locale  {"LOCALE"};
string_ref const jc::filetype_item    {"ITEM"};
string_ref const jc::filetype_entity  {"ENTITY"};
string_ref const jc::filetype_tilemap {"TILEMAP"};
string_ref const jc::filetype_keymap  {"KEYMAP"};
//------------------------------------------------------------------------------
string_ref const jc::stringtype_messages  {"MESSAGE"};

//------------------------------------------------------------------------------
#if BOOST_OS_WINDOWS
bkrl::path_string jc::get_filename(cref value) {
    auto const str = require_string(value[field_filename]);
    
    path_string result;
    result.reserve(str.length()); //TODO a bit wasteful

    utf8::utf8to16(str.begin(), str.end(), std::back_inserter(result));

    return result;
}
#else
bkrl::path_string jc::get_filename(cref value) {
    auto const str = require_string(value[field_filename]);
    return str.to_string();
}
#endif

//------------------------------------------------------------------------------
namespace {

struct random_parser {
    using dist_t = bkrl::random::random_dist;
    using hash_t = bkrl::hash_t;

    //--------------------------------------------------------------------------
    random_parser(cref value) {
        json::require_array(value);
        rule_type(value);
    }

    //--------------------------------------------------------------------------
    void rule_type(cref value) {
        using key_t = bkrl::hashed_string_ref const;

        static key_t type_constant {"constant"};
        static key_t type_uniform  {"uniform"};
        static key_t type_dice     {"dice"};
        static key_t type_normal   {"normal"};

        key_t type = json::require_string(value[0]);

        if      (type == type_constant) { rule_constant(value); }
        else if (type == type_uniform)  { rule_uniform(value); }
        else if (type == type_dice)     { rule_dice(value); }
        else if (type == type_normal)   { rule_normal(value); }
        else                            { BK_TODO_FAIL(); }
    }

    //--------------------------------------------------------------------------
    void rule_constant(cref value) {
        json::require_array(value, 2, 2);

        auto const n = json::require_int(value[1]);
        dist_.set_constant(n);
    }

    //--------------------------------------------------------------------------
    void rule_uniform(cref value) {
        json::require_array(value, 3, 3);

        auto const lo = json::require_int(value[1]);
        auto const hi = json::require_int(value[2]);

        if (lo > hi) {
            BK_TODO_FAIL();
        }

        dist_.set_uniform(lo, hi);
    }

    //--------------------------------------------------------------------------
    void rule_dice(cref value) {
        json::require_array(value, 3, 4);

        auto const count = json::require_int(value[1]);
        auto const sides = json::require_int(value[2]);
        auto const mod   = json::default_int(value[3], 0);

        if (count < 1) {
            BK_TODO_FAIL();
        }

        if (sides < 1) {
            BK_TODO_FAIL();
        }

        dist_.set_dice(count, sides, mod);
    }

    //--------------------------------------------------------------------------
    void rule_normal(cref value) {
        static auto const def_min = std::numeric_limits<int>::min();
        static auto const def_max = std::numeric_limits<int>::max();
        
        json::require_array(value, 3, 5);

        auto const mean  = json::require_float<double>(value[1]);
        auto const sigma = json::require_float<double>(value[2]);
        auto const min   = json::default_int(value[3], def_min);
        auto const max   = json::default_int(value[4], def_max);

        dist_.set_normal(mean, sigma, min, max);
    }

    //--------------------------------------------------------------------------
    operator dist_t() const {
        return dist_;
    }
private:
    //--------------------------------------------------------------------------
    dist_t dist_;
};

} //namespace

bkrl::random::random_dist
json::common::get_random(cref json) {
    return random_parser {json};
}
