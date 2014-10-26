#include "items.hpp"

#include <unordered_map>

#include "assert.hpp"
#include "algorithm.hpp"
#include "json.hpp"

namespace jc = bkrl::json::common;

////////////////////////////////////////////////////////////////////////////////
namespace std {
////////////////////////////////////////////////////////////////////////////////
    template <> struct hash<bkrl::item_id> {
        inline size_t operator()(bkrl::item_id const x) const noexcept {
            return hash<bkrl::item_id::value_type>{}(bkrl::id_to_value(x));
        }
    };
////////////////////////////////////////////////////////////////////////////////
} //namespace std
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
namespace {
////////////////////////////////////////////////////////////////////////////////
template <typename T>
std::vector<T>& merge(std::vector<T>& lhs, std::vector<T>& rhs) {
    BK_ASSERT(&lhs != &rhs);

    auto const left = lhs.capacity() >= rhs.capacity();
    auto& dst = left ? lhs : rhs;
    auto& src = left ? rhs : lhs;

    std::move(std::begin(src), std::end(src), std::back_inserter(dst));

    src.clear();

    return dst;
}

template <typename T, typename K>
static auto& require_at(T& container, K const& key) {
    auto const it = container.find(key);

    BK_ASSERT(it != std::end(container));

    return it->second;
}

template <typename T>
void call_destructor(T& object) noexcept {
    (void)object;
    object.~T();
}

template <typename T>
void placement_move(T& dst, T& src) {
    new (&dst) T {std::move(src)};
}

////////////////////////////////////////////////////////////////////////////////
} //namespace
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// item_stack
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void bkrl::item_stack::insert(item_id const id) {
    items_.emplace_back(id);
    
    bkrl::sort(items_);
}

//------------------------------------------------------------------------------
void bkrl::item_stack::insert(item_stack&& other) {
    auto& result =  merge(items_, other.items_);
    if (&result != &items_) {
        items_ = std::move(result);
    }

    bkrl::sort(items_);
}

//------------------------------------------------------------------------------
bkrl::item_id
bkrl::item_stack::remove(item_id const id) {
    auto const it = std::lower_bound(std::begin(items_), std::end(items_), id);
    BK_ASSERT(it != std::end(items_));

    items_.erase(it);

    return id;
}

//------------------------------------------------------------------------------
bkrl::item_id
bkrl::item_stack::remove(int const index) {
    auto it = std::begin(items_);
    std::advance(it, index);

    auto const id = *it;

    items_.erase(it);

    return id;
}

////////////////////////////////////////////////////////////////////////////////
// item
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
bkrl::item::item(item&& other)
    : id     {std::move(other.id)}
    , origin {std::move(other.origin)}
  //, data   {must init the union}
    , type   {std::move(other.type)}
{
    auto& dst = data;
    auto& src = other.data;

    using it = item_type;

    switch (type) {
    default            : BK_TODO_FAIL();
    case it::none      : break;
    case it::container : placement_move(dst.stack,  src.stack);  break;
    case it::weapon    : placement_move(dst.weapon, src.weapon); break;
    case it::armor     : placement_move(dst.armor,  src.armor);  break;
    }
}

//------------------------------------------------------------------------------
bkrl::item::~item() {
    using it = item_type;

    switch (type) {
    default            : BK_TODO_FAIL();
    case it::none      : break;
    case it::container : call_destructor(data.stack);  break;
    case it::weapon    : call_destructor(data.weapon); break;
    case it::armor     : call_destructor(data.armor);  break;
    }
}

////////////////////////////////////////////////////////////////////////////////
// item_store
////////////////////////////////////////////////////////////////////////////////
class bkrl::detail::item_store_impl {
public:
    using rvalue          = item&&;
    using reference       = item&;
    using const_reference = item const&;
    using key             = item_id; 

    key insert(rvalue value) {
        auto const size = static_cast<key::value_type>(data_.size());
        auto const result_key = key {size};

        auto const result = data_.insert(std::make_pair(result_key, std::move(value)));
        BK_ASSERT(result.second);

        return result_key;
    }

