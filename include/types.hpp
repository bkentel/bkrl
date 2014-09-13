//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Project-wide common types.
//##############################################################################
#pragma once

#include <cstdint>
#include <boost/utility/string_ref.hpp>

#include "math.hpp" //need math types

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
enum class key          : uint16_t;

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
