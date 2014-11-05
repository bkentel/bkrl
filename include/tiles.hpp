//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Tile types.
//##############################################################################
#pragma once

#include "integers.hpp"
#include "hash.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////

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

  , enum_size
};

//==============================================================================
//! Visual variations for tile_types.
//==============================================================================
enum class texture_type : uint16_t {
    invalid

  , floor

  , wall_none
  , wall_n, wall_s, wall_e, wall_w
  , wall_ns, wall_ew, wall_se, wall_sw, wall_ne, wall_nw
  , wall_nse, wall_nsw, wall_sew, wall_new
  , wall_nsew

  , door_closed
  , door_opened

  , corridor

  , stair_up
  , stair_down

  , enum_size
};

extern template texture_type from_hash(hash_t hash);
extern template tile_type    from_hash(hash_t hash);

////////////////////////////////////////////////////////////////////////////////
} //namespace bkrl
////////////////////////////////////////////////////////////////////////////////
