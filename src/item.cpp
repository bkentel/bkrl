#include "item.hpp"
#include "json.hpp"

#include <boost/container/flat_map.hpp>

namespace jc = bkrl::json::common;

//==============================================================================
//!
//==============================================================================
class bkrl::detail::item_definitions_impl {
public:
    using definition = item_definitions::definition;
    using locale     = item_definitions::locale;
    using cref = bkrl::json::cref;  

    item_definitions_impl() {
    }

    ////////////////////////////////////////////////////////////////////////////
    void load_definitions(cref data) {
        rule_def_root(data);
    }
    
    void rule_def_root(cref value) {
        json::require_object(value);

        rule_def_filetype(value);
        rule_def_definitions(value);
    }

    void rule_def_filetype(cref value) {
        jc::get_filetype(value, jc::filetype_item);
    }

    void rule_def_definitions(cref value) {
        auto const& defs  = json::require_array(value[jc::field_definitions]);
        auto const& array = defs.array_items();

        definitions_.reserve(array.size());
        
        for (auto&& def : array) {
            rule_def_definition(def);
        }
    }

    void rule_def_definition(cref value) {
        rule_def_id(value);
        rule_def_stack(value);
        rule_def_damage(value);
        rule_def_slots(value);

        definitions_.emplace(cur_def_.id.hash, std::move(cur_def_));
    }

    void rule_def_id(cref value) {
        auto const str = json::require_string(value[jc::field_id]);
        if (str.length() < 1) {
            BK_TODO_FAIL();
        }

        cur_def_.id = str;
    }

    void rule_def_stack(cref value) {
        auto const stack = json::default_int(value[jc::field_stack], 1);
        if (stack < 1) {
            BK_TODO_FAIL();
        }

        cur_def_.max_stack = stack;
    }

    void rule_def_damage(cref value) {
        using dist_t = item_definitions::dist_t;

        auto const dmg_min = value[jc::field_damage_min];
        auto const dmg_max = value[jc::field_damage_max];

        auto const no_min = dmg_min.is_null();
        auto const no_max = dmg_max.is_null();

        if (no_min ^ no_max) {
            BK_TODO_FAIL();
        }

        auto const has_damage = !no_min && !no_max;

        cur_def_.damage_min = has_damage ? jc::get_random(dmg_min) : dist_t {};
        cur_def_.damage_max = has_damage ? jc::get_random(dmg_max) : dist_t {};
    }

    void rule_def_slots(cref value) {
        static string_ref const field_slots {"slot"};

        auto& flags = cur_def_.slot_flags;
        flags.reset();

        if (!json::has_field(value, field_slots)) {
            return;
        }

        static hashed_string_ref const none {"none"};
        static hashed_string_ref const head {"head"};
        static hashed_string_ref const arms_upper {"arms_upper"};
        static hashed_string_ref const arms_lower {"arms_lower"};
        static hashed_string_ref const hands {"hands"};
        static hashed_string_ref const chest {"chest"};
        static hashed_string_ref const waist {"waist"};
        static hashed_string_ref const legs_upper {"legs_upper"};
        static hashed_string_ref const legs_lower {"legs_lower"};
        static hashed_string_ref const feet {"feet"};
        static hashed_string_ref const finger_left {"finger_left"};
        static hashed_string_ref const finger_right {"finger_right"};
        static hashed_string_ref const neck {"neck"};
        static hashed_string_ref const back {"back"};
        static hashed_string_ref const hand_main {"hand_main"};
        static hashed_string_ref const hand_off {"hand_off"};
        static hashed_string_ref const ammo {"ammo"};

        auto const slots = json::require_array(value[field_slots], 1);

        for (auto const& slot : slots.array_items()) {
            hashed_string_ref const s = json::require_string(slot);

            if (s == none) {
                BK_ASSERT_DBG(slots.array_items().size() == 1);
            }
            else if (s == head) { flags.set(enum_value(equip_slot::head)); }
            else if (s == arms_upper) { flags.set(enum_value(equip_slot::arms_upper)); }
            else if (s == arms_lower) { flags.set(enum_value(equip_slot::arms_lower)); }
            else if (s == hands) { flags.set(enum_value(equip_slot::hands)); }
            else if (s == chest) { flags.set(enum_value(equip_slot::chest)); }
            else if (s == waist) { flags.set(enum_value(equip_slot::waist)); }
            else if (s == legs_upper) { flags.set(enum_value(equip_slot::legs_upper)); }
            else if (s == legs_lower) { flags.set(enum_value(equip_slot::legs_lower)); }
            else if (s == feet) { flags.set(enum_value(equip_slot::feet)); }
            else if (s == finger_left) { flags.set(enum_value(equip_slot::finger_left)); }
            else if (s == finger_right) { flags.set(enum_value(equip_slot::finger_right)); }
            else if (s == neck) { flags.set(enum_value(equip_slot::neck)); }
            else if (s == back) { flags.set(enum_value(equip_slot::back)); }
            else if (s == hand_main) { flags.set(enum_value(equip_slot::hand_main)); }
            else if (s == hand_off) { flags.set(enum_value(equip_slot::hand_off)); }
            else if (s == ammo) { flags.set(enum_value(equip_slot::ammo)); }
            else {
                BK_TODO_FAIL();
            }
        }

    }
    ////////////////////////////////////////////////////////////////////////////
    void load_locale(cref data) {
        rule_loc_root(data);
    }

