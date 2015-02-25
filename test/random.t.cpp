#include "catch/catch.hpp"

#include "assert.hpp"
#include <boost/random.hpp>

#define BK_FORCE_INLINE __forceinline

namespace bkrl {

using random_t = boost::mt19937;

//------------------------------------------------------------------------------
template <int N> struct sized_int;

template <> struct sized_int<1> { using type = int8_t;  };
template <> struct sized_int<2> { using type = int16_t; };
template <> struct sized_int<4> { using type = int32_t; };
template <> struct sized_int<8> { using type = int64_t; };

//------------------------------------------------------------------------------
template <typename T>
inline auto float_to_ordered_int(T const f) noexcept {
    static_assert(std::is_floating_point<T>::value, "");

    constexpr auto size = sizeof(T);
    
    using int_t = sized_int<size>::type;
    
    constexpr auto bits  = size * 8;
    constexpr auto shift = int_t {1} << (bits - 1);

    auto const i = *reinterpret_cast<int_t const*>(&f);
    return (i < int_t {0}) ? (shift - i) : (i);
}

//------------------------------------------------------------------------------
template <int N = 4, typename T>
bool almost_equal_2s_complement(T const a, T const b) noexcept {
    //TODO should use a different value for (long) doubles.
    static_assert(N > 0 && N < 4 * 1024 * 1024, "");

    auto const ai = float_to_ordered_int(a);
    auto const bi = float_to_ordered_int(b);

    auto const delta = std::abs(ai - bi);

    return delta <= N;
}

//------------------------------------------------------------------------------
//! represents a rational number of the form num / den where den != 0.
//------------------------------------------------------------------------------
template <typename T = int>
struct rational {
    static_assert(std::is_integral<T>::value, "");

    rational(T const numerator, T const denominator) noexcept
      : num {numerator}
      , den {denominator}
    {
        BK_ASSERT(den != T {0});
    }

    template <typename F = double>
    F as_float() const noexcept {
        static_assert(std::is_floating_point<F>::value, "");
        return static_cast<F>(num) / static_cast<F>(den);
    }

    T num;
    T den;
};

//------------------------------------------------------------------------------
//! represents a proper fraction of the form num / den where |num| >= |den| and
//! den != 0
//------------------------------------------------------------------------------
template <typename T = int>
struct proper_fraction : public rational<T> {
    proper_fraction(T const numerator, T const denominator) noexcept
      : rational {numerator, denominator}
    {
        BK_ASSERT(std::abs(num) <= std::abs(den));
    }
};

//------------------------------------------------------------------------------
//! represents a proper fraction of the form num / den where num >= den > 0.
//------------------------------------------------------------------------------
template <typename T = int>
struct positive_proper_fraction : public rational<T> {
    positive_proper_fraction(T const numerator, T const denominator) noexcept
      : rational {numerator, denominator}
    {
        BK_ASSERT(num >= 0 && num <= den);
    }
};

//------------------------------------------------------------------------------
//! represents a percentage p where 0 <= p <= 100.
//------------------------------------------------------------------------------
template <typename T = int>
struct percentage {
    static_assert(std::is_integral<T>::value, "");

    explicit percentage(T const percent) noexcept
      : value {percent}
    {
        BK_ASSERT(percent >= T {0} && percent <= T {100});
    }

    template <typename F = double>
    F as_float() const noexcept {
        static_assert(std::is_floating_point<F>::value, "");
        return static_cast<F>(value) / F {100};
    }

    operator positive_proper_fraction<T>() const noexcept {
        return positive_proper_fraction<T>{value, T {100}};
    }

    bool operator< (percentage const rhs) const noexcept { return value <  rhs.value; }
    bool operator> (percentage const rhs) const noexcept { return value >  rhs.value; }
    bool operator<=(percentage const rhs) const noexcept { return value <= rhs.value; }
    bool operator>=(percentage const rhs) const noexcept { return value >= rhs.value; }
    bool operator==(percentage const rhs) const noexcept { return value == rhs.value; }
    bool operator!=(percentage const rhs) const noexcept { return value != rhs.value; }

