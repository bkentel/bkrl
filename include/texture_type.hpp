#pragma once

#include "json11/json11.hpp"
#include "enum_map.hpp"

namespace bkrl {

using texture_id = unsigned;
using texture_index = unsigned;

//changes here must be reflected in the init function.
enum class texture_type : unsigned {
    invalid
  , floor
  , wall_none
  , wall_n, wall_s, wall_e, wall_w
  , wall_ns, wall_ew, wall_se, wall_sw, wall_ne, wall_nw
  , wall_nse, wall_nsw, wall_sew, wall_new
  , wall_nsew

  , enum_size //must be last
};

extern template struct enum_map<texture_type>;

template <typename Enum, typename Value>
struct mapping_info {
    static_assert(std::is_enum<Enum>::value, "");

    static bool less_enum_value(mapping_info const& a, mapping_info const& b) {
        return a.enum_value < b.enum_value;
    }

    static bool less_hash(mapping_info const& a, mapping_info const& b) {
        return a.hash < b.hash;
    }

    static bool equal_hash(mapping_info const& a, mapping_info const& b) {
        return a.hash == b.hash;
    }

    mapping_info(Enum enum_value, boost::string_ref string)
      : string {string}
      , hash {slash_hash32(string.data(), string.length())}
      , enum_value {enum_value}
    {
    }

    boost::string_ref string     {""};
    size_t            hash       {};
    Enum              enum_value {};
    Value             value      {};
};

class texture_map {
public:
    explicit texture_map(std::string const& source);
    ~texture_map();

    texture_id operator[](texture_type type) const;
private:
    struct impl_t;
    std::unique_ptr<impl_t> impl_;
};

} //namespace bkrl
