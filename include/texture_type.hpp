//##############################################################################
//! @author Brandon Kentel
//!
//! Texture types.
//##############################################################################
#pragma once

#include "enum_map.hpp"

namespace bkrl {

//! An index into a tilemap.
using texture_id = unsigned;

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

  , enum_size //must be last
};

//TODO neccessary?
extern template class enum_map<texture_type>;

//==============================================================================
// mapping_info
//
// helper class for mapping between an enum and a string, and string and enum.
//==============================================================================
//template <typename Enum, typename Value>
//struct mapping_info {
//    static_assert(std::is_enum<Enum>::value, "");
//
//    static bool less_enum_value(mapping_info const& a, mapping_info const& b) {
//        return a.enum_value < b.enum_value;
//    }
//
//    static bool less_hash(mapping_info const& a, mapping_info const& b) {
//        return a.hash < b.hash;
//    }
//
//    static bool equal_hash(mapping_info const& a, mapping_info const& b) {
//        return a.hash == b.hash;
//    }
//
//    mapping_info(Enum enum_value, boost::string_ref string)
//      : string {string}
//      , hash {slash_hash32(string.data(), string.length())}
//      , enum_value {enum_value}
//    {
//    }
//
//    boost::string_ref string     {""};
//    size_t            hash       {};
//    Enum              enum_value {};
//    Value             value      {};
//};

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
