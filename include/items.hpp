//##############################################################################
//! @file
//! @author Brandon Kentel
//! @todo copyright / licence
//##############################################################################
#pragma once

#include <type_traits>
#include <memory>
#include <vector>
#include <string>

#include "identifier.hpp"

namespace json11 { class Json; }

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////
using utf8string = std::string;

namespace json   { using cref = ::json11::Json const&; }
namespace random { class generator; }

using random_t = random::generator;

class item;
class item_stack;
class item_store;
class item_definitions;

struct item_definition;
struct item_locale;

namespace detail { class item_store_impl; }
namespace detail { class item_definitions_impl; }
////////////////////////////////////////////////////////////////////////////////

//==============================================================================
//! General item type.
//==============================================================================
enum class item_type : uint8_t {
    none
  , weapon
  , armor
  , scroll
  , potion
  , container
  , enum_size
};

//==============================================================================
//! Damage type.
//==============================================================================
enum class damage_type : uint8_t {
    none
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
//! Equipment slots for items; can be combined in a bitmask.
//==============================================================================
enum class equip_slot : uint8_t {
    none
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
    void insert(item_id const id);

    void insert(item_stack&& other);

    item_id remove(item_id const id);
    item_id remove(int const index);

    auto empty() const noexcept { return items_.empty(); }
    
    auto size() const noexcept { return items_.size(); }

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
struct consumable_data {
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
    item_stack      stack;
    weapon_data     weapon;
    armor_data      armor;
    consumable_data consumable;
    size_t          dummy;

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
    ~item_definitions();

    void load_definitions(json::cref data);
    void load_locale(json::cref data);

    item_locale const& get_locale(item_def_id const id) const;
    item_definition const& get_definition(item_def_id const id) const;

    void set_locale(lang_id const lang);
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
//! Generate an item instance.
//==============================================================================
item_id generate_item(
    random_t&               gen
  , item_store&             store
  , item_definitions const& defs
  , loot_table       const& table
);

////////////////////////////////////////////////////////////////////////////////
} //namespace bkrl
////////////////////////////////////////////////////////////////////////////////
