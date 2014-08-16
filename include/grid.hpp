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

struct texture_info {
    texture_type  type;
    texture_index index;
};

//==============================================================================
// attributes
//==============================================================================
namespace attribute {
    //--------------------------------------------------------------------------
    // tag types
    //--------------------------------------------------------------------------
    struct type_t    {};
    struct texture_t {};
    struct room_id_t {};

    //--------------------------------------------------------------------------
    // attribute instances
    //--------------------------------------------------------------------------
    constexpr type_t        type        {};
    constexpr texture_t     texture     {};
    constexpr room_id_t     room_id     {};

    //--------------------------------------------------------------------------
    // attribute traits
    //--------------------------------------------------------------------------
    template <typename T> struct traits;

    template <> struct traits<type_t> {
        using type = tile_type;
    };

    template <> struct traits<texture_t> {
        using type = texture_info;
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
    using attribute_type_t    = attribute::value_t<attribute::type_t>;
    using attribute_texture_t = attribute::value_t<attribute::texture_t>;

    grid_storage(grid_size const w, grid_size const h)
      : width_  {w}
      , height_ {h}
    {
        auto const size = w * h;

        type_.resize(size, attribute_type_t {});
        texture_.resize(size, attribute_texture_t {});
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
        BK_ASSERT(source.width()  <= width());
        BK_ASSERT(source.height() <= height());
        BK_ASSERT(source.width()  + where.x <= width());
        BK_ASSERT(source.height() + where.y <= height());

        //TODO change copy method to do each attribute seperately
        auto const copy = [&](grid_point const from, grid_point const to) {
            set(attribute::type,    to, source.get(attribute::type,    from));
            set(attribute::texture, to, source.get(attribute::texture, from));
        };

        auto const h = source.height();
        auto const w = source.width();

        for (grid_index yi = 0; yi < h; ++yi) {
            for (grid_index xi = 0; xi < w; ++xi) {
                copy(
                    grid_point {xi, yi}
                  , grid_point {where.x + xi, where.y + yi}
                );
            }
        }
    }

    size_t width()  const { return width_; }
    size_t height() const { return height_; }

    bool is_valid(grid_index const x, grid_index const y) const {
        return x < width() && y < height();
    }
private:
    //tile type
    attribute_type_t get_(attribute::type_t, grid_index const x, grid_index const y) const {
        auto const i = linearize(width_, height_, x, y);
        return type_[i];
    }

    void set_(attribute::type_t, grid_index const x, grid_index const y, attribute_type_t const value) {
        auto const i = linearize(width_, height_, x, y);
        type_[i] = value;
    }

    //texture
    attribute_texture_t get_(attribute::texture_t, grid_index const x, grid_index const y) const {
        auto const i = linearize(width_, height_, x, y);
        return texture_[i];
    }

    void set_(attribute::texture_t, grid_index const x, grid_index const y, attribute_texture_t const value) {
        auto const i = linearize(width_, height_, x, y);
        texture_[i] = value;
    }
private:
    size_t width_;
    size_t height_;

    std::vector<attribute_type_t>    type_;
    std::vector<attribute_texture_t> texture_;
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
void for_each_edge(grid_region region, Function function) {
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
template <typename Attribute, typename Value>
inline uint8_t check_grid_block5(
    grid_storage const& grid
  , grid_index const x
  , grid_index const y
  , Attribute const attribute
  , Value const value
) {
    uint8_t result = 0;

    for (int i = 0; i < 5; ++i) {
        auto const xx = x + x_off5[i];
        auto const yy = y + y_off5[i];

        if (grid.is_valid(xx, yy) &&
            grid.get(attribute, xx, yy) == value
        ) {
            result |= (1 << i);
        }
    }

    return result;
}

template <typename Attribute, typename Value>
inline uint8_t check_grid_block9(
    grid_storage const& grid
  , grid_index const x
  , grid_index const y
  , Attribute const attribute
  , Value const value
) {
    uint8_t result = 0;

    for (int i = 0; i < 9; ++i) {
        auto const xx = x + x_off9[i];
        auto const yy = y + y_off9[i];

        if (grid.is_valid(xx, yy) &&
            grid.get(attribute, xx, yy) == value
        ) {
            result |= (1 << i);
        }
    }

    return result;
}

} //namespace bkrl
