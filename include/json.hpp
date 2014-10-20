#pragma once

#include <json11/json11.hpp>
#include "assert.hpp"
#include "types.hpp"
#include "util.hpp"
#include "random.hpp"

namespace bkrl {
namespace json {

using cref = json11::Json const&;

//==============================================================================
//==============================================================================
template <typename T> optional<string_ref> optional_string(T) = delete;

inline optional<string_ref> optional_string(cref value) {
    return value.is_string()
        ? optional<string_ref> {value.string_value()}
        : optional<string_ref> {};
}

//==============================================================================
//==============================================================================
template <typename T> string_ref default_string(T, string_ref) = delete;

inline string_ref default_string(cref value, string_ref const def) {
    auto const result = optional_string(value);
    return result ? *result : def;
}

//==============================================================================
//==============================================================================
template <typename T> string_ref require_string(T) = delete;

inline string_ref require_string(cref value) {
    auto const result = optional_string(value);
    if (!result) {
        BK_TODO_FAIL();
    }

    return *result;
}

//==============================================================================
//==============================================================================
template <typename T, typename U> optional<T> optional_int(U) = delete;

template <typename T = int>
inline optional<T> optional_int(cref value) {
    static_assert(std::is_integral<T>::value, "");

    using limits = std::numeric_limits<T>;

    static auto const min = static_cast<double>(limits::min());
    static auto const max = static_cast<double>(limits::max());

    if (!value.is_number()) {
        return boost::none;
    }

    auto const result = value.number_value();
    
    if (result < min) {
        //TODO warning
        return static_cast<T>(min);
    } else if (result > max) {
        //TODO warning
        return static_cast<T>(max);
    }

    return static_cast<T>(result);
}

//==============================================================================
//==============================================================================
template <typename T, typename U> T default_int(U, T) = delete;

template <typename T = int>
inline T default_int(cref value, T const def) {
    auto const result = optional_int<T>(value);
    return result ? *result : def;
}

//==============================================================================
//==============================================================================
template <typename T, typename U> T require_int(U) = delete;

template <typename T = int>
inline T require_int(cref value) {
    auto const result = optional_int<T>(value);
    if (!result) {
        BK_TODO_FAIL();
    }

    return *result;
}

//==============================================================================
//==============================================================================
template <typename T, typename U> optional<T> optional_float(U) = delete;

template <typename T = float>
inline optional<T> optional_float(cref value) {
    static_assert(std::is_floating_point<T>::value, "");

    using limits = std::numeric_limits<T>;

    static auto const min = static_cast<double>(limits::min());
    static auto const max = static_cast<double>(limits::max());

    if (!value.is_number()) {
        return boost::none;
    }

    auto const result = value.number_value();
    
    if (result < min) {
        //TODO warning
        return static_cast<T>(min);
    } else if (result > max) {
        //TODO warning
        return static_cast<T>(max);
    }

    return static_cast<T>(result);
}

//==============================================================================
//==============================================================================
template <typename T, typename U> T default_float(U, T) = delete;

template <typename T = float>
inline T default_float(cref value, T const def) {
    auto const result = optional_float<T>(value);
    return result ? *result : def;
}

//==============================================================================
//==============================================================================
template <typename T, typename U> T require_float(U) = delete;

template <typename T = float>
inline T require_float(cref value) {
    auto const result = optional_float<T>(value);
    if (!result) {
        BK_TODO_FAIL();
    }

    return *result;
}

//==============================================================================
//==============================================================================
template <typename T, typename U> T require_float(U, T, T) = delete;

template <typename T = float>
inline T require_float(cref value, T const min, T const max) {
    auto const result = require_float<T>(value);
    if (result < min) {
        BK_TODO_FAIL();
    } else if (result > max) {
        BK_TODO_FAIL();
    }

    return result;
}

//==============================================================================
//==============================================================================
template <typename T> cref require_object(T) = delete;

inline cref require_object(cref json) {
    if (!json.is_object()) {
        BK_TODO_FAIL();
    }

    return json;
}

//==============================================================================
//==============================================================================
template <typename T> cref require_array(T, size_t, size_t) = delete;

inline cref require_array(cref json, size_t const min_size = 0, size_t const max_size = 0) {
    if (!json.is_array()) {
        BK_TODO_FAIL();
    }

    auto const size = json.array_items().size();

    if (size < min_size) {
        BK_TODO_FAIL();
    } else if (max_size && (size > max_size)) {
        BK_TODO_FAIL();
    }

    return json;
}

//==============================================================================
//==============================================================================
template <typename T> bool has_field(T, string_ref) = delete;

inline bool has_field(cref value, string_ref const field) {
    return value.is_object() && !value[field].is_null();
}

//==============================================================================
//==============================================================================
template <typename T> bool has_field(T, size_t) = delete;

inline bool has_field(cref value, size_t const i) {
    return value.is_array() && !value[i].is_null();
}

//==============================================================================
//==============================================================================
namespace common {
//------------------------------------------------------------------------------
extern string_ref const field_filetype;
extern string_ref const field_stringtype;
extern string_ref const field_language;
extern string_ref const field_definitions;
extern string_ref const field_id;
extern string_ref const field_name;
extern string_ref const field_text;
extern string_ref const field_sort;
extern string_ref const field_mappings;
extern string_ref const field_filename;
extern string_ref const field_tile_size;
extern string_ref const field_stack;
extern string_ref const field_damage_min;
extern string_ref const field_damage_max;
extern string_ref const field_tile;
extern string_ref const field_color;
extern string_ref const field_items;
extern string_ref const field_health;
//------------------------------------------------------------------------------
extern string_ref const filetype_config;
extern string_ref const filetype_locale;
extern string_ref const filetype_item;
extern string_ref const filetype_entity;
extern string_ref const filetype_tilemap;
extern string_ref const filetype_keymap;
extern string_ref const filetype_messages;
//------------------------------------------------------------------------------

//==============================================================================
//==============================================================================
template <typename T> path_string get_filename(T) = delete;
path_string get_filename(cref value);

//==============================================================================
//==============================================================================
template <typename T> string_ref get_filetype(T) = delete;
inline string_ref get_filetype(cref value) {
    return require_string(value[field_filetype]);
}

//==============================================================================
//==============================================================================
template <typename T> string_ref get_filetype(T, string_ref) = delete;
inline string_ref get_filetype(cref value, string_ref const expected) {
    auto const result = get_filetype(value);
    if (result != expected) {
        BK_TODO_FAIL();
    }

    return result;
}

//==============================================================================
//==============================================================================
inline json11::Json from_memory(utf8string const& data) {
    std::string error;
    auto const json = json11::Json::parse(data, error);

    if (!error.empty()) {
        BK_TODO_FAIL();
    }

    return json;
}

//==============================================================================
//==============================================================================
inline json11::Json from_file(path_string_ref const filename) {
    auto const data = read_file(filename);
    return from_memory(data);
}

//==============================================================================
//==============================================================================
template <typename T> optional<lang_id> get_locale(T, string_ref) = delete;
inline optional<lang_id> get_locale(cref value, string_ref const expected_type) {
    auto const type = require_string(value[field_stringtype]);
    if (type != expected_type) {
        return optional<lang_id> {};
    }

    auto const lang = require_string(value[field_language]);
    auto const size = lang.size();
    if (size == 2) {
        return BK_MAKE_LANG_CODE2(lang[0], lang[1]);
    } else if (size == 3) {
        return BK_MAKE_LANG_CODE3(lang[0], lang[1], lang[2]);
    }

    return optional<lang_id> {};
}

//==============================================================================
//==============================================================================
template <typename T> random::random_dist get_random(T) = delete;

random::random_dist get_random(cref json);

//struct random {
//    using cref   = json::cref;
//    using dist_t = bkrl::random::random_dist;
//
//    random(cref value) {
//        require_array(value, 1);
//        rule_type(value);
//    }
//
//    void rule_type(cref value) {
//        static string_ref const type_constant_str {"constant"};
//        static string_ref const type_uniform_str  {"uniform"};
//        static string_ref const type_dice_str     {"dice"};
//        static string_ref const type_normal_str   {"normal"};
//
//        static hash_t const type_constant_hash = slash_hash32(type_constant_str);
//        static hash_t const type_uniform_hash  = slash_hash32(type_uniform_str);
//        static hash_t const type_dice_hash     = slash_hash32(type_dice_str);
//        static hash_t const type_normal_hash   = slash_hash32(type_normal_str);
//
//        auto const type_str  = require_string(value[0]);
//        auto const type_hash = slash_hash32(type_str);
//
//        if (type_hash == type_constant_hash) {
//            rule_constant(value);
//        } else if (type_hash == type_uniform_hash) {
//            rule_uniform(value);
//        } else if (type_hash == type_dice_hash) {
//            rule_dice(value);
//        } else if (type_hash == type_normal_hash) {
//            rule_normal(value);
//        } else {
//            BK_TODO_FAIL();
//        }
//    }
//
//    void rule_constant(cref value) {
//        require_array(value, 2, 2);
//
//        auto const n = require_int(value[1]);
//
//        dist_.set_constant(n);
//    }
//
//    void rule_uniform(cref value) {
//        require_array(value, 3, 3);
//
//        auto const lo = require_int(value[1]);
//        auto const hi = require_int(value[2]);
//
//        if (lo > hi) {
//            BK_TODO_FAIL();
//        }
//
//        dist_.set_uniform(lo, hi);
//    }
//
//    void rule_dice(cref value) {
//        require_array(value, 3, 4);
//
//        auto const count = require_int(value[1]);
//        auto const sides = require_int(value[2]);
//        auto const mod   = default_int(value[3], 0);
//
//        if (count < 1) {
//            BK_TODO_FAIL();
//        }
//
//        if (sides < 1) {
//            BK_TODO_FAIL();
//        }
//
//        dist_.set_dice(count, sides, mod);
//    }
//
//    void rule_normal(cref value) {
//        require_array(value, 3, 5);
//
//        auto const mean  = require_float<double>(value[1]);
//        auto const sigma = require_float<double>(value[2]);
//
//        auto const min = default_int(value[3], std::numeric_limits<int>::min());
//        auto const max = default_int(value[4], std::numeric_limits<int>::max());
//
//        dist_.set_normal(mean, sigma, min, max);
//    }
//
//    operator dist_t() const {
//        return dist_;
//    }
//
//    dist_t dist_;
//};

} //namespace common

}} //namespace bkrl::json
