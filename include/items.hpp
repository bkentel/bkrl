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
#include "optional.hpp"
#include "hash.hpp"
#include "string.hpp"
#include "render_types.hpp"
#include "json_forward.hpp"
#include "random_forward.hpp"
#include "combat_types.hpp"

#include "algorithm.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////
class item;
class item_stack;
class item_store;
class item_definitions;
class equipment;

class message_map;

struct item_definition;
struct item_locale;

namespace detail { class item_store_impl; }
namespace detail { class item_definitions_impl; }
namespace detail { class equipment_impl; }

using weight_t = int16_t;
////////////////////////////////////////////////////////////////////////////////

//==============================================================================
//! item_collection
//==============================================================================
class item_collection {
public:
    bool empty() const noexcept { return items_.empty(); }
    int  size()  const noexcept { return static_cast<int>(items_.size()); }

    void clear() {
        items_.clear();
    }

    void reserve(size_t const n) {
        items_.reserve(n);
    }

    void insert(item_id const itm) {
        items_.push_back(itm);
    }

    bool remove(item_id const itm) {
        auto result = false;
        
        bkrl::find_and(items_, itm, [&](auto& it) {
            result = true;
            items_.erase(it);    
        });

        return result;
    }

    template <typename Function>
    void remove_nth_and(int const n, Function&& function) {
        BK_ASSERT_DBG(n >= 0 && n < items_.size());

        auto const it = items_.begin() + n;

        function(*it);

        items_.erase(it);
    }

    template <typename Function>
    void remove_all_and(Function&& function) {
        for (auto& i : items_) {
            function(i);
        }

        items_.clear();
    }

    template <typename Function>
    void for_each_item(Function&& function) const {
        for (auto const& i : items_) {
            function(i);
        }
    }
private:
    std::vector<item_id> items_;
};

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
string_ref to_string(message_map const& msgs, item_type type); //TODO

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
    item_collection stack;
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
//! The data needed to render an entity.
//==============================================================================
struct item_render_info_t {
    tex_point_i tex_position;
    argb8       tex_color;
};

//==============================================================================
//! Instance specific realization of an item definition.
//==============================================================================
class item {
public:
    using defs_t = item_definitions const&;
    using msg_t  = message_map const&;

    item()                       = default;
    item(item const&)            = delete;
    item& operator=(item const&) = delete;
    item& operator=(item&&)      = delete;

    item(item&&);
    ~item();

    //--------------------------------------------------------------------------
    string_ref name(defs_t defs) const;

    //--------------------------------------------------------------------------
    bool is_equippable(defs_t defs) const;
    
    //--------------------------------------------------------------------------
    equip_slot_flags equip_slots(defs_t defs) const;

    //--------------------------------------------------------------------------
    utf8string short_description(msg_t messages) const;
    
    //--------------------------------------------------------------------------
    utf8string long_description(msg_t messages) const;

    //--------------------------------------------------------------------------
    weight_t weight(defs_t defs) const;

    //--------------------------------------------------------------------------
    item_render_info_t render_info(defs_t defs) const;

