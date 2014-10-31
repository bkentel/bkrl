//##############################################################################
//! @file
//! @author Brandon Kentel
//! @todo copyright / licence
//##############################################################################
#pragma once

#include <type_traits>
#include <memory>
#include <vector>
#include <bitset>

#include "identifier.hpp"
#include "types.hpp"
#include "hash.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////
class item;
class item_stack;
class item_store;
class item_definitions;
class equipment;

struct item_definition;
struct item_locale;

namespace detail { class item_store_impl; }
namespace detail { class item_definitions_impl; }
namespace detail { class equipment_impl; }
////////////////////////////////////////////////////////////////////////////////

//==============================================================================
//! General item type.
//==============================================================================
enum class item_type : uint8_t {
    invalid
  
  , none = 0
  , weapon
  , armor
  , scroll
  , potion
  , container

  , enum_size
};

extern template item_type from_hash(hash_t hash);

//==============================================================================
//! Item equipment slots.
//==============================================================================
enum class equip_slot : uint8_t {
    invalid

  , none = 0
  , head
  , arms_upper
  , arms_lower
  , hands
  , chest
  , waist
  , legs_upper
  , legs_lower
  , feet
  , finger_left
  , finger_right
  , neck
  , back
  , hand_main
  , hand_off
  , ammo

  , enum_size
};

extern template equip_slot from_hash(hash_t hash);

//==============================================================================
//! Slots occupied by an item.
//==============================================================================
using equip_slot_flags = std::bitset<
    static_cast<size_t>(equip_slot::enum_size) - 1
>;

//==============================================================================
//! Damage type.
//==============================================================================
enum class damage_type : uint8_t {
    invalid

  , none = 0
  , slash
  , pierce
  , blunt
  , fire
  , cold
  , electric
  , acid

  , enum_size
};

//==============================================================================
//! A description of where and how an item was generated.
//==============================================================================
class item_birthplace {
public:
    enum source_type : uint8_t {
        floor, item, entity
    };

    using id_type = uint32_t;

    static_assert(std::is_same<item_def_id::value_type,   id_type>::value, "");
    static_assert(std::is_same<entity_def_id::value_type, id_type>::value, "");

    id_type     id     = id_type {};
    int         area   = int     {};
    uint8_t     level  = uint8_t {};
    source_type type   = source_type::floor;
};

//==============================================================================
//! A stack or pile of one or more items.
//==============================================================================
class item_stack {
public:
    using defs_t  = item_definitions const&;
    using items_t = item_store const&;

    void insert(item_id const id, defs_t defs, items_t items);

    void insert(item_stack&& other, defs_t defs, items_t items);

    item_id remove(item_id const id);
    item_id remove(int const index);

    item_id at(int const index) const {
        BK_ASSERT(index >= 0);
        return items_.at(static_cast<size_t>(index));
    }

    auto empty() const noexcept { return items_.empty(); }
    
    auto size() const noexcept { return static_cast<int>(items_.size()); }

    auto begin()       { return std::begin(items_); }
    auto begin() const { return std::begin(items_); }

    auto end()       { return std::end(items_); }
    auto end() const { return std::end(items_); }

    auto cbegin() const { return std::cbegin(items_); }
    auto cend()   const { return std::cend(items_); }
private:
    std::vector<item_id> items_;
};

//==============================================================================
//! Weapon (item_type::weapon) specific data.
//==============================================================================
struct weapon_data {
    int16_t     dmg_min  = int16_t {0};
    int16_t     dmg_max  = int16_t {0};
    damage_type dmg_type = damage_type::none;
};

//==============================================================================
//! Armor (item_type::armor) specific data.
//==============================================================================
struct armor_data {
    using resist_t  = int8_t;
    using resists_t = std::array<resist_t, static_cast<size_t>(damage_type::enum_size) - 1>;

    int16_t   base    = int16_t   {0};
    resists_t resists = resists_t {0};
};

//==============================================================================
//!
//==============================================================================
struct potion_data {
    int16_t count = int16_t {0};
};

