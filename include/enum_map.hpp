#pragma once

#include <vector>
#include "types.hpp"
#include "algorithm.hpp"
#include "util.hpp"
#include "assert.hpp"

namespace bkrl {
//==============================================================================
// enum_string
//==============================================================================
template <typename Enum>
struct enum_string {
    static_assert(std::is_enum<Enum>::value, "");

    enum_string() = default;
    enum_string(enum_string const&) = default;
    enum_string& operator=(enum_string const&) = default;

    enum_string(string_ref const string, Enum const value)
      : string {string}
      , hash {slash_hash32(string.data(), string.length())}
      , value {value}
    {
    }

    static bool less_hash(enum_string const& a, enum_string const& b) {
        return a.hash < b.hash;
    }

    static bool less_enum(enum_string const& a, enum_string const& b) {
        return a.value < b.value;
    }

    bool operator==(enum_string const& rhs) const {
        return value == rhs.value;
    }

    string_ref string {};
    hash_t     hash   {};
    Enum       value  {};
};

//==============================================================================
// enum_map
//==============================================================================
template <typename T>
struct enum_map {
    static_assert(std::is_enum<T>::value, "");

    using value_type = enum_string<T>;

    static value_type get(string_ref const string) {
        auto const hash = slash_hash32(string.data(), string.length());
        return get(hash);
    }

    static value_type get(hash_t const hash) {
        return lower_bound_or(string_to_value_, hash, [](value_type const& v, hash_t const h) {
            return v.hash < h;
        });
    }
    
    static value_type get(T const value) {
        auto const i = static_cast<size_t>(value);
        return value_to_string_[i];
    }

    static bool check() {
        auto const it = std::adjacent_find(
            std::cbegin(string_to_value_), std::cend(string_to_value_)
        );

        BK_ASSERT(it == std::cend(string_to_value_)); //hash collision

        //sparse enum
        for (size_t i = 0; i < value_to_string_.size(); ++i) {
            BK_ASSERT(i == static_cast<size_t>(value_to_string_[i].value));
        }

        return true; //TODO
    }

    static const std::vector<value_type> string_to_value_;
    static const std::vector<value_type> value_to_string_;
    static const bool checked_; //TODO could remove?
};

} //namespace bkrl
