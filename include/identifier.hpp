//##############################################################################
//! @file
//! @author Brandon Kentel
//! @todo copyright / licence
//##############################################################################
#pragma once

#include <type_traits>
#include "integers.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////
template <typename Tag, typename T = uint32_t> class tagged_id;
////////////////////////////////////////////////////////////////////////////////

//==============================================================================
//! Get the value out of a tagged_id<>
//==============================================================================
template <typename Tag, typename T>
T id_to_value(tagged_id<Tag, T> const id) noexcept {
    return id.value_;
}

//==============================================================================
//! A simple wrapper around an integral type to tag it as a unique type.
//==============================================================================
template <typename Tag, typename T>
class tagged_id {
    static_assert(std::is_integral<T>::value, "Integer types required.");

    friend T id_to_value<Tag, T>(tagged_id) noexcept;
public:
    using value_type = T;

    tagged_id() noexcept : value_ {0} {}
    explicit tagged_id(T const value) noexcept : value_ {value} {}

    bool operator< (tagged_id const rhs) const noexcept { return value_ <  rhs.value_; }
    bool operator==(tagged_id const rhs) const noexcept { return value_ == rhs.value_; }
    bool operator!=(tagged_id const rhs) const noexcept { return value_ != rhs.value_; }
private:
    T value_;
};

namespace detail {
    struct tag_item_def;
    struct tag_item_instance;
    struct tag_entity_def;
    struct tag_entity_instance;
    struct tag_loot_table_def;
    struct tag_spawn_table_def;
    struct tag_language;
} //namespace detail

using item_def_id        = tagged_id<detail::tag_item_def>;
using item_id            = tagged_id<detail::tag_item_instance>;
using entity_def_id      = tagged_id<detail::tag_entity_def>;
using entity_id          = tagged_id<detail::tag_entity_instance>;
using loot_table_def_id  = tagged_id<detail::tag_loot_table_def>;
using spawn_table_def_id = tagged_id<detail::tag_spawn_table_def>;
using lang_id            = tagged_id<detail::tag_language>;

////////////////////////////////////////////////////////////////////////////////
} //namespace bkrl
////////////////////////////////////////////////////////////////////////////////
