#pragma once

namespace bkrl {

template <typename It>
class iterable {
public:
    iterable() = default;

    iterable(It first, It last)
      : first_ {first}, last_ {last}
    {
    }

    It begin() const { return first_; }
    It end()   const { return last_; }
private:
    It first_, last_;
};

template <typename It>
inline iterable<It> make_iterable(It a, It b) {
    return {a, b};
}

} //namespace bkrl