    T value;
};

template <typename T>
bool operator<(rational<T> const lhs, rational<T> const rhs) noexcept {
    return lhs.as_float() < rhs.as_float();
}

template <typename T>
bool operator==(rational<T> const lhs, rational<T> const rhs) noexcept {
    return (lhs.den == rhs.den) && (lhs.num == rhs.den)
        || almost_equal_2s_complement(lhs.as_float(), rhs.as_float());
}

//------------------------------------------------------------------------------
template <typename T, typename U>
inline bool is_equal_tolerance(
    T const actual
  , U const expected
  , positive_proper_fraction<> const tolerance = positive_proper_fraction<> {1, 100}
) noexcept {
    auto const e = std::abs(expected - actual) /  std::abs(expected);
    auto const a = std::abs(expected)          * static_cast<double>(tolerance.num);
    auto const b = std::abs(expected - actual) * static_cast<double>(tolerance.den);

    return a >= b;
}

namespace detail {

//------------------------------------------------------------------------------
template <typename Head>
BK_FORCE_INLINE void
call_nth_element(size_t const n, Head&& head) {
    if (n == 0) {
        head();
    }
}

//------------------------------------------------------------------------------
template <typename Head, typename... Tail>
BK_FORCE_INLINE void
call_nth_element(size_t const n, Head&& head, Tail&&... tail) {
    if (n == 0) {
        head();
    } else {
        detail::call_nth_element(n - 1, std::forward<Tail>(tail)...);
    }
}

} //namespace detail

namespace random {

//------------------------------------------------------------------------------
template <typename Generator, typename F0, typename F1, typename... Fs>
BK_FORCE_INLINE void
do_one_of(Generator& gen, F0&& f0, F1&& f1, Fs&&... fs) {
    auto const n = 2 + sizeof...(Fs);
    auto const i = boost::uniform_smallint<size_t> {0, n - 1}(gen);

    detail::call_nth_element(
        i
      , std::forward<F0>(f0)
      , std::forward<F1>(f1)
      , std::forward<Fs>(fs)...
    );
}

//------------------------------------------------------------------------------
template <typename Generator, typename Pass, typename Fail>
BK_FORCE_INLINE void
do_chance(
    Generator& gen
  , positive_proper_fraction<> const chance
  , Pass&& pass
  , Fail&& fail
) {
    auto const roll = boost::uniform_smallint<> {0, chance.den}(gen);

    if (roll < chance.num) {
        pass();
    } else {
        fail();
    }
}

//------------------------------------------------------------------------------
template <typename Generator, typename Pass>
BK_FORCE_INLINE void
do_chance(
    Generator& gen
  , positive_proper_fraction<> const chance
  , Pass&& pass
) {
    do_chance(
        gen
      , chance
      , std::forward<Pass>(pass)
      , []{}
    );
}

//------------------------------------------------------------------------------
template <typename Generator, typename Pass, typename Fail>
BK_FORCE_INLINE void
do_percent(
    Generator& gen
  , percentage<> const chance
  , Pass&& pass
  , Fail&& fail
) {
    do_chance(
        gen
      , chance
      , std::forward<Pass>(pass)
      , std::forward<Fail>(fail)
    );
}

//------------------------------------------------------------------------------
template <typename Generator, typename Pass>
BK_FORCE_INLINE void
do_percent(
    Generator& gen
  , percentage<> const chance
  , Pass&& pass
) {
    do_percent(
        gen
      , chance
      , std::forward<Pass>(pass)
      , []{}
    );
}

//------------------------------------------------------------------------------
template <typename Generator, typename T>
T uniform(Generator& gen, T const lo, T const hi) {
    return boost::random::uniform_int_distribution<T> {lo, hi}(gen);
}

//------------------------------------------------------------------------------
template <typename Generator, typename T>
T uniform_small(Generator& gen, T const lo, T const hi) {
    return boost::random::uniform_smallint<T> {lo, hi}(gen);
}

//------------------------------------------------------------------------------
template <typename Generator>
int dice(Generator& gen, int const n, int const sides, int const mod = 0) {
    BK_ASSERT_DBG(n > 0 && sides > 1);

    auto sum = mod;

    for (auto i = 0; i < n; ++i) {
        sum += uniform_small(gen, 1, sides);
    }

    return sum;
}

//------------------------------------------------------------------------------
// generate n where 0 < n < distance(first, last) and follows a piecewise
// linear distribution specified by the weights in [first, last).
//
// @pre the range [first, last) is sorted in increasing order.
//------------------------------------------------------------------------------
template <typename Generator, typename Iterator>
int weighted(Generator& gen, Iterator first, Iterator const last) {   
    using std::distance;

    auto const n = distance(first, last);
    
    BK_ASSERT_DBG(n >= 0);

    if (n < 2) {
        return n;
    }

    auto const hi = *(first + (n - 1)) - 1;
    auto const lo = decltype(hi) {0};

    auto const roll = uniform_small(gen, lo, hi);

    int i = 0;
    while (roll >= *(first++)) { ++i; }

    return i;
}

//------------------------------------------------------------------------------
template <typename Generator, typename Container>
int weighted(Generator& gen, Container const& c) {
    using std::cbegin;
    using std::cend;

    return weighted(gen, cbegin(c), cend(c));
}

//------------------------------------------------------------------------------
template <typename Generator, typename T>
int weighted(Generator& gen, std::initializer_list<T> const& c) {
    return weighted(gen, c.begin(), c.end());
}

//------------------------------------------------------------------------------
template <typename InIt, typename OutIt> 
void make_weight_distribution(
    InIt  ifirst, InIt  const ilast
  , OutIt ofirst, OutIt const olast
) {
    auto sum = std::decay_t<decltype(*ifirst)> {0};
    while (ifirst != ilast && ofirst != olast) {
        *ofirst++ = (sum = (sum + *ifirst++));
    }
}

//------------------------------------------------------------------------------
template <typename Container> 
void make_weight_distribution(Container& c) {
    using std::begin;
    using std::end;

    make_weight_distribution(begin(c), end(c), begin(c), end(c));
}

//------------------------------------------------------------------------------
template <typename Container> 
auto make_weight_distribution(Container const& c) {
    using std::begin;
    using std::end;

    auto result = std::vector<typename Container::value_type> (begin(c), end(c));

    make_weight_distribution(result);

    return result;
}

} //namespace random

template <typename F, typename T>
void repeat(T const n, F&& function) {
    for (auto i = T {0}; i < n; ++i) {
        function();
    }
}

template <typename T>
struct counter_t {
    struct iterator {
        explicit iterator(T const n = T{0}) noexcept : n {n} {}

