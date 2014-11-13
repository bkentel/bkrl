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
    utf8string name;
    utf8string text;
};

//==============================================================================
//! A container of defintions used to generate entities.
//==============================================================================
class entity_definitions {
public:
    static path_string_ref tile_filename();
    static tex_point_i     tile_size();
    static tex_point_i     player_tile();

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
//! The data common to all entities.
//==============================================================================
struct entity_data_t {
    health_t   max_health;
    health_t   cur_health;
    ipoint2    position;
    item_stack items;
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
    using point_t = ipoint2;
    using defs_t  = entity_definitions const&;

    entity()                         = default;
    entity(entity&&)                 = default;
    entity& operator=(entity&&)      = default;
    entity(entity const&)            = delete;
    entity& operator=(entity const&) = delete;

    entity_render_info_t render_info(defs_t defs) const;

    string_ref name(defs_t defs) const;

    item_stack&       items()       { return data.items; }
    item_stack const& items() const { return data.items; }

    auto position() const noexcept { return data.position; }

    void move_to(point_t const& p) { data.position =  p; }
    void move_by(ivec2 const& v)   { data.position += v; }

    auto health() const { return range<int> {data.cur_health, data.max_health}; }

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

    item_stack get_equippable(defs_t idefs, items_t istore) const;

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
