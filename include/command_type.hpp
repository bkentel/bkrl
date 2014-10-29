//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Game commands.
//##############################################################################
#pragma once
#include "hash.hpp"
#include "integers.hpp"

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
  , drop
  , inventory
  , wield_wear

  , enum_size //!< last
};

extern template command_type from_hash(hash_t hash);

} //namespace bkrl
