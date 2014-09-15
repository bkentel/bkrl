//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Various utilities.
//##############################################################################
#pragma once
#include <string>
#include "types.hpp"

#define BK_NO_COPY(class_name) \
class_name(class_name const&) = delete; \
class_name& operator=(class_name const&) = delete

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

std::string read_file(string_ref filename);

inline std::string read_file(std::string const& filename) {
    return read_file(string_ref{filename});
}

template <typename T, size_t N>
inline size_t array_len(T const (&)[N]) {
    return N;
}

template <typename T, size_t N>
inline size_t string_len(T const (&)[N]) {
    return N - 1;
}

template <size_t Bytes> struct unsigned_int {
    static_assert(Bytes >= 1, "");
    static_assert(Bytes <= 9, "");

    using type =
    std::conditional_t<
        Bytes <= 1, uint8_t, std::conditional_t<
            Bytes <= 2, uint16_t, std::conditional_t<
                Bytes <= 4, uint32_t, std::conditional_t<
                    Bytes <= 8, uint64_t, std::false_type
                >
            >
        >
    >;
};

template <size_t Bytes>
using unsigned_int_t = typename unsigned_int<Bytes>::type;

} //namespace bkrl
