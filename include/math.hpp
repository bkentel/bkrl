//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Math and geometry.
//##############################################################################
#pragma once

#include <cmath>
#include <utility>
#include <functional>

#include "assert.hpp"

namespace bkrl {
//==============================================================================
//! clamp the value @p n to the range [lo, hi]
//==============================================================================
template <typename T>
inline T
clamp(
    T const n
  , T const lo
  , T const hi
) noexcept {
    return (n < lo)
      ? lo
      : (n > hi)
        ? hi
        : n;
}

//==============================================================================
//! linearize a 2d value to a 1d value.
//==============================================================================
inline int next_nearest_power_of_2(int value) noexcept {
    int result = 1;

    while (result < value) {
        result <<= 1;
    }

    return result;
}


//==============================================================================
//! linearize a 2d value to a 1d value.
//==============================================================================
template <typename T>
inline T
linearize(
    T const w
  , T const h
  , T const x
  , T const y
) noexcept {
    BK_ASSERT_DBG(x < w);
    BK_ASSERT_DBG(y < h);

    return y*w + x;
}

//==============================================================================
enum class geometry {
    vector, point
};

//==============================================================================
template <typename T, geometry>
struct xy_type {
    T x, y;
};

template <typename T>
using point2d = xy_type<T, geometry::point>;

template <typename T>
using vector2d = xy_type<T, geometry::vector>;

//------------------------------------------------------------------------------
template <typename T, geometry G>
inline bool
operator==(
    xy_type<T, G> const p
  , xy_type<T, G> const q
) noexcept {
    static_assert(!std::is_floating_point<T>::value, "TODO");
    return (p.x == q.x) && (p.y == q.y);
}

//------------------------------------------------------------------------------
template <typename T, geometry G>
inline bool
operator!=(
    xy_type<T, G> const p
  , xy_type<T, G> const q
) noexcept {
    return !(p == q);
}

//------------------------------------------------------------------------------
template <typename T, geometry G>
inline xy_type<T, G>
operator+(
    xy_type<T, G> const p
  , xy_type<T, G> const q
) noexcept {
    return {p.x + q.x, p.y + q.y};
}

//------------------------------------------------------------------------------
template <typename T>
inline xy_type<T, geometry::point>
operator+(
    xy_type<T, geometry::point>  const p
  , xy_type<T, geometry::vector> const q
) noexcept {
    return {p.x + q.x, p.y + q.y};
}

//------------------------------------------------------------------------------
template <typename T>
inline xy_type<T, geometry::point>
operator+=(
    xy_type<T, geometry::point>&       p
  , xy_type<T, geometry::vector> const q
) noexcept {
    return (p = p + q);
}

//------------------------------------------------------------------------------
template <typename T>
inline xy_type<T, geometry::point>
operator-(
    xy_type<T, geometry::point>  const p
  , xy_type<T, geometry::vector> const q
) noexcept {
    return {p.x - q.x, p.y - q.y};
}

//------------------------------------------------------------------------------
template <typename T>
inline xy_type<T, geometry::vector>
operator-(
    xy_type<T, geometry::point> const p
  , xy_type<T, geometry::point> const q
) noexcept {
    return {p.x - q.x, p.y - q.y};
}

//------------------------------------------------------------------------------
template <typename T>
inline xy_type<T, geometry::vector>
operator-(
    xy_type<T, geometry::vector> const p
) noexcept {
    return {-p.x, p.y};
}

//------------------------------------------------------------------------------
template <typename T, geometry G>
inline bool
lexicographical_compare(
    xy_type<T, G> const a
  , xy_type<T, G> const b
) noexcept {
    return (a.x < b.x)
        ? true
        : (a.x == b.x)
            ? (a.y < b.y)
            : false;
}

//------------------------------------------------------------------------------
//template <typename T>
//inline auto position(
//    xy_type<T, geometry::point> const p
//) noexcept {
//    return p;
//}

//==============================================================================
//! range
//==============================================================================
template <
    typename T
  , typename Lo = std::greater_equal<>
  , typename Hi = std::less_equal<>
>
struct range {
    T size() const {
        return hi - lo; //TODO
    }

    explicit operator bool() const {
        return lo <= hi;
    }

    bool contains(T const n) const {
        return Lo {}(n, lo) && Hi {}(n, hi);
    }

    bool intersects(range const& other) const {
        return !(hi < other.lo || lo > other.hi);
    }

    T lo, hi;
};

template <
    typename T
  , typename Lo = std::greater_equal<>
  , typename Hi = std::less_equal<>
  , typename R  = bkrl::range<T, Lo, Hi>
>
bool operator<(R const& lhs, R const& rhs) {
    return lhs.hi < rhs.lo;
}

//==============================================================================
//! axis_aligned_rect @TODO
//==============================================================================
template <typename T>
struct axis_aligned_rect {
    T width()  const noexcept { return right - left; }
    T height() const noexcept { return bottom - top; }

    explicit operator bool() const noexcept {
        return left <= right && top <= bottom;
    }

    T area() const {
        BK_ASSERT_DBG(!!*this);
        return width() * height();
    }

    template <typename R = T>
    point2d<R> center() const {
        auto const half = R {2};
        return point2d<R> {
            (static_cast<R>(left + (right  - left)) / half)
          , (static_cast<R>(top  + (bottom - top )) / half)
        };
    }

