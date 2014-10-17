#include "tile_sheet.hpp"
#include "texture_type.hpp"
#include "util.hpp"
#include "assert.hpp"
#include "algorithm.hpp"
#include "json.hpp"

#include <utf8.h>

using namespace bkrl;

////////////////////////////////////////////////////////////////////////////////
// bkrl::texture_map::impl_t implementation
////////////////////////////////////////////////////////////////////////////////
//class tile_map::impl_t {
//public:
//    using cref       = json::cref;
//    using value_type = std::pair<texture_type, texture_id>;
//
//    explicit impl_t(std::string const& source);
//    explicit impl_t(path_string_ref filename);
//
//    texture_id operator[](texture_type const t) const;
//
//    int tile_w() const noexcept { return tile_width_; }
//    int tile_h() const noexcept { return tile_height_; }
//    path_string_ref filename() const { return filename_; }
//
//    ////////////////////////////////////////////////////////////////////////////
//    void rule_root(cref value);
//    void rule_file_type(cref value);
//    void rule_file_name(cref value);
//    void rule_tile_size(cref value);
//    void rule_mappings(cref value);
//    void rule_mapping_pair(cref value);
//private:
//    std::vector<value_type> mappings_;
//    path_string filename_;
//    int tile_width_;
//    int tile_height_;
//};

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
        auto const filename = json::require_string(value[json::common::field_filename]);

        map_.filename.reserve(filename.length() / 2);
        utf8::utf8to16( //TODO this should be under windows only
            std::cbegin(filename)
          , std::cend(filename)
          , std::back_inserter(map_.filename)
        );
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

////------------------------------------------------------------------------------
//texture_id tile_map::impl_t::operator[](texture_type const t) const {
//    return mappings_[static_cast<size_t>(t)].second;
//}
//
////------------------------------------------------------------------------------
//tile_map::impl_t::impl_t(path_string_ref const filename)
//  : impl_t {bkrl::read_file(filename)}
//{
//}
//
////------------------------------------------------------------------------------
//tile_map::impl_t::impl_t(utf8string const& source) {
//    auto const root = json::common::from_memory(source);
//    rule_root(root);
//
//    //sort by type
//    bkrl::sort(mappings_, [](auto const& a, auto const& b) {
//        return a.first < b.first;
//    });
//
//    //check for duplicates
//    auto const it = bkrl::adjacent_find(mappings_);
//
//    if (it != std::cend(mappings_)) {
//        BK_TODO_FAIL(); //duplicate mapping
//    }
//
//    //check for missing values
//    if (mappings_.size() != enum_value(texture_type::enum_size)) {
//        for (size_t i = 0; i < mappings_.size(); ++i) {
//            if (i != enum_value(mappings_[i].first)) {
//                BK_TODO_FAIL(); //fill in defaulted missing values
//            }
//        }
//    }
//}
//
////------------------------------------------------------------------------------
//void
//tile_map::impl_t::rule_root(cref value) {
//    json::require_object(value);
//
//    rule_file_type(value);
//    rule_file_name(value);
//    rule_tile_size(value);
//    rule_mappings(value);
//}
//
////------------------------------------------------------------------------------
//void tile_map::impl_t::rule_file_type(cref value) {
//    json::common::get_filetype(value, json::common::filetype_texture_map);
//}
//
////------------------------------------------------------------------------------
//void tile_map::impl_t::rule_file_name(cref value) {
//    auto const& field = json::common::field_filename;
//    filename_ = json::require_string(value[field]).to_string();
//}
//
////------------------------------------------------------------------------------
//void tile_map::impl_t::rule_tile_size(cref value) {
//    auto const& field = json::common::field_tile_size;
//
//    cref size = json::require_array(value[field], 2, 2);
//
//    auto const w = json::require_int(size[0]);
//    auto const h = json::require_int(size[1]);
//
//    if (w <= 0) {
//        BK_TODO_FAIL();
//    }
//
//    if (h <= 0) {
//        BK_TODO_FAIL();
//    }
//
//    tile_width_  = w;
//    tile_height_ = h;
//}
//
////------------------------------------------------------------------------------
//void tile_map::impl_t::rule_mappings(cref value) {
//    auto const& field = json::common::field_mappings;
//
//    cref mappings = json::require_array(value[field]);
//
//    for (cref mapping_pair : mappings.array_items()) {
//        rule_mapping_pair(mapping_pair);
//    }
//}
//
////------------------------------------------------------------------------------
//void tile_map::impl_t::rule_mapping_pair(cref value) {
//    cref array = json::require_array(value, 2, 2);
//
//    auto const& str  = json::require_string(array[0]);
//    auto const  id   = json::require_int(array[1]);
//    auto const  hash = slash_hash32(str);
//    auto const  e    = enum_map<texture_type>::get(hash);
//
//    if (e.value == texture_type::invalid) {
//        if (hash != e.hash) {
//            BK_TODO_FAIL();
//        }
//    }
//
//    mappings_.emplace_back(e.value, id);
//}

////////////////////////////////////////////////////////////////////////////////
// bkrl::tile_sheet implementation
////////////////////////////////////////////////////////////////////////////////
tile_sheet::tile_sheet(tilemap const& map, renderer& render)
  : map_ {&map}
  , tile_texture_ {render.create_texture(map.filename)}
  , tile_x_ {tile_texture_.width()  / tile_width()}
  , tile_y_ {tile_texture_.height() / tile_height()}
{
}



////////////////////////////////////////////////////////////////////////////////
// bkrl::tile_map implementation
////////////////////////////////////////////////////////////////////////////////
////------------------------------------------------------------------------------
//tile_map::~tile_map() = default;
//
////------------------------------------------------------------------------------
//tile_map::tile_map(std::string const& source)
//  : impl_ {std::make_unique<impl_t>(source)}
//{
//}
//
//tile_map::tile_map(string_ref const filename)
//  : impl_ {std::make_unique<impl_t>(filename)}
//{
//}
//
////------------------------------------------------------------------------------
//bkrl::texture_id
//tile_map::operator[](
//    bkrl::texture_type const type
//) const {
//    return (*impl_)[type];
//}
//
//int
//tile_map::tile_w() const noexcept {
//    return impl_->tile_w();
//}
//
//int
//tile_map::tile_h() const noexcept {
//    return impl_->tile_h();
//}
//
//string_ref
//tile_map::filename() const {
//    return impl_->filename();
//}
