//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Keyboard and key bindings.
//##############################################################################
#pragma once

#include <bitset>
#include <memory>

#include "scancode.hpp"
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

extern template key_modifier_type from_hash(hash_t hash);

//==============================================================================
//! key_modifier
//==============================================================================
struct key_modifier {
    void set(key_modifier_type const mod) noexcept {
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

    bool test(key_modifier const& other) const noexcept {
        auto const a = value.to_ulong();
        auto const b = other.value.to_ulong();

        return (!a && !b)
            || ((a & b) && ((a & b) == b));
    }

    bool test(key_modifier_type const mod) const noexcept {
        auto const m = static_cast<unsigned>(mod);

        switch (mod) {
        case key_modifier_type::ctrl :
            return test(key_modifier_type::ctrl_left)
                || test(key_modifier_type::ctrl_right);
        case key_modifier_type::alt :
            return test(key_modifier_type::alt_left)
                || test(key_modifier_type::alt_right);
        case key_modifier_type::shift :
            return test(key_modifier_type::shift_left)
                || test(key_modifier_type::shift_right);
        default :
            break;
        }

        return value.test(m);
    }

    void clear(key_modifier_type const mod) noexcept {
        auto const m = static_cast<unsigned>(mod);

        switch (mod) {
        case key_modifier_type::ctrl :
            clear(key_modifier_type::ctrl_left);
            clear(key_modifier_type::ctrl_right);
            break;
        case key_modifier_type::alt :
            clear(key_modifier_type::alt_left);
            clear(key_modifier_type::alt_right);
            break;
        case key_modifier_type::shift :
            clear(key_modifier_type::shift_left);
            clear(key_modifier_type::shift_right);
            break;
        default :
            value.reset(m);
        }
    }

    std::bitset<
        static_cast<unsigned>(key_modifier_type::enum_size) - 1
    > value;
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
