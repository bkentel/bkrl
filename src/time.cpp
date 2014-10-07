#include "time.hpp"
#include "algorithm.hpp"

using namespace bkrl;

////////////////////////////////////////////////////////////////////////////////

class timer::impl_t {
public:
    using clock_t      = timer::clock_t;
    using time_point_t = timer::time_point_t;
    using duration_t   = timer::duration_t;
    using id_t         = timer::id_t;
    using callback_t   = timer::callback_t;

    struct record_t {
        time_point_t start;
        time_point_t deadline;
        unsigned     index;
        id_t         id; //TODO this "could" wrap around.
    };

    impl_t()
      : last_ {clock_t::now()}
    {
    }

    void update();

    id_t add_timer(duration_t period, callback_t&& callback);

    void remove_timer(id_t id);
private:
    void sort_() {
        bkrl::sort(deadlines_, [](record_t const& lhs, record_t const& rhs) {
            return lhs.deadline < rhs.deadline;
        });
    }

    time_point_t last_;
    unsigned     next_id_ = 0;

    std::vector<callback_t> callbacks_;
    std::vector<record_t>   deadlines_;
};

////////////////////////////////////////////////////////////////////////////////

void timer::impl_t::update() {
    static auto const max  = time_point_t::max();
    static auto const zero = duration_t::zero();

    auto const now = clock_t::now();
    last_ = now;

    //for each timer
    for (size_t i = 0; i < deadlines_.size(); ++i) {
        auto& cur = deadlines_[i];

        if (cur.deadline > now) {
            break; //nothing left
        }

        auto const delta  = now - cur.start;
        auto const result = callbacks_[cur.index](cur.id, delta);
            
        cur.deadline = (result != zero) //if 0 was not returned
            ? (now + result)              //then update the deadline
            : (max);                      //otherwise set the deadline to max
    }
        
    //all deadlines set to max will be at the back
    sort_();
       
    //remove dead timers
    while (!deadlines_.empty()) {
        auto const back = deadlines_.back();
        if (back.deadline != max) {
            break;
        }

        deadlines_.pop_back();

        //and also remove the associated callbacks
        callbacks_.erase(std::begin(callbacks_) + back.index);
    }
}

timer::id_t timer::impl_t::add_timer(duration_t const period, callback_t&& callback) {
    auto const id = id_t {++next_id_};
        
    callbacks_.emplace_back(std::move(callback));        
    deadlines_.emplace_back(record_t {
        last_
        , last_ + period
        , callbacks_.size() - 1
        , id
    });

    sort_();

    return id;
}

void timer::impl_t::remove_timer(id_t const id) {
    auto const it = bkrl::lower_bound(deadlines_, id, [](auto const& lhs, auto const& rhs) {
        return lhs.id < rhs;
    });

    if (it == std::cend(deadlines_)) {
        BK_TODO_FAIL();
    }

    auto const index = it->index;

    deadlines_.erase(it);
    callbacks_.erase(std::begin(callbacks_) + index);
}

////////////////////////////////////////////////////////////////////////////////

timer::timer()
  : impl_ {std::make_unique<impl_t>()}
{
}

timer::~timer() = default;

void timer::update() {
    impl_->update();
}

timer::id_t timer::add_timer(duration_t const period, callback_t callback) {
    return impl_->add_timer(period, std::move(callback));
}

void timer::remove_timer(id_t const id) {
    return impl_->remove_timer(id);
}
