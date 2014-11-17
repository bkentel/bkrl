//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Tile sheet and tile texture mappings.
//##############################################################################
#pragma once

#include <memory>

#include "integers.hpp"
#include "json_forward.hpp"
#include "math.hpp"
#include "string.hpp"
#include "enum_forward.hpp"
#include "render_types.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace bkrl {
////////////////////////////////////////////////////////////////////////////////
namespace detail { class tile_map_impl; }
namespace detail { class tile_sheet_impl; }

struct tile_map_info {
    tile_map_info() = default;
    tile_map_info(tex_coord_i const w, tex_coord_i const h, path_string const& path)
        : tile_w {w}, tile_h {h}, filename(path)
    {
    }

    tex_coord_i tile_w  = 0;
    tex_coord_i tile_h  = 0;
    path_string filename = path_string {};
};

//==============================================================================
//! tile_map
//==============================================================================
class tile_map {
public:
    using tile_pos = tex_point_i;

    tile_map();
    tile_map(tile_map&&);
    ~tile_map();

    tile_pos operator[](texture_type type) const;

    void load(json::cref data);

    path_string_ref filename() const noexcept;

    tex_coord_i tile_w() const noexcept;
    tex_coord_i tile_h() const noexcept;

    tile_map_info const& info() const noexcept;
private:
    std::unique_ptr<detail::tile_map_impl> impl_;
};

//==============================================================================
//! tile_sheet
//==============================================================================
class tile_sheet {
public:
    using tile_pos = tile_map::tile_pos;

    tile_sheet(renderer& r, tile_map const& map);
    tile_sheet(renderer& r, tile_map_info const& info);
    tile_sheet(tile_sheet&&);
    ~tile_sheet();

    tex_coord_i sheet_w() const noexcept;
    tex_coord_i sheet_h() const noexcept;

    tex_coord_i tile_w() const noexcept;
    tex_coord_i tile_h() const noexcept;

    tex_coord_i tile_count_x() const noexcept;
    tex_coord_i tile_count_y() const noexcept;

    void render(renderer& r, tile_pos tp, int x, int y) const;
    void render(renderer& r, tile_pos tp, ipoint2 p) const;

    texture&       get_texture()       noexcept;
    texture const& get_texture() const noexcept;

    tile_map const& get_map() const noexcept;
private:
    std::unique_ptr<detail::tile_sheet_impl> impl_;
};

////////////////////////////////////////////////////////////////////////////////
} //namespace bkrl
////////////////////////////////////////////////////////////////////////////////
