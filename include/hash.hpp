#pragma once

#include <algorithm>
#include <iterator>

#include "string.hpp"
#include "integers.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////
//==============================================================================
//! 64bit hashing algorithm.
//==============================================================================
uint64_t slash_hash64(char const* s, size_t len);

//==============================================================================
//! 32bit hashing algorithm.
//==============================================================================
uint32_t slash_hash32(char const* s, size_t len);

inline uint64_t slash_hash64(string_ref const ref) {
    return slash_hash64(ref.data(), ref.length());
}

inline uint32_t slash_hash32(string_ref const ref) {
    return slash_hash32(ref.data(), ref.length());
}

//==============================================================================
//! Provides a mapping from a hash_t -> T.
//==============================================================================
template <typename T>
T from_hash(hash_t hash);

//==============================================================================
//! Provides a mapping from a string -> T.
//==============================================================================
template <typename T>
inline T from_string(string_ref const str) {
    return from_hash<T>(slash_hash32(str));
}

//==============================================================================
//! A helper to map from a string to a T.
//==============================================================================
template <typename T>
struct string_ref_mapping {
    static_assert(std::is_pod<T>::value, "Only safe for PODs for now.");

    using value_t = T;

    inline string_ref_mapping(string_ref const str, value_t const v) noexcept
      : string {str}
      , hash   {slash_hash32(str)}
      , value  {v}
    {
    }

    inline operator hash_t()  const noexcept { return hash;   }
    inline operator value_t() const noexcept { return value;  }

    string_ref string;
    hash_t     hash;
    value_t    value;
};

//==============================================================================
//! Return the value matchin key in [beg, end), otherwise return fallback.
//==============================================================================
template <typename T>
inline T find_mapping(
    string_ref_mapping<T> const* const beg
  , string_ref_mapping<T> const* const end
  , hash_t                       const key
  , T                            const fallback
) {
    auto const it = std::find_if(beg, end, [key](hash_t const hash) {
        return key == hash;
    });

    return (it != end) ? it->value : fallback;
}

//==============================================================================
//! Return the value matching key int values from [0, N), otherwise return fallback.
//==============================================================================
template <typename T, size_t N>
inline T find_mapping(
    string_ref_mapping<T> const (&values)[N]
  , hash_t const key
  , T      const fallback
) {
    return find_mapping(std::cbegin(values), std::cend(values), key, fallback);
}

////////////////////////////////////////////////////////////////////////////////
} //namespce bkrl
////////////////////////////////////////////////////////////////////////////////
