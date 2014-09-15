#include "tile_sheet.hpp"
#include "texture_type.hpp"
#include "util.hpp"
#include "assert.hpp"

#include "json11/json11.hpp"

using namespace bkrl;

////////////////////////////////////////////////////////////////////////////////
// bkrl::texture_map::impl_t implementation
////////////////////////////////////////////////////////////////////////////////
class tile_map::impl_t {
public:
    using json_value = json11::Json;
    using cref       = json_value const&;
    using value_type = std::pair<texture_type, texture_id>;

    explicit impl_t(std::string const& source);
    explicit impl_t(string_ref filename);

    texture_id operator[](texture_type const t) const;

    int tile_w() const noexcept { return tile_width_; }
    int tile_h() const noexcept { return tile_height_; }
    string_ref filename() const { return filename_; }


    ////////////////////////////////////////////////////////////////////////////
    void rule_root(cref value);
    void rule_file_type(cref value);
    void rule_file_name(cref value);
    void rule_tile_size(cref value);
    void rule_mappings(cref value);
    void rule_mapping_pair(cref value);
private:
    std::vector<value_type> mappings_;
    std::string filename_;
    int tile_width_;
    int tile_height_;
};

//------------------------------------------------------------------------------
texture_id tile_map::impl_t::operator[](texture_type const t) const {
    return mappings_[static_cast<size_t>(t)].second;
}

//------------------------------------------------------------------------------
tile_map::impl_t::impl_t(string_ref filename)
  : impl_t {bkrl::read_file(filename)}
{
}

//------------------------------------------------------------------------------
tile_map::impl_t::impl_t(std::string const& source) {
    std::string err;
    auto root = json11::Json::parse(source, err);

    rule_root(root);

    //sort by type
    bkrl::sort(mappings_, [](value_type const& a, value_type const& b) {
        return a.first < b.first;
    });

    //check for duplicates
    auto const it = std::adjacent_find(
        std::cbegin(mappings_), std::cend(mappings_)
    );

    if (it != std::cend(mappings_)) {
        BK_TODO_FAIL(); //duplicate mapping
    }

    //check for missing values
    if (mappings_.size() != enum_value(texture_type::enum_size)) {
        for (size_t i = 0; i < mappings_.size(); ++i) {
            if (i != enum_value(mappings_[i].first)) {
                BK_TODO_FAIL(); //fill in defaulted missing values
            }
        }
    }
}

//------------------------------------------------------------------------------
void
tile_map::impl_t::rule_root(cref value) {
    if (!value.is_object()) {
        BK_TODO_FAIL();
    }

    rule_file_type(value);
    rule_file_name(value);
    rule_tile_size(value);
    rule_mappings(value);
}

//------------------------------------------------------------------------------
void
tile_map::impl_t::rule_file_type(cref value) {
    static std::string const key {"file_type"};
    static std::string const valid_file_type {"texture_map"};

    cref file_type = value[key];
    if (!file_type.is_string()) {
        BK_TODO_FAIL();
    }

    if (file_type != valid_file_type) {
        BK_TODO_FAIL();
    }
}

//------------------------------------------------------------------------------
void
tile_map::impl_t::rule_file_name(cref value) {
    static std::string const key {"file_name"};

    cref file_name = value[key];
    if (!file_name.is_string()) {
        BK_TODO_FAIL();
    }

    filename_ = file_name.string_value();
}

//------------------------------------------------------------------------------
void
tile_map::impl_t::rule_tile_size(cref value) {
    static std::string const key {"tile_size"};

    cref size = value[key];
    if (!size.is_array()) {
        BK_TODO_FAIL();
    }

    if (size.array_items().size() != 2) {
        BK_TODO_FAIL();
    }

    auto const w = size[0].int_value();
    auto const h = size[1].int_value();

    if (w <= 0) {
        BK_TODO_FAIL();
    }

    if (h <= 0) {
        BK_TODO_FAIL();
    }

    tile_width_  = w;
    tile_height_ = h;
}

//------------------------------------------------------------------------------
void
tile_map::impl_t::rule_mappings(cref value) {
    static std::string const key {"mappings"};

    cref mappings = value[key];
    if (!mappings.is_array()) {
        BK_TODO_FAIL();
    }

    for (auto const& mapping_pair : mappings.array_items()) {
        rule_mapping_pair(mapping_pair);
    }
}

//------------------------------------------------------------------------------
void
tile_map::impl_t::rule_mapping_pair(cref value) {
    if (!value.is_array()) {
        BK_TODO_FAIL();
    }
    
    if (value.array_items().size() != 2) {
        BK_TODO_FAIL();
    }

    auto const& array = value.array_items();
    if (!array[0].is_string()) {
        BK_TODO_FAIL();
    }

    if (!array[1].is_number()) {
        BK_TODO_FAIL();
    }

    auto const& str  = array[0].string_value();
    auto const  id   = array[1].int_value();
    auto const  hash = slash_hash32(str);
    auto const  e    = enum_map<texture_type>::get(hash);

    if (e.value == texture_type::invalid) {
        if (hash != e.hash) {
            BK_TODO_FAIL();
        }
    }

    mappings_.emplace_back(e.value, id);
}

////////////////////////////////////////////////////////////////////////////////
// bkrl::tile_sheet implementation
////////////////////////////////////////////////////////////////////////////////
tile_sheet::tile_sheet(string_ref filename, renderer& render)
  : map {filename}
  , tile_texture {render.create_texture(map.filename())}
  , tile_x {tile_texture.width() / tile_width()}
  , tile_y {tile_texture.height() / tile_height()}
{
}

////////////////////////////////////////////////////////////////////////////////
// bkrl::tile_map implementation
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
tile_map::~tile_map() = default;

//------------------------------------------------------------------------------
tile_map::tile_map(std::string const& source)
  : impl_ {std::make_unique<impl_t>(source)}
{
}

tile_map::tile_map(string_ref const filename)
  : impl_ {std::make_unique<impl_t>(filename)}
{
}

//------------------------------------------------------------------------------
bkrl::texture_id
tile_map::operator[](
    bkrl::texture_type const type
) const {
    return (*impl_)[type];
}

int
tile_map::tile_w() const noexcept {
    return impl_->tile_w();
}

int
tile_map::tile_h() const noexcept {
    return impl_->tile_h();
}

string_ref
tile_map::filename() const {
    return impl_->filename();
}
