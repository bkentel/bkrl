//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Project-wide common types.
//##############################################################################
#pragma once

#include "integers.hpp"
#include "math.hpp"

#pragma message( "!!types is deprecated" )

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////

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