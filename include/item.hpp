#pragma once

#include "types.hpp"
#include "util.hpp"
#include "locale.hpp"
#include "random.hpp"

#include "algorithm.hpp" //TODO temp
#include <vector> //TODO temp

namespace bkrl {

//class item_material_def;
//class item_color_def;
//class item_type_def;
//class item_tag_def;
//class item_def;
//
//template <typename T> class definition;
//
////==============================================================================
////==============================================================================
//class item_material_def : definition_base<item_material_def> {
//public:
//    struct locale {
//        utf8string name;
//        utf8string text;
//    };
//
//    static definition_t load_definitions(utf8string const& data);
//    static localized_t  load_localized_strings(utf8string const& data);
//
//    string_id id;
//    string_id color;
//
//    float     weight_mod;
//    float     value_mod;
//
//    std::vector<string_id> tags;
//};
//
////==============================================================================
////==============================================================================
//class item_color_def {
//public:
//    struct locale {
//        utf8string name;
//    };
//
//    string_id id;
//    //color_ref color;
//};

//==============================================================================
//!
//==============================================================================
class item_def : public definition_base<item_def> {
public:
    using dist_t = random::random_dist;

    struct locale {
        utf8string name;
        utf8string sort;
        utf8string text;
    };

    string_id id;
    int       stack;
    dist_t    damage_min;
    dist_t    damage_max;
};

item_def::definition_t load_items(json::cref data);
item_def::localized_t  load_items_locale(json::cref data);

//==============================================================================
//!
//==============================================================================
class item {
public:
    using localized_t  = item_def::localized_t;
    using definition_t = item_def::definition_t;

    //TODO make atomic
    static localized_t const* current_locale;

    explicit item(
        random::generator&  gen
      , definition_t const& defs
      , identifier   const  id
    )
      : id {id}
    {
        auto const& def = defs[id];
        
        if (!!def.damage_min) {
            BK_ASSERT_DBG(!!def.damage_max);

            damage_min = def.damage_min(gen);
            damage_max = def.damage_max(gen);
        }
    }

    bool operator<(item const& other) const {
        return sort_string() < other.sort_string();
    }

    bool operator==(item const& other) const {
        return id.hash == other.id.hash;
    }

    bool operator!=(item const& other) const {
        return !(*this == other);
    }

    bool can_stack(definition_t const& defs) const {
        return max_stack(defs) > 1;
    }

    int max_stack(definition_t const& defs) const {
        return defs[id].stack;
    }

    string_ref name(localized_t const& locale) const {
        return locale[id].name;
    }

    string_ref sort_string() const {
        return (*current_locale)[id].sort;
    }

    identifier id;
    int        count = 1;
    int        damage_min = 0;
    int        damage_max = 0;
};

class item_stack {
public:
    BK_NOCOPY(item_stack);
    BK_DEFMOVE(item_stack);
    item_stack() = default;

    using cref = item_def::definition_t const&;

    void insert(item&& itm, cref item_defs) {
        //first try to increase an existing stack, then make a new stack
        if (!insert_stack_(itm, item_defs)) {
            insert_new_(std::move(itm));
        }
    }

    item remove(int index, int n = 0) {
        BK_ASSERT_DBG(index < size());
        
        item result = std::move(items_[index]);
        
        auto where = std::begin(items_);
        std::advance(where, index);
        items_.erase(where);

        return result;
    }

    int size() const {
        return items_.size();
    }

    bool empty() const {
        return items_.empty();
    }

    auto begin()       { return items_.begin(); }
    auto begin() const { return items_.begin(); }

    auto end()       { return items_.end(); }
    auto end() const { return items_.end(); }

    auto cbegin() const { return items_.cbegin(); }
    auto cend()   const { return items_.cend(); }
private:
    bool insert_stack_(item const& itm, cref item_defs) {
        //make sure we have a stackable item first
        if (!itm.can_stack(item_defs)) {
            return false;
        }
    
        auto const end = std::end(items_);
        auto const beg = std::begin(items_);

        //find a matching item with enough spare stack
        auto const it = std::find_if(beg, end,
            [&](item const& other) {
                return (itm == other)
                    && (other.count < other.max_stack(item_defs));
            }
        );

        //nothing found
        if (it == end) {
            return false;
        }

        //ok
        it->count += itm.count;
        return true;
    }

    void insert_new_(item&& itm) {
        items_.emplace_back(std::move(itm));
        bkrl::sort(items_);
    }

    std::vector<item> items_;
};

} //namespace bkrl
