#pragma once

#include "integers.hpp"
#include "hash.hpp"
#include "string.hpp"

namespace bkrl {

class message_map;

//==============================================================================
//! Damage type.
//==============================================================================
enum class damage_type : uint8_t {
    invalid

  , none = 0
  , slash
  , pierce
  , blunt
  , fire
  , cold
  , electric
  , acid

  , enum_size
};

extern template damage_type from_hash(hash_t hash);
string_ref to_string(message_map const& msgs, damage_type type); //TODO

struct damage_t {
    int16_t     value;
    damage_type type;
};

using defence_t = damage_t;
using health_t  = int16_t;

} //namespace bkrl
