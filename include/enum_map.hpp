//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Mapping between enum types and strings.
//##############################################################################
#pragma once

#include <iterator>
#include <vector>

#include "types.hpp"
#include "util.hpp"
#include "assert.hpp"
#include "algorithm.hpp"

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
        auto const result = (value == rhs.value);

        BK_ASSERT_DBG(!result || hash == rhs.hash);

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
        auto const it = lower_bound(
            string_to_value_
          , hash
          , [](value_type const& lhs, hash_t const rhs) {
                return lhs.hash < rhs;
            }
        );

        if ((it == std::cend(string_to_value_)) || (it->hash != hash)) {
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
    static bool check(bool const allow_sparse = false) {
        //check for duplicates => hash collision
        auto const it = adjacent_find(string_to_value_);

        BK_ASSERT_SAFE(it == std::cend(string_to_value_)); //hash collision

        //check for "sparse" enums
        for (size_t i = 0; !allow_sparse && i < value_to_string_.size(); ++i) {
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
