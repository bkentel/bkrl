#pragma once

#include <json11/json11.hpp>
#include "assert.hpp"
#include "types.hpp"
#include "util.hpp"

namespace bkrl {
namespace json {

using cref = json11::Json const&;

//==============================================================================
//==============================================================================
inline optional<string_ref> optional_string(cref value) {
    return value.is_string()
        ? optional<string_ref> {value.string_value()}
        : optional<string_ref> {boost::none};
}

//==============================================================================
//==============================================================================
inline string_ref default_string(cref value, string_ref const def) {
    auto const result = optional_string(value);
    return result ? *result : def;
}

//==============================================================================
//==============================================================================
inline string_ref require_string(cref value) {
    auto const result = optional_string(value);
    if (!result) {
        BK_TODO_FAIL();
    }

    return *result;
}

//==============================================================================
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
//==============================================================================
template <typename T = int>
inline T default_int(cref value, T const def) {
    auto const result = optional_int<T>(value);
    return result ? *result : def;
}

//==============================================================================
//==============================================================================
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
template <typename T = float>
inline T default_float(cref value, T const def) {
    auto const result = optional_float<T>(value);
    return result ? *result : def;
}

//==============================================================================
//==============================================================================
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
inline cref require_object(cref json) {
    if (!json.is_object()) {
        BK_TODO_FAIL();
    }

    return json;
}

//==============================================================================
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
//==============================================================================

inline bool has_field(cref value, utf8string const& field) {
    return value.is_object() && !value[field].is_null();
}

//==============================================================================
//==============================================================================
inline bool has_field(cref value, size_t const i) {
    return value.is_array() && !value[i].is_null();
}

//==============================================================================
//==============================================================================
//template <typename T = float>
//inline T require_float(cref value) {
//    static_assert(std::is_floating_point<T>::value, "");
//
//    using limits = std::numeric_limits<T>;
//
//    static auto const min = static_cast<double>(limits::min());
//    static auto const max = static_cast<double>(limits::max());
//
//    if (!value.is_number()) {
//        BK_TODO_FAIL();
//    }
//
//    auto const result = value.number_value();
//
//    if (result < min) {
//        BK_TODO_FAIL();
//    } else if (result > max) {
//        BK_TODO_FAIL();
//    }
//
//    return static_cast<T>(result);
//}

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

template <typename T = float>
inline T require_float(cref value, utf8string const& field) {
    return require_float<T>(value[field]);
}

template <typename T = float>
inline T require_float(cref value, utf8string const& field, T const min, T const max) {
    return require_float<T>(value[field], min, max);
}

//==============================================================================
//==============================================================================

//==============================================================================
//==============================================================================
struct hashed_string {
    hash_t     hash;
    string_ref value;
};

inline hashed_string get_hashed_string(json::cref value) {
    auto const string = require_string(value);
    return {bkrl::slash_hash32(string), string};
}

inline hashed_string get_hashed_string(json::cref value, utf8string const& field) {
    require_object(value);
    return get_hashed_string(value[field]);
}

//==============================================================================
//==============================================================================
namespace common {

extern utf8string const field_filetype;

inline string_ref get_filetype(cref value) {
    require_object(value);
    return require_string(value[field_filetype]);
}

inline string_ref get_filetype(cref value, string_ref const expected) {
    auto const result = get_filetype(value);
    if (result != expected) {
        BK_TODO_FAIL();
    }

    return result;
}

inline json11::Json from_memory(utf8string const& data) {
    std::string error;
    auto const json = json11::Json::parse(data, error);   

    if (!error.empty()) {
        BK_TODO_FAIL();
    }

    return json;
}

inline json11::Json from_file(string_ref filename) {
    auto const data = read_file(filename);
    return from_memory(data);
}

} //namespace common

}} //namespace bkrl::json
