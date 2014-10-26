//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Project-wide common types.
//##############################################################################
#pragma once

#include <string>
#include <boost/predef.h>
#include <boost/utility/string_ref.hpp>
#include <boost/optional.hpp>

#include "integers.hpp"
#include "math.hpp"

//TODO move this out of here to another file
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

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////
namespace json   { using cref = json11::Json const&; }
namespace random { class generator; }

using random_t = random::generator;

template <typename T>
using optional = boost::optional<T>;

using hash_t = uint32_t;

using utf8string = std::string;

//------------------------------------------------------------------------------
// A string observer (UTF-8)
//------------------------------------------------------------------------------
using string_ref = boost::string_ref;

//------------------------------------------------------------------------------
// Platform specific path string types.
//------------------------------------------------------------------------------
#if BOOST_OS_WINDOWS
using path_char = wchar_t;
#define BK_PATH_LITERAL(str) L ## str
#else
using path_char = char;
#define BK_PATH_LITERAL(str) str
#endif

using path_string_ref = boost::basic_string_ref<path_char>;
using path_string     = std::basic_string<path_char>;
//------------------------------------------------------------------------------

using codepoint_t = uint32_t;

//forward declarations
enum class command_type      : uint16_t;
enum class tile_type         : uint16_t;
enum class texture_type      : uint16_t;
enum class key               : uint16_t;
enum class scancode          : uint16_t;
enum class key_modifier_type : uint16_t;
enum class message_type      : uint16_t;

using texture_id = uint32_t;
using room_id    = uint32_t;

using grid_size   = int;
using grid_index  = int;
using grid_point  = point2d<grid_index>;
using grid_region = axis_aligned_rect<grid_index>;
using grid_data_value = uint32_t;

union grid_data {
    grid_data() : grid_data {0} {}
    explicit grid_data(grid_data_value const value) : value {value} {}

    void*           ptr;
    grid_data_value value;
};

////////////////////////////////////////////////////////////////////////////////
} //namespace bkrl
////////////////////////////////////////////////////////////////////////////////