        iterator& operator++() noexcept { return ++n, *this; }

        bool operator!=(iterator const rhs) const noexcept { return n != rhs.n; }

        T const& operator*() const noexcept { return n; }

        T n;
    };

    counter_t(T const count = T {0}) noexcept
      : first {}, last {count}
    {
    }

    auto begin() const noexcept { return first; };
    auto end()   const noexcept { return last;  };

    iterator first;
    iterator last;
};

template <typename T>
auto counter(T const count) {
    return counter_t<T> {count};
}

} //namespace bkrl

//------------------------------------------------------------------------------
TEST_CASE("float almost_equal_2s_complement", "[math]") {
    using namespace bkrl;

    constexpr auto value      =  2.0f;
    constexpr auto very_close =  1.9999999f;
    constexpr auto less_close =  1.9999995f;
    constexpr auto not_close  =  1.99999f;

    REQUIRE(almost_equal_2s_complement(0.0f, -0.0f) == true);

    REQUIRE(almost_equal_2s_complement(value, very_close) == true);
    REQUIRE(almost_equal_2s_complement(very_close, value) == true);

    REQUIRE(almost_equal_2s_complement(value, less_close) == true);
    REQUIRE(almost_equal_2s_complement(less_close, value) == true);

    REQUIRE(almost_equal_2s_complement(value, not_close) == false);
    REQUIRE(almost_equal_2s_complement(not_close, value) == false);
}

//------------------------------------------------------------------------------
TEST_CASE("double almost_equal_2s_complement", "[math]") {
    using namespace bkrl;

    constexpr auto value      =  2.0;
    constexpr auto very_close =  1.9999999999999995;
    constexpr auto less_close =  1.9999999999999991;
    constexpr auto not_close  =  1.99999999999999;

    REQUIRE(almost_equal_2s_complement(0.0, -0.0) == true);

    REQUIRE(almost_equal_2s_complement(value, very_close) == true);
    REQUIRE(almost_equal_2s_complement(very_close, value) == true);

    REQUIRE(almost_equal_2s_complement(value, less_close) == true);
    REQUIRE(almost_equal_2s_complement(less_close, value) == true);

    REQUIRE(almost_equal_2s_complement(value, not_close) == false);
    REQUIRE(almost_equal_2s_complement(not_close, value) == false);
}

//------------------------------------------------------------------------------
TEST_CASE("percentage", "[math]") {
    for (int i = 0; i <= 100; ++i) {
        REQUIRE(bkrl::percentage<> {i}.value == i);
    }
}

