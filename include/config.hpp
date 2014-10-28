#pragma once

#include "types.hpp"
#include "util.hpp"

#include "identifier.hpp"

namespace bkrl {

class config {
public:
    uint32_t           substantive_seed;
    uint32_t           trivial_seed;
    path_string        font_name;
    optional<uint32_t> window_w;
    optional<uint32_t> window_h;
    optional<int32_t>  window_x;
    optional<int32_t>  window_y;
    lang_id            language;

    int                scroll_delta  = 4;
    float              zoom_factor   = 0.1f;
    float              zoom_min      = 0.1f;
    float              zoom_max      = 5.0f;
    float              auto_scroll_w = 0.1f;
};

config load_config(json::cref data);

} //namespace bkrl
