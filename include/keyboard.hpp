//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Keyboard and key bindings.
//##############################################################################
#pragma once

#include "types.hpp"
#include "scancode.hpp"
#include "command_type.hpp"

namespace bkrl {

//==============================================================================
//! key_modifier
//==============================================================================
struct key_modifier {
    enum : unsigned {
        ctrl_left
      , ctrl_right
      , alt_left
      , alt_right
      , shift_left
      , shift_right

      , enum_size
    };

    std::bitset<enum_size> value;
};

//==============================================================================
//! key_combo
//==============================================================================
struct key_combo {
    scancode     key;
    key_modifier modifier;
};

//==============================================================================
//! key_mapping
//==============================================================================
struct key_mapping {
    key_combo     keys;
    command_type  command;
};

//==============================================================================
//! keymap
//==============================================================================
class keymap {
public:
    keymap();
    explicit keymap(string_ref filename);
    ~keymap();

    command_type operator[](key_combo const key);
private:
    class impl_t;
    std::unique_ptr<impl_t> impl_;
};

////////////////////////////////////////////////////////////////////////////////
inline bool operator==(key_modifier const lhs, key_modifier const rhs) noexcept {
    return lhs.value.to_ulong() == rhs.value.to_ulong();
}

inline bool operator==(key_combo const lhs, key_combo const rhs) {
    return (lhs.key == rhs.key) && (lhs.modifier == rhs.modifier);
}

inline bool operator!=(key_combo const lhs, key_combo const rhs) {
    return !(lhs == rhs);
}

inline bool operator<(key_modifier const lhs, key_modifier const rhs) noexcept {
    return lhs.value.to_ulong() < rhs.value.to_ulong();
}

inline bool operator<(key_combo const lhs, key_combo const rhs) noexcept {
    return (lhs.key < rhs.key)
        || ((lhs.key == rhs.key) && (lhs.modifier < rhs.modifier));
}



} //namespace bkrl
