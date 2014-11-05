#include "tile_sheet.hpp"
#include "tiles.hpp"
#include "json.hpp"
#include "renderer.hpp"

#include <boost/container/flat_map.hpp>

namespace jc = bkrl::json::common;

class bkrl::detail::tile_map_impl {
public:
    using tile_pos = tile_map::tile_pos;

    tile_pos operator[](texture_type const type) const {
        auto const it = mappings_.find(type);
        if (it == std::cend(mappings_)) {
            BK_TODO_FAIL();
        }

        return it->second;
    }

    path_string_ref filename() const noexcept { return info().filename; }

    tex_coord_i tile_w() const noexcept { return info().tile_w; }
    tex_coord_i tile_h() const noexcept { return info().tile_h; }

    tile_map_info const& info() const noexcept {
        return info_;
    }

    void load(json::cref data) {
        reset_();
        rule_root(data);
    }
    
    void rule_root(json::cref value) {
        jc::get_filetype(value, jc::filetype_tilemap);

        rule_tile_size(value);
        rule_filename(value);
        rule_mappings(value);
    }

    void rule_tile_size(json::cref value) {
        auto const& size = json::require_array(value[jc::field_tile_size], 2, 2);

        info_.tile_w = json::require_int<int16_t>(size[0]);
        info_.tile_h = json::require_int<int16_t>(size[1]);

        if (info_.tile_w <= 0) {
            BK_TODO_FAIL();
        }

        if (info_.tile_h <= 0) {
            BK_TODO_FAIL();
        }
    }

    void rule_filename(json::cref value) {
        info_.filename = jc::get_filename(value);
    }

    void rule_mappings(json::cref value) {
        auto const& mappings = json::require_array(value[jc::field_mappings]);

        for (auto const& mapping : mappings.array_items()) {
            rule_mapping(mapping);
        }
    }

    void rule_mapping(json::cref value) {
        json::require_array(value, 3, 3);

        auto const texture = json::require_string(value[0]);
        auto const ix      = json::require_int<int16_t>(value[1]);
        auto const iy      = json::require_int<int16_t>(value[2]);
        
        if (ix < 0 || iy < 0) {
            BK_TODO_FAIL();
        }

        auto const hash = slash_hash32(texture);
        auto const type = from_hash<texture_type>(hash);

        if (type == texture_type::invalid
         && hash != slash_hash32("invalid")
        ) {
            BK_TODO_FAIL();
        }

        auto const id = tile_pos {ix * info_.tile_w, iy * info_.tile_h};

        auto const result = mappings_.emplace(type, id);
        if (!result.second) {
            BK_TODO_FAIL();
        }
    }
private:
    void reset_() {
        info_.tile_w = 0;
        info_.tile_h = 0;
        info_.filename.clear();
        mappings_.clear();
    }

    tile_map_info info_;

    boost::container::flat_map<texture_type, tile_pos> mappings_;
};

class bkrl::detail::tile_sheet_impl {
public:
    using tile_pos = tile_sheet::tile_pos;

    tile_sheet_impl(renderer& r, tile_map const& map)
      : tile_map_  {&map}
      , tile_info_ {&map.info()}
      , texture_   {r.create_texture(map.filename())}
      , tiles_x_   {texture_.width()  / map.tile_w()}
      , tiles_y_   {texture_.height() / map.tile_h()}
    {
    }

    tile_sheet_impl(renderer& r, tile_map_info const& info)
      : texture_   {r.create_texture(info.filename)}
      , tile_info_ {&info_}
      , info_      {info}
      , tiles_x_   {texture_.width()  / info.tile_w}
      , tiles_y_   {texture_.height() / info.tile_h}
    {
    }

    tex_coord_i sheet_w() const noexcept { return texture_.width(); }
    tex_coord_i sheet_h() const noexcept { return texture_.height(); }

    tex_coord_i tile_w() const noexcept { return tile_info_->tile_w; }
    tex_coord_i tile_h() const noexcept { return tile_info_->tile_h; }

    tex_coord_i tile_count_x() const noexcept { return tiles_x_; }
    tex_coord_i tile_count_y() const noexcept { return tiles_y_; }

