#pragma once

template <typename F>
class on_scope_exit_t {
public:
    on_scope_exit_t() = delete;
    on_scope_exit_t(on_scope_exit_t const&) = delete;
    on_scope_exit_t(on_scope_exit_t&&) = delete;
    on_scope_exit_t& operator=(on_scope_exit_t const&) = delete;
    on_scope_exit_t& operator=(on_scope_exit_t&&) = delete;

    on_scope_exit_t(F& action) : f_ {action} {}
    ~on_scope_exit_t() { f_(); }
private:
    F& f_;
};

#define BK_OSE_TOKEN_PASTE2(x, y) x ## y
#define BK_OSE_TOKEN_PASTE(x, y) BK_OSE_TOKEN_PASTE2(x, y)
#define BK_OSE_IMPL1(lname, aname, ...) \
    auto lname = [&] { __VA_ARGS__; }; \
    ::on_scope_exit_t<decltype(lname)> aname {lname}

#define BK_OSE_IMPL2(ctr, ...) \
BK_OSE_IMPL1( BK_OSE_TOKEN_PASTE(on_scope_exit_func_, ctr) \
            , BK_OSE_TOKEN_PASTE(on_scope_exit_inst_, ctr) \
            , __VA_ARGS__ )

#define on_scope_exit(...) BK_OSE_IMPL2(__COUNTER__, __VA_ARGS__)
