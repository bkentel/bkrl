//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Tile types.
//##############################################################################
#pragma once

namespace bkrl {

//==============================================================================
//! Basic tile classifications.
//==============================================================================
enum class tile_type : uint16_t {
    invalid
  , empty
  , floor
  , wall
  , door
  , stair

  , enum_size //last
};

} //namespace bkrl