//------------------------------------------------------------------------------
TEST_CASE("random weighted", "[random]") {
    bkrl::random_t gen;

    constexpr int iterations = 100000;
    constexpr int size       = 7;
    auto const    tolerance  = bkrl::percentage<> {5};

    auto const w    = {10, 10, 20, 40, 10, 5, 5};
    auto const dist = bkrl::random::make_weight_distribution(w);
    auto const sum  = dist.back();

    std::array<int, size> results = {0};

    bkrl::repeat(iterations, [&] {
        auto const i = bkrl::random::weighted(gen, dist);
        results[i]++;
    });

    for (auto const i : bkrl::counter(size)) {
        auto const weight   = static_cast<double>(*(w.begin() + i));
        auto const actual   = results[i];
        auto const expected = (weight / sum) * iterations;

        REQUIRE(bkrl::is_equal_tolerance(actual, expected, tolerance));
    }
}

//------------------------------------------------------------------------------
TEST_CASE("random roll 2d6", "[random]") {
    bkrl::random_t gen;

    constexpr int iterations = 100000;
    constexpr int n          = 11;
    auto const    tolerance  = bkrl::percentage<> {5};

    std::array<int, n> results = {0};

    using ratio = bkrl::positive_proper_fraction<>;
    std::array<ratio, n> const expected = {
        ratio {1, 36}
      , ratio {2, 36}
      , ratio {3, 36}
      , ratio {4, 36}
      , ratio {5, 36}
      , ratio {6, 36}
      , ratio {5, 36}
      , ratio {4, 36}
      , ratio {3, 36}
      , ratio {2, 36}
      , ratio {1, 36}
    };

    for (int i = 0; i < iterations; ++i) {
        auto const roll = bkrl::random::dice(gen, 2, 6, -2);
        results[roll]++;
    }

    for (int i = 0; i < n; ++i) {
        REQUIRE(bkrl::is_equal_tolerance(
            results[i]
          , expected[i].as_float() * iterations
          , tolerance
        ));
    }
}


//------------------------------------------------------------------------------
TEST_CASE("random percent pass and fail", "[random]") {
    bkrl::random_t gen;

    constexpr int iterations = 10000;
    auto const chance        = bkrl::percentage<> {10};
    auto const tolerance     = bkrl::percentage<> {5};

    int passed = 0;
    int failed = 0;

    for (int i = 0; i < iterations; ++i) {
        bkrl::random::do_percent(
            gen
          , chance
          , [&] { passed++; }
          , [&] { failed++; }
        );
    }

    REQUIRE(bkrl::is_equal_tolerance(
          passed
        , chance.as_float() * iterations 
        , tolerance
    ));

    REQUIRE(bkrl::is_equal_tolerance(
          failed
        , (1.0 - chance.as_float()) * iterations 
        , tolerance
    ));
}

//------------------------------------------------------------------------------
TEST_CASE("random choices", "[random]") {
    bkrl::random_t gen;

    constexpr int iterations = 100000;
    auto const tolerance     = bkrl::percentage<> {5};

    auto const check = [&](int const actual, int const n) {
        REQUIRE(bkrl::is_equal_tolerance(
              actual
            , (1.0 / n) * iterations 
            , tolerance
        ));
    };

    SECTION("2 choices") {
        constexpr auto n = 2;

        std::array<int, n> results = {0};

        for (int i = 0; i < iterations; ++i) {
            bkrl::random::do_one_of(gen,
                [&] { results[0]++; }
              , [&] { results[1]++; }
            );
        }

        for (int i = 0; i < n; ++i) {
            check(results[i], n);
        }
    }

    SECTION("5 choices") {
        constexpr auto n = 5;

        std::array<int, n> results = {0};

        for (int i = 0; i < iterations; ++i) {
            bkrl::random::do_one_of(gen,
                [&] { results[0]++; }
              , [&] { results[1]++; }
              , [&] { results[2]++; }
              , [&] { results[3]++; }
              , [&] { results[4]++; }
            );
        }

        for (int i = 0; i < n; ++i) {
            check(results[i], n);
        }
    }
}

//------------------------------------------------------------------------------
TEST_CASE("random chance pass and fail", "[random]") {
    bkrl::random_t gen;

    constexpr int iterations = 10000;
    auto const    chance     = bkrl::positive_proper_fraction<> {99, 150};
    auto const    tolerance  = bkrl::percentage<> {5};

    int passed = 0;
    int failed = 0;

    for (int i = 0; i < iterations; ++i) {
        bkrl::random::do_chance(
            gen
          , chance
          , [&] { passed++; }
          , [&] { failed++; }
        );
    }

    REQUIRE(bkrl::is_equal_tolerance(
          passed
        , chance.as_float() * iterations 
        , tolerance
    ));

    REQUIRE(bkrl::is_equal_tolerance(
          failed
        , (1.0 - chance.as_float()) * iterations 
        , tolerance
    ));
}

