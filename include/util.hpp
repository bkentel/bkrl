//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Various utilities.
//##############################################################################
#pragma once

#include <memory>
#include <type_traits>

#include "string.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline std::remove_reference_t<T> const& as_const(T&& value) noexcept {
    return value;
}
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename Tag>
struct tagged_type {
    using type = T;

    static_assert(std::is_fundamental<T>::value, "");

    tagged_type() noexcept
      : tagged_type {T {}}
    {
    }

    explicit tagged_type(T const value) noexcept
      : value {value}
    {
    }

    explicit operator T() const noexcept { return value; }

    bool operator==(tagged_type const rhs) const noexcept { return value == rhs.value; }
    bool operator!=(tagged_type const rhs) const noexcept { return value != rhs.value; }
    bool operator<=(tagged_type const rhs) const noexcept { return value <= rhs.value; }
    bool operator>=(tagged_type const rhs) const noexcept { return value >= rhs.value; }
    bool operator<(tagged_type const rhs)  const noexcept { return value <  rhs.value; }
    bool operator>(tagged_type const rhs)  const noexcept { return value >  rhs.value; }

    T value;
};

template <typename T, typename Tag>
inline T value_of(tagged_type<T, Tag> const tagged) noexcept {
    return static_cast<T>(tagged);
}

////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct opaque_handle {
    opaque_handle() : value {0} {}

    opaque_handle(std::nullptr_t)
      : value {0}
    {
    }

    template <typename U, typename D>
    opaque_handle(std::unique_ptr<U, D> const& uptr) noexcept
      : opaque_handle {uptr.get()}
    {
    }

    template <typename U, typename D>
    opaque_handle(std::unique_ptr<U, D>& uptr) noexcept
      : opaque_handle {uptr.get()}
    {
    }

    template <typename U>
    opaque_handle(U* ptr) noexcept
      : value {reinterpret_cast<std::intptr_t>(ptr)}
    {
    }

    template <typename R>
    R as() const noexcept {
        static_assert(
            std::is_pointer<T>::value
         || sizeof(R) == sizeof(std::intptr_t)
          , ""
        );

        return reinterpret_cast<R>(value);
    }

    std::intptr_t value;
};

////////////////////////////////////////////////////////////////////////////////
template <typename Enum>
inline std::underlying_type_t<Enum> enum_value(Enum const e) {
    static_assert(std::is_enum<Enum>::value, "");
    return static_cast<std::underlying_type_t<Enum>>(e);
}

////////////////////////////////////////////////////////////////////////////////

utf8string read_file(path_string_ref filename);

inline utf8string read_file(path_string const& filename) {
    return read_file(path_string_ref {filename});
}
////////////////////////////////////////////////////////////////////////////////

template <typename T, size_t N>
inline size_t array_len(T const (&)[N]) {
    return N;
}

template <typename T, size_t N>
inline size_t string_len(T const (&)[N]) {
    return N - 1;
}
////////////////////////////////////////////////////////////////////////////////

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

    static_assert(sizeof(type) >= Bytes, "");
};

template <size_t Bytes>
using unsigned_int_t = typename unsigned_int<Bytes>::type;

#define BK_MAKE_LANG_CODE3(a, b, c) bkrl::lang_id { \
  static_cast<uint32_t>((a & 0xFF) << 16) \
| static_cast<uint32_t>((b & 0xFF) <<  8) \
| static_cast<uint32_t>((c & 0xFF) <<  0) \
}

#define BK_MAKE_LANG_CODE2(a, b) BK_MAKE_LANG_CODE3(0, a, b)

inline static void assign(utf8string& out, string_ref const in) {
    out.clear();
    out.assign(in.data(), in.size());
}

////////////////////////////////////////////////////////////////////////////////
} //namespace bkrl
////////////////////////////////////////////////////////////////////////////////
