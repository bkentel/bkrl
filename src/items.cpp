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

namespace bkrl {
template <typename Enum>
struct hashed_enum_string {
    hashed_enum_string(string_ref const str, Enum const val)
      : string {str}
      , hash   {slash_hash32(str)}
      , value  {val}
    {
    }

    string_ref string;
    hash_t     hash;
    Enum       value;
};

//TODO make constexpr
template <typename T, size_t N>
inline size_t sizeof_array(T const (&)[N]) noexcept {
    return N;
}

template <> bkrl::item_type
bkrl::from_string(string_ref const str) {
    using es_t = hashed_enum_string<item_type> const;

    static es_t types[] = {
        {"none",      item_type::none}
      , {"weapon",    item_type::weapon}
      , {"armor",     item_type::armor}
      , {"scroll",    item_type::scroll}
      , {"potion",    item_type::potion}
      , {"container", item_type::container}
    };

    auto const hash = slash_hash32(str);

    auto const sz  = sizeof_array(types);
    auto const beg = types;
    auto const end = types + sz;
    
    BK_ASSERT(sz == enum_value(item_type::enum_size));
    
    auto const it  = std::find_if(beg, end, [&](es_t const& x) {
        return hash == x.hash;
    });

    return (it != end) ? it->value : item_type::none;
}

template <> bkrl::equip_slot
bkrl::from_string(string_ref const str) {
    using es_t = hashed_enum_string<equip_slot> const;

    static es_t slots[] = {
        {"none",         equip_slot::none}
      , {"head",         equip_slot::head}
      , {"arms_upper",   equip_slot::arms_upper}
      , {"arms_lower",   equip_slot::arms_lower}
      , {"hands",        equip_slot::hands}
      , {"chest",        equip_slot::chest}
      , {"waist",        equip_slot::waist}
      , {"legs_upper",   equip_slot::legs_upper}
      , {"legs_lower",   equip_slot::legs_lower}
      , {"feet",         equip_slot::feet}
      , {"finger_left",  equip_slot::finger_left}
      , {"finger_right", equip_slot::finger_right}
      , {"neck",         equip_slot::neck}
      , {"back",         equip_slot::back}
      , {"hand_main",    equip_slot::hand_main}
      , {"hand_off",     equip_slot::hand_off}
      , {"ammo",         equip_slot::ammo}
    };

    auto const hash = slash_hash32(str);

    auto const sz  = sizeof_array(slots);
    auto const beg = slots;
    auto const end = slots + sz;
    
    BK_ASSERT(sz == enum_value(equip_slot::enum_size));
    
    auto const it  = std::find_if(beg, end, [&](es_t const& x) {
        return hash == x.hash;
    });

    return (it != end) ? it->value : equip_slot::none;
}

}

////////////////////////////////////////////////////////////////////////////////
// item_stack
////////////////////////////////////////////////////////////////////////////////
namespace {

template <typename T>
static void sort_items(
    T& container
  , bkrl::item_definitions const& defs
  , bkrl::item_store const& items
) {
    bkrl::sort(container, [&](bkrl::item_id const lhs, bkrl::item_id const rhs) {
        auto const& lloc = get_item_locale(lhs, defs, items);
        auto const& rloc = get_item_locale(rhs, defs, items);

        auto const& left  = (!lloc.sort.empty() ? lloc.sort : lloc.name);
        auto const& right = (!rloc.sort.empty() ? rloc.sort : rloc.name);

        return left < right;
    });
}

}

//------------------------------------------------------------------------------
void bkrl::item_stack::insert(item_id const id, defs_t defs, items_t items) {
    items_.emplace_back(id);
    
    sort_items(items_, defs, items);
}

//------------------------------------------------------------------------------
void bkrl::item_stack::insert(item_stack&& other, defs_t defs, items_t items) {
    auto& result =  merge(items_, other.items_);
    if (&result != &items_) {
        items_ = std::move(result);
    }

    sort_items(items_, defs, items);
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
    case it::potion    : placement_move(dst.potion, src.potion); break;
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
    case it::potion    : call_destructor(data.potion); break;
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
        auto const result_key = key {next_id_++};

        auto const result = data_.insert(std::make_pair(result_key, std::move(value)));
        BK_ASSERT(result.second);

        return result_key;
    }

    void remove(key) {
    }

    reference       operator[](key const id)       { return require_at(data_, id); }
    const_reference operator[](key const id) const { return require_at(data_, id); }
private:
    uint32_t next_id_ = 0x80000000;
    std::unordered_map<key, item> data_;
};

//------------------------------------------------------------------------------
bkrl::item_store::item_store()
  : impl_ {std::make_unique<detail::item_store_impl>()}
{
}