    //--------------------------------------------------------------------------
    template <typename Visitor>
    void with_data(Visitor&& visitor) const {
        using it = item_type;

        switch (type) {
        default :
        case it::enum_size:
        case it::scroll:
        case it::container:
            BK_TODO_FAIL();
            break;
        case it::none   :                       break;
        case it::weapon : visitor(data.weapon); break;
        case it::armor  : visitor(data.armor);  break;
        case it::potion : visitor(data.potion); break;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////////////////////////

    item_def_id     id;
    item_birthplace origin;
    item_data_t     data;
    item_type       type;
};

//==============================================================================
//! item_map
//==============================================================================
class item_map {
public:
    using point_t = ipoint2;

    struct record_t {
        static bool less(point_t const lhs, point_t const rhs) noexcept {
            return bkrl::lexicographical_compare(lhs, rhs);
        }

        item_id id()  const noexcept { return value.first; }
        point_t pos() const noexcept { return value.second; }

        friend bool operator<(record_t const lhs, record_t const rhs) noexcept { return less(lhs.pos(), rhs.pos()); }
        friend bool operator<(point_t  const lhs, record_t const rhs) noexcept { return less(lhs,       rhs.pos()); }
        friend bool operator<(record_t const lhs, point_t  const rhs) noexcept { return less(lhs.pos(), rhs      ); }

        friend bool operator==(record_t const lhs, record_t const rhs) noexcept {
            return lhs.value.second == rhs.value.second;
        }

        friend bool operator!=(record_t const lhs, record_t const rhs) noexcept {
            return !(lhs == rhs);
        }

        std::pair<item_id, point_t> value;
    };

    ////////////////////////////////////////////////////////////////////////////
    // insert
    ////////////////////////////////////////////////////////////////////////////

    //--------------------------------------------------------------------------
    //! insert @p itm at @p p.
    //--------------------------------------------------------------------------
    void insert_at(point_t const p, item_id const itm) {
        insert_(p, itm);
        bkrl::sort(items_);
    }

    //--------------------------------------------------------------------------
    //! removes all the items from @p items and inserts them at @p p.
    //--------------------------------------------------------------------------
    void insert_at(point_t const p, item_collection& items) {
        items.remove_all_and([&](item_id const itm) {
            insert_(p, itm);
        });
        
        bkrl::sort(items_);
    }

    //--------------------------------------------------------------------------
    //! inserts all the items in the range given by [@p beg, @p end) at @p p.
    //--------------------------------------------------------------------------
    template <typename Iterator>
    void insert_at(point_t const p, Iterator const beg, Iterator const end) {
        std::for_each(beg, end, [&](item_id const itm) {
            insert_(p, itm);
        });
        
        bkrl::sort(items_);
    }

    ////////////////////////////////////////////////////////////////////////////
    // visitation
    ////////////////////////////////////////////////////////////////////////////

    //--------------------------------------------------------------------------
    //! for each item at @p p calls @p function(item_id).
    //--------------------------------------------------------------------------
    template <typename Function>
    void for_each_item_at(point_t const p, Function&& function) const {
        bkrl::for_each(bkrl::equal_range(items_, p), [&](record_t const& r) {
            function(r.id());
        });
    }

    //--------------------------------------------------------------------------
    //! for every item calls function(item_id, point_t).
    //--------------------------------------------------------------------------
    template <typename Function>
    void for_each_item(Function&& function) const {
        for (auto const& i : items_) {
            function(i.id(), i.pos());
        }
    }

    //--------------------------------------------------------------------------
    //! for each range of items at the same position calls
    //! function(p, i, n) where
    //! p = the position
    //! i = the first item at p
    //! n = the number of items at p
    //--------------------------------------------------------------------------
    template <typename Function>
    void for_each_stack(Function&& function) const {
        auto const beg = std::begin(items_);
        auto const end = std::end(items_);

        auto range = std::make_pair(beg, end);

        for (auto it = beg; it != end; it = range.second) {
            auto const i = it->id();
            auto const p = it->pos();

            range = std::equal_range(it, end, p);

            function(p, i, std::distance(range.first, range.second));
        }
    }

    bool remove_item_at(point_t const p, item_id const itm) {
        auto const beg   = std::begin(items_);
        auto const end   = std::end(items_);
        auto const range = std::equal_range(beg, end, p);

        auto const it = std::find_if(range.first, range.second, [&](record_t const& r) {
            return r.id() == itm;
        });

        if (it == end) {
            return false;
        }

        items_.erase(it);
        return true;
    }

    template <typename Function>
    bool remove_nth_item_at_and(point_t const p, int const n, Function&& function) {
        return with_nth_at_(p, n, [&](auto const& it) {
            function(it->id());
            items_.erase(it);
        });
    }

    template <typename Function>
    int remove_items_at_and(point_t const p, Function&& function) {
        auto const range = bkrl::equal_range(items_, p);

        bkrl::for_each(range, [&](record_t const& r) {
            function(r.id());
        });

        auto const n = std::distance(range.first, range.second);
        items_.erase(range.first, range.second);
        return n;
    }

    template <typename Function>
    bool with_nth_at(point_t const p, int const n, Function&& function) const {
        return with_nth_at_(p, n, [&](auto const& it) {
            function(it->id());
        });
    }

    int count_items_at(point_t const p) const {
        auto const range = bkrl::equal_range(items_, p);
        return std::distance(range.first, range.second);
    }
private:
    template <typename Function>
    bool with_nth_at_(point_t const p, int const n, Function&& function) const {
        auto  range = bkrl::equal_range(items_, p);
        auto& it    = range.first;

        if (!advance_(it, range.second, n)) {
            return false;
        }

        function(it);

        return true;
    }

    template <typename It>
    static bool advance_(It& it, It const& end, int const n) {
        if (n < 0 || std::distance(it, end) <= n) {
            return false;
        }

        std::advance(it, n);

        return true;
    }

    void insert_(point_t const p, item_id const itm) {
        items_.push_back(record_t {
            {itm, p}
        });
    }

    std::vector<record_t> items_;
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
    static path_string_ref tile_filename();
    static tex_point_i tile_size();

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
    optional<item_id> unequip(item_id id, defs_t defs, items_t items);

    optional<item_id> in_slot(equip_slot slot) const;

    optional<item_id> match_any(equip_slot_flags flags) const;

    item_collection list() const;
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

item_definition const& get_definition(
    item_id                 id
  , item_definitions const& defs
  , item_store       const& store
);

item_locale const& get_locale(
    item_id                 id
  , item_definitions const& defs
  , item_store       const& store
);

string_ref get_localized_name(
    item_id                 id
  , item_definitions const& defs
  , item_store       const& store
);

inline string_ref get_localized_name(
    item_locale const& locale
) {
    return locale.name;
}

////////////////////////////////////////////////////////////////////////////////
} //namespace bkrl
////////////////////////////////////////////////////////////////////////////////
