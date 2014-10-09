#pragma once

#include "math.hpp"
#include "types.hpp"

namespace bkrl {

//==============================================================================
//==============================================================================
class entity {
public:
    entity(ipoint2 const pos)
      : pos_ (pos)
    {
    }

    entity()
      : entity {ipoint2{0, 0}}
    {
    }

    ipoint2 position() const {
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

    void move_to(ipoint2 const p) {
        move_to(p.x, p.y);
    }
private:
    ipoint2 pos_;
};

//==============================================================================
//==============================================================================
class player : public entity {
public:
    using entity::entity;
private:
};

} //namespace bkrl
