#pragma once

#include "types.hpp"
#include "renderer.hpp"

namespace bkrl {

//==============================================================================
//! tile_map
//! map texture_type -> texture_id
//==============================================================================
class tile_map {
public:
    //--------------------------------------------------------------------------
    //! @param source The json describing the mappings.
    //--------------------------------------------------------------------------
    explicit tile_map(string_ref filename);
    explicit tile_map(utf8string const& source);
    ~tile_map();

    texture_id operator[](texture_type type) const;

    int tile_w() const noexcept;
    int tile_h() const noexcept;
    string_ref filename() const;
private:
    class impl_t;
    std::unique_ptr<impl_t> impl_;
};

//==============================================================================
//
//==============================================================================
class tile_sheet {
public:
    using rect_t = axis_aligned_rect<int>;

    tile_sheet(string_ref filename, renderer& render);

    rect_t at(int const i) const noexcept {
        auto const d = std::div(i, tile_x);
        auto const x = d.rem;
        auto const y = d.quot;
        
        return at(x, y);
    }

    rect_t at(int const x, int const y) const noexcept {
        BK_ASSERT_SAFE(x >= 0 && x < tile_x);
        BK_ASSERT_SAFE(y >= 0 && y < tile_y);

        auto const w = tile_width();
        auto const h = tile_height();

        auto const l = x * w;
        auto const t = y * h;
        auto const r = l + w;
        auto const b = t + h;

        return rect_t {l, t, r, b};
    }
     
    tile_map map;
    texture  tile_texture;
    
    int tile_width() const noexcept {
        return map.tile_w();
    }

    int tile_height() const noexcept {
        return map.tile_h();
    }

    int tile_x; //tiles per row
    int tile_y; //rows
};

} //namespace bkrl