    tile_map const& get_map() const noexcept {
        return *tile_map_;
    }

    texture& get_texture() noexcept { return texture_; }
    texture const& get_texture() const noexcept { return texture_; }

    void render(renderer& r, tile_pos const tp, int const x, int const y) const {
        auto const w = tile_w();
        auto const h = tile_h();

        auto const src = bkrl::make_rect_size<int>(tp.x,  tp.y,  w, h);
        auto const dst = bkrl::make_rect_size<int>(x * w, y * w, w, h);
        
        r.draw_texture(texture_, src, dst);
    }
    
    void render(renderer& r, tile_pos const tp, ipoint2 const p) const {
        render(r, tp, p.x, p.y);
    }
private:
    tile_map const*      tile_map_  = nullptr;
    tile_map_info const* tile_info_ = nullptr;

    tile_map_info info_;
    texture       texture_;

    tex_coord_i tiles_x_   = 0;
    tex_coord_i tiles_y_   = 0;
};

////////////////////////////////////////////////////////////////////////////////
// tile_map
////////////////////////////////////////////////////////////////////////////////
bkrl::tile_map::tile_map()
  : impl_ {std::make_unique<detail::tile_map_impl>()}
{
}

bkrl::tile_map::tile_map(tile_map&&) = default;
bkrl::tile_map::~tile_map() = default;

bkrl::tile_map::tile_pos bkrl::tile_map::operator[](texture_type const type) const {
    return (*impl_)[type];
}

void bkrl::tile_map::load(json::cref data) {
    impl_->load(data);
}

bkrl::path_string_ref bkrl::tile_map::filename() const noexcept {
    return impl_->filename();
}

bkrl::tex_coord_i bkrl::tile_map::tile_w() const noexcept {
    return impl_->tile_w();
}

bkrl::tex_coord_i bkrl::tile_map::tile_h() const noexcept {
    return impl_->tile_h();
}

bkrl::tile_map_info const& bkrl::tile_map::info() const noexcept {
    return impl_->info();
}

////////////////////////////////////////////////////////////////////////////////
// tile_sheet
////////////////////////////////////////////////////////////////////////////////
bkrl::tile_sheet::tile_sheet(renderer& r, tile_map const& map)
  : impl_ {std::make_unique<detail::tile_sheet_impl>(r, map)}
{
}

bkrl::tile_sheet::tile_sheet(renderer& r, tile_map_info const& info)
  : impl_ {std::make_unique<detail::tile_sheet_impl>(r, info)}
{
}

bkrl::tile_sheet::tile_sheet(tile_sheet&&) = default;
bkrl::tile_sheet::~tile_sheet() = default;

bkrl::tex_coord_i bkrl::tile_sheet::sheet_w() const noexcept {
    return impl_->sheet_w();
}

bkrl::tex_coord_i bkrl::tile_sheet::sheet_h() const noexcept {
    return impl_->sheet_h();
}

bkrl::tex_coord_i bkrl::tile_sheet::tile_w() const noexcept {
    return impl_->tile_w();
}

bkrl::tex_coord_i bkrl::tile_sheet::tile_h() const noexcept {
    return impl_->tile_h();
}

bkrl::tex_coord_i bkrl::tile_sheet::tile_count_x() const noexcept {
    return impl_->tile_count_x();
}

bkrl::tex_coord_i bkrl::tile_sheet::tile_count_y() const noexcept {
    return impl_->tile_count_y();
}

void bkrl::tile_sheet::render(renderer& r, tile_pos tp, int x, int y) const {
    impl_->render(r, tp, x, y);
}

void bkrl::tile_sheet::render(renderer& r, tile_pos tp, ipoint2 p) const {
    impl_->render(r, tp, p);
}

bkrl::tile_map const& bkrl::tile_sheet::get_map() const noexcept {
    return impl_->get_map();
}

bkrl::texture& bkrl::tile_sheet::get_texture() noexcept {
    return impl_->get_texture();
}

bkrl::texture const& bkrl::tile_sheet::get_texture() const noexcept {
    return impl_->get_texture();
}
