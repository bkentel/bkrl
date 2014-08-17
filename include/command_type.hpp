//##############################################################################
//! @author Brandon Kentel
//!
//! Game commands.
//##############################################################################
#pragma once

namespace bkrl {

enum class command_type : uint16_t {
    none
  , cancel
  , accept
  , north
  , south
  , east
  , west
  , zoom_in
  , zoom_out
  , scroll_n
  , scroll_s
  , scroll_e
  , scroll_w

  , enum_size //!< last
};

} //namespace bkrl
