#pragma once

#include "math.hpp"
#include "types.hpp"

namespace bkrl {

//==============================================================================
//==============================================================================
class entity {
public:
    using point_t = ipoint2;

    entity(point_t const pos)
      : pos_ (pos)
    {
    }

    entity()
      : entity {point_t{0, 0}}
    {
    }

    point_t position() const {
        return pos_;
    }

    void move_by(int const dx, int const dy) {
        pos_.x += dx;
        pos_.y += dy;
    }

    void move_by(ivec2 const v) {
        move_by(v.x, v.y);
    }

    void move_to(int const x, int const y) {
        pos_.x = x;
        pos_.y = y;
    }

    void move_to(point_t const p) {
        move_to(p.x, p.y);
    }
private:
    point_t pos_;
};

//==============================================================================
//==============================================================================
class player : public entity {
public:
    using entity::entity;
private:
};

inline entity::point_t position(entity const& e) {
    return e.position();
}

} //namespace bkrl
