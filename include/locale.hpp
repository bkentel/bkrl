#pragma once

#include <boost/container/flat_map.hpp>

#include "types.hpp"

namespace bkrl {

//==============================================================================
//==============================================================================
template <typename T>
class localized_string {
public:
    BK_NOCOPY(localized_string);
    BK_DEFMOVE(localized_string);

    using locale_t = typename T::locale;
    using map_t = boost::container::flat_map<hash_t, locale_t>;

    localized_string() = default;

    explicit localized_string(map_t&& data)
      : data_ {std::move(data)}
    {
    }

    locale_t const& operator[](hash_t const key) const {
        auto const it = data_.find(key);
        if (it == std::cend(data_)) {
            BK_TODO_FAIL();
        }

        return it->second;
    }
private:
    map_t data_;
};

//==============================================================================
//==============================================================================
template <typename T>
class definition {
public:
    BK_NOCOPY(definition);
    BK_DEFMOVE(definition);

    using map_t = boost::container::flat_map<hash_t, T>;

    definition() = default;

    explicit definition(map_t&& data)
      : data_ {std::move(data)}
    {
    }

    T const& operator[](hash_t const key) const {
        auto const it = data_.find(key);

        if (it == std::cend(data_)) {
            BK_TODO_FAIL();
        }

        return it->second;
    }

    size_t size() const noexcept {
        return data_.size();
    }

    //TODO temp for testing
    T const& at_index(size_t index) const {
        auto it = data_.cbegin();
        std::advance(it, index);
        return it->second;
    }
private:
    map_t data_;
};

//==============================================================================
//==============================================================================
template <typename T>
struct definition_base {
    using definition_t = definition<T>;
    using localized_t  = localized_string<T>;

    BK_NOCOPY(definition_base);
    BK_DEFMOVE(definition_base);

    definition_base() = default;
};


} //namesapce bkrl