//------------------------------------------------------------------------------
bkrl::item_store::~item_store() = default;
bkrl::item_store::item_store(item_store&&) = default;

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

    equip_slot_flags slots;

    int max_stack;

    dist_t damage_min;
    dist_t damage_max;  

    item_type type;
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

    static utf8string const undefined_name;
    static utf8string const undefined_sort;
    static utf8string const undefined_text;
    static locale     const undefined_locale;

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
            return undefined_locale;
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

    auto get_definitions_size() const {
        return static_cast<int>(definitions_.size());
    }
    
    auto const& get_definition_at(int const index) const {
        BK_ASSERT(index < get_definitions_size());

        auto it = definitions_.begin();
        std::advance(it, index);
        return it->second;
    }

    ////////////////////////////////////////////////////////////////////////////
    void load_definitions(cref data) {
        rule_def_root(data);
    }
    
    //--------------------------------------------------------------------------
    void rule_def_root(cref value) {
        json::require_object(value);

        rule_def_filetype(value);
        rule_def_definitions(value);
    }

    //--------------------------------------------------------------------------
    void rule_def_filetype(cref value) {
        jc::get_filetype(value, jc::filetype_item);
    }

    //--------------------------------------------------------------------------
    void rule_def_definitions(cref value) {
        auto const& defs  = json::require_array(value[jc::field_definitions]);
        auto const& array = defs.array_items();

        definitions_.reserve(array.size());
        
        for (auto&& def : array) {
            rule_def_definition(def);
        }
    }

    //--------------------------------------------------------------------------
    void rule_def_definition(cref value) {
        rule_def_id(value);
        rule_def_type(value);

        definitions_.emplace(cur_def_.id, std::move(cur_def_));
    }

    //--------------------------------------------------------------------------
    void rule_def_id(cref value) {
        auto const str = json::require_string(value[jc::field_id]);
        if (str.length() < 1) {
            BK_TODO_FAIL();
        }

        cur_def_.id = item_def_id {bkrl::slash_hash32(str)};
        cur_def_.id_string = str.to_string();
    }

    //--------------------------------------------------------------------------
    void rule_def_type(cref value) {
        auto const& str  = json::require_string(value[jc::field_type]);
        auto const  type = from_string<item_type>(str);

        cur_def_.type = type;

        using it = item_type;

        switch (type) {
        default : //fall through
        case it::none   : BK_TODO_FAIL();         break;
        case it::weapon : rule_def_weapon(value); break;
        case it::armor  : rule_def_armor(value);  break;
        case it::potion : rule_def_potion(value); break;
        }
    }

    //--------------------------------------------------------------------------
    void rule_def_weapon(cref value) {
    }

    //--------------------------------------------------------------------------
    void rule_def_armor(cref value) {
    }

    //--------------------------------------------------------------------------
    void rule_def_potion(cref value) {
    }

    //--------------------------------------------------------------------------
    void rule_def_stack(cref value) {
        auto const stack = json::default_int(value[jc::field_stack], 1);
        if (stack < 1) {
            BK_TODO_FAIL();
        }

        cur_def_.max_stack = stack;
    }

    //--------------------------------------------------------------------------
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

    //--------------------------------------------------------------------------
    void rule_def_slots(cref value) {
        auto& flags = cur_def_.slots;
        flags.reset();

        auto const& slots = json::has_field(value, jc::field_slot);
        if (!slots) {
            return;
        }
       
        auto const& array = json::require_array(*slots, 1).array_items();

        for (auto const& slot_str : array) {
            auto const& str  = json::require_string(slot_str);
            auto const  slot = from_string<equip_slot>(str);
            auto const  flag = enum_value(slot);

            if (slot == equip_slot::none && array.size() != 1) {
                BK_TODO_FAIL();
            }

            if (flags.test(flag)) {
                BK_TODO_FAIL();
            }

            flags.set(flag);
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
        hashed_string_ref const id = json::require_string(value[jc::field_id]);
        
        assign(cur_loc_.name, json::default_string(value[jc::field_name], undefined_name));
        assign(cur_loc_.text, json::default_string(value[jc::field_text], undefined_text));
        
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

bkrl::utf8string const bkrl::detail::item_definitions_impl::undefined_name {"{undefined name}"};
bkrl::utf8string const bkrl::detail::item_definitions_impl::undefined_sort {"{undefined sort}"};
bkrl::utf8string const bkrl::detail::item_definitions_impl::undefined_text {"{undefined text}"};

bkrl::item_locale const bkrl::detail::item_definitions_impl::undefined_locale {
    undefined_name
  , undefined_sort
  , undefined_text
};

//------------------------------------------------------------------------------
bkrl::item_definitions::~item_definitions() = default;
bkrl::item_definitions::item_definitions(item_definitions&&) = default;

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

//------------------------------------------------------------------------------
int bkrl::item_definitions::get_definitions_size() const {
    return impl_->get_definitions_size();
}

//------------------------------------------------------------------------------
bkrl::item_definition const&
bkrl::item_definitions::get_definition_at(int const index) const {
    return impl_->get_definition_at(index);
}

////////////////////////////////////////////////////////////////////////////////
// generate_item
////////////////////////////////////////////////////////////////////////////////
bkrl::item_id
bkrl::generate_item(
    random_t&               gen
  , item_store&             store
  , item_definitions const& item_defs
  , loot_table       const& table
  , item_birthplace  const  origin
) {
    auto const size = item_defs.get_definitions_size();
    auto const i    = random::uniform_range(gen, 0, size - 1);

    auto const& def = item_defs.get_definition_at(i);
    
    item itm;
    itm.id     = def.id;;
    itm.type   = def.type;
    itm.origin = origin;

    return store.insert(std::move(itm));
}

#ifdef BK_TEST
bkrl::item_id
bkrl::generate_item(
    item_def_id             id
  , item_store&             store
  , item_definitions const& defs
) {
    auto const& def = defs.get_definition(id);
    
    item itm;
    itm.id = def.id;
    itm.type = def.type;

    return store.insert(std::move(itm));
}
#endif

////////////////////////////////////////////////////////////////////////////////
// equipment
////////////////////////////////////////////////////////////////////////////////
class bkrl::detail::equipment_impl {
public:
    using defs_t  = item_definitions const&;
    using items_t = item_store const&;

    bool can_equip(item_id const id, defs_t defs, items_t items) const {
        auto const& def = get_item_definition(id, defs, items);

        if (!def.slots.any() || (flags_ & def.slots).any()) {
            return false;
        }

        return true;
    }

    void equip(item_id const id, defs_t defs, items_t items) {
        if (!can_equip(id, defs, items)) {
            BK_TODO_FAIL();
        }

        auto const& def = get_item_definition(id, defs, items);

        flags_ |= def.slots;

        for (int i = 1; i < equip_size; ++i) {
            if (!def.slots.test(i)) {
                continue;
            }

            auto& slot = items_[i - 1];
            BK_ASSERT_DBG(slot == item_id {});
            slot = id;
        }
    }

    optional<item_id> in_slot(equip_slot const slot) const {
        if (!test_(slot)) {
            return {};
        }

        auto const i  = slot_to_index_(slot);
        auto const id = items_[i];

        return {id};
    }

    optional<item_id> unequip(equip_slot const slot, defs_t defs, items_t items) {
        auto const itm = in_slot(slot);
        if (!itm) {
            return itm;
        }

        auto const id = *itm;
        auto const& def = get_item_definition(id, defs, items);

        for (int i = 1; i < equip_size; ++i) {
            if (!def.slots.test(i)) {
                continue;
            }

            auto& s = items_[i - 1];
            BK_ASSERT_DBG(s == id);
            s = item_id {};
        }

        flags_ &= ~def.slots;

        return itm;
    }
private:
    static size_t slot_to_index_(equip_slot const slot) {
        auto const result = static_cast<size_t>(enum_value(slot) - 1);
        BK_ASSERT(result < equip_size);
        return result;
    }

    bool test_(equip_slot const slot) const {
        if (slot == equip_slot::invalid) {
            return false;
        }

        auto const result = flags_.test(enum_value(slot));

        BK_ASSERT(result
          ? (items_[slot_to_index_(slot)] != item_id {})
          : (items_[slot_to_index_(slot)] == item_id {})
        );

        return result;        
    }

    //TODO
    enum { equip_size = static_cast<size_t>(equip_slot::enum_size) - 1 };

    equip_slot_flags flags_;

    std::array<item_id, equip_size> items_;
};

bkrl::equipment::equipment()
  : impl_ {std::make_unique<detail::equipment_impl>()}
{
}

bkrl::equipment::~equipment() = default;
bkrl::equipment::equipment(equipment&&) = default;

bool bkrl::equipment::can_equip(item_id id, defs_t defs, items_t items) const {
    return impl_->can_equip(id, defs, items);
}

void bkrl::equipment::equip(item_id id, defs_t defs, items_t items) {
    impl_->equip(id, defs, items);
}

bkrl::optional<bkrl::item_id>
bkrl::equipment::unequip(equip_slot slot, defs_t defs, items_t items) {
    return impl_->unequip(slot, defs, items);
}

bkrl::optional<bkrl::item_id>
bkrl::equipment::in_slot(equip_slot const slot) const {
    return impl_->in_slot(slot);
}