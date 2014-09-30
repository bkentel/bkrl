//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Assertion and debug macros.
//##############################################################################
#include <boost/predef.h>

#if BOOST_COMP_MSVC
#define BK_NORETURN __declspec(noreturn)
#define BK_THREADLOCAL __declspec(thread)
#else
#define BK_NORETURN [[noreturn]]
#define BK_THREADLOCAL thread_local
#endif

#define BK_NOCOPY(name)     \
name(name const&) = delete; \
name& operator=(name const&) = delete

#define BK_NOMOVE(name) \
name(name&&) = delete;  \
name& operator=(name&&) = delete

#define BK_DEFAULT_MOVE(name) \
name(name&&) = default;  \
name& operator=(name&&) = default

#define BK_DEFAULT_COPY(name) \
name(name const&) = default;  \
name& operator=(name const&) = default

#undef BK_ASSERT_OPT_IS_ACTIVE
#undef BK_ASSERT_DBG_IS_ACTIVE
#undef BK_ASSERT_SAFE_IS_ACTIVE
#undef BK_ASSERT_IS_ACTIVE

#undef BK_ASSERT_OPT
#undef BK_ASSERT_DBG
#undef BK_ASSERT_SAFE
#undef BK_ASSERT

#if !defined(BK_ASSERT_LEVEL_NONE) \
 && !defined(BK_ASSERT_LEVEL_OPT)  \
 && !defined(BK_ASSERT_LEVEL_DBG)  \
 && !defined(BK_ASSERT_LEVEL_SAFE)
#   define BK_ASSERT_LEVEL_DBG
#elif 1 != \
    defined(BK_ASSERT_LEVEL_NONE) \
  + defined(BK_ASSERT_LEVEL_OPT)  \
  + defined(BK_ASSERT_LEVEL_DBG)  \
  + defined(BK_ASSERT_LEVEL_SAFE)
#   error "Cannot define more than one BK_ASSERT_LEVEL*"
#endif

#if defined(BK_ASSERT_LEVEL_OPT)
#   define BK_ASSERT_OPT_IS_ACTIVE
#elif defined(BK_ASSERT_LEVEL_DBG)
#   define BK_ASSERT_OPT_IS_ACTIVE
#   define BK_ASSERT_DBG_IS_ACTIVE
#   define BK_ASSERT_IS_ACTIVE
#elif defined(BK_ASSERT_LEVEL_SAFE)
#   define BK_ASSERT_OPT_IS_ACTIVE
#   define BK_ASSERT_DBG_IS_ACTIVE
#   define BK_ASSERT_SAFE_IS_ACTIVE
#   define BK_ASSERT_IS_ACTIVE
#endif

#if defined(BK_ASSERT_OPT_IS_ACTIVE)
#   define BK_ASSERT_OPT(...) (void)((!!(__VA_ARGS__)) ||  \
    (bkrl::on_assert_failure(bkrl::assert_info {           \
        bkrl::assert_type::optimized                       \
      , #__VA_ARGS__                                       \
      , __func__                                           \
      , __FILE__                                           \
      , __LINE__                                           \
    }), 0))
#else
#   define BK_ASSERT_OPT(...) ((void)0)
#endif

#if defined(BK_ASSERT_DBG_IS_ACTIVE)
#   define BK_ASSERT_DBG(...) (void)((!!(__VA_ARGS__)) ||  \
    (bkrl::on_assert_failure(bkrl::assert_info {           \
        bkrl::assert_type::debug                           \
      , #__VA_ARGS__                                       \
      , __func__                                           \
      , __FILE__                                           \
      , __LINE__                                           \
    }), 0))
#else
#   define BK_ASSERT_DBG(...) ((void)0)
#endif

#if defined(BK_ASSERT_SAFE_IS_ACTIVE)
#   define BK_ASSERT_SAFE(...) (void)((!!(__VA_ARGS__)) || \
    (bkrl::on_assert_failure(bkrl::assert_info {           \
        bkrl::assert_type::safe                            \
      , #__VA_ARGS__                                       \
      , __func__                                           \
      , __FILE__                                           \
      , __LINE__                                           \
    }), 0))
#else
#   define BK_ASSERT_SAFE(...) ((void)0)
#endif

#define BK_ASSERT(...) BK_ASSERT_DBG(__VA_ARGS__)

#ifndef BK_ASSERT_H
#define BK_ASSERT_H

namespace bkrl {

enum class assert_type {
    optimized, debug, safe
};

struct assert_info {
    assert_type type;
    char const* expression;
    char const* function;
    char const* filename;
    size_t      line_number;
};

using assert_handler = void (*)(assert_info const&);

assert_handler set_assert_handler(assert_handler handler) noexcept;
assert_handler get_assert_handler() noexcept;
BK_NORETURN void on_assert_failure(assert_info const& info);

class assert_handler_guard {
public:
    assert_handler_guard(assert_handler_guard const&) = delete;
    assert_handler_guard& operator=(assert_handler_guard const&) = delete;
    assert_handler_guard(assert_handler_guard&&) = delete;
    assert_handler_guard& operator=(assert_handler_guard&&) = delete;

    explicit assert_handler_guard(assert_handler handler);
    ~assert_handler_guard();
private:
    assert_handler old_;
};

#define BK_TODO_FAIL() __debugbreak(); std::terminate()
#define BK_PRECONDITION(x) BK_ASSERT(x)

} //namespace bkrl

#endif
