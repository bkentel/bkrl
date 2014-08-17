#pragma once
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace bkrl {
namespace random {

class generator : public boost::mt19937 {
public:
    using boost::mt19937::mersenne_twister_engine;
};

class uniform_int {
public:
    template <typename T>
    T generate(generator& gen, T const lo, T const hi) const {
        return boost::random::uniform_int_distribution<T> {lo, hi}(gen);
    }
};

inline unsigned uniform_range(generator& gen, unsigned lo, unsigned hi) {
    return uniform_int{}.generate(gen, lo, hi);
}

using range_generator = std::function<unsigned (unsigned lo, unsigned hi)>;

} //namespace random
} //namespace bkrl
