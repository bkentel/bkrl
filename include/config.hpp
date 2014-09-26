#pragma once

#include "types.hpp"

namespace bkrl {

struct config {
    uint32_t   substantive_seed;
    uint32_t   trivial_seed;
    utf8string font_name;
    uint32_t   window_w;
    uint32_t   window_h;
    int32_t    window_x;
    int32_t    window_y;
};

config load_config(string_ref filename);

} //namespace bkrl
