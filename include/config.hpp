#pragma once

#include "types.hpp"
#include "util.hpp"

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
};

config load_config(json::cref data);

} //namespace bkrl
