#pragma once

#include <string>

#include "types.hpp"

namespace bkrl {

template <typename Enum>
inline std::underlying_type_t<Enum> enum_value(Enum const e) {
    static_assert(std::is_enum<Enum>::value, "");
    return static_cast<std::underlying_type_t<Enum>>(e);
}

uint64_t slash_hash64(char const* s, size_t len = 0);
uint32_t slash_hash32(char const* s, size_t len = 0);

inline uint64_t slash_hash64(string_ref const ref) {
    return slash_hash64(ref.data(), ref.length());
}

inline uint32_t slash_hash32(string_ref const ref) {
    return slash_hash32(ref.data(), ref.length());
}

std::string read_file(std::string const& filename);

template <typename T, size_t N>
inline size_t array_len(T const (&)[N]) {
    return N;
}

template <typename T, size_t N>
inline size_t string_len(T const (&)[N]) {
    return N - 1;
}


} //namespace bkrl
