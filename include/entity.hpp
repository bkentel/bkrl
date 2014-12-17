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

#include "grid.hpp" //change to fwd def

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////
class loot_table_definitions;

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
    ipoint2                position;
    item_collection        items;
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
    bool is_player() const noexcept;

    //--------------------------------------------------------------------------
    string_ref name(defs_t defs) const;
    string_ref description(defs_t defs) const;

    item_collection&       items()       { return data.items; }
    item_collection const& items() const { return data.items; }

    auto position() const noexcept { return data.position; }

    void move_to(point_t const& p) { data.position =  p; }
    void move_by(ivec2 const& v)   { data.position += v; }

    auto health() const { return data.health; }

    bool apply_damage(health_t delta);

    damage_t  get_attack_value(random_t& gen, defs_t defs);
    defence_t get_defence_value(random_t& gen, defs_t defs, damage_type type);

    bool can_pass_tile(ipoint2 const p, grid_storage const& grid) const {
        using tt = tile_type;

        BK_ASSERT(grid.is_valid(p));

        auto const type = grid.get(attribute::tile_type, p);

        switch (type) {
        case tt::floor :    //fallthrough true
        case tt::corridor : //fallthrough true
        case tt::stair :
            return true;

        case tt::door :
            return door_data {grid, p}.is_open();

        case tt::invalid : //fallthrough false
        case tt::empty :   //fallthrough false
        case tt::wall :    //fallthrough false
        default :
            break;
        }

        return false;
    }

    //--------------------------------------------------------------------------
    friend bool operator==(entity const& lhs, entity const& rhs) noexcept {
        return lhs.instance_id == rhs.instance_id;
    }

    friend bool operator==(entity_id const lhs, entity const& rhs) noexcept {
        return lhs == rhs.instance_id;
    }

    friend bool operator==(entity const& lhs, entity_id const rhs) noexcept {
        return lhs.instance_id == rhs;
    }

    friend bool operator!=(entity const& lhs, entity const& rhs) noexcept {
        return !(lhs == rhs);
    }
public:
    entity_id     instance_id;
    entity_def_id id;
    entity_data_t data;
};

//==============================================================================
//! entity_map
//==============================================================================
class entity_map {
public:
    using point_t = ipoint2;

    struct less_pos_t {
        static bool less(point_t const lhs, point_t const rhs) noexcept {
            return bkrl::lexicographical_compare(lhs, rhs);
        }

        bool operator()(entity const& lhs, entity const& rhs) const noexcept {
            return less(lhs.position(), rhs.position());
        }

        bool operator()(point_t const lhs, entity const& rhs) const noexcept {
            return less(lhs, rhs.position());
        }

        bool operator()(entity const& lhs, point_t const rhs) const noexcept {
            return less(lhs.position(), rhs);
        }

        bool operator()(entity const* lhs, entity const* rhs) const noexcept {
            return less(lhs->position(), rhs->position());
        }

        bool operator()(point_t const lhs, entity const* rhs) const noexcept {
            return less(lhs, rhs->position());
        }

        bool operator()(entity const* lhs, point_t const rhs) const noexcept {
            return less(lhs->position(), rhs);
        }
    };

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    bool insert_at(point_t const p, entity&& ent) {
        ent.move_to(p);

        auto const result = lower_bound(map_, p, less_pos_t {});
        auto const it     = result.first;

        if (result.second) {
            auto const& other = **it;
            if (other.position() == ent.position()) {
                return false;
            }
        }

        auto const resize = instances_.size() == instances_.capacity();
        if (resize) {
            instances_.reserve(
                8 + instances_.capacity() * 2
            );
        }
        
        instances_.push_back(std::move(ent));

        if (!resize) {
            map_.insert(it, &instances_.back());
        } else {
            rebuild_map_();
        }
        
        return true;
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    bool remove(point_t const p, entity_id const id) {
        auto const beg = std::begin(instances_);
        auto const end = std::end(instances_);
        auto const it  = std::find_if(beg, end
          , [&](entity const& e) { return e.position() == p; }
        );
        
        if (it == end) {
            return false;
        }

        BK_ASSERT(it->instance_id == id);

        instances_.erase(it);
        rebuild_map_();

        return true;
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    optional<entity&> at_(point_t const p) const {
        auto const beg = std::begin(map_);
        auto const end = std::end(map_);
        auto const it  = std::lower_bound(beg, end, p, less_pos_t {});

        if (it == end) {
            return {};
        }

        auto& ent = *(*it);
        if (p != ent.position()) {
            return {};
        }

        return {ent};
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    inline optional<entity const&> at(point_t const p) const {
        auto const result = at_(p);
        if (!result) {
            return {};
        }

        return {result.get()};
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    template <typename Function>
    bool with_entity_at(point_t const p, Function&& function) {
        auto const& maybe_ent = at_(p);
        if (!maybe_ent) {
            return false;
        }
        
        auto& ent = *maybe_ent;

        function(ent);

        if (p != ent.position()) {
            sort_map_();
        }

        return true;
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    template <typename Function>
    bool with_entity_at(point_t const p, Function&& function) const {
        return const_cast<entity_map*>(this)->with_entity_at(p, [&](entity const& ent) {
            function(ent);
        });
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    template <typename Function>
    void with_each_entity(Function&& function) {
        for (auto& ent : instances_) {
            auto const id = ent.instance_id;
            auto const p  = ent.position();

            function(ent);

            //would cause mayhem
            BK_ASSERT_DBG(id == ent.instance_id);

            if (p != ent.position()) {
                sort_map_();
            }
        }
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    template <typename Function>
    void for_each(Function&& function) const {
        for (auto const& e : instances_) {
            function(e);
        }
    }
private:
    std::vector<entity>  instances_;
    std::vector<entity*> map_;

    void rebuild_map_() {
        map_.clear();
        map_.reserve(instances_.capacity());
        for (auto& e : instances_) { map_.push_back(&e); }
        sort_map_();
    }
    
    void sort_instances_() {
        bkrl::sort(instances_, [](entity const& lhs, entity const& rhs) {
            return lhs.instance_id < rhs.instance_id;
        });
    }

    void sort_map_() {
        bkrl::sort(map_, less_pos_t {});
    }

    template <typename Container, typename T, typename Predicate = std::less<>>
    static auto optional_find(Container&& c, T const& value, Predicate&& predicate = Predicate {}) {
        auto const result = bkrl::lower_bound(
            std::forward<Container>(c)
          , value
          , std::forward<Predicate>(predicate)
        );

        using result_t = bkrl::optional<decltype(result.first)>;

        return result.second ? result_t {result.first} : result_t {};        
    }
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
  , loot_table_definitions const& loot_defs
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

    bool can_pass_tile(ipoint2 const p, grid_storage const& grid) const {
        if (grid.get(attribute::tile_type, p) == tile_type::invalid) {
            return true;
        }

        return entity::can_pass_tile(p, grid);
    }
private:
    equipment equip_;
};

inline auto position(entity const& e) {
    return e.data.position;
}

} //namespace bkrl
