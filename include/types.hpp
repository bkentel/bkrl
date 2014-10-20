//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Project-wide common types.
//##############################################################################
#pragma once

#include <cstdint>
#include <string>
#include <boost/utility/string_ref.hpp>
#include <boost/optional.hpp>
#include <boost/predef.h>

#include "math.hpp" //need math types

#if !BOOST_COMP_MSVC
namespace std {
template<class _Container>
	auto inline cbegin(const _Container& _Cont)
		-> decltype(std::begin(_Cont))
	{	// get beginning of sequence
	return (std::begin(_Cont));
	}

template<class _Container>
	auto inline cend(const _Container& _Cont)
		-> decltype(std::end(_Cont))
	{	// get end of sequence
	return (std::end(_Cont));
	}

template<class _Container>
	auto inline rbegin(_Container& _Cont) -> decltype(_Cont.rbegin())
	{	// get beginning of reversed sequence
	return (_Cont.rbegin());
	}

template<class _Container>
	auto inline rbegin(const _Container& _Cont) -> decltype(_Cont.rbegin())
	{	// get beginning of reversed sequence
	return (_Cont.rbegin());
	}

template<class _Container>
	auto inline rend(_Container& _Cont) -> decltype(_Cont.rend())
	{	// get end of reversed sequence
	return (_Cont.rend());
	}

template<class _Container>
	auto inline rend(const _Container& _Cont) -> decltype(_Cont.rend())
	{	// get end of reversed sequence
	return (_Cont.rend());
	}

template<class _Container>
	auto inline crbegin(const _Container& _Cont)
		-> decltype(std::rbegin(_Cont))
	{	// get beginning of reversed sequence
	return (std::rbegin(_Cont));
	}

template<class _Container>
	auto inline crend(const _Container& _Cont)
		-> decltype(std::rend(_Cont))
	{	// get end of reversed sequence
	return (std::rend(_Cont));
	}
}
#endif

namespace json11 { class Json; }
namespace bkrl { namespace json {
    using cref = json11::Json const&;
}}

namespace bkrl {

using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;

using std::uint64_t;
using std::uint32_t;
using std::uint16_t;
using std::uint8_t;

template <typename T>
using optional = boost::optional<T>;

using hash_t = uint32_t;

using utf8string = std::string;

//! UTF-8
using string_ref  = boost::string_ref;

#if BOOST_OS_WINDOWS
using path_char = wchar_t;
#define BK_PATH_LITERAL(str) L ## str
#else
using path_char = char;
#define BK_PATH_LITERAL(str) str
#endif

using path_string_ref = boost::basic_string_ref<path_char>;
using path_string     = std::basic_string<path_char>;

using codepoint_t = uint32_t;

using irect   = axis_aligned_rect<int>;
using ipoint2 = point2d<int>;
using ivec2   = vector2d<int>;

using fvec2   = vector2d<float>;
using fpoint2 = point2d<float>;
using frect2  = axis_aligned_rect<float>;

using rect   = axis_aligned_rect<float>;
using point2 = point2d<float>;
using vec2   = vector2d<float>;

//forward declarations
enum class command_type      : uint16_t;
enum class tile_type         : uint16_t;
enum class texture_type      : uint16_t;
enum class key               : uint16_t;
enum class scancode          : uint16_t;
enum class key_modifier_type : uint16_t;
enum class message_type      : uint16_t;

using texture_id = unsigned;

using grid_size   = signed;
using grid_index  = signed;
using grid_point  = point2d<grid_index>;
using grid_region = axis_aligned_rect<grid_index>;
using grid_data_value = uint32_t;

union grid_data {
    grid_data() : grid_data {0} {}
    explicit grid_data(grid_data_value const value) : value {value} {}

    void*           ptr;
    grid_data_value value;
};

using room_id = unsigned;

} //namespace bkrl
