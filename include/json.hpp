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
extern utf8string const field_stringtype;
extern utf8string const field_language;
extern utf8string const field_definitions;
extern utf8string const field_id;
extern utf8string const field_name;
extern utf8string const field_text;
extern utf8string const field_sort;
extern utf8string const field_mappings;
extern utf8string const field_filename;
extern utf8string const field_tile_size;
//------------------------------------------------------------
extern string_ref const filetype_config;
extern string_ref const filetype_locale;
extern string_ref const filetype_item;
extern string_ref const filetype_entity;
extern string_ref const filetype_tilemap;
extern string_ref const filetype_keymap;
//------------------------------------------------------------
extern string_ref const stringtype_messages;
//------------------------------------------------------------

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

inline json11::Json from_file(path_string_ref const filename) {
    auto const data = read_file(filename);
    return from_memory(data);
}

struct locale {
    using cref = json::cref;

    explicit locale(cref value) {
        rule_file_type(value);
        rule_string_type(value);
        rule_language(value);
    }

    explicit locale(path_string_ref const filename)
      : locale {from_file(filename)}
    {
    }

    void rule_file_type(cref value) {
        get_filetype(value, filetype_locale);
    }

    void rule_string_type(cref value) {
        string_type = require_string(value[field_stringtype]);
    }

    void rule_language(cref value) {
        language = require_string(value[field_language]);
    }

    cref definitions(cref value) {
        return require_array(value[field_definitions]);
    }

    json11::Json root;

    string_ref string_type;
    string_ref language;
};

struct random {
    using cref   = json::cref;
    using dist_t = bkrl::random::random_dist;

    random(cref value) {
        require_array(value, 1);
        rule_type(value);
    }

    void rule_type(cref value) {
        static string_ref const type_constant_str {"constant"};
        static string_ref const type_uniform_str  {"uniform"};
        static string_ref const type_dice_str     {"dice"};
        static string_ref const type_normal_str   {"normal"};

        static hash_t const type_constant_hash = slash_hash32(type_constant_str);
        static hash_t const type_uniform_hash  = slash_hash32(type_uniform_str);
        static hash_t const type_dice_hash     = slash_hash32(type_dice_str);
        static hash_t const type_normal_hash   = slash_hash32(type_normal_str);

        auto const type_str  = require_string(value[0]);
        auto const type_hash = slash_hash32(type_str);

        if (type_hash == type_constant_hash) {
            rule_constant(value);
        } else if (type_hash == type_uniform_hash) {
            rule_uniform(value);
        } else if (type_hash == type_dice_hash) {
            rule_dice(value);
        } else if (type_hash == type_normal_hash) {
            rule_normal(value);
        } else {
            BK_TODO_FAIL();
        }
    }

    void rule_constant(cref value) {
        require_array(value, 2, 2);

        auto const n = require_int(value[1]);

        dist_.set_constant(n);
    }

    void rule_uniform(cref value) {
        require_array(value, 3, 3);

        auto const lo = require_int(value[1]);
        auto const hi = require_int(value[2]);

        if (lo > hi) {
            BK_TODO_FAIL();
        }

        dist_.set_uniform(lo, hi);
    }

    void rule_dice(cref value) {
        require_array(value, 3, 4);

        auto const count = require_int(value[1]);
        auto const sides = require_int(value[2]);
        auto const mod   = default_int(value[3], 0);

        if (count < 1) {
            BK_TODO_FAIL();
        }

        if (sides < 1) {
            BK_TODO_FAIL();
        }

        dist_.set_dice(count, sides, mod);
    }

    void rule_normal(cref value) {
        require_array(value, 3, 5);

        auto const mean  = require_float<double>(value[1]);
        auto const sigma = require_float<double>(value[2]);

        auto const min = default_int(value[3], std::numeric_limits<int>::min());
        auto const max = default_int(value[4], std::numeric_limits<int>::max());

        dist_.set_normal(mean, sigma, min, max);
    }

    operator dist_t() const {
        return dist_;
    }

    dist_t dist_;
};

} //namespace common

}} //namespace bkrl::json
