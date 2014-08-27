//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Project-wide common types.
//##############################################################################
#pragma once

#include <cstdint>
#include <boost/utility/string_ref.hpp>

namespace bkrl {

using std::uint64_t;
using std::uint32_t;
using std::uint16_t;
using std::uint8_t;

using hash_t = uint32_t;

using utf8string = std::string;

//! UTF-8
using string_ref = ::boost::string_ref;

//! UTF-32
using codepoint_t = uint32_t;

//forward declarations
enum class command_type : uint16_t;
enum class tile_type    : uint16_t;
enum class texture_type : uint16_t;

} //namespace bkrl
