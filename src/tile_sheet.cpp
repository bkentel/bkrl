#include "tile_sheet.hpp"
#include "texture_type.hpp"
#include "util.hpp"
#include "assert.hpp"
#include "algorithm.hpp"
#include "json.hpp"
#include "hash.hpp"

#include <utf8.h>

using namespace bkrl;

namespace {
class tilemap_parser {
public:
    using cref = json::cref;

    tilemap_parser(cref data) {
        json::require_object(data);
        rule_root(data);
    }

    void rule_root(cref value) {
        json::common::get_filetype(value, json::common::filetype_tilemap);
        rule_tile_size(value);
        rule_filename(value);
        rule_mappings(value);

        map_.reset(std::move(mappings_));
    }

    void rule_tile_size(cref value) {
        auto const size = json::require_array(value[json::common::field_tile_size], 2, 2);

        map_.tile_w = json::require_int(size[0]);
        map_.tile_h = json::require_int(size[1]);
    }

    void rule_filename(cref value) {
        map_.filename = json::common::get_filename(value);
    }

    void rule_mappings(cref value) {
        auto const mappings = json::require_array(value[json::common::field_mappings]);
        for (auto const& mapping : mappings.array_items()) {
            rule_mapping(mapping);
        }
    }

    void rule_mapping(cref value) {
        json::require_array(value, 2, 2);

        auto const type_str  = json::require_string(value[0]);
        auto const id        = json::require_int(value[1]);

        auto const type_hash = slash_hash32(type_str);

        auto const type = enum_map<texture_type>::get(type_hash);
        if (type.value == texture_type::invalid) {
            if (type_hash != type.hash) {
                BK_TODO_FAIL();
            }
        }

        auto const result = mappings_.emplace(type.value, id);
        if (!result.second) {
            BK_TODO_FAIL();
        }
    }

    operator tilemap&&() && {
        return std::move(map_);
    }
private:
    tilemap::map_t mappings_;
    tilemap map_;
};

}

tilemap bkrl::load_tilemap(json::cref data) {
    return tilemap_parser {data};
}

////////////////////////////////////////////////////////////////////////////////
// bkrl::tile_sheet implementation
////////////////////////////////////////////////////////////////////////////////
tile_sheet::tile_sheet(tilemap const& map, renderer& render)
  : map_ {&map}
  , tile_size_ (ipoint2 {map.tile_w, map.tile_h})
  , tile_texture_ {render.create_texture(map.filename)}
  , tile_x_ {tile_texture_.width()  / map.tile_w}
  , tile_y_ {tile_texture_.height() / map.tile_h}
{
}

tile_sheet::tile_sheet(renderer& render, path_string_ref const filename, ipoint2 const size)
  : tile_texture_ {render.create_texture(filename)}
  , tile_size_ (size)
  , tile_x_ {tile_texture_.width()  / tile_size_.x}
  , tile_y_ {tile_texture_.height() / tile_size_.y}
{
}
