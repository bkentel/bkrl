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

  , enum_size //must be last
};

extern template class enum_map<texture_type>;

//==============================================================================
//! texture_map
//! map texture_type -> texture_id
//==============================================================================
class texture_map {
public:
    //--------------------------------------------------------------------------
    //! @param source The json describing the mappings.
    //--------------------------------------------------------------------------
    explicit texture_map(utf8string const& source);

    //needed for pimpl
    ~texture_map();

    texture_id operator[](texture_type type) const;
private:
    struct impl_t;
    std::unique_ptr<impl_t> impl_;
};

} //namespace bkrl
