//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Procedural generation.
//##############################################################################
#pragma once
#include "types.hpp"
#include "grid.hpp"

namespace bkrl {

namespace random { class generator; }

namespace generate {

//==============================================================================
//! simple_room
//==============================================================================
class simple_room {
public:
    room generate(random::generator& gen, grid_region bounds, room_id id);
};

//==============================================================================
//! simple_room
//==============================================================================
class circle_room {
public:
    room generate(random::generator& gen, grid_region bounds, room_id id);
};

} //namespace generate
} //namespace bkrl