    void remove(key) {
    }

    reference       operator[](key const id)       { return require_at(data_, id); }
    const_reference operator[](key const id) const { return require_at(data_, id); }
private:
    std::unordered_map<key, item> data_;
};

//------------------------------------------------------------------------------
bkrl::item_store::item_store()
  : impl_ {std::make_unique<detail::item_store_impl>()}
{
}

//------------------------------------------------------------------------------
bkrl::item_store::~item_store() = default;

//------------------------------------------------------------------------------
bkrl::item_id
bkrl::item_store::insert(rvalue value) {
    return impl_->insert(std::move(value));
}

//------------------------------------------------------------------------------
void bkrl::item_store::remove(item_id id) {
    impl_->remove(id);
}

//------------------------------------------------------------------------------
bkrl::item_store::reference
bkrl::item_store::operator[](item_id id) {
    return (*impl_)[id];
}

//------------------------------------------------------------------------------
bkrl::item_store::const_reference
bkrl::item_store::operator[](item_id id) const {
    return (*impl_)[id];
}

////////////////////////////////////////////////////////////////////////////////
// item_definition
////////////////////////////////////////////////////////////////////////////////
struct bkrl::item_definition {
    using dist_t = random::random_dist;

    item_def_id id;
    utf8string  id_string;

    //equip_slot_flags slot_flags;

    int       max_stack;
    dist_t    damage_min;
    dist_t    damage_max;  
};

////////////////////////////////////////////////////////////////////////////////
// item_definitions
////////////////////////////////////////////////////////////////////////////////

//==============================================================================
//! detail::item_definitions_impl
//==============================================================================
class bkrl::detail::item_definitions_impl {
public:
    using definition = item_definition;
    using locale     = item_locale;
    using cref = bkrl::json::cref;  

    ////////////////////////////////////////////////////////////////////////////
    item_definitions_impl() {
    }

    definition const& get_definition(item_def_id const id) const {
        auto const it = definitions_.find(id);
        if (it == std::end(definitions_)) {
            BK_TODO_FAIL();
        }

        return it->second;
    }
    
    locale const& get_locale(item_def_id const id) const {
        auto const it = current_locale_->find(id);
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

        definitions_.emplace(cur_def_.id, std::move(cur_def_));
    }

    void rule_def_id(cref value) {
        auto const str = json::require_string(value[jc::field_id]);
        if (str.length() < 1) {
            BK_TODO_FAIL();
        }

        cur_def_.id = item_def_id {bkrl::slash_hash32(str)};
        cur_def_.id_string = str.to_string();
    }

    void rule_def_stack(cref value) {
        auto const stack = json::default_int(value[jc::field_stack], 1);
        if (stack < 1) {
            BK_TODO_FAIL();
        }

        cur_def_.max_stack = stack;
    }

