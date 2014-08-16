#pragma once

#include <cassert>

namespace bkrl {

#define BK_TODO_FAIL() __debugbreak(); ::terminate()
#define BK_ASSERT(x) assert(x)

} //namespace bkrl
