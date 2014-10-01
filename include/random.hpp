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

namespace bkrl {
namespace random {

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

using range_generator = std::function<unsigned (unsigned lo, unsigned hi)>;

} //namespace random
} //namespace bkrl
