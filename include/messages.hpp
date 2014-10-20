//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Messages and prompts.
//##############################################################################
#pragma once

#include <memory>
#include "enum_map.hpp"
#include "types.hpp"

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

  , enum_size
};

extern template class enum_map<message_type>;

class message_map {
public:
    message_map();
     ~message_map();

    void load(json::cref data);

    string_ref operator[](message_type msg) const;

    void set_locale(lang_id lang);
private:
    class impl_t;
    std::unique_ptr<impl_t> impl_;
};

} //namespace bkrl
