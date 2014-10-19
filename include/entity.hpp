#pragma once

#include "types.hpp"
#include "math.hpp"
#include "item.hpp"

namespace bkrl {

namespace detail { class entity_definitions_impl; }

//==============================================================================
//!
//==============================================================================
class entity_definitions {
public:
    using dist_t = random::random_dist;

    struct definition {
        static path_string tile_filename;
        static ipoint2     tile_size;

        string_id id;      //!< entity id
        dist_t    items;   //!< number of carried items
        int16_t   tile_x;  //!< tile x index
        int16_t   tile_y;  //!< tile y index
        uint8_t   r, g, b; //!< color
        dist_t    health;  //!< health
    };

    struct locale {
        utf8string name;
        utf8string text;
    };

    entity_definitions();
    ~entity_definitions();

    void load_definitions(json::cref data);
    void load_locale(json::cref data);

    definition const& get_definition(identifier id) const;
    locale     const& get_locale(identifier id) const;

    void set_locale(lang_id lang);

    int definitions_size() const;
    definition const& get_definition_at(int index) const;
private:
    std::unique_ptr<detail::entity_definitions_impl> impl_;
};

//==============================================================================
//==============================================================================
class entity {
public:
    BK_NOCOPY(entity);
    //BK_DEFMOVE(entity);
    entity() = default;

    using point_t = ipoint2;

    using defs_t = entity_definitions const&;

    entity(
        random::generator&      gen
      , identifier       const  id
      , point_t          const  pos
      , item_definitions const& items
      , defs_t                  entities
    )
      : id_ {id}
      , pos_ (pos)
    {
        auto const& edef = entities.get_definition(id);

        health_total_ = edef.health(gen);
        health_ = health_total_;

        auto const count = edef.items(gen);
        for (int i = 0; i < count; ++i) {
            add_item(generate_item(gen, items, loot_table {}), items);
        }
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

    string_ref name(defs_t defs) const {
        return defs.get_locale(id_).name;
    }

    void add_item(item&& itm, item_definitions const& defs) {
        items_.insert(std::move(itm), defs);
    }

    item_stack&       items()       { return items_; }
    item_stack const& items() const { return items_; }

    identifier id() const { return id_; }

    entity(entity&& other)
      : id_  {other.id_}
      , items_ {std::move(other.items_)}
      , pos_ (other.pos_)
      , health_ {other.health_}
      , health_total_ {other.health_total_}
    {
    }

    entity& operator=(entity&& rhs) {
        //TODO use swap
        id_    = rhs.id_;
        pos_   = rhs.pos_;
        items_ = std::move(rhs.items_);
        health_ = rhs.health_;
        health_total_ = rhs.health_total_;

        return *this;
    }

    bool apply_damage(int n) {
        health_ -= n;
        return health_ <= 0;
    }

    range<int> health() const {
        return range<int> {health_, health_total_};
    }
private:
    identifier id_;
    item_stack items_;
    point_t    pos_          = point_t {0, 0};
    int        health_       = 1;
    int        health_total_ = 1;
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
