#pragma once

#include "math.hpp"
#include "types.hpp"
#include "locale.hpp"
#include "algorithm.hpp"
#include "item.hpp" //TODO temp

namespace bkrl {

class entity_def : public definition_base<entity_def> {
public:
    struct locale {
        utf8string name;
        utf8string text;
    };

    string_id      id;
    range<int16_t> items;
    int16_t        tile_x;
    int16_t        tile_y;
    uint8_t        r, g, b;
};

entity_def::definition_t load_entities(json::cref data);
entity_def::localized_t  load_entities_locale(json::cref data);

//==============================================================================
//==============================================================================
class entity {
public:
    BK_NOCOPY(entity);
    //BK_DEFMOVE(entity);
    entity() = default;

    entity(entity&& other)
      : id_  {other.id_}
      , pos_ (other.pos_)
      , items_ {std::move(other.items_)}
    {
    }

    entity& operator=(entity&& rhs) {
        //TODO use swap
        id_    = rhs.id_;
        pos_   = rhs.pos_;
        items_ = std::move(rhs.items_);

        return *this;
    }

    using point_t = ipoint2;

    entity(point_t const pos, string_id const id)
      : id_ {id}
      , pos_ (pos)
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

    void add_item(item&& itm, item_def::definition_t const& defs) {
        items_.insert(std::move(itm), defs);
    }

    item_stack&       items()       { return items_; }
    item_stack const& items() const { return items_; }

    identifier id() const { return id_; }
private:
    point_t    pos_ = {};
    identifier id_;
    item_stack items_;
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