    void rule_loc_root(cref value) {
        auto const locale = jc::get_locale(value, jc::filetype_item);
        if (!locale) {
            BK_TODO_FAIL();
        }
        
        cur_lang_ = *locale;

        rule_loc_definitions(value);
    }

    void rule_loc_definitions(cref value) {
        auto const& defs  = json::require_array(value[jc::field_definitions]);
        auto const& array = defs.array_items();

        cur_loc_map_.reserve(array.size());

        for (auto&& def : array) {
            rule_loc_definition(def);
        }

        locales_.emplace(cur_lang_, std::move(cur_loc_map_));
    }

    void rule_loc_definition(cref value) {
        static string_ref const undefined {"<undefined string>"};

        hashed_string_ref const id = json::require_string(value[jc::field_id]);
        
        assign(cur_loc_.name, json::default_string(value[jc::field_name], undefined));
        assign(cur_loc_.text, json::default_string(value[jc::field_text], undefined));
        assign(cur_loc_.sort, json::default_string(value[jc::field_sort], ""));

        cur_loc_map_.emplace(id.hash, std::move(cur_loc_));
    }

    ////////////////////////////////////////////////////////////////////////////
    definition const& get_definition(identifier const id) const {
        auto const it = definitions_.find(id.hash);
        if (it == std::end(definitions_)) {
            BK_TODO_FAIL();
        }

        return it->second;
    }
    
    locale const& get_locale(identifier const id) const {
        auto const it = current_locale_->find(id.hash);
        if (it == std::end(*current_locale_)) {
            BK_TODO_FAIL();
        }

        return it->second;
    }

    void set_locale(lang_id const lang) {
        auto const it = locales_.find(lang);
        if (it == std::end(locales_)) {
            BK_TODO_FAIL();
        }

        current_locale_ = &it->second;
    }

    int definitions_size() const {
        return static_cast<int>(definitions_.size());
    }
    
    definition const& get_definition_at(int const index) const {
        BK_ASSERT(index < definitions_size());

        auto it = definitions_.begin();
        std::advance(it, index);
        return it->second;
    }
private:
    template <typename K, typename V>
    using map_t = boost::container::flat_map<K, V, std::less<>>;

    using locale_map = map_t<hash_t, locale>;

    item_definitions::definition cur_def_;
    item_definitions::locale     cur_loc_;
    lang_id                      cur_lang_;
    locale_map                   cur_loc_map_;

    map_t<hash_t,  definition> definitions_;
    map_t<lang_id, locale_map> locales_;

