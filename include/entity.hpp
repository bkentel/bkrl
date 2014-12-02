//##############################################################################
//! @file
//! @author Brandon Kentel
//! @todo copyright / licence
//##############################################################################
#pragma once

#include "math.hpp"
#include "string.hpp"
#include "integers.hpp"
#include "items.hpp"
#include "identifier.hpp"
#include "render_types.hpp"
#include "random.hpp"
#include "combat_types.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////
class entity;
class entity_definitions;

struct entity_definition;
struct entity_locale;

namespace detail { class entity_store_impl; }
namespace detail { class entity_definitions_impl; }

//==============================================================================
//! Localized entity strings.
//==============================================================================
struct entity_locale {
    utf8string name; //!< entity name
    utf8string text; //!< entity description
};

//==============================================================================
//! A container of defintions used to generate entities.
//==============================================================================
class entity_definitions {
public:
    static path_string_ref tile_filename(); //!< TODO filename for the entity tiles
    static tex_point_i     tile_size();     //!< TODO dimensions for the entity tiles
    static tex_point_i     player_tile();   //!< TODO index of the player tile

    entity_definitions();
    ~entity_definitions();

    void load_definitions(json::cref data);
    void load_locale(json::cref data);

    entity_locale     const& get_locale(entity_def_id id)     const;
    entity_definition const& get_definition(entity_def_id id) const;

    void set_locale(lang_id lang);

    int get_definitions_size() const;
    entity_definition const& get_definition_at(int index) const;
private:
    std::unique_ptr<detail::entity_definitions_impl> impl_;
};

//==============================================================================
//class entity_map {
//public:
//    using point_t = ipoint2;
//
//    struct record_t {
//        static bool less(point_t const lhs, point_t const rhs) noexcept {
//            return bkrl::lexicographical_compare(lhs, rhs);
//        }
//
//        operator point_t() const noexcept { return pos(); }
//
//        entity_id id()  const noexcept { return value.first; }
//        point_t   pos() const noexcept { return value.second; }
//
//        friend bool operator<(point_t  const lhs, record_t const rhs) noexcept { return less(lhs, rhs); }
//        friend bool operator<(point_t  const lhs, point_t  const rhs) noexcept { return less(lhs, rhs); }
//        friend bool operator<(record_t const lhs, record_t const rhs) noexcept { return less(lhs, rhs); }
//
//        friend bool operator==(record_t const lhs, record_t const rhs) noexcept {
//            return lhs.value.second == rhs.value.second;
//        }
//
//        friend bool operator!=(record_t const lhs, record_t const rhs) noexcept {
//            return !(lhs == rhs);
//        }
//
//        std::pair<entity_id, point_t> value;
//    };
//
//private:
//    std::vector<entity>   entities_;
//    std::vector<record_t> data_;
//};

//==============================================================================

template <typename T>
class ranged_value {
public:
    static_assert(std::is_integral<T>::value, "");

    using value_type = T;

    enum : T { minimum = 0 };
    
    ranged_value(T const max, T const cur) noexcept
      : current {cur}
      , maximum {max}
    {
        BK_ASSERT_DBG(current <= maximum);
        BK_ASSERT_DBG(current >= minimum);
    }
    
    ranged_value(T const max) noexcept
      : ranged_value {max, max}
    {
    }

    ranged_value() = default;

    void modify(T const delta) noexcept {
        set_to(safe_add(current, delta));
    }

    bool is_min() const noexcept { return current <= minimum; }
    bool is_max() const noexcept { return current >= maximum; }

    void set_to_min() noexcept { current = minimum; }
    void set_to_max() noexcept { current = maximum; }
    
    void set_to(T const value) noexcept {
        current = clamp(value, T {minimum}, T {maximum});
    }

    template <typename U>
    U scale_to(U const value) const noexcept {       
        return (current * value) / maximum;
    }

    T current;
    T maximum;
};

//==============================================================================
//! The data common to all entities.
//==============================================================================
struct entity_data_t {
    ranged_value<health_t> health;
    ipoint2    position;
    item_collection items;
};

//==============================================================================
//! The data needed to render an entity.
//==============================================================================
struct entity_render_info_t {
    tex_point_i tex_position;
    argb8       tex_color;
};

//==============================================================================
//! An actual instance of a generated entity.
//==============================================================================
class entity {
public:
    ////////////////////////////////////////////////////////////////////////////
    using point_t = ipoint2;
    using defs_t  = entity_definitions const&;
    
    ////////////////////////////////////////////////////////////////////////////
    entity()                         = default;
    entity(entity&&)                 = default;
    entity& operator=(entity&&)      = default;
    entity(entity const&)            = delete;
    entity& operator=(entity const&) = delete;
    
    ////////////////////////////////////////////////////////////////////////////

    //--------------------------------------------------------------------------
    entity_render_info_t render_info(defs_t defs) const;

    //--------------------------------------------------------------------------
    string_ref name(defs_t defs) const;

    item_collection&       items()       { return data.items; }
    item_collection const& items() const { return data.items; }

    auto position() const noexcept { return data.position; }

    void move_to(point_t const& p) { data.position =  p; }
    void move_by(ivec2 const& v)   { data.position += v; }

    auto health() const { return data.health; }

    bool apply_damage(health_t delta);

    friend bool operator==(entity const& rhs, entity const& lhs) noexcept {
        return lhs.instance_id == rhs.instance_id;
    }

    friend bool operator!=(entity const& rhs, entity const& lhs) noexcept {
        return !(lhs == rhs);
    }

    damage_t  get_attack_value(random_t& gen, defs_t defs);
    defence_t get_defence_value(random_t& gen, defs_t defs, damage_type type);
public:
    entity_id     instance_id;
    entity_def_id id;
    entity_data_t data;
};

//==============================================================================
//! @TODO
//==============================================================================
class spawn_table {
public:
    entity_def_id operator()(random_t&) const {
        return entity_def_id {0};
    }
private:
};

//==============================================================================
//!
//==============================================================================
entity generate_entity(
    random::generator&        gen
  , entity_definitions const& entity_defs
  , item_definitions   const& item_defs
  , item_store&               items
  , spawn_table        const& table
);

//==============================================================================
//!
//==============================================================================
class player : public entity {
public:
    using defs_t  = item_definitions const&;
    using items_t = item_store const&;

    using entity::entity;

    entity_render_info_t render_info(entity_definitions const&) const;

    item_collection get_equippable(defs_t idefs, items_t istore) const;

    equipment::result_t equip_item(item_id iid, defs_t defs, items_t istore);

    equipment&       equip();
    equipment const& equip() const;

    damage_t get_attack_value(random_t& gen, items_t items);
    
    defence_t get_defence_value(random_t& gen, defs_t defs, damage_type type);

    
private:
    equipment equip_;
};

inline auto position(entity const& e) {
    return e.data.position;
}

} //namespace bkrl
