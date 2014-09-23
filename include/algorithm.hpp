//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Modified std algorithms.
//##############################################################################
#pragma once

#include <algorithm>

namespace bkrl {

//==============================================================================
//! as std::lower_bound, but for the entire container
//==============================================================================
template <
    typename Container
  , typename Type
  , typename Predicate = std::less<>
>
inline auto lower_bound(
    Container&  container
  , Type const& value
  , Predicate&& predicate = Predicate{}
) {
    return std::lower_bound(
        std::begin(container)
      , std::end(container)
      , value
      , predicate
    );
}

//==============================================================================
//! as std::sort, but for the entire container
//==============================================================================
template <
    typename Container
  , typename Predicate = std::less<>
>
inline void sort(
    Container& container
  , Predicate&& predicate = {}
) {
    std::sort(
        std::begin(container)
      , std::end(container)
      , predicate
    );
}

//==============================================================================
//! as std::lower_bound, but for the entire container, and with
//==============================================================================
template <
    typename Container
  , typename Type
  , typename Predicate
  , typename Fallback = typename Container::value_type
>
inline decltype(auto) lower_bound_or(
    Container&  container
  , Type const& value
  , Predicate&& predicate
  , Fallback&&  fallback = Fallback {}
) {
    BK_TODO_FAIL();

    auto const it = bkrl::lower_bound(container, value, predicate);
    return (it == std::end(container))
      ? fallback
      : *it;
}

} //namespace bkrl
