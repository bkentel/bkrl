#pragma once

#include "types.hpp"

namespace bkrl {

class config {
public:
    uint32_t           substantive_seed;
    uint32_t           trivial_seed;
    utf8string         font_name;
    optional<uint32_t> window_w;
    optional<uint32_t> window_h;
    optional<int32_t>  window_x;
    optional<int32_t>  window_y;
};

config load_config(string_ref filename);

} //namespace bkrl
