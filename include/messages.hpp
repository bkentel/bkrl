//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Messages and prompts.
//##############################################################################
#pragma once

#include <memory>

#include "json_forward.hpp"
#include "identifier.hpp"
#include "hash.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////

enum class message_type : uint16_t {
    invalid = 0
  
  , none = 0

  , welcome
  , direction_prompt
  , canceled

  , door_no_door
  , door_is_open
  , door_is_closed
  , door_blocked

  , stairs_no_stairs
  , stairs_no_down
  , stairs_no_up

  , get_no_items
  , get_which_prompt
  , get_ok

  , drop_nothing
  , drop_ok

  , attack_regular
  , kill_regular

  , title_inventory
  , title_wield_wear
  , title_get
  , title_drop
  , title_equipment
  , title_take_off

  , inventory_nothing
  
  , take_off_nothing
  , take_off_ok

  , wield_wear_conflict
  , wield_wear_nothing
  , wield_wear_ok

  , header_items
  , header_slot

  , eqslot_head
  , eqslot_arms_upper
  , eqslot_arms_lower
  , eqslot_hands
  , eqslot_chest
  , eqslot_waist
  , eqslot_legs_upper
  , eqslot_legs_lower
  , eqslot_feet
  , eqslot_finger_left
  , eqslot_finger_right
  , eqslot_neck
  , eqslot_back
  , eqslot_hand_main
  , eqslot_hand_off
  , eqslot_ammo

  , enum_size
};

extern template message_type from_hash(hash_t hash);

namespace detail { class message_map_impl; }

class message_map {
public:
    message_map();
    ~message_map();

    void load(json::cref data);

    string_ref operator[](message_type msg) const;

    void set_locale(lang_id lang);
private:
    std::unique_ptr<detail::message_map_impl> impl_;
};

////////////////////////////////////////////////////////////////////////////////
} //namespace bkrl
////////////////////////////////////////////////////////////////////////////////
