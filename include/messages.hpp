//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Messages and prompts.
//##############################################################################
#pragma once

#include <memory>

#include "types.hpp"
#include "identifier.hpp"
#include "hash.hpp"

namespace bkrl {

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

  , wield_wear_conflict
  , wield_wear_nothing
  , wield_wear_ok

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

} //namespace bkrl
