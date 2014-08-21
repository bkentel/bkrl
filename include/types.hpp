#pragma once

#include <cstdint>
#include <boost/utility/string_ref.hpp>

namespace bkrl {

using std::uint64_t;
using std::uint32_t;
using std::uint16_t;
using std::uint8_t;

using hash_t = ::uint32_t;
using string_ref = ::boost::string_ref;

using utf8string = std::string;

enum class command_type : uint16_t;
enum class tile_type    : uint16_t;
enum class texture_type : uint16_t;

} //namespace bkrl
