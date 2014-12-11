#include "json.hpp"
#include "hash.hpp"

#if BOOST_OS_WINDOWS
#   include <utf8.h>
#endif

namespace json = bkrl::json;
namespace jc   = bkrl::json::common;

using bkrl::json::common::field_string;
using bkrl::json::cref;

//------------------------------------------------------------------------------
field_string const jc::field_filetype    {"file_type"};
field_string const jc::field_stringtype  {"string_type"};
field_string const jc::field_language    {"language"};
field_string const jc::field_definitions {"definitions"};
field_string const jc::field_id          {"id"};
field_string const jc::field_name        {"name"};
field_string const jc::field_text        {"text"};
field_string const jc::field_sort        {"sort"};
field_string const jc::field_mappings    {"mappings"};
field_string const jc::field_filename    {"file_name"};
field_string const jc::field_tile_size   {"tile_size"};
field_string const jc::field_stack       {"stack"};
field_string const jc::field_damage_min  {"damage_min"};
field_string const jc::field_damage_max  {"damage_max"};
field_string const jc::field_damage_type {"damage_type"};
field_string const jc::field_tile        {"tile"};
field_string const jc::field_color       {"color"};
field_string const jc::field_items       {"items"};
field_string const jc::field_health      {"health"};
field_string const jc::field_substantive_seed {"substantive_seed"};
field_string const jc::field_trivial_seed     {"trivial_seed"};
field_string const jc::field_window_size      {"window_size"};
field_string const jc::field_window_pos       {"window_pos"};
field_string const jc::field_font             {"font"};
field_string const jc::field_type             {"type"};
field_string const jc::field_slot             {"slot"};
field_string const jc::field_player           {"player"};
field_string const jc::field_armor_level      {"armor_level"};
field_string const jc::field_weight           {"weight"};
//------------------------------------------------------------------------------
field_string const jc::filetype_config   {"CONFIG"};
field_string const jc::filetype_locale   {"LOCALE"};
field_string const jc::filetype_item     {"ITEM"};
field_string const jc::filetype_entity   {"ENTITY"};
field_string const jc::filetype_tilemap  {"TILEMAP"};
field_string const jc::filetype_keymap   {"KEYMAP"};
field_string const jc::filetype_messages {"MESSAGE"};
field_string const jc::filetype_loot     {"LOOT"};
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#if BOOST_OS_WINDOWS
bkrl::path_string jc::get_path_string(cref value) {
    auto const str = require_string(value);
    
    path_string result;
    result.reserve(str.length()); //TODO a bit wasteful

    utf8::utf8to16(str.begin(), str.end(), std::back_inserter(result));

    return result;
}
#else
bkrl::path_string jc::get_path_string(cref value) {
    auto const str = require_string(value);
    return str.to_string();
}
#endif

//------------------------------------------------------------------------------
bkrl::path_string jc::get_filename(cref value) {
    return get_path_string(value[field_filename]);
}

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
        using mapping_t = bkrl::string_ref_mapping<int> const;

        static mapping_t mappings[] = {
            {"constant", 0}
          , {"uniform",  1}
          , {"dice",     2}
          , {"normal",   3}
        };

        auto const& type = json::require_string(value[0]);
        auto const  hash = bkrl::slash_hash32(type);

        auto const which = find_mapping(mappings, hash, -1);

        switch (which) {
        case 0  : rule_constant(value); break;
        case 1  : rule_uniform(value);  break;
        case 2  : rule_dice(value);     break;
        case 3  : rule_normal(value);   break;
        default : BK_TODO_FAIL();       break;
        }
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
