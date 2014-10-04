#include <atomic>
#include <cstdlib>

#include "assert.hpp"

using namespace bkrl;

namespace {

void abort_handler(assert_info const&) {
    std::abort();
}

std::atomic<assert_handler>   global_handler {abort_handler};
BK_THREADLOCAL assert_handler local_handler  {nullptr};

}

assert_handler bkrl::set_assert_handler(assert_handler handler) noexcept {
    if (handler) {
        return std::atomic_exchange(&global_handler, handler);
    } else {
        return std::atomic_exchange(&global_handler, abort_handler);
    }
}

assert_handler bkrl::get_assert_handler() noexcept {
    return std::atomic_load(&global_handler);
}

BK_NORETURN void bkrl::on_assert_failure(assert_info const& info) {
    auto handler = local_handler;

    if (handler) {
        handler(info);
    }


    handler = get_assert_handler();
    if (handler) {
        handler(info);
    }

    std::abort();
}
