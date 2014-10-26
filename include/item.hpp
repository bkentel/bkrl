#pragma once

#include <vector>
#include <bitset>

#include "types.hpp"
#include "util.hpp"
#include "random.hpp"
#include "algorithm.hpp"

#include "identifier.hpp"

namespace bkrl {

namespace detail { class item_definitions_impl; }

enum class equip_slot {
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

using equip_slot_flags = std::bitset<static_cast<size_t>(equip_slot::enum_size) - 1>;

//==============================================================================
//!
//==============================================================================
class item_definitions {
public:
    using dist_t = random::random_dist;

    struct definition {
        string_id id;
        equip_slot_flags slot_flags;
        int       max_stack;
        dist_t    damage_min;
        dist_t    damage_max;
    };

    struct locale {
        utf8string name;
        utf8string sort;
        utf8string text;
    };

    item_definitions();
    ~item_definitions();

    void load_definitions(json::cref data);
    void load_locale(json::cref data);

    definition const& get_definition(identifier id) const;
    locale     const& get_locale(identifier id) const;

    void set_locale(lang_id lang);

    int definitions_size() const;
    definition const& get_definition_at(int index) const;
private:
    std::unique_ptr<detail::item_definitions_impl> impl_;
};

//==============================================================================
//!
//==============================================================================
class item {
public:
    using defs_t = item_definitions const&;

    bool operator==(item const& other) const {
        return id.hash == other.id.hash;
    }

    bool operator!=(item const& other) const {
        return !(*this == other);
    }

    bool can_stack(defs_t defs) const {
        return max_stack(defs) > 1;
    }

    int max_stack(defs_t defs) const {
        return defs.get_definition(id).max_stack;
    }

    string_ref name(defs_t defs) const {
        return defs.get_locale(id).name;
    }

    string_ref sort_string(defs_t defs) const {
        auto const& sort = defs.get_locale(id).sort;

        if (sort.empty()) {
            return name(defs);
        }

        return sort;
    }

    identifier id;
    int        count = 1;
    int        damage_min = 0;
    int        damage_max = 0;
};

//==============================================================================
//!
//==============================================================================
class loot_table {
};

//==============================================================================
//!
//==============================================================================
item generate_item(
    random::generator&      gen
  , item_definitions const& defs
  , loot_table       const& table
);

//==============================================================================
//!
//==============================================================================
item generate_item(
    random::generator& gen
  , item_definitions::definition const& def
);

//==============================================================================
//!
//==============================================================================
class item_stack {
public:
    BK_NOCOPY(item_stack);
    BK_DEFMOVE(item_stack);
    item_stack() = default;

    using defs_t = item_definitions const&;

    auto begin()        { return items_.begin(); }
    auto begin()  const { return items_.begin(); }
    auto cbegin() const { return items_.cbegin(); }

    auto end()          { return items_.end(); }
    auto end()    const { return items_.end(); }
    auto cend()   const { return items_.cend(); }

    int size()    const { return static_cast<int>(items_.size()); }

    bool empty()  const { return items_.empty(); }

    void insert(item&& itm, defs_t defs) {
        //first try to increase an existing stack, then make a new stack
        if (!insert_stack_(itm, defs)) {
            insert_new_(std::move(itm), defs);
        }
    }

    void merge(item_stack&& other, defs_t defs) {
        for (auto&& itm : other) {
            insert(std::move(itm), defs);
        }
    }

    item remove(int index, int n = 0);
private:
    bool insert_stack_(item const& itm, defs_t defs);

    void insert_new_(item&& itm, defs_t defs);

    void sort_(defs_t defs);

    //TODO could use some sort of small buffer optimization
    std::vector<item> items_;
};

//==============================================================================
//!
//==============================================================================
class equipment {
public:
    using defs_t = item_definitions const&;
    using idef_t = item_definitions::definition const&;

    bool can_equip(item const& itm, idef_t idef) const;

    bool can_equip(item const& itm, defs_t defs) const {
        return can_equip(itm, defs.get_definition(itm.id));
    }

    void equip(item&& itm, idef_t idef);

    void equip(item&& itm, defs_t defs) {
        equip(std::move(itm), defs.get_definition(itm.id));
    }

    item unequip(equip_slot slot, defs_t defs);

    optional<item const&> in_slot(equip_slot slot, defs_t defs) const;
private:
    enum { equip_size = static_cast<size_t>(equip_slot::enum_size) - 1 };

    equip_slot_flags flags_;
    boost::container::static_vector<item, equip_size> items_;
};

} //namespace bkrl