//==============================================================================
//! An unrestriced union of item specific data types. Care must be take to
//! properly construct / destruct it.
//==============================================================================
#if BOOST_COMP_MSVC
#   pragma warning( disable : 4582 4583 )
#endif
union item_data_t {
    friend item;
public:
    item_stack  stack;
    weapon_data weapon;
    armor_data  armor;
    potion_data potion;
    size_t      dummy;

    ~item_data_t() { }
private:
    item_data_t() noexcept { new (&dummy) size_t {}; }
};
#if BOOST_COMP_MSVC
#   pragma warning( default : 4582 4583 )
#endif

//==============================================================================
//! Instance specific realization of an item definition.
//==============================================================================
class item {
public:
    item()                       = default;
    item(item const&)            = delete;
    item& operator=(item const&) = delete;
    item& operator=(item&&)      = delete;

    item(item&&);
    ~item();

    using defs_t = item_definitions const&;

    string_ref get_name(defs_t defs) const;

    bool can_equip(defs_t defs) const;
    bool can_equip(equipment const& eq, defs_t defs) const;

    item_def_id     id;
    item_birthplace origin;
    item_data_t     data;
    item_type       type;
};



//==============================================================================
//! A container of all items.
//==============================================================================
class item_store {
public:
    using rvalue          = item&&;
    using reference       = item&;
    using const_reference = item const&;
    
    item_store();
    item_store(item_store&&);
    ~item_store();

    item_id insert(rvalue value);
    void    remove(item_id id);

    reference       operator[](item_id id);
    const_reference operator[](item_id id) const;
private:
    std::unique_ptr<detail::item_store_impl> impl_;
};

//==============================================================================
//! Locale (language) specific item data.
//==============================================================================
struct item_locale {
    utf8string name;
    utf8string sort;
    utf8string text;
};

//==============================================================================
//! A container of item_definition records.
//==============================================================================
class item_definitions {
public:
    item_definitions();
    item_definitions(item_definitions&&);
    ~item_definitions();

    void load_definitions(json::cref data);
    void load_locale(json::cref data);

    item_locale     const& get_locale(item_def_id id)     const;
    item_definition const& get_definition(item_def_id id) const;

    void set_locale(lang_id const lang);

    int get_definitions_size() const;
    item_definition const& get_definition_at(int index) const;
private:
    std::unique_ptr<detail::item_definitions_impl> impl_;
};

//==============================================================================
//! @TODO
//==============================================================================
class loot_table {
public:
    item_def_id operator()(random_t&) const {
        return item_def_id {0};
    }
private:
};

//==============================================================================
//!
//==============================================================================
class equipment {
public:
    using defs_t   = item_definitions const&;
    using items_t  = item_store const&;
    using result_t = std::pair<equip_slot_flags, bool>;

    equipment();
    equipment(equipment&&);
    ~equipment();

    result_t can_equip(item_id id, defs_t defs, items_t items) const;

    result_t equip(item_id id, defs_t defs, items_t items);

    optional<item_id> unequip(equip_slot slot, defs_t defs, items_t items);

    optional<item_id> in_slot(equip_slot slot) const;

    optional<item_id> match_any(equip_slot_flags flags) const;
private:
    std::unique_ptr<detail::equipment_impl> impl_;
};

//==============================================================================
//! Generate an item instance.
//==============================================================================
item_id generate_item(
    random_t&               gen
  , item_store&             store
  , item_definitions const& defs
  , loot_table       const& table
  , item_birthplace         origin
);

#ifdef BK_TEST
//==============================================================================
//! Generate an item instance from a definition id; for testing only.
//==============================================================================
item_id generate_item(
    item_def_id             id
  , item_store&             store
  , item_definitions const& defs
);
#endif

item_definition const& get_item_def(
    item_id const id
  , item_definitions const& defs
  , item_store const& store
);

item_locale const& get_item_loc(
    item_id const id
  , item_definitions const& defs
  , item_store const& store
);

////////////////////////////////////////////////////////////////////////////////
} //namespace bkrl
////////////////////////////////////////////////////////////////////////////////
