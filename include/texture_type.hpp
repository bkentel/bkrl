//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Texture types.
//##############################################################################
#pragma once
#include <memory>
#include "types.hpp"
#include "enum_map.hpp"

namespace bkrl {

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

  , enum_size //must be last
};

extern template class enum_map<texture_type>;

} //namespace bkrl
