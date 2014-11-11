#include "messages.hpp"
#include "json.hpp"

#include <boost/container/flat_map.hpp>

namespace jc = bkrl::json::common;

template <> bkrl::message_type
bkrl::from_hash(hash_t const hash) {
    using mapping_t = string_ref_mapping<message_type> const;
    using mt = message_type;

    static mapping_t mappings[] = {
        {"none",             mt::none}

      , {"welcome",          mt::welcome}
      , {"direction_prompt", mt::direction_prompt}
      , {"canceled",         mt::canceled}

      , {"door_no_door",     mt::door_no_door}
      , {"door_is_open",     mt::door_is_open}
      , {"door_is_closed",   mt::door_is_closed}
      , {"door_blocked",     mt::door_blocked}

      , {"stairs_no_stairs", mt::stairs_no_stairs}
      , {"stairs_no_down",   mt::stairs_no_down}
      , {"stairs_no_up",     mt::stairs_no_up}

      , {"get_no_items",     mt::get_no_items}
      , {"get_which_prompt", mt::get_which_prompt}
      , {"get_ok",           mt::get_ok}

      , {"drop_nothing",     mt::drop_nothing}
      , {"drop_ok",          mt::drop_ok}

      , {"attack_regular",   mt::attack_regular}
      , {"kill_regular",     mt::kill_regular}

      , {"title_inventory",  mt::title_inventory}
      , {"title_wield_wear", mt::title_wield_wear}
      , {"title_get",        mt::title_get}
      , {"title_drop",       mt::title_drop}
      , {"title_equipment",  mt::title_equipment}
      , {"title_take_off",   mt::title_take_off}

      , {"inventory_nothing", mt::inventory_nothing}

      , {"take_off_nothing", mt::take_off_nothing}
      , {"take_off_ok",      mt::take_off_ok}

      , {"wield_wear_conflict", mt::wield_wear_conflict}
      , {"wield_wear_nothing",  mt::wield_wear_nothing}
      , {"wield_wear_ok",       mt::wield_wear_ok}

      , {"header_items", mt::header_items}
      , {"header_slot",  mt::header_slot}

      , {"eqslot_head",         mt::eqslot_head}
      , {"eqslot_arms_upper",   mt::eqslot_arms_upper}
      , {"eqslot_arms_lower",   mt::eqslot_arms_lower}
      , {"eqslot_hands",        mt::eqslot_hands}
      , {"eqslot_chest",        mt::eqslot_chest}
      , {"eqslot_waist",        mt::eqslot_waist}
      , {"eqslot_legs_upper",   mt::eqslot_legs_upper}
      , {"eqslot_legs_lower",   mt::eqslot_legs_lower}
      , {"eqslot_feet",         mt::eqslot_feet}
      , {"eqslot_finger_left",  mt::eqslot_finger_left}
      , {"eqslot_finger_right", mt::eqslot_finger_right}
      , {"eqslot_neck",         mt::eqslot_neck}
      , {"eqslot_back",         mt::eqslot_back}
      , {"eqslot_hand_main",    mt::eqslot_hand_main}
      , {"eqslot_hand_off",     mt::eqslot_hand_off}
      , {"eqslot_ammo",         mt::eqslot_ammo}
    };

    return find_mapping(mappings, hash, mt::invalid);
}

////////////////////////////////////////////////////////////////////////////////
// item_stack
////////////////////////////////////////////////////////////////////////////////

class bkrl::detail::message_map_impl {
public:
    using cref = json::cref;

    //--------------------------------------------------------------------------
    void load(json::cref data) {
        rule_root(data);
    }

    //--------------------------------------------------------------------------
    void rule_root(cref value) {
        rule_file_type(value);
        rule_language(value);
        rule_definitions(value);
    }
    
    //--------------------------------------------------------------------------
    void rule_file_type(cref value) {
        jc::get_filetype(value, jc::filetype_locale);
    }
       
    //--------------------------------------------------------------------------
    void rule_language(cref value) {
        auto const locale = jc::get_locale(value, jc::filetype_messages);
        if (!locale) {
            BK_TODO_FAIL();
        }

        cur_lang_ = *locale;
    }
    
    //--------------------------------------------------------------------------
    void rule_definitions(cref value) {
        auto const defs = json::require_array(value[jc::field_definitions]);
        cur_loc_.reserve(enum_value(message_type::enum_size));

        for (auto const& def : defs.array_items()) {
            rule_definition(def);
        }

        locales_.emplace(cur_lang_, std::move(cur_loc_));
    }
    
    //--------------------------------------------------------------------------
    void rule_definition(cref value) {
        auto const def = json::require_array(value, 2, 2);
        
        auto const id  = json::require_string(def[0]);
        auto const str = json::require_string(def[1]);

        auto const hash = slash_hash32(id);
        auto const msg  = from_hash<message_type>(hash);

        if (msg == message_type::invalid) {
            BK_TODO_FAIL();
        }

        cur_loc_.emplace(msg, str.to_string());
    }

    //--------------------------------------------------------------------------
    string_ref operator[](message_type const msg) const {
        static string_ref const undefined {"<undefined message>"};

        auto const it = current_locale_->find(msg);
        if (it == std::cend(*current_locale_)) {
            return undefined;
        }

        return it->second;
    }

    //--------------------------------------------------------------------------
    void set_locale(lang_id const lang) {
        auto const it = locales_.find(lang);
        if (it == std::end(locales_)) {
            BK_TODO_FAIL();
        }

        current_locale_ = &it->second;
    }
private:
    template <typename K, typename V>
    using map_t = boost::container::flat_map<K, V, std::less<>>;
    using locale_map = map_t<message_type, utf8string>;
    
    lang_id    cur_lang_;
    locale_map cur_loc_;

    map_t<lang_id, locale_map> locales_;

    locale_map const* current_locale_ = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
// message_map
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
bkrl::message_map::~message_map() = default;

//------------------------------------------------------------------------------
bkrl::message_map::message_map()
  : impl_ {std::make_unique<detail::message_map_impl>()}
{
}

//------------------------------------------------------------------------------
void
bkrl::message_map::load(json::cref value) {
    impl_->load(value);
}

//------------------------------------------------------------------------------
bkrl::string_ref
bkrl::message_map::operator[](message_type const msg) const {
    return (*impl_)[msg];
}

//------------------------------------------------------------------------------
void
bkrl::message_map::set_locale(lang_id const lang) {
    impl_->set_locale(lang);
}
