//##############################################################################
//! @file
//! @author Brandon Kentel
//! @todo copyright / licence
//##############################################################################
#pragma once

#include <cstdint>

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;

using std::size_t;

using hash_t = uint32_t;

template <typename T> struct min_max_value;

#define BK_DEF_MINMAX(ty, lo, hi)       \
template <> struct min_max_value<ty> { \
    enum : ty { min = lo, max = hi };   \
}

BK_DEF_MINMAX(int8_t,  INT8_MIN,  INT8_MAX );
BK_DEF_MINMAX(int16_t, INT16_MIN, INT16_MAX);
BK_DEF_MINMAX(int32_t, INT32_MIN, INT32_MAX);
BK_DEF_MINMAX(int64_t, INT64_MIN, INT64_MAX);

BK_DEF_MINMAX(uint8_t,  0, UINT8_MAX );
BK_DEF_MINMAX(uint16_t, 0, UINT16_MAX);
BK_DEF_MINMAX(uint32_t, 0, UINT32_MAX);
BK_DEF_MINMAX(uint64_t, 0, UINT64_MAX);

#undef BK_DEF_MINMAX

////////////////////////////////////////////////////////////////////////////////
} //namespace bkrl
////////////////////////////////////////////////////////////////////////////////
