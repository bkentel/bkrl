//##############################################################################
//! @author Brandon Kentel
//!
//! Assertion and debug macros.
//##############################################################################
#pragma once

#include <cassert>

namespace bkrl {

#define BK_TODO_FAIL() __debugbreak(); ::terminate()
#define BK_ASSERT(x) assert(x)
#define BK_PRECONDITION(x) BK_ASSERT(x)

} //namespace bkrl
