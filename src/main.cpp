#include "assert.hpp"
#include "engine_client.hpp"


template <typename T>
struct range_restrict_check {
    range_restrict_check(T const min, T const max)
        : min {min}, max {max} {}

    bool operator()(T const value) const {
        return value >= min && value <= max;
    }

    T min;
    T max;
};

struct range_restrict_fail_assert {
    template <typename T, typename Check>
    T operator()(T const value, Check&& check) const {
        BK_ASSERT(check(value));
        return value;
    }

};

template <typename T, T Min, T Max, typename OnFail>
struct range_restrict {
    template <typename U>
    range_restrict(U const value)
      : value_ {
          OnFail {}(value, range_restrict_check<T> {Min, Max})
      }
    {
    }

    operator T() const { return value_; }

    T value_;
};

using integral_percentage = range_restrict<int, 0, 100, range_restrict_fail_assert>;

class engine_server {
};


int main(int, char**) {
    integral_percentage percent = 100;

    bkrl::engine_client client;

    return 0;
}
