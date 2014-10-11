#pragma once

#include <boost/predef.h>

#define BK_DO_NOTHING(x) ((void)x)

#if BOOST_COMP_MSVC
#   define BK_NORETURN __declspec(noreturn)
#   define BK_THREADLOCAL thread_local
#else
#   define BK_NORETURN [[noreturn]]
#   define BK_THREADLOCAL thread_local
#endif

#if BOOST_COMP_MSVC
#   define BK_DEBUG_BREAK() __debugbreak()
#else
#   define BK_DEBUG_BREAK() BK_DO_NOTHING(0)
#endif

#define BK_TODO_FAIL() BK_DEBUG_BREAK(); std::terminate()

#define BK_NOCOPY(name)     \
name(name const&) = delete; \
name& operator=(name const&) = delete

#define BK_NOMOVE(name) \
name(name&&) = delete;  \
name& operator=(name&&) = delete

#define BK_DEFMOVE(name) \
name(name&&) = default;  \
name& operator=(name&&) = default

#define BK_DEFCOPY(name) \
name(name const&) = default;  \
name& operator=(name const&) = default
