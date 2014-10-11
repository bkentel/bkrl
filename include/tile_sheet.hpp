//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Tile sheet and tile texture mappings.
//##############################################################################
#pragma once

#include "types.hpp"
#include "renderer.hpp"

namespace bkrl {

class renderer;
class texture;

//==============================================================================
//! tile_map
//! map texture_type -> texture_id
//==============================================================================
class tile_map {
public:
    BK_NOCOPY(tile_map);

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
// tile_sheet
//==============================================================================
class tile_sheet {
public:
    tile_sheet(string_ref filename, renderer& render);

    rect get_rect(int const i) const {
        auto const d = std::div(i, tiles_x());
        auto const x = d.rem;
        auto const y = d.quot;
        
        return get_rect(x, y);
    }

    rect get_rect(int const x, int const y) const {
        BK_ASSERT_SAFE(x >= 0 && x < tiles_x());
        BK_ASSERT_SAFE(y >= 0 && y < tiles_y());

        auto const w = tile_width();
        auto const h = tile_height();

        auto const l = static_cast<float>(x * w);
        auto const t = static_cast<float>(y * h);
        auto const r = static_cast<float>(l + w);
        auto const b = static_cast<float>(t + h);

        return {l, t, r, b};
    }
     
    int tile_width() const noexcept {
        return tile_map_.tile_w();
    }

    int tile_height() const noexcept {
        return tile_map_.tile_h();
    }

    int tiles_x() const noexcept {
        return tile_x_;
    }

    int tiles_y() const noexcept {
        return tile_y_;
    }

    void render(renderer& render, int const tile_x, int const tile_y, int const x, int const y) const {
        auto const w = tile_width();
        auto const h = tile_height();

        auto const src_r = get_rect(tile_x, tile_y);
        auto const dst_r = rect {
            static_cast<float>(x * w)
          , static_cast<float>(y * h)
          , static_cast<float>(x * w + w)
          , static_cast<float>(y * h + h)
        };

        render.draw_texture(tile_texture_, src_r, dst_r);
    }

    tile_map const& map() const {
        return tile_map_;
    }

    texture& get_texture() const {
        //TODO just for testing
        return const_cast<tile_sheet*>(this)->tile_texture_;
    }
private:
    tile_map tile_map_;
    texture  tile_texture_;

    int tile_x_; //tiles per row
    int tile_y_; //rows
};

} //namespace bkrl
