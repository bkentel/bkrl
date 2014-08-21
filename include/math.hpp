#pragma once

#include "assert.hpp"

namespace bkrl {

template <typename T>
struct point2d {
    T x, y;
};

template <typename T, typename R>
point2d<T> translate(point2d<T> const p, R const dx, R const dy) {
    return point2d<T> {
        static_cast<T>(p.x + dx)
      , static_cast<T>(p.y + dy)
    };
}

////

////
template <typename T>
struct axis_aligned_rect {
    T width()  const { return right - left; }
    T height() const { return bottom - top; }

    T left, top, right, bottom;
};

template <typename T, typename R>
axis_aligned_rect<T> translate(axis_aligned_rect<T> const r, R const dx, R const dy) {
    return axis_aligned_rect<T> {
        static_cast<T>(r.left + dx)
      , static_cast<T>(r.top + dy)
      , static_cast<T>(r.right + dx)
      , static_cast<T>(r.bottom + dy)
    };
}

template <typename T>
inline bool operator==(axis_aligned_rect<T> const a, axis_aligned_rect<T> const b) {
    return (a.left   == b.left)
        && (a.top    == b.top)
        && (a.right  == b.right)
        && (a.bottom == b.bottom)
    ;
}

template <typename T>
inline bool operator!=(axis_aligned_rect<T> const a, axis_aligned_rect<T> const b) {
    return !(a == b);
}

inline size_t linearize(size_t const w, size_t const h, size_t const x, size_t const y) {
    BK_PRECONDITION(x < w);
    BK_PRECONDITION(y < h);

    return y*w + x;
}

} //namespace bkrl
