#pragma once

#include <string>

#include <boost/predef.h>
#include <boost/utility/string_ref.hpp>

#include "integers.hpp"

namespace bkrl {

//------------------------------------------------------------------------------
//! A synonym for std::string; all strings are internally utf8
//------------------------------------------------------------------------------
using utf8string = std::string;

//------------------------------------------------------------------------------
//! A string observer; utf8
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

} //namespce bkrl
