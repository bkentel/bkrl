#pragma once

#include "math.hpp"
#include "types.hpp"
#include "texture_type.hpp"
#include "tile_type.hpp"

namespace bkrl {

using grid_size   = unsigned;
using grid_index  = unsigned;
using grid_point  = bkrl::point2d<grid_index>;
using grid_region = bkrl::axis_aligned_rect<grid_index>;

using grid_data_value = uint32_t;

union grid_data {
    grid_data() : value {0} {}
    explicit grid_data(grid_data_value const value) : value {value} {}

    void*           ptr;
    grid_data_value value;
};

//==============================================================================
// attributes
//==============================================================================
namespace attribute {
    //--------------------------------------------------------------------------
    // tag types
    //--------------------------------------------------------------------------
    struct tile_type_t    {};
    struct texture_type_t {};
    struct texture_id_t   {};
    struct room_id_t      {};
    struct data_t         {};

    //--------------------------------------------------------------------------
    // attribute instances
    //--------------------------------------------------------------------------
    constexpr tile_type_t    tile_type    {};
    constexpr texture_type_t texture_type {};
    constexpr texture_id_t   texture_id   {};
    constexpr room_id_t      room_id      {};
    constexpr data_t         data         {};

    //--------------------------------------------------------------------------
    // attribute traits
    //--------------------------------------------------------------------------
    template <typename T> struct traits;

    template <> struct traits<tile_type_t> {
        using type = bkrl::tile_type;
    };

    template <> struct traits<texture_type_t> {
        using type = bkrl::texture_type;
    };

    template <> struct traits<texture_id_t> {
        using type = bkrl::texture_id;
    };

    template <> struct traits<data_t> {
        using type = bkrl::grid_data;
    };

    //
    template <typename T>
    using value_t = typename traits<T>::type;
} //namespace bkrl::attribute

//==============================================================================
// grid_storage
//==============================================================================
class grid_storage {
public:
    using tile_type_t    = attribute::value_t<attribute::tile_type_t>;
    using texture_type_t = attribute::value_t<attribute::texture_type_t>;
    using texture_id_t   = attribute::value_t<attribute::texture_id_t>;
    using data_t         = attribute::value_t<attribute::data_t>;

    grid_storage(grid_size const w, grid_size const h)
      : width_  {w}
      , height_ {h}
    {
        auto const size = w * h;

        tile_type_.resize(    size, tile_type_t    {} );
        texture_type_.resize( size, texture_type_t {} );
        texture_id_.resize(   size, texture_id_t   {} );
        data_.resize(         size, data_t         {} );
    }

    explicit grid_storage(grid_region const bounds)
      : grid_storage {bounds.width(), bounds.height()}
    {
    }

    ////
    template <typename Attribute, typename Value = attribute::value_t<Attribute>>
    Value get(Attribute const attribute, grid_index const x, grid_index const y) const {
        return get_(attribute, x, y);
    }

    template <typename Attribute, typename Value = attribute::value_t<Attribute>>
    Value get(Attribute const attribute, grid_point const p) const {
        return get_(attribute, p.x, p.y);
    }

    template <typename Attribute, typename Value>
    void set(Attribute const attribute, grid_index const x, grid_index const y, Value const value) {
        set_(attribute, x, y, value);
    }

    template <typename Attribute, typename Value>
    void set(Attribute const attribute, grid_point const p, Value const value) {
        set_(attribute, p.x, p.y, value);
    }
    ////
    void write(grid_storage const& source, grid_point const where = {0, 0}) {
        write(source, where.x, where.y);
    }

    void write(grid_storage const& source, grid_index const x, grid_index const y) {
        BK_ASSERT(source.width()  <= width());
        BK_ASSERT(source.height() <= height());
        BK_ASSERT(source.width()  + x <= width());
        BK_ASSERT(source.height() + y <= height());

        //TODO change copy method to do each attribute seperately
        auto const copy = [&](grid_point const from, grid_point const to) {
            set(attribute::tile_type,    to, source.get(attribute::tile_type,    from));
            set(attribute::texture_type, to, source.get(attribute::texture_type, from));
            set(attribute::texture_id,   to, source.get(attribute::texture_id,   from));
        };

        auto const h = source.height();
        auto const w = source.width();

        for (grid_index yi = 0; yi < h; ++yi) {
            for (grid_index xi = 0; xi < w; ++xi) {
                copy(grid_point {xi, yi}, grid_point {x + xi, y + yi});
            }
        }
    }

