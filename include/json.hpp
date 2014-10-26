#pragma once

#include <json11/json11.hpp>

#include "assert.hpp"
#include "types.hpp"
#include "util.hpp"
#include "random.hpp"

#include "identifier.hpp"

namespace bkrl {
namespace json {

using cref = json11::Json const&;

template <typename T> optional<string_ref> optional_string(T) = delete;
template <typename T> string_ref default_string(T, string_ref) = delete;
template <typename T> string_ref require_string(T) = delete;
template <typename T, typename U> optional<T> optional_int(U) = delete;
template <typename T, typename U> T default_int(U, T) = delete;
template <typename T, typename U> T require_int(U) = delete;
template <typename T, typename U> optional<T> optional_float(U) = delete;
template <typename T, typename U> T default_float(U, T) = delete;
template <typename T, typename U> T require_float(U) = delete;
template <typename T, typename U> T require_float(U, T, T) = delete;
template <typename T> cref require_object(T) = delete;
template <typename T> cref require_array(T, size_t, size_t) = delete;
template <typename T> bool has_field(T, string_ref) = delete;
template <typename T> bool has_field(T, size_t) = delete;

//==============================================================================
//!
//==============================================================================
inline optional<string_ref> optional_string(cref value) {
    return value.is_string()
        ? optional<string_ref> {value.string_value()}
        : optional<string_ref> {};
}

//==============================================================================
//!
//==============================================================================
inline string_ref default_string(cref value, string_ref const def) {
    auto const result = optional_string(value);
    return result ? *result : def;
}

//==============================================================================
//!
//==============================================================================
inline string_ref require_string(cref value) {
    auto const result = optional_string(value);
    if (!result) {
        BK_TODO_FAIL(); //throw expected string, got <x>
    }

    return *result;
}

//==============================================================================
//!
//==============================================================================
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
//!
//==============================================================================
template <typename T = int>
inline T default_int(cref value, T const def) {
    auto const result = optional_int<T>(value);
    return result ? *result : def;
}

//==============================================================================
//!
//==============================================================================
template <typename T = int>
inline T require_int(cref value) {
    auto const result = optional_int<T>(value);
    if (!result) {
        BK_TODO_FAIL(); //throw expected int, got <x>
    }

    return *result;
}

//==============================================================================
//!
//==============================================================================
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
//!
//==============================================================================
template <typename T = float>
inline T default_float(cref value, T const def) {
    auto const result = optional_float<T>(value);
    return result ? *result : def;
}

//==============================================================================
//!
//==============================================================================
template <typename T = float>
inline T require_float(cref value) {
    auto const result = optional_float<T>(value);
    if (!result) {
        BK_TODO_FAIL(); //throw expected float, got <x>
    }

    return *result;
}

//==============================================================================
//!
//==============================================================================
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
//!
//==============================================================================
inline cref require_object(cref json) {
    if (!json.is_object()) {
        BK_TODO_FAIL();
    }

    return json;
}

//==============================================================================
//!
//==============================================================================
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
//!
//==============================================================================
inline bool has_field(cref value, string_ref const field) {
    return value.is_object() && !value[field].is_null();
}

//==============================================================================
//!
//==============================================================================
inline bool has_field(cref value, size_t const i) {
    return value.is_array() && !value[i].is_null();
}

//==============================================================================
//==============================================================================
namespace common {
//------------------------------------------------------------------------------
//TODO libstdc++ doesn't support transparent comparators for containers yet.
#if BOOST_COMP_MSVC
using field_string = string_ref;
#else
using field_string = utf8string;
#endif
//------------------------------------------------------------------------------
extern field_string const field_filetype;
extern field_string const field_stringtype;
extern field_string const field_language;
extern field_string const field_definitions;
extern field_string const field_id;
extern field_string const field_name;
extern field_string const field_text;
extern field_string const field_sort;
extern field_string const field_mappings;
extern field_string const field_filename;
extern field_string const field_tile_size;
extern field_string const field_stack;
extern field_string const field_damage_min;
extern field_string const field_damage_max;
extern field_string const field_tile;
extern field_string const field_color;
extern field_string const field_items;
extern field_string const field_health;
extern field_string const field_substantive_seed;
extern field_string const field_trivial_seed;
extern field_string const field_window_size;
extern field_string const field_window_pos;
extern field_string const field_font;
//------------------------------------------------------------------------------
extern field_string const filetype_config;
extern field_string const filetype_locale;
extern field_string const filetype_item;
extern field_string const filetype_entity;
extern field_string const filetype_tilemap;
extern field_string const filetype_keymap;
extern field_string const filetype_messages;
//------------------------------------------------------------------------------
template <typename T> path_string         get_path_string(T)          = delete;
template <typename T> path_string         get_filename(T)             = delete;
template <typename T> string_ref          get_filetype(T)             = delete;
template <typename T> string_ref          get_filetype(T, string_ref) = delete;
template <typename T> optional<lang_id>   get_locale(T)               = delete;
template <typename T> optional<lang_id>   get_locale(T, string_ref)   = delete;
template <typename T> random::random_dist get_random(T)               = delete;

//==============================================================================
//!
//==============================================================================
path_string get_path_string(cref value);

//==============================================================================
//!
//==============================================================================
path_string get_filename(cref value);

//==============================================================================
//!
//==============================================================================
inline string_ref get_filetype(cref value) {
    return require_string(value[field_filetype]);
}

//==============================================================================
//!
//==============================================================================
inline string_ref get_filetype(cref value, string_ref const expected) {
    auto const result = get_filetype(value);
    if (result != expected) {
        BK_TODO_FAIL();
    }

    return result;
}

//==============================================================================
//!
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
//!
//==============================================================================
inline json11::Json from_file(path_string_ref const filename) {
    auto const data = read_file(filename);
    return from_memory(data);
}

//==============================================================================
//!
//==============================================================================
inline optional<lang_id> get_locale(cref value) {
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
//!
//==============================================================================
inline optional<lang_id> get_locale(cref value, string_ref const expected_type) {
    auto const type = require_string(value[field_stringtype]);
    if (type != expected_type) {
        return optional<lang_id> {};
    }

    return get_locale(value);
}

//==============================================================================
//!
//==============================================================================
random::random_dist get_random(cref json);

} //namespace common
}} //namespace bkrl::json