    point2d<T> top_left() const noexcept {
        return {left, top};
    }

    T left, top, right, bottom;
};

//------------------------------------------------------------------------------
template <typename T>
inline bool
operator==(
    axis_aligned_rect<T> const a
  , axis_aligned_rect<T> const b
) noexcept {
    return (a.left   == b.left)
        && (a.top    == b.top)
        && (a.right  == b.right)
        && (a.bottom == b.bottom)
    ;
}

//------------------------------------------------------------------------------
template <typename T>
inline bool
operator!=(
    axis_aligned_rect<T> const a
  , axis_aligned_rect<T> const b
) noexcept {
    return !(a == b);
}

//------------------------------------------------------------------------------
template <typename T>
inline axis_aligned_rect<T>
make_rect_size(
    T const x
  , T const y
  , T const w
  , T const h
) noexcept {
    return {x, y, x + w, y + h};
}

//------------------------------------------------------------------------------
template <typename T>
inline axis_aligned_rect<T>
make_rect_bounds(
    T const left
  , T const top
  , T const right
  , T const bottom
) noexcept {
    return {left, top, right, bottom};
}

//------------------------------------------------------------------------------
template <
    typename Scalar
  , typename Rect   = axis_aligned_rect<Scalar>
  , typename Result = std::pair<Rect, Rect>
>
inline Result
split_y(
    Rect   const rect
  , Scalar const split
) noexcept {
    BK_ASSERT_DBG(!!rect);
    BK_ASSERT_DBG(split >= rect.left);
    BK_ASSERT_DBG(split <= rect.right);

    auto const l0 = rect.left;
    auto const t0 = rect.top;
    auto const r0 = split;
    auto const b0 = rect.bottom;

    auto const l1 = r0;
    auto const t1 = rect.top;
    auto const r1 = rect.right;
    auto const b1 = rect.bottom;

    return std::make_pair(
        make_rect_bounds(l0, t0, r0, b0)
      , make_rect_bounds(l1, t1, r1, b1)
    );
}

//------------------------------------------------------------------------------
template <
    typename Scalar
  , typename Rect   = axis_aligned_rect<Scalar>
  , typename Result = std::pair<Rect, Rect>
>
inline Result
split_x(
    Rect   const rect
  , Scalar const split
) noexcept {
    BK_ASSERT_DBG(!!rect);
    BK_ASSERT_DBG(split >= rect.top);
    BK_ASSERT_DBG(split <= rect.bottom);

    auto const l0 = rect.left;
    auto const t0 = rect.top;
    auto const r0 = rect.right;
    auto const b0 = split;

    auto const l1 = rect.left;
    auto const t1 = b0;
    auto const r1 = rect.right;
    auto const b1 = rect.bottom;

    return std::make_pair(
        make_rect_bounds(l0, t0, r0, b0)
      , make_rect_bounds(l1, t1, r1, b1)
    );
}

//------------------------------------------------------------------------------
template <typename T, geometry G, typename R>
inline xy_type<T, G>
translate(
    xy_type<T, G> const p
  , R             const dx
  , R             const dy
) noexcept {
    return {
        static_cast<T>(p.x + dx)
      , static_cast<T>(p.y + dy)
    };
}

//------------------------------------------------------------------------------
template <typename T, typename R>
inline axis_aligned_rect<T>
translate(
    axis_aligned_rect<T> const r
  , R                    const dx
  , R                    const dy
) noexcept {
    return make_rect_bounds(
        static_cast<T>(r.left   + dx)
      , static_cast<T>(r.top    + dy)
      , static_cast<T>(r.right  + dx)
      , static_cast<T>(r.bottom + dy)
    );
}

//------------------------------------------------------------------------------
template <typename T>
inline bool
intersects(
    point2d<T> const a
  , point2d<T> const b
) {
    return (a == b);
}

//------------------------------------------------------------------------------
template <typename Float = double, typename T>
inline bool
intersects(
    point2d<T>  const p
  , vector2d<T> const v
) {
    auto const slope = static_cast<Float>(v.x / v.y);
    auto const y = slope * p.x;

    return p.y == static_cast<T>(std::round(y));
}

//------------------------------------------------------------------------------
template <typename Float = double, typename T>
inline bool
intersects(
    vector2d<T> const v
  , point2d<T>  const p
) {
    return intersects<Float>(p, v);
}

//------------------------------------------------------------------------------
template <typename T>
inline bool
intersects(
    point2d<T>           const p
  , axis_aligned_rect<T> const r
) noexcept {
    return (p.x >= r.left)   &&
           (p.x <  r.right)  &&
           (p.y >= r.top)    &&
           (p.y <  r.bottom)
    ;
}

//------------------------------------------------------------------------------
template <typename T>
inline bool
intersects(
    axis_aligned_rect<T> const r
  , point2d<T>           const p
) noexcept {
    return intersects(p, r);
}

extern template struct xy_type<int,   geometry::point>;
extern template struct xy_type<float, geometry::point>;

extern template struct xy_type<int,   geometry::vector>;
extern template struct xy_type<float, geometry::vector>;

extern template struct axis_aligned_rect<int>;
extern template struct axis_aligned_rect<float>;

} //namespace bkrl