    size_t width()  const { return width_; }
    size_t height() const { return height_; }

    bool is_valid(grid_index const x, grid_index const y) const {
        return x < width() && y < height();
    }

    bool is_valid(grid_point const p) const {
        return is_valid(p.x, p.y);
    }
private:
    //--------------------------------------------------------------------------
    // tile_type
    //--------------------------------------------------------------------------
    tile_type_t get_(attribute::tile_type_t, grid_index const x, grid_index const y) const {
        auto const i = linearize(width_, height_, x, y);
        return tile_type_[i];
    }

    void set_(attribute::tile_type_t, grid_index const x, grid_index const y, tile_type_t const value) {
        auto const i = linearize(width_, height_, x, y);
        tile_type_[i] = value;
    }
    //--------------------------------------------------------------------------
    // texture_type
    //--------------------------------------------------------------------------
    texture_type_t get_(attribute::texture_type_t, grid_index const x, grid_index const y) const {
        auto const i = linearize(width_, height_, x, y);
        return texture_type_[i];
    }

    void set_(attribute::texture_type_t, grid_index const x, grid_index const y, texture_type_t const value) {
        auto const i = linearize(width_, height_, x, y);
        texture_type_[i] = value;
    }
    //--------------------------------------------------------------------------
    // texture_id
    //--------------------------------------------------------------------------
    texture_id_t get_(attribute::texture_id_t, grid_index const x, grid_index const y) const {
        auto const i = linearize(width_, height_, x, y);
        return texture_id_[i];
    }

    void set_(attribute::texture_id_t, grid_index const x, grid_index const y, texture_id_t const value) {
        auto const i = linearize(width_, height_, x, y);
        texture_id_[i] = value;
    }
    //--------------------------------------------------------------------------
    // data
    //--------------------------------------------------------------------------
    data_t get_(attribute::data_t, grid_index const x, grid_index const y) const {
        auto const i = linearize(width_, height_, x, y);
        return data_[i];
    }

    void set_(attribute::data_t, grid_index const x, grid_index const y, data_t const value) {
        auto const i = linearize(width_, height_, x, y);
        data_[i] = value;
    }
private:
    size_t width_;
    size_t height_;

    std::vector<tile_type_t>    tile_type_;
    std::vector<texture_type_t> texture_type_;
    std::vector<texture_id_t>   texture_id_;
    std::vector<data_t>         data_;
};

//TODO specialize for other Function type (ie. return bool to break out)
template <typename T, typename Function>
void for_each_xy(T& grid, Function&& function) {
    auto const w = grid.width();
    auto const h = grid.height();

    for (grid_index y = 0; y < h; ++y) {
        for (grid_index x = 0; x < w; ++x) {
            function(x, y);
        }
    }
}

//==============================================================================
// room
//==============================================================================
class room : public grid_storage {
public:
    explicit room(bkrl::grid_region const bounds)
      : grid_storage {bounds}
      , location_    {{bounds.left, bounds.top}}
    {
    }

    grid_region bounds() const {
        auto const l = location_.x;
        auto const t = location_.y;
        auto const r = l + width();
        auto const b = t + height();

        return bkrl::grid_region {l, t, r, b};
    }

