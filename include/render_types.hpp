#pragma once

#include "integers.hpp"
#include "math.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////
class renderer;
class texture;

using rgb8  = color3<uint8_t>;
using argb8 = color4<uint8_t>;

using tex_coord_i = int16_t;
using tex_point_i = point2d<tex_coord_i>;
using tex_rect_i  = axis_aligned_rect<tex_coord_i>;

////////////////////////////////////////////////////////////////////////////////
} //namespace bkrl
////////////////////////////////////////////////////////////////////////////////
