#include "math.hpp"

using bkrl::geometry;

template struct bkrl::xy_type<int,   geometry::point>;
template struct bkrl::xy_type<float, geometry::point>;

template struct bkrl::xy_type<int,   geometry::vector>;
template struct bkrl::xy_type<float, geometry::vector>;

template struct bkrl::axis_aligned_rect<int>;
template struct bkrl::axis_aligned_rect<float>;
