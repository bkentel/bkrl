//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Mapping between enum types and strings.
//##############################################################################
#pragma once

#include <vector>
#include "types.hpp"
#include "algorithm.hpp"
#include "util.hpp"
#include "assert.hpp"

namespace bkrl {
//==============================================================================
//! Value class used to represent a mapping from enum <-> string.
//==============================================================================
template <typename Enum>
struct enum_string {
    static_assert(std::is_enum<Enum>::value, "");

    //--------------------------------------------------------------------------
    //! comparison by hash predicate.
    //--------------------------------------------------------------------------
    static bool less_hash(enum_string const& a, enum_string const& b) {
        return a.hash < b.hash;
    }

    //--------------------------------------------------------------------------
    //! comparison by enum value predicate.
    //--------------------------------------------------------------------------
    static bool less_enum(enum_string const& a, enum_string const& b) {
        return a.value < b.value;
    }

    enum_string() = default;
    enum_string(enum_string const&) = default;
    enum_string& operator=(enum_string const&) = default;

    enum_string(string_ref const string, Enum const value)
      : string {string}
      , hash {slash_hash32(string.data(), string.length())}
      , value {value}
    {
    }

    bool operator==(enum_string const& rhs) const {
        auto const result = value == rhs.value;

        BK_ASSERT(!result || hash == rhs.hash);

        return result;
    }

    string_ref string {}; //!< The stringified enum.
    hash_t     hash   {}; //!< The string's hash.
    Enum       value  {}; //!< The enum value.
};

//==============================================================================
//! statically creates a map between strings and enum values.
//!
//! @tparam T and enum type to be mapped.
//==============================================================================
template <typename T>
class enum_map {
public:
    static_assert(std::is_enum<T>::value, "");

    using underlying_type = std::underlying_type_t<T>;

    static_assert(static_cast<underlying_type>(T::invalid)   == 0, "");
    static_assert(static_cast<underlying_type>(T::enum_size) != 0, "");

    using value_type = enum_string<T>;

    //--------------------------------------------------------------------------
    //! string -> mapping.
    //--------------------------------------------------------------------------
    static value_type get(string_ref const string) {
        auto const hash = slash_hash32(string.data(), string.length());
        return get(hash);
    }

    //--------------------------------------------------------------------------
    //! string (hash) -> mapping.
    //--------------------------------------------------------------------------
    static value_type get(hash_t const hash) {
        auto const it = std::lower_bound(
            std::cbegin(string_to_value_)  
          , std::cend(string_to_value_)
          , hash
          , [](value_type const& lhs, hash_t const rhs) {
                return lhs.hash < rhs;
            }
        );

        if (it == std::cend(string_to_value_)) {
            return value_type {};
        } else if (it->hash != hash) {
            return value_type {};
        }

        return *it;
    }

    //--------------------------------------------------------------------------
    //! enum -> mapping.
    //--------------------------------------------------------------------------
    static value_type get(T const value) {
        auto const i = static_cast<size_t>(value);
        return value_to_string_[i];
    }

    //--------------------------------------------------------------------------
    //! sanity checks
    //--------------------------------------------------------------------------
    static bool check() {
        //check for duplicates => hash collision
        auto const it = std::adjacent_find(
            std::cbegin(string_to_value_), std::cend(string_to_value_)
        );

        BK_ASSERT(it == std::cend(string_to_value_)); //hash collision

        //check for "sparse" enums
        for (size_t i = 0; i < value_to_string_.size(); ++i) {
            //TODO
            if (i != static_cast<size_t>(value_to_string_[i].value)) {
                std::cout << "warning sparse enum\n";
            }
        }

        return true; //TODO
    }
private:
    static const std::vector<value_type> string_to_value_; //sorted by hash
    static const std::vector<value_type> value_to_string_; //sorted by enum value
    static const bool checked_; //TODO could remove?
};

} //namespace bkrl

//==============================================================================
//! convenience macro get a unique reference to a compile-time cstring.
//==============================================================================
#define BK_ENUMMAP_MAKE_STRING(ENUM, VALUE) \
[]() -> ::bkrl::string_ref { \
    static char const string[] {#VALUE}; \
    return {string, ::bkrl::string_len(string)}; \
}()

//==============================================================================
//! convenience macro to add a mapping,
//==============================================================================
#define BK_ENUMMAP_ADD_STRING(OUT, ENUM, VALUE) \
    OUT.emplace_back(BK_ENUMMAP_MAKE_STRING(ENUM, VALUE), ENUM::VALUE)
