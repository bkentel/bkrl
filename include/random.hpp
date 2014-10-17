//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! (Pseudo) random number generation.
//##############################################################################
#pragma once

#include <random>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/normal_distribution.hpp>

#include "math.hpp"

namespace bkrl {
namespace random {

using normal_dist = boost::normal_distribution<>;

inline uint32_t true_random() {
    return std::random_device {}();
}

class generator : public boost::mt19937 {
public:
    explicit generator(uint32_t const seed)
      : boost::mt19937 {seed}
      , seed_ {seed}
    {
    }

    uint32_t get_seed() const noexcept {
        return seed_;
    }
private:
    uint32_t seed_;
};

class uniform_int {
public:
    template <typename T>
    T generate(generator& gen, T const lo, T const hi) const {
        return boost::random::uniform_int_distribution<T> {lo, hi}(gen);
    }
};

template <typename T>
inline T uniform_range(generator& gen, T const lo, T const hi) {
    return uniform_int{}.generate(gen, lo, hi);
}

template <typename T>
inline T uniform_range(generator& gen, range<T> const r) {
    return uniform_range(gen, r.lo, r.hi);
}

template <typename T = int>
inline vector2d<T> direction(generator& gen) {
    constexpr auto min = T {-1};
    constexpr auto max = T { 1};

    auto const dx = uniform_range<T>(gen, min, max);
    auto const dy = uniform_range<T>(gen, min, max);

    return {dx, dy};
}

template <typename T = int>
inline T percent(generator& gen) {
    constexpr auto min = T {0};
    constexpr auto max = T {100};

    return uniform_range<T>(gen, min, max);
}

template <typename T>
inline T roll_dice(generator& gen, T const count, T const sides, T const mod) {
    auto result = mod;
    
    for (auto i = T {0}; i < count; ++i) {
        result += uniform_range(gen, T {1}, sides);
    }

    return result;
}

struct random_dist {
    enum dist_type : int {
        none, constant, uniform, dice, normal
    };

    void clear() {
        type = none;
    }

    void set_constant(int const value) {
        type = constant;
        data.constant.value = value;
    }

    void set_uniform(int const lo, int const hi) {
        BK_ASSERT_DBG(lo <= hi);

        type = uniform;
        data.uniform.lo = lo;
        data.uniform.hi = hi;
    }

    void set_dice(int const count, int const sides, int const mod) {
        BK_ASSERT_DBG(count > 0);
        BK_ASSERT_DBG(sides > 0);

        type = dice;
        data.dice.count = count;
        data.dice.sides = sides;
        data.dice.mod   = mod;
    }

    void set_normal(double const mean, double const sigma, int const min, int const max) {
        BK_ASSERT_DBG(min <= max);

        type = normal;
        data.normal.min = min;
        data.normal.max = max;
        new (&data.normal.dist) normal_dist {mean, sigma};
    }

    int operator()(generator& gen) const {
        switch (type) {
        default :
        case dist_type::none :
            break;
        case dist_type::constant : return data.constant.value;
        case dist_type::uniform  : return uniform_range(gen, data.uniform.lo, data.uniform.hi);
        case dist_type::dice     : return roll_dice(gen, data.dice.count, data.dice.sides, data.dice.mod);
        case dist_type::normal   :
            return clamp(
                static_cast<int>(
                    std::round(
                        data.normal.dist(gen)
                    )
                )
                , data.normal.min
                , data.normal.max
            );
        }

        BK_TODO_FAIL();
    }

    explicit operator bool() const {
        return type != none;
    }

    dist_type type = none;

#pragma warning( disable : 4582 )
    union union_t {
        int dummy;

        struct constant_t {
            int value;
        } constant;

        struct uniform_t {
            int lo;
            int hi;
        } uniform;

        struct dice_t {
            int count;
            int sides;
            int mod;
        } dice;

        struct normal_t {
            int                 min;
            int                 max;
            mutable normal_dist dist;
        } normal;

        union_t() : dummy (0) {}
    } data;
#pragma warning( default : 4582 )
};

using range_generator = std::function<unsigned (unsigned lo, unsigned hi)>;

} //namespace random
} //namespace bkrl
