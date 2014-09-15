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

  , zoom_in
  , zoom_out

  , scroll_n
  , scroll_s
  , scroll_e
  , scroll_w

  , open
  , close

  , enum_size //!< last
};

//TODO neccessary?
extern template class enum_map<command_type>;

//==============================================================================
//! command_map
//! map texture_type -> texture_id
//==============================================================================
//class command_map {
//public:
//    //--------------------------------------------------------------------------
//    //! @param source The json describing the mappings.
//    //--------------------------------------------------------------------------
//    explicit texture_map(utf8string const& source);
//
//    //needed for pimpl
//    ~texture_map();
//
//    texture_id operator[](texture_type type) const;
//private:
//    struct impl_t;
//    std::unique_ptr<impl_t> impl_;
//};


} //namespace bkrl
