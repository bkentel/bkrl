#pragma once

#include "math.hpp"
#include "types.hpp"
#include "locale.hpp"

#include "item.hpp" //TODO temp

namespace bkrl {

class entity_def : public definition_base<entity_def> {
public:
    struct locale {
        utf8string name;
        utf8string text;
    };

    static definition_t load_definitions(utf8string const& data);
    static definition_t load_definitions(string_ref filename);
    static localized_t  load_localized_strings(utf8string const& data);
    static localized_t  load_localized_strings(string_ref filename);

    string_id  id;
    range<int> items;
};


//==============================================================================
//==============================================================================
class entity {
public:
    using point_t = ipoint2;

    explicit entity(point_t const pos, string_id const id)
      : id_ {id}
      , pos_ (pos)
    {
    }

    entity()
      : pos_ (point_t{0, 0})
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

    void add_item(item&& itm) {
        items_.emplace_back(std::move(itm));
    }

    auto items_begin() const {
        return std::cbegin(items_);
    }

    auto items_end() const {
        return std::cend(items_);
    }

    string_id id() const { return id_; }
private:
    string_id id_;
    point_t pos_;
    std::vector<item> items_;
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
