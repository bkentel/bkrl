#pragma once

#include <vector>
#include <type_traits>

#include "algorithm.hpp"
#include "math.hpp"

namespace bkrl {

template <typename value_type, typename point_type>
class spatial_map_base;

template <typename value_type>
class spatial_map_base<value_type, void> {
public:
    using point_t = typename value_type::point_t;
    using value_t = value_type;
    using data_t  = value_type;

    template <typename... Params>
    void emplace(Params&&... params) {
        data_.emplace_back(std::forward<Params>(params)...);
    }

    void insert(value_t value) {
        data_.emplace_back(std::move(value));
    }

    static point_t        get_point(data_t const& v) noexcept { return position(v); }
    static value_t const& get_value(data_t const& v) noexcept { return v; }
    static value_t&       get_value(data_t      & v) noexcept { return v; }
protected:
    std::vector<data_t> data_;
};

template <typename value_type, typename point_type>
class spatial_map_base {
public:
    using point_t = point2d<point_type>;
    using value_t = value_type;
    using data_t = std::pair<point_t, value_t>;

    template <typename... Params>
    void emplace(point_t where, Params&&... params) {
        data_.emplace_back(where, std::forward<Params>(params)...);
    }

    void insert(point_t where, value_t value) {
        data_.emplace_back(where, std::move(value));
    }

    void insert(point_t where) {
        data_.emplace_back(where, value_t {});
    }

    static point_t        get_point(data_t const& v) noexcept { return v.first; }
    static value_t const& get_value(data_t const& v) noexcept { return v.second; }
    static value_t&       get_value(data_t      & v) noexcept { return v.second; }
protected:
    std::vector<data_t> data_;
};

template <
    typename value_type
    , typename point_type = void
>
class spatial_map : public spatial_map_base<value_type, point_type> {
public:
    using base_t  = spatial_map_base<value_type, point_type>;
    using point_t = typename base_t::point_t;
    using data_t  = typename base_t::data_t;

    //--------------------------------------------------------------------------
    struct less {
        bool compare(point_t const& lhs, point_t const& rhs) const noexcept {
            return lexicographical_compare(lhs, rhs);
        }

        bool operator()(data_t const& lhs, point_t const& rhs) const noexcept {
            return compare(base_t::get_point(lhs), rhs);
        }

        bool operator()(point_t const& lhs, data_t const& rhs) const noexcept {
            return compare(lhs, base_t::get_point(rhs));
        }

        bool operator()(data_t const& lhs, data_t const& rhs) const noexcept {
            return compare(base_t::get_point(lhs), base_t::get_point(rhs));
        }
    };
    //--------------------------------------------------------------------------
    auto begin()        { return base_t::data_.begin(); }
    auto begin()  const { return base_t::data_.begin(); }
    auto cbegin() const { return base_t::data_.cbegin(); }

    auto end()        { return base_t::data_.end(); }
    auto end()  const { return base_t::data_.end(); }
    auto cend() const { return base_t::data_.cend(); }

    //--------------------------------------------------------------------------
    auto at(point_t const& p) {
        using result_t = optional<value_t&>;

        auto const ptr = at_(p);
        return ptr ? result_t {*ptr} : result_t {};
    }

    auto at(point_t const& p) const {
        using result_t = optional<value_t const&>;

        auto const ptr = const_cast<spatial_map*>(this)->at_(p);
        return ptr ? result_t {*ptr} : result_t {};
    }

    //--------------------------------------------------------------------------
    void remove(point_t const& p) {
        auto const range = bkrl::equal_range(base_t::data_, p, less {});

        auto const n = std::distance(range.first, range.second);
        if (n == 0) {
            return;
        } else if (n > 1) {
            BK_TODO_FAIL();
        }

        base_t::data_.erase(range.first);
    }

    //--------------------------------------------------------------------------
    void sort() {
        bkrl::sort(base_t::data_, less {});
    }
private:
    value_t* at_(point_t const& p) {
        auto const range = bkrl::equal_range(base_t::data_, p, less {});
        auto const n = std::distance(range.first, range.second);

        if (n == 0) {
            return nullptr;
        }

        BK_ASSERT(n == 1);

        return &get_value(*range.first);
    }
};

} //namespace bkrl