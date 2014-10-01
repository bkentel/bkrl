//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Tile types.
//##############################################################################
#pragma once

#include "enum_map.hpp"

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
  , corridor

  , enum_size //last
};

extern template class enum_map<tile_type>;

} //namespace bkrl
