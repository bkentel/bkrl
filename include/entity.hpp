#pragma once

#include "math.hpp"
#include "types.hpp"
#include "locale.hpp"
#include "algorithm.hpp"
#include "item.hpp" //TODO temp

namespace bkrl {

class entity_def : public definition_base<entity_def> {
public:
    using dist_t = random::random_dist;

    struct locale {
        utf8string name;
        utf8string text;
    };

    string_id id;      //!< entity id
    dist_t    items;   //!< number of carried items
    int16_t   tile_x;  //!< tile x index
    int16_t   tile_y;  //!< tile y index
    uint8_t   r, g, b; //!< color
    dist_t    health;  //!< health
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

    using point_t = ipoint2;

    entity(
        random::generator& gen
      , identifier const id
      , point_t    const pos
      , item_def::definition_t   const& items
      , entity_def::definition_t const& entities
    )
      : id_ {id}
      , pos_ (pos)
    {
        auto const& edef = entities[id];

        health_     = edef.health(gen);
        item_count_ = edef.items(gen);
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

    string_ref name(entity_def::localized_t const& defs) const {
        return defs[id_].name;
    }

    void add_item(item&& itm, item_def::definition_t const& defs) {
        items_.insert(std::move(itm), defs);
    }

    item_stack&       items()       { return items_; }
    item_stack const& items() const { return items_; }

    identifier id() const { return id_; }

    //TODO temp
    int item_count() const { return item_count_; }

    entity(entity&& other)
      : id_  {other.id_}
      , pos_ (other.pos_)
      , items_ {std::move(other.items_)}
      , item_count_ {other.item_count_}
      , health_ {other.health_}
    {
    }

    entity& operator=(entity&& rhs) {
        //TODO use swap
        id_    = rhs.id_;
        pos_   = rhs.pos_;
        items_ = std::move(rhs.items_);
        item_count_ = rhs.item_count_;
        health_ = rhs.health_;

        return *this;
    }

    bool apply_damage(int n) {
        health_ -= n;
        return health_ <= 0;
    }
private:
    identifier id_;
    point_t    pos_ = {};
    item_stack items_;
    int        item_count_;
    int        health_;
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
