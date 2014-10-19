#pragma once

#include "types.hpp"
#include "util.hpp"
#include "locale.hpp"
#include "random.hpp"

#include "algorithm.hpp" //TODO temp
#include <vector> //TODO temp

namespace bkrl {

namespace detail { class item_definitions_impl; }

//==============================================================================
//!
//==============================================================================
class item_definitions {
public:
    using dist_t = random::random_dist;

    struct definition {
        string_id id;
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
class item_def : public definition_base<item_def> {
public:
    using dist_t = random::random_dist;

    struct locale {
        utf8string name;
        utf8string sort;
        utf8string text;
    };

    static locale const& undefined();

    string_id id;
    int       stack;
    dist_t    damage_min;
    dist_t    damage_max;
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
        auto const sort = defs.get_locale(id).sort;
        return sort.empty() ? name(defs) : sort;
    }

    identifier id;
    int        count = 1;
    int        damage_min = 0;
    int        damage_max = 0;
};

class loot_table {
};

item generate_item(
    random::generator&      gen
  , item_definitions const& defs
  , loot_table       const& table
);

class item_stack {
public:
    BK_NOCOPY(item_stack);
    BK_DEFMOVE(item_stack);
    item_stack() = default;

    using defs_t = item_definitions const&;

    auto begin()       { return items_.begin(); }
    auto begin() const { return items_.begin(); }

    auto end()       { return items_.end(); }
    auto end() const { return items_.end(); }

    auto cbegin() const { return items_.cbegin(); }
    auto cend()   const { return items_.cend(); }

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
private:
    bool insert_stack_(item const& itm, defs_t defs) {
        //make sure we have a stackable item first
        if (!itm.can_stack(defs)) {
            return false;
        }
    
        auto const end = std::end(items_);
        auto const beg = std::begin(items_);

        //find a matching item with enough spare stack
        auto const it = std::find_if(beg, end,
            [&](item const& other) {
                return (itm == other)
                    && (other.count < other.max_stack(defs));
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

    void insert_new_(item&& itm, defs_t defs) {
        items_.emplace_back(std::move(itm));
        sort_(defs);
    }

    void sort_(defs_t defs) {
        bkrl::sort(items_, [&](item const& lhs, item const& rhs) {
            return lhs.sort_string(defs) < rhs.sort_string(defs);
        });
    }

    std::vector<item> items_;
};

} //namespace bkrl
