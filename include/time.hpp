#pragma once

#include <chrono>
#include <functional>
#include <memory>

#include "util.hpp"

namespace bkrl {

class timer {
public:
    using clock_t      = std::chrono::high_resolution_clock;
    using time_point_t = clock_t::time_point;
    using duration_t   = time_point_t::duration;
    using id_t         = tagged_type<int, timer>;

    //! A return value of 0 indicates that the timer should be removed.
    using callback_t = std::function<duration_t (id_t id, duration_t delta)>;

    timer();
    ~timer();

    void update();

    id_t add_timer(duration_t period, callback_t callback);

    void remove_timer(id_t id);
private:
    class impl_t;
    std::unique_ptr<impl_t> impl_;
};

} //namespace bkrl
