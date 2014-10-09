//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Game commands.
//##############################################################################
#pragma once
#include "enum_map.hpp"

namespace bkrl {

//==============================================================================
//! Game command types.
//==============================================================================
enum class command_type : uint16_t {
    invalid

  , cancel
  , accept

  , here
  , north
  , south
  , east
  , west
  , north_west
  , north_east
  , south_west
  , south_east

  , up
  , down

  , zoom_in
  , zoom_out
  , zoom_reset

  , scroll_n
  , scroll_s
  , scroll_e
  , scroll_w

  , open
  , close
  , get

  , enum_size //!< last
};

extern template class enum_map<command_type>;

} //namespace bkrl
