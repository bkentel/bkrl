//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Math and geometry.
//##############################################################################
#pragma once

#include "assert.hpp"

namespace bkrl {

//==============================================================================
//! 2d point type
//==============================================================================
template <typename T>
struct point2d {
    T x, y;
};

template <typename T>
inline bool operator==(point2d<T> const p, point2d<T> const q) noexcept {
    return (p.x == q.x) && (p.y == q.y);
}

template <typename T>
inline bool operator!=(point2d<T> const p, point2d<T> const q) noexcept {
    return !(p == q);
}

template <typename T, typename R>
inline point2d<T> translate(point2d<T> const p, R const dx, R const dy) {
    return point2d<T> {
        static_cast<T>(p.x + dx)
      , static_cast<T>(p.y + dy)
    };
}

//==============================================================================
//! 2d vector type
//==============================================================================
template <typename T = signed>
struct vector2d {
    T x, y;
};

template <typename T>
inline point2d<T> operator+(point2d<T> const p, vector2d<T> const v) {
    return point2d<T> {p.x + v.x, p.y + v.y};
}

template <
    typename T
  , typename U = std::make_signed_t<T>
>
inline point2d<U> operator-(point2d<T> const p, vector2d<T> const v) {
    return point2d<U> {
        static_cast<U>(p.x) - static_cast<U>(v.x)
      , static_cast<U>(p.y) - static_cast<U>(v.y)
    };
}

template <
    typename T
  , typename U = std::make_signed_t<T>
>
inline vector2d<U> operator-(point2d<T> const p, point2d<T> const q) {
    return vector2d<U> {
        static_cast<U>(p.x) - static_cast<U>(q.x)
      , static_cast<U>(p.y) - static_cast<U>(q.y)
    };
}

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
    T width()  const { return right - left; }
    T height() const { return bottom - top; }

    explicit operator bool() const {
        return left <= right && top <= bottom;
    }

    T area() const {
        BK_PRECONDITION(!!*this);
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

    bool contains(T const x, T const y) const {
        return (x >= left && x < right)
            && (y >= top  && y < bottom);
    }

    bool contains(point2d<T> const p) const {
        return contains(p.x, p.y);
    }

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

template <
    typename T
  , typename P = axis_aligned_rect<T>
  , typename R = std::pair<P, P>
>
inline R split_y(P const& rect, T const split) {
    BK_PRECONDITION(!!rect);
    BK_PRECONDITION(split >= rect.left);
    BK_PRECONDITION(split <= rect.right);

    auto const l0 = rect.left;
    auto const t0 = rect.top;
    auto const r0 = split;
    auto const b0 = rect.bottom;

    auto const l1 = r0;
    auto const t1 = rect.top;
    auto const r1 = rect.right;
    auto const b1 = rect.bottom;

    return std::make_pair(
        P {l0, t0, r0, b0}
      , P {l1, t1, r1, b1}
    );
}

template <
    typename T
  , typename P = axis_aligned_rect<T>
  , typename R = std::pair<P, P>
>
inline R split_x(P const& rect, T const split) {
    BK_PRECONDITION(!!rect);
    BK_PRECONDITION(split >= rect.top);
    BK_PRECONDITION(split <= rect.bottom);

    auto const l0 = rect.left;
    auto const t0 = rect.top;
    auto const r0 = rect.right;
    auto const b0 = split;

    auto const l1 = rect.left;
    auto const t1 = b0;
    auto const r1 = rect.right;
    auto const b1 = rect.bottom;

    return std::make_pair(
        P {l0, t0, r0, b0}
      , P {l1, t1, r1, b1}
    );
}

//==============================================================================
//! linearize a 2d value to a 1d value.
//==============================================================================
inline size_t linearize(size_t const w, size_t const h, size_t const x, size_t const y) {
    BK_PRECONDITION(x < w);
    BK_PRECONDITION(y < h);

    return y*w + x;
}

} //namespace bkrl