    room& translate(int dx, int dy) {
        location_ = bkrl::translate(location_, dx, dy);
        return *this;
    }
private:
     grid_point location_;
};

constexpr int y_off9[9] = {-1, -1, -1
                          , 0,  0,  0
                          , 1,  1,  1};

constexpr int x_off9[9] = {-1,  0,  1
                          ,-1,  0,  1
                          ,-1,  0,  1};

constexpr int y_off5[5] = {    -1
                          , 0,  0,  0
                          ,     1    };

constexpr int x_off5[5] = {     0
                          ,-1,  0,  1
                          ,     0    };

template <typename Function>
void for_each_edge(grid_region region, Function&& function) {
    auto const w = region.width();
    auto const h = region.height();

    auto const l = region.left;
    auto const t = region.top;
    auto const r = region.right;
    auto const b = region.bottom;

    for (grid_index x = l; x < r; ++x) {
        function(x, t);
    }

    for (grid_index y = t; y < b; ++y) {
        function(l, y);
        function(r-1, y);
    }

    for (grid_index x = l; x < r; ++x) {
        function(x, b-1);
    }
};

//TODO could refactor these to share common code?
template <typename Attribute, typename Predicate>
inline uint8_t check_grid_block5f(
    grid_storage const& grid
  , grid_index   const  x
  , grid_index   const  y
  , Attribute    const  attribute
  , Predicate&&         predicate
) {
    auto const get = [&](size_t const i) -> uint8_t {
        auto const xx = x + x_off5[i];
        auto const yy = y + y_off5[i];

        return (grid.is_valid(xx, yy) && predicate(grid.get(attribute, xx, yy)))
            ? 1u : 0u;
    };

    //skip the "here" position
    return                 (get(0) << 0)
        |  (get(1) << 1) |               (get(3) << 2)
        |                  (get(4) << 3);
}


template <typename Attribute, typename Value>
inline uint8_t check_grid_block5(
    grid_storage const& grid
  , grid_index   const  x
  , grid_index   const  y
  , Attribute    const  attribute
  , Value        const  value
) {
    return check_grid_block5f(grid, x, y, attribute, [&](tile_type const type) {
        return type == value;
    });
}

enum : unsigned {
    g9nw, g9n, g9ne, g9w, g9e, g9sw, g9s, g9se
};

inline grid_point grid_check_to_point(unsigned const x, unsigned const y, uint8_t const n) {
    std::bitset<8> const bits {n};

    for (size_t i = 0; i < 4; ++i) {
        if (bits[i]) {
            return {x + x_off9[i], y + y_off9[i]};
        }
    }

    for (size_t i = 4; i < 8; ++i) {
        if (bits[i]) {
            return {x + x_off9[i+1], y + y_off9[i+1]};
        }
    }

    return {0u, 0u};
}

inline grid_point grid_check_to_point(grid_point const& p, uint8_t const n) {
    return grid_check_to_point(p.x, p.y, n);
}

template <typename Attribute, typename Predicate>
inline uint8_t check_grid_block9f(
    grid_storage const& grid
  , grid_index const x
  , grid_index const y
  , Attribute const attribute
  , Predicate&& predicate
) {
    auto const get = [&](size_t const i) -> uint8_t {
        auto const xx = x + x_off9[i];
        auto const yy = y + y_off9[i];

        return (grid.is_valid(xx, yy) && predicate(grid.get(attribute, xx, yy)))
            ? 1u : 0u;
    };

    return (get(0) << 0) | (get(1) << 1) | (get(2) << 2)
         | (get(3) << 3) |                 (get(5) << 4)
         | (get(6) << 5) | (get(7) << 6) | (get(8) << 7);
}

template <typename Attribute, typename Value>
inline uint8_t check_grid_block9(
    grid_storage const& grid
  , grid_index const x
  , grid_index const y
  , Attribute const attribute
  , Value const value
) {
    return check_grid_block9f(grid, x, y, attribute, [&](tile_type const type) {
        return type == value;
    });
}

///

struct door_data {
    enum flag : uint32_t {
        is_open_flag
      , is_locked_flag
      , is_broken_flag
    };

    door_data(grid_storage const& grid, grid_point const where)
      : value_ {grid.get(attribute::data, where).value}
    {
        BK_ASSERT(grid.get(attribute::tile_type, where) == tile_type::door);
    }

    operator grid_data() const {
        return grid_data {value_.to_ulong()};
    }

    bool is_closed() const {
        return !value_.test(is_open_flag);
    }

    void close() {
        BK_ASSERT(!is_closed());
        value_.reset(is_open_flag);
    }

    bool is_open() const {
        return value_.test(is_open_flag);
    }

    void open() {
        BK_ASSERT(!is_open());
        value_.set(is_open_flag);
    }

private:
    std::bitset<sizeof(grid_data_value)*8> value_;
};

} //namespace bkrl
