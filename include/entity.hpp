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

    static definition_t load_definitions(utf8string const& data);
    static definition_t load_definitions(string_ref filename);
    static localized_t  load_localized_strings(utf8string const& data);
    static localized_t  load_localized_strings(string_ref filename);

    string_id      id;
    range<int16_t> items;
    int16_t        tile_x;
    int16_t        tile_y;
    uint8_t        r, g, b;
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

    void add_item(item&& itm, item_def::definition_t const& defs) {
        auto const can_stack = itm.can_stack(defs);

        if (can_stack) {
            auto const end = std::end(items_);
            auto const beg = std::begin(items_);
            auto const it  = std::find_if(beg, end, [&](item const& other) {
                return (itm == other) && (other.count < other.max_stack(defs));
            });

            if (it != end) {
                it->count += itm.count;
                return;
            }
        }

        items_.emplace_back(std::move(itm));
        bkrl::sort(items_);
    }

    auto items_begin() const {
        return std::cbegin(items_);
    }

    auto items_end() const {
        return std::cend(items_);
    }

    identifier id() const { return id_; }
private:
    identifier id_;
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
