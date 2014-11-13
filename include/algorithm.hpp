//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Modified std algorithms.
//##############################################################################
#pragma once

#include <algorithm>
#include <boost/predef.h>

#if !BOOST_COMP_MSVC
namespace std {
template<class _Container>
	auto inline cbegin(const _Container& _Cont)
		-> decltype(std::begin(_Cont))
	{	// get beginning of sequence
	return (std::begin(_Cont));
	}

template<class _Container>
	auto inline cend(const _Container& _Cont)
		-> decltype(std::end(_Cont))
	{	// get end of sequence
	return (std::end(_Cont));
	}

template<class _Container>
	auto inline rbegin(_Container& _Cont) -> decltype(_Cont.rbegin())
	{	// get beginning of reversed sequence
	return (_Cont.rbegin());
	}

template<class _Container>
	auto inline rbegin(const _Container& _Cont) -> decltype(_Cont.rbegin())
	{	// get beginning of reversed sequence
	return (_Cont.rbegin());
	}

template<class _Container>
	auto inline rend(_Container& _Cont) -> decltype(_Cont.rend())
	{	// get end of reversed sequence
	return (_Cont.rend());
	}

template<class _Container>
	auto inline rend(const _Container& _Cont) -> decltype(_Cont.rend())
	{	// get end of reversed sequence
	return (_Cont.rend());
	}

template<class _Container>
	auto inline crbegin(const _Container& _Cont)
		-> decltype(std::rbegin(_Cont))
	{	// get beginning of reversed sequence
	return (std::rbegin(_Cont));
	}

template<class _Container>
	auto inline crend(const _Container& _Cont)
		-> decltype(std::rend(_Cont))
	{	// get end of reversed sequence
	return (std::rend(_Cont));
	}
}
#endif

namespace bkrl {

//==============================================================================
//! as std::equal_range, but for the entire container
//==============================================================================
template <
    typename Container
  , typename Type
  , typename Predicate = std::less<>
>
inline auto equal_range(
    Container&  container
  , Type const& value
  , Predicate   predicate = Predicate {}
) {
    return std::equal_range(
        std::begin(container)
      , std::end(container)
      , value
      , predicate
    );
}


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
  , Predicate   predicate = Predicate {}
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
  , Predicate  predicate = Predicate {}
) {
    std::sort(
        std::begin(container)
      , std::end(container)
      , predicate
    );
}

//==============================================================================
//==============================================================================
template <
    typename Container
  , typename Predicate = std::equal_to<>
>
inline auto adjacent_find(
    Container& container
  , Predicate  predicate = Predicate {}
) {
    return std::adjacent_find(
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