    locale_map const* current_locale_ = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
// item_definitions
////////////////////////////////////////////////////////////////////////////////
using bkrl::item_definitions;

//------------------------------------------------------------------------------
item_definitions::~item_definitions() = default;

//------------------------------------------------------------------------------
item_definitions::item_definitions()
  : impl_ {std::make_unique<detail::item_definitions_impl>()}
{
}

//------------------------------------------------------------------------------
void
bkrl::item_definitions::load_definitions(json::cref data) {
    impl_->load_definitions(data);
}

//------------------------------------------------------------------------------
void
bkrl::item_definitions::load_locale(json::cref data) {
    impl_->load_locale(data);
}

//------------------------------------------------------------------------------
item_definitions::definition const&
bkrl::item_definitions::get_definition(identifier const id) const {
    return impl_->get_definition(id);
}

//------------------------------------------------------------------------------
item_definitions::locale const&
bkrl::item_definitions::get_locale(identifier const id) const {
    return impl_->get_locale(id);
}

//------------------------------------------------------------------------------
void bkrl::item_definitions::set_locale(lang_id const lang) {
    impl_->set_locale(lang);
}

//------------------------------------------------------------------------------
int item_definitions::definitions_size() const {
    return impl_->definitions_size();
}

//------------------------------------------------------------------------------
item_definitions::definition const&
item_definitions::get_definition_at(int const index) const {
    return impl_->get_definition_at(index);
}

////////////////////////////////////////////////////////////////////////////////
// generate_item
////////////////////////////////////////////////////////////////////////////////
bkrl::item
bkrl::generate_item(
    random::generator&      gen
  , item_definitions const& defs
  , loot_table       const& table
) {
    auto const size = defs.definitions_size();
    auto const i    = random::uniform_range(gen, 0, size - 1);

    return generate_item(gen, defs.get_definition_at(i));
}

//------------------------------------------------------------------------------
bkrl::item
bkrl::generate_item(
    random::generator& gen
  , item_definitions::definition const& def
) {
    item result;

    result.id = def.id;

    auto const has_dmg_min = !!def.damage_min;
    auto const has_dmg_max = !!def.damage_max;

    BK_ASSERT_DBG(
         (has_dmg_min &&  has_dmg_max)
     || (!has_dmg_min && !has_dmg_max)
    );

    if (has_dmg_min && has_dmg_max) {
        result.damage_min = def.damage_min(gen);
        result.damage_max = def.damage_max(gen);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
// item_stack
////////////////////////////////////////////////////////////////////////////////
bkrl::item
bkrl::item_stack::remove(
    int const index
  , int const n
) {
    BK_ASSERT_DBG(index < size());
        
    item result = std::move(items_[index]);
        
    auto where = std::begin(items_);
    std::advance(where, index);
    items_.erase(where);

    return result;
}

//------------------------------------------------------------------------------
bool
bkrl::item_stack::insert_stack_(
    item const& itm
  , defs_t      defs
) {
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

//------------------------------------------------------------------------------
void
bkrl::item_stack::insert_new_(item&& itm, defs_t defs) {
    items_.emplace_back(std::move(itm));
    sort_(defs);
}

//------------------------------------------------------------------------------
void
bkrl::item_stack::sort_(defs_t defs) {
    bkrl::sort(items_, [&](item const& lhs, item const& rhs) {
        return lhs.sort_string(defs) < rhs.sort_string(defs);
    });
}

////////////////////////////////////////////////////////////////////////////////
// equipment
////////////////////////////////////////////////////////////////////////////////

bool
bkrl::equipment::can_equip(item const& itm, idef_t idef) const {
    auto const& item_flags = idef.slot_flags;

    if (item_flags.any() && (item_flags & flags_) == 0) {
        return true;
    }

    return false;
}

void
bkrl::equipment::equip(item&& itm, idef_t idef) {
    BK_ASSERT_DBG(can_equip(itm, idef));

    flags_ |= idef.slot_flags;

    items_.emplace_back(std::move(itm));
}

bkrl::item
bkrl::equipment::unequip(equip_slot const slot, defs_t defs) {
    auto const flag = enum_value(slot);

    for (auto it = std::begin(items_); it != std::end(items_); ++it) {
        auto const& idef = defs.get_definition(it->id);
        if (idef.slot_flags.test(flag)) {
            item result = std::move(*it);
                
            items_.erase(it);
            flags_ &= ~idef.slot_flags;

            return result;
        }
    }

    BK_TODO_FAIL();
}

bkrl::optional<bkrl::item const&>
bkrl::equipment::in_slot(equip_slot const slot, defs_t defs) const {
    auto const flag = enum_value(slot);

    for (auto&& itm : items_) {
        auto const& idef = defs.get_definition(itm.id);
        if (idef.slot_flags.test(flag)) {
            return optional<item const&> {itm};
        }
    }

    return optional<item const&> {};
}