    void rule_def_damage(cref value) {
        using dist_t = definition::dist_t;

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
        //static string_ref const field_slots {"slot"};

        //auto& flags = cur_def_.slot_flags;
        //flags.reset();

        //if (!json::has_field(value, field_slots)) {
        //    return;
        //}

        //static hashed_string_ref const none {"none"};
        //static hashed_string_ref const head {"head"};
        //static hashed_string_ref const arms_upper {"arms_upper"};
        //static hashed_string_ref const arms_lower {"arms_lower"};
        //static hashed_string_ref const hands {"hands"};
        //static hashed_string_ref const chest {"chest"};
        //static hashed_string_ref const waist {"waist"};
        //static hashed_string_ref const legs_upper {"legs_upper"};
        //static hashed_string_ref const legs_lower {"legs_lower"};
        //static hashed_string_ref const feet {"feet"};
        //static hashed_string_ref const finger_left {"finger_left"};
        //static hashed_string_ref const finger_right {"finger_right"};
        //static hashed_string_ref const neck {"neck"};
        //static hashed_string_ref const back {"back"};
        //static hashed_string_ref const hand_main {"hand_main"};
        //static hashed_string_ref const hand_off {"hand_off"};
        //static hashed_string_ref const ammo {"ammo"};

        //auto const slots = json::require_array(value[field_slots], 1);

        //for (auto const& slot : slots.array_items()) {
        //    hashed_string_ref const s = json::require_string(slot);

        //    if (s == none) {
        //        BK_ASSERT_DBG(slots.array_items().size() == 1);
        //    }
        //    else if (s == head) { flags.set(enum_value(equip_slot::head)); }
        //    else if (s == arms_upper) { flags.set(enum_value(equip_slot::arms_upper)); }
        //    else if (s == arms_lower) { flags.set(enum_value(equip_slot::arms_lower)); }
        //    else if (s == hands) { flags.set(enum_value(equip_slot::hands)); }
        //    else if (s == chest) { flags.set(enum_value(equip_slot::chest)); }
        //    else if (s == waist) { flags.set(enum_value(equip_slot::waist)); }
        //    else if (s == legs_upper) { flags.set(enum_value(equip_slot::legs_upper)); }
        //    else if (s == legs_lower) { flags.set(enum_value(equip_slot::legs_lower)); }
        //    else if (s == feet) { flags.set(enum_value(equip_slot::feet)); }
        //    else if (s == finger_left) { flags.set(enum_value(equip_slot::finger_left)); }
        //    else if (s == finger_right) { flags.set(enum_value(equip_slot::finger_right)); }
        //    else if (s == neck) { flags.set(enum_value(equip_slot::neck)); }
        //    else if (s == back) { flags.set(enum_value(equip_slot::back)); }
        //    else if (s == hand_main) { flags.set(enum_value(equip_slot::hand_main)); }
        //    else if (s == hand_off) { flags.set(enum_value(equip_slot::hand_off)); }
        //    else if (s == ammo) { flags.set(enum_value(equip_slot::ammo)); }
        //    else {
        //        BK_TODO_FAIL();
        //    }
        //}

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
        
        auto const sort = json::optional_string(value[jc::field_sort]);
        if (sort) {
            assign(cur_loc_.sort, *sort);
        } else {
            cur_loc_.sort.clear();
        }

        cur_loc_map_.emplace(id.hash, std::move(cur_loc_));
    }

    ////////////////////////////////////////////////////////////////////////////
private:
    template <typename K, typename V>
    using map_t = boost::container::flat_map<K, V, std::less<>>;

    using locale_map = map_t<item_def_id, locale>;

    definition cur_def_;
    locale     cur_loc_;
    lang_id    cur_lang_;
    locale_map cur_loc_map_;

    map_t<item_def_id, definition> definitions_;
    map_t<lang_id,     locale_map> locales_;

    locale_map const* current_locale_ = nullptr;
};

//------------------------------------------------------------------------------
bkrl::item_definitions::~item_definitions() = default;

//------------------------------------------------------------------------------
bkrl::item_definitions::item_definitions()
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
bkrl::item_definition const&
bkrl::item_definitions::get_definition(item_def_id const id) const {
    return impl_->get_definition(id);
}

//------------------------------------------------------------------------------
bkrl::item_locale const&
bkrl::item_definitions::get_locale(item_def_id const id) const {
    return impl_->get_locale(id);
}

//------------------------------------------------------------------------------
void bkrl::item_definitions::set_locale(lang_id const lang) {
    impl_->set_locale(lang);
}


////////////////////////////////////////////////////////////////////////////////
// generate_item
////////////////////////////////////////////////////////////////////////////////
bkrl::item_id
bkrl::generate_item(
    random_t&               gen
  , item_store&             store
  , item_definitions const& defs
  , loot_table       const& table
) {
    auto const id0 = table(gen);
    auto const id = item_def_id {slash_hash32("WEAPON_LONG_SWORD")};
    auto const& def = defs.get_definition(id);
    
    item itm;
    itm.id = id;;
    itm.type = item_type::weapon;
    itm.data.weapon.dmg_min  = 1;
    itm.data.weapon.dmg_max  = 10;
    itm.data.weapon.dmg_type = damage_type::slash;

    return store.insert(std::move(itm));
}
