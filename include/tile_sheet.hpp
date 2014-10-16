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

template <typename Key, typename Value>
class mapping {
public:
    using map_t   = boost::container::flat_map<Key, Value>;
    using value_t = Value;
    using key_t   = Key;

    static_assert(std::is_scalar<Key>::value, "");

    BK_NOCOPY(mapping);
    BK_DEFMOVE(mapping);

    mapping()  = default;
    ~mapping() = default;

    void reset(map_t&& data) {
        data_ = std::move(data);
    }

    value_t const& operator[](key_t const k) const {
        auto const it = data_.find(k);
        if (it == std::end(data_)) {
            BK_TODO_FAIL();
        }

        return it->second;
    }

    size_t size() const {
        return data_.size();
    }

    bool empty() const {
        return data_.empty();
    }
private:
    map_t data_;
};

//==============================================================================
//! tilemap
//! map texture_type -> texture_id
//==============================================================================
class tilemap : public mapping<texture_type, texture_id> {
public:
    using mapping<texture_type, texture_id>::mapping;

    int tile_w = 0;
    int tile_h = 0;

    path_string filename = path_string {};
};

tilemap load_tilemap(json::cref data);

//==============================================================================
// tile_sheet
//==============================================================================
class tile_sheet {
public:
    using rect_t = renderer::rect_t;

    tile_sheet(tilemap const& map, renderer& render);

    rect_t get_rect(int const i) const {
        auto const d = std::div(i, tiles_x());
        auto const x = d.rem;
        auto const y = d.quot;
        
        return get_rect(x, y);
    }

    rect_t get_rect(int const x, int const y) const {
        BK_ASSERT_SAFE(x >= 0 && x < tiles_x());
        BK_ASSERT_SAFE(y >= 0 && y < tiles_y());

        auto const w = tile_width();
        auto const h = tile_height();

        return make_rect_size(x * w, y * h, w, h);
    }
     
    int tile_width() const noexcept {
        return map().tile_w;
    }

    int tile_height() const noexcept {
        return map().tile_h;
    }

    int tiles_x() const noexcept {
        return tile_x_;
    }

    int tiles_y() const noexcept {
        return tile_y_;
    }

    void render(renderer& r, int const tile_x, int const tile_y, int const x, int const y) const {
        auto const w = tile_width();
        auto const h = tile_height();

        auto const src_r = get_rect(tile_x, tile_y);
        auto const dst_r = make_rect_size(x * w, y * h, w, h);

        r.draw_texture(tile_texture_, src_r, dst_r);
    }

    void render(renderer& r, int const index, int const x, int const y) const {
        auto const d = std::div(index, tile_x_);
        render(r, d.rem, d.quot, x, y);
    }

    tilemap const& map() const {
        return *map_;
    }

    texture& get_texture() const {
        //TODO just for testing
        return const_cast<tile_sheet*>(this)->tile_texture_;
    }
private:
    tilemap const* map_;
    texture        tile_texture_;

    int tile_x_; //tiles per row
    int tile_y_; //rows
};

} //namespace bkrl
