#pragma once

#include <vector>
#include <type_traits>

#include "algorithm.hpp"
#include "math.hpp"

namespace bkrl {

template <typename T, typename P = int>
class spatial_map {
public:
    using point_t = point2d<P>;

    struct value_t {
        point_t pos;
        T       value;
    };

    struct less {
        bool compare(point_t const& lhs, point_t const& rhs) const noexcept {
            return lexicographical_compare(lhs, rhs);
        }

        bool operator()(value_t const& lhs, point_t const& rhs) const noexcept {
            return compare(lhs.pos, rhs);
        }

        bool operator()(point_t const& lhs, value_t const& rhs) const noexcept {
            return compare(lhs, rhs.pos);
        }

        bool operator()(value_t const& lhs, value_t const& rhs) const noexcept {
            return compare(lhs.pos, rhs.pos);
        }
    };

    //--------------------------------------------------------------------------
    void insert(point_t where, T item) {
        items_.emplace_back(value_t {where, std::move(item)});
    }
    
    //--------------------------------------------------------------------------
    template <typename... Params>
    void emplace(point_t where, Params&&... params) {
        items_.emplace_back(value_t {where, T(std::forward<Params>(params)...)});
    }

    //--------------------------------------------------------------------------
    auto begin()        { return items_.begin();  }
    auto begin()  const { return items_.begin();  }
    auto cbegin() const { return items_.cbegin(); }

    auto end()        { return items_.end();  }
    auto end()  const { return items_.end();  }
    auto cend() const { return items_.cend(); }

    //--------------------------------------------------------------------------
    template <typename F>
    void find(point_t const& p, F function) {
        auto const range = bkrl::equal_range(items_, p, less {});
        std::for_each(range.first, range.second, [&](value_t& v) { function(v.value); });
    }

    template <typename F>
    void find(point_t const& p, F function) const {
        auto const range = bkrl::equal_range(items_, p, less {});
        std::for_each(range.first, range.second, [&](value_t const& v) { function(v.value); });
    }

    //--------------------------------------------------------------------------
    template <typename Predicate>
    void remove(point_t const& p, Predicate pred) {
        auto range = bkrl::equal_range(items_, p, less {});
        
        items_.erase(
            std::remove_if(range.first, range.second, [&](value_t& v) { return pred(v.value); })
          , range.second
        );
    }

    void remove(point_t const& p) {
        auto range = bkrl::equal_range(items_, p, less {});
        
        items_.erase(
            std::remove_if(range.first, range.second, [](auto const&) { return true; })
          , range.second
        );
    }

    //--------------------------------------------------------------------------
    void sort() {
        bkrl::sort(items_, less {});
    }
private:
    std::vector<value_t> items_;
};


};
