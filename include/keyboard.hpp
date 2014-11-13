//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Keyboard and key bindings.
//##############################################################################
#pragma once

#include <bitset>
#include <memory>

//#include "types.hpp"
#include "scancode.hpp"
#include "enum_map.hpp"
#include "enum_forward.hpp"
#include "json_forward.hpp"

namespace bkrl {

enum class key_modifier_type : uint16_t {
    invalid

  , ctrl_left
  , ctrl_right
  , alt_left
  , alt_right
  , shift_left
  , shift_right

  , ctrl
  , alt
  , shift

  , enum_size
};

//==============================================================================
//! key_modifier
//==============================================================================
struct key_modifier {
    void set(key_modifier_type const mod) {
        auto const m = static_cast<unsigned>(mod);

        switch (mod) {
        case key_modifier_type::ctrl :
            set(key_modifier_type::ctrl_left);
            set(key_modifier_type::ctrl_right);
            break;
        case key_modifier_type::alt :
            set(key_modifier_type::alt_left);
            set(key_modifier_type::alt_right);
            break;
        case key_modifier_type::shift :
            set(key_modifier_type::shift_left);
            set(key_modifier_type::shift_right);
            break;
        default :
            value.set(m);
        }
    }

    bool test(key_modifier const& other) const {
        auto const a = value.to_ulong();
        auto const b = other.value.to_ulong();

        return (!a && !b)
            || ((a & b) && ((a & b) == b));
    }

    std::bitset<
        static_cast<unsigned>(key_modifier_type::enum_size) - 1
    > value;
};

extern template class enum_map<key_modifier_type>;

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
    BK_NOCOPY(keymap);

    keymap();
    keymap(keymap&&);
    keymap& operator=(keymap&&);
    ~keymap();

    explicit keymap(json::cref data);

    command_type operator[](key_combo const key) const;
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
