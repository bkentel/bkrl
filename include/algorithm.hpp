#pragma once

#include <algorithm>

namespace bkrl {

template <typename Container, typename Type, typename Predicate>
inline auto lower_bound(Container& container, const Type& value, Predicate predicate) {
    return std::lower_bound(std::begin(container), std::end(container), value, predicate);
}

template <typename Container, typename Predicate>
inline void sort(Container& container, Predicate predicate) {
    std::sort(std::begin(container), std::end(container), predicate);
}

template <typename Container, typename Type, typename Predicate, typename Fallback = typename Container::value_type>
auto lower_bound_or(Container& container, const Type& value, Predicate predicate, const Fallback& fallback = Fallback {}) {
    auto const it = bkrl::lower_bound(container, value, predicate);
    return it == std::end(container) ? fallback : *it;
}

} //namespace bkrl
