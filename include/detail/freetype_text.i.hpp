#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <deque>

#include "utf8.h"

#include "font.hpp"
#include "assert.hpp"
#include "math.hpp"
#include "util.hpp"

#include "renderer.hpp"

namespace bkrl {

namespace font {
////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct deleter;

template <> struct deleter<FT_Library> {
    void operator()(FT_Library const ptr) const noexcept {
        auto const result = ::FT_Done_FreeType(ptr);
        if (result) {
            BK_TODO_FAIL();
        }
    }
};

template <> struct deleter<FT_Face> {
    void operator()(FT_Face const ptr) const noexcept {
        auto const result = ::FT_Done_Face(ptr);
        if (result) {
            BK_TODO_FAIL();
        }
    }
};

template <> struct deleter<FT_Glyph> {
    void operator()(FT_Glyph const ptr) const noexcept {
        ::FT_Done_Glyph(ptr);
    }
};

template <> struct deleter<FT_BitmapGlyph> {
    void operator()(FT_BitmapGlyph const ptr) const noexcept {
        ::FT_Done_Glyph(reinterpret_cast<FT_Glyph>(ptr));
    }
};

template <typename T, typename U = std::remove_pointer_t<T>>
using unique = std::unique_ptr<U, deleter<T>>;

////////////////////////////////////////////////////////////////////////////////

class library;
class face;
class glyph;
class bitmap_glyph;
class block_cache;

//==============================================================================
//! Font library; wrapper for operations involving FT_Library objects.
//==============================================================================
class library {
public:
    BK_NOCOPY(library);

    library();

    operator FT_Library() const;
private:
    unique<FT_Library> lib_;
};

//==============================================================================
//! Font face; wrapper for operations involving FT_Face objects.
//==============================================================================
class face {
public:
    BK_NOCOPY(face);

    face(FT_Library lib, string_ref const filename, unsigned size);

    face(library& lib, string_ref const filename, unsigned size);

    operator FT_Face() const;

    void set_size(FT_UInt const height);

    void set_size(FT_UInt const width, FT_UInt const height);

    int line_gap() const;

    glyph_index index_of(unicode::codepoint const cp);

    void load_glyph(glyph_index const index);

    void load_glyph(unicode::codepoint const cp);

    unique<FT_Glyph> get_glyph(glyph_index const index);

    unique<FT_Glyph> get_glyph(unicode::codepoint const cp);

    FT_Glyph_Format glyph_format() const noexcept;

    unique<FT_Glyph> get_glyph();

    bool has_kerning() const noexcept;

    FT_Vector get_kerning(glyph_index const lhs, glyph_index const rhs) const;
private:
    unique<FT_Face> face_;
};

//==============================================================================
//! Font glyph; wrapper for operations involving FT_Glyph objects.
//==============================================================================
class glyph {
public:
    BK_NOCOPY(glyph);
    BK_DEFMOVE(glyph);

    glyph() = default;

    glyph(face& source_face, unicode::codepoint const cp);

    glyph(face& source_face, glyph_index const index);

    operator FT_Glyph() const;

    explicit operator bool() const noexcept;
private:
    unique<FT_Glyph> glyph_;
};

//==============================================================================
//! Font bitmap glyph; wrapper for operations involving FT_BitmapGlyph objects.
//==============================================================================
class bitmap_glyph {
public:
    BK_NOCOPY(bitmap_glyph);
    BK_DEFMOVE(bitmap_glyph);

    bitmap_glyph(glyph const& source);

    void render(
        uint8_t* const buffer
      , int const xoff   //x offset into buffer to write to
      , int const yoff   //y offset into buffer to write to
      , int const width  //target buffer width in pixels
      , int const height //target buffer height in pixels
      , int const stride //target buffer stride in bytes per pixel
      , int const pitch  //target buffer pitch in bytes per row
    ) const;

    FT_Int    left()    const noexcept { return glyph_->left; }
    FT_Int    top()     const noexcept { return glyph_->top; }
    FT_Vector advance() const noexcept { return glyph_->root.advance; }
    int       width()   const noexcept { return glyph_->bitmap.width; }
    int       height()  const noexcept { return glyph_->bitmap.rows; }

    explicit operator bool() const noexcept;
private:
    unique<FT_BitmapGlyph> glyph_;
};

//==============================================================================
//! Unicode block cache.
//==============================================================================
class block_cache {
public:
    BK_NOCOPY(block_cache);
    BK_DEFMOVE(block_cache);

    block_cache(face& font_face, unicode::block_value const block);

    void render(
        uint8_t* buffer     //!< output buffer
      , int&     xoff       //!< x offset into buffer to write to
      , int&     yoff       //!< y offset into buffer to write to
      , int      width      //!< target buffer width in pixels
      , int      height     //!< target buffer height in pixels
      , int      stride     //!< target buffer stride in bytes per pixel
      , int      pitch      //!< target buffer pitch in bytes per row
      , int&     row_height //!< height of the current row
    );

    void set_position(unicode::codepoint const cp, FT_Vector const p);

    FT_Vector get_position(unicode::codepoint const cp) const;

    bitmap_glyph const* get_glyph(unicode::codepoint const cp) const;

    glyph_metrics metrics(unicode::codepoint const cp) const;

    int required_area() const noexcept;

    bool contains(unicode::codepoint const cp) const noexcept;
private:
    unicode::block_value      block_;
    int                       required_area_;
    std::vector<bitmap_glyph> glyphs_;
    std::vector<FT_Vector>    pos_;
};

////////////////////////////////////////////////////////////////////////////////
// library
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
library::library() {
    FT_Library library {};
    auto const error = FT_Init_FreeType(&library);
    if (error) {
        BK_TODO_FAIL();
    }

    lib_.reset(library);
}

//------------------------------------------------------------------------------
library::operator FT_Library() const {
    BK_ASSERT_DBG(lib_);
    return lib_.get();
}

////////////////////////////////////////////////////////////////////////////////
// face
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
face::face(
    FT_Library const lib
  , string_ref const //filename //TODO
  , unsigned   const size
) {
    FT_Face face {};
    auto error = FT_New_Face(lib, R"(C:\windows\fonts\meiryo.ttc)", 0, &face);
    if (error) {
        BK_TODO_FAIL();
    }

    face_.reset(face);

    set_size(size);
}

//------------------------------------------------------------------------------
face::face(library& lib, string_ref const filename, unsigned size)
    : face (static_cast<FT_Library>(lib), filename, size)
{
}

//------------------------------------------------------------------------------
face::operator FT_Face() const {
    BK_ASSERT_DBG(face_);
    return face_.get();
}

//------------------------------------------------------------------------------
void
face::set_size(FT_UInt const height) {
    set_size(0, height);
}

//------------------------------------------------------------------------------
void
face::set_size(FT_UInt const width, FT_UInt const height) {
    auto const error = FT_Set_Pixel_Sizes(*this, width, height);
    if (error) {
        BK_TODO_FAIL();
    }
}

//------------------------------------------------------------------------------
int
face::line_gap() const {
    return face_->size->metrics.height >> 6;
}

//------------------------------------------------------------------------------
glyph_index
face::index_of(unicode::codepoint const cp) {
    return glyph_index {
        FT_Get_Char_Index(*this, value_of(cp))
    };
}

//------------------------------------------------------------------------------
void
face::load_glyph(glyph_index const index) {
    auto const i = value_of(index);
    if (!i) {
        return;
    }

    auto const error = FT_Load_Glyph(*this, i, FT_LOAD_DEFAULT);
    if (error) {
        BK_TODO_FAIL();
    }
}

//------------------------------------------------------------------------------
void
face::load_glyph(unicode::codepoint const cp) {
    load_glyph(index_of(cp));
}

//------------------------------------------------------------------------------
unique<FT_Glyph>
face::get_glyph(glyph_index const index) {
    load_glyph(index);
    return get_glyph();
}

//------------------------------------------------------------------------------
unique<FT_Glyph>
face::get_glyph(unicode::codepoint const cp) {
    return get_glyph(index_of(cp));
}

//------------------------------------------------------------------------------
FT_Glyph_Format
face::glyph_format() const noexcept {
    return face_->glyph->format;
}

//------------------------------------------------------------------------------
unique<FT_Glyph>
face::get_glyph() {
    FT_Glyph glyph {};

    if (glyph_format() == FT_GLYPH_FORMAT_NONE) {
        return {};
    }

    auto const error = FT_Get_Glyph(face_->glyph, &glyph);
    if (error) {
        BK_TODO_FAIL();
    }

    return unique<FT_Glyph> {glyph};
}

//------------------------------------------------------------------------------
bool
face::has_kerning() const noexcept {
    return !!FT_HAS_KERNING(face_.get());
}

//------------------------------------------------------------------------------
FT_Vector
face::get_kerning(glyph_index const lhs, glyph_index const rhs) const {
    auto const left  = value_of(lhs);
    auto const right = value_of(rhs);

    if (!has_kerning() || !left || !right) {
        return FT_Vector {};
    }

    FT_Vector kerning {};
    auto const error = FT_Get_Kerning(*this, left, right, FT_KERNING_DEFAULT, &kerning);
    if (error) {
        BK_TODO_FAIL();
    }

    kerning.x >>= 6;
    kerning.y >>= 6;

    return kerning;
}

////////////////////////////////////////////////////////////////////////////////
// glyph
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
glyph::glyph(face& source_face, unicode::codepoint const cp)
    : glyph(source_face, source_face.index_of(cp))
{
}

//------------------------------------------------------------------------------
glyph::glyph(face& source_face, glyph_index const index) {
    glyph_ = source_face.get_glyph(index);
}

//------------------------------------------------------------------------------
glyph::operator FT_Glyph() const {
    BK_ASSERT_DBG(glyph_);
    return glyph_.get();
}

//------------------------------------------------------------------------------
glyph::operator bool() const noexcept {
    return !!glyph_;
}

////////////////////////////////////////////////////////////////////////////////
// bitmap_glyph
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
bitmap_glyph::bitmap_glyph(glyph const& source) {
    if (!source) {
        return;
    }

    FT_Glyph g = source;

    if (g->format != FT_GLYPH_FORMAT_BITMAP) {
        auto const error = FT_Glyph_To_Bitmap(&g, FT_RENDER_MODE_NORMAL, nullptr, false);
        if (error) {
            BK_TODO_FAIL();
        }
    }

    BK_ASSERT_DBG(g->format == FT_GLYPH_FORMAT_BITMAP);

    glyph_.reset(reinterpret_cast<FT_BitmapGlyph>(g));
}

//------------------------------------------------------------------------------
void bitmap_glyph::render(
    uint8_t* const buffer
    , int const xoff   //x offset into buffer to write to
    , int const yoff   //y offset into buffer to write to
    , int const width  //target buffer width in pixels
    , int const height //target buffer height in pixels
    , int const stride //target buffer stride in bytes per pixel
    , int const pitch  //target buffer pitch in bytes per row
) const {
    BK_ASSERT_DBG(xoff >= 0);
    BK_ASSERT_DBG(yoff >= 0);

    BK_ASSERT_DBG(width  > 0);
    BK_ASSERT_DBG(height > 0);

    BK_ASSERT_DBG(stride >= 1);
    BK_ASSERT_DBG(stride <= 4);

    BK_ASSERT_DBG(pitch >= width * stride);

    auto const bitmap = glyph_->bitmap;

    auto const src_w = bitmap.width;
    auto const src_h = bitmap.rows;

    BK_ASSERT_DBG(xoff + src_w <= width);
    BK_ASSERT_DBG(yoff + src_h <= height);

    //TEMP TODO
    BK_ASSERT_OPT(stride == 4);

    auto dst = buffer + (yoff * pitch) + (xoff * stride);
    auto src = bitmap.buffer;

    for (int y = 0; y < src_h; ++y) {
        for (int x = 0; x < src_w; ++x) {
            auto const out  = dst + x*stride;
            auto const from = src + x;

            //TODO split out into a family of functions
            out[0] = 0xFF;  //b
            out[1] = 0xFF;  //g
            out[2] = 0xFF;  //r
            out[3] = *from; //a
        }

        dst += pitch;
        src += bitmap.pitch;
    }
}

//------------------------------------------------------------------------------
bitmap_glyph::operator bool() const noexcept {
    return !!glyph_;
}

////////////////////////////////////////////////////////////////////////////////
// block_cache
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
block_cache::block_cache(face& font_face, unicode::block_value const block)
    : block_ {block}
    , required_area_ {0}
{
    auto const first = value_of(block.first);
    auto const last  = value_of(block.last);
    auto const size  = block.size();

    glyphs_.reserve(size);
    pos_.resize(size);

    int max_h = 0;
    int max_w = 0;
    int count = 0;

    for (auto i = first; i <= last; ++i) {
        auto const cp = unicode::codepoint {i};

        glyphs_.emplace_back(
            glyph {font_face, cp}
        );

        auto const& bmp_glyph = glyphs_.back();
        if (!bmp_glyph) {
            continue;
        }

        auto const h = bmp_glyph.width();
        auto const w = bmp_glyph.height();

        if (w && h) {
            max_h = std::max(h, max_h);
            max_w = std::max(w, max_w);
            count++;
        }

        BK_ASSERT_DBG(!w || (w && h));
        BK_ASSERT_DBG(!h || (w && h));
    }

    required_area_ = max_h * max_w * count;
}

//------------------------------------------------------------------------------
void block_cache::render(
    uint8_t* const buffer
    , int & xoff   //x offset into buffer to write to
    , int & yoff   //y offset into buffer to write to
    , int const width  //target buffer width in pixels
    , int const height //target buffer height in pixels
    , int const stride //target buffer stride in bytes per pixel
    , int const pitch  //target buffer pitch in bytes per row
    , int& row_height //height of the current row
) {
    constexpr auto padding = 1;

    auto i = 0;
    for (auto const& g : glyphs_) {
        if (!g) {
            pos_[i++] = FT_Vector {};
            continue;
        }

        auto const w = g.width();
        auto const h = g.height();

        if (xoff + w > width) {
            BK_ASSERT_DBG(row_height > 0);

            xoff =  0;
            yoff += row_height + padding;
            row_height = 0;
        }

        pos_[i++] = FT_Vector {xoff, yoff};

        row_height = std::max(row_height, h);
        g.render(buffer, xoff, yoff, width, height, stride, pitch);

        xoff += w + padding;
    }
}

//------------------------------------------------------------------------------
void block_cache::set_position(unicode::codepoint const cp, FT_Vector const p) {
    auto const i = block_.offset(cp);
    pos_[i] = p;
}

//------------------------------------------------------------------------------
FT_Vector block_cache::get_position(unicode::codepoint const cp) const {
    auto const i = block_.offset(cp);
    return pos_[i];
}

//------------------------------------------------------------------------------
bitmap_glyph const* block_cache::get_glyph(unicode::codepoint const cp) const {
    auto const i = block_.offset(cp);
    auto const& g = glyphs_[i];
        
    return (!!g) ? &g : nullptr;
}

//------------------------------------------------------------------------------
glyph_metrics block_cache::metrics(unicode::codepoint const cp) const {
    auto const g = get_glyph(cp);
    if (!g) {
        return {};
    }

    return glyph_metrics {
        g->width()
        , g->height()
        , g->left()
        , g->top()
        , g->advance().x >> 16
        , g->advance().y >> 16
    };
}

//------------------------------------------------------------------------------
int block_cache::required_area() const noexcept {
    return required_area_;
}

//------------------------------------------------------------------------------
bool block_cache::contains(unicode::codepoint const cp) const noexcept {
    return block_.contains(cp);
}

/////////////////////
//////

//------------------------------------------------------------------------------
struct cached_glyph {
    unicode::codepoint codepoint;
    glyph_index        index;

    opaque_handle<texture> texture;

    uint16_t tex_x;
    uint16_t tex_y;

    uint8_t width;
    uint8_t height;
    uint8_t left;
    uint8_t top;

    uint8_t advance;   
    uint8_t reserved[3];

    operator bool() const noexcept {
        return index.value != 0;
    }
};

//------------------------------------------------------------------------------
cached_glyph load_glyph(face& f, unicode::codepoint const cp) {
    cached_glyph result {};
    
    result.codepoint = cp;
    result.index     = f.index_of(cp);

    if (result.index == glyph_index {0}) {
        return result;
    }

    auto g   = glyph        {f, result.index};
    auto bmp = bitmap_glyph {g};

    result.width   = bmp.width();
    result.height  = bmp.height();
    result.left    = bmp.left();
    result.top     = bmp.top();
    result.advance = (bmp.advance().x >> 16);

    return result;
}

void render_glyph(
    cached_glyph const& info
  , std::vector<uint8_t>& buffer
  , renderer& r
  , texture& tex
  , face& f
) {
    if (!info) {
        return;
    }

    if (info.width == 0 || info.height == 0) {
        BK_ASSERT_DBG(info.width == 0 && info.height == 0);
        return;
    }

    auto g = glyph {f, info.index};
    auto const bmp = bitmap_glyph {g};
            
    auto const w      = bmp.width();
    auto const h      = bmp.height();
    auto const stride = 4;
    auto const pitch  = w*stride;

    buffer.reserve(h * pitch);
            
    auto const out = buffer.data();
            
    bmp.render(out, 0, 0, w, h, stride, pitch);

    r.update_texture(tex, out, pitch, info.tex_x, info.tex_y, w, h);
};

//------------------------------------------------------------------------------
class static_glyph_cache {
public:
    using key_t   = unicode::block_value;
    using value_t = std::vector<cached_glyph>;

    static value_t init_block(face& f, unicode::block_value const block) {
        auto const beg = value_of(block.first);
        auto const end = value_of(block.last);
        auto const len = block.size();

        value_t result;
        result.reserve(len);

        for (auto i = beg; i <= end; ++i) {
            auto const cp = unicode::codepoint {i};
            result.emplace_back(load_glyph(f, cp));
        }

        return result;
    }

    static_glyph_cache() = default;

    //static_glyph_cache(std::initializer_list<unicode::block_value> blocks) {
    //    map_.reserve(blocks.size());

    //    for (auto const& block : blocks) {
    //        map_.emplace(block, init_block(block));
    //    }
    //}

    void add_block(face& f, unicode::block_value const block) {
        map_.emplace(block, init_block(f, block));
    }

    optional<cached_glyph const&> operator[](unicode::codepoint const cp) const {
        auto const it = map_.find(unicode::block_value {cp});

        if (it == std::cend(map_)) {
            return {};
        }

        auto const& block  = it->first;
        auto const& glyphs = it->second;

        BK_ASSERT_DBG(block.contains(cp));

        auto const i = block.offset(cp);
        return glyphs[i];
    }

    ipoint2 update_texture_coords(int const w, int const h) {
        using x_type = decltype(cached_glyph::tex_x);
        using y_type = decltype(cached_glyph::tex_y);

        static_assert(std::is_same<x_type, y_type>::value, "");
        
        using type = x_type;

        static auto const max = std::numeric_limits<type>::max();

        BK_ASSERT_SAFE(w > 0 && w <= max);
        BK_ASSERT_SAFE(h > 0 && h <= max);

        auto const tw = static_cast<type>(w);
        auto const th = static_cast<type>(h);

        type cur_x = 0;
        type cur_y = 0;
        type row_h = 0;

        auto const update_glyph = [&cur_x, &cur_y, &row_h, tw, th](cached_glyph& glyph) {
            type const gw = glyph.width;
            type const gh = glyph.height;

            if (cur_x + gw > tw) {
                cur_x  = 0;
                cur_y += row_h;
                row_h  = 0;
            }

            if ((cur_x + gw > tw) || (cur_y + gh > th)) {
                BK_TODO_FAIL();
            }
            
            glyph.tex_x = cur_x;
            glyph.tex_y = cur_y;

            row_h  = std::max(row_h, gh);
            cur_x += gw;
        };

        for (auto& block : map_) {
            for (auto& glyph : block.second) {
                update_glyph(glyph);
            }
        }

        //move to the next row
        return {0, cur_y + row_h};
    }

    void render(renderer& r, texture& tex, face& f) {
        std::vector<uint8_t> buffer;

        auto const render_glyph = [&](cached_glyph const& info) {
            if (!info) {
                return;
            }

            if (info.width == 0 || info.height == 0) {
                BK_ASSERT_DBG(info.width == 0 && info.height == 0);
                return;
            }

            auto g = glyph {f, info.index};
            auto const bmp = bitmap_glyph {g};
            
            auto const w      = bmp.width();
            auto const h      = bmp.height();
            auto const stride = 4;
            auto const pitch  = w*stride;

            buffer.reserve(h * pitch);
            
            auto const out = buffer.data();
            
            bmp.render(out, 0, 0, w, h, stride, pitch);

            r.update_texture(tex, out, pitch, info.tex_x, info.tex_y, w, h);
        };

        for (auto& block : map_) {
            for (auto& glyph : block.second) {
                render_glyph(glyph);
            }
        }
    }
private:
    boost::container::flat_map<key_t, value_t> map_;
};

//------------------------------------------------------------------------------
class glyph_cache {
public:
    template <typename Key, typename Value>
    using map_t = boost::container::flat_map<Key, Value>;

    using codepoint = unicode::codepoint;
    using index_t   = int;
    
    using value_t = cached_glyph;

    //--------------------------------------------------------------------------
    explicit glyph_cache(renderer& r, face& f)
      : renderer_ {r}
      , face_ {f}
    {
        constexpr auto tex_w = 1024;
        constexpr auto tex_h = 1024;
        constexpr auto cell_size = 20;

        static_.add_block(f, unicode::basic_latin    {});
        static_.add_block(f, unicode::basic_japanese {});

        auto const p = static_.update_texture_coords(tex_w, tex_h);
        tex_offset_ = p;

        auto const cells = (tex_w / cell_size) * ((tex_h - p.y) / cell_size);
        BK_ASSERT_SAFE(cells > 0);

        limit_ = static_cast<size_t>(cells);

        cached_.reserve(limit_);
        map_.reserve(limit_);

        for (index_t i = 0; i < cells; ++i) {
            lru_.push_back(i);
        }
    }

    //--------------------------------------------------------------------------
    index_t oldest() const {
        std::cout << "evict\n";

        return lru_.back();
    }

    //--------------------------------------------------------------------------
    index_t next_available() {
        auto const size = cached_.size();

        auto const i = (size >= limit_)
          ? oldest()
          : (cached_.resize(size + 1), size);

        freshen(i);

        return i;
    }

    //--------------------------------------------------------------------------
    value_t load_value(codepoint const cp, int x, int y) {
        std::cout << "load\n";

        auto result = load_glyph(face_, cp);
        result.tex_x = x;
        result.tex_y = y;
        
       
        render_glyph(result, buffer_, renderer_, *texture_, face_);

        return result;
    }

    //--------------------------------------------------------------------------
    void freshen(index_t const i) {
        auto const beg = std::begin(lru_);
        auto const end = std::end(lru_);

        auto const it = std::find(beg, end, i);
        if (it == beg) {
            //i is already the mru value.
            return;
        } else if (it == end) {
            //should never happed
            BK_TODO_FAIL();
        }


        //make i the mru value
        auto const value = *it;

        lru_.erase(it);
        lru_.push_front(value);
    }

    //--------------------------------------------------------------------------
    value_t const& insert(codepoint const cp) {
        std::cout << "insert\n";

        auto const i = next_available();
        auto& cached = cached_[i];

        auto const old_cp = unicode::codepoint {cached.codepoint};
        map_.erase(old_cp);
        map_.emplace(cp, i);

        auto const gw = 1024 / 20;
        auto const gh = 1024 / 20;

        auto const x = (i % gw) * 20;
        auto const y = tex_offset_.y + (i / gw) * 20;

        cached = load_value(cp, x, y);

        return cached;
    }

    //--------------------------------------------------------------------------
    value_t const& operator[](codepoint const cp) {
        auto const static_value = static_[cp];
        if (static_value) {
            return *static_value;
        }

        auto const it = map_.find(cp);
        if (it != map_.cend()) {
            std::cout << "hit\n";

            auto const i = it->second;
            freshen(i);
            return cached_[i];
        }

        return insert(cp);
    }

    //--------------------------------------------------------------------------
    void render(renderer& r, texture& t, face& f) {
        static_.render(r, t, f);
        texture_ = &t;
        //TODO
    }
    //--------------------------------------------------------------------------
    ipoint2 get_required_texture_size() {
        return {1024, 1024};
    }
private:
    renderer&                 renderer_;
    face&                     face_;
    size_t                    limit_;  //!< cache size
    static_glyph_cache        static_; //!< statically cached values
    std::vector<value_t>      cached_; //!< cached values
    std::deque<index_t>       lru_;    //!< LRU eviction scheme
    map_t<codepoint, index_t> map_;    //!< map from codepoint -> cache index

    std::vector<uint8_t> buffer_;
    texture* texture_ = nullptr;
    ipoint2 tex_offset_;
};

////
////////

} //namespace font


namespace detail {

//==============================================================================
//! font_libary implementation
//==============================================================================
class font_library_impl {
public:
    font_library_impl() = default;

    font_libary::handle_t handle() const {
        return {static_cast<FT_Library>(lib_)};
    }
private:
    font::library lib_;
};

//==============================================================================
//! font_face implementation
//==============================================================================
class font_face_impl {
public:
    BK_NOCOPY(font_face_impl);

    font_face_impl(renderer& r, font_libary& lib, string_ref filename, unsigned size);

    glyph_metrics metrics(unicode::codepoint lhs, unicode::codepoint rhs);
    glyph_metrics metrics(unicode::codepoint cp);

    font_face::texture_info get_texture(unicode::codepoint cp);

    int line_gap() const { return face_.line_gap(); }
private:   
    renderer&         renderer_;
    font::face        face_;
    unsigned          face_size_;
    font::glyph_cache cache_;   
    texture           texture_;
};

//------------------------------------------------------------------------------
//std::vector<font::block_cache>
//font_face_impl::load_blocks_(font::face& cur_face) {
//    std::vector<font::block_cache> blocks;
//
//    blocks.reserve(4);
//
//    blocks.emplace_back(cur_face, unicode::block_value {unicode::basic_latin {}});
//    blocks.emplace_back(cur_face, unicode::block_value {unicode::basic_japanese {}});
//}
//
//texture
//font_face_impl::create_texture_(blocks_t const& blocks) {
//    int total_area = 0;
//    for (auto const& block : blocks) {
//        total_area += block.required_area();
//    }
//
//    auto const ideal_size = static_cast<int>(
//        std::ceil(std::sqrt(total_area))
//    );
//
//    auto const w = next_nearest_power_of_2(ideal_size);
//    auto const h = next_nearest_power_of_2(total_area / w);
//
//    std::vector<uint8_t> buffer;
//    buffer.resize(w * h * 4, 0);
//
//    int xoff  = 0;
//    int yoff  = 0;
//    int row_h = 0;
//
//    for (auto& block : blocks) {
//        block.render(buffer.data(), xoff, yoff, w, h, 4, w*4, row_h);
//    }
//
//    block_texture_ = r.create_texture(buffer.data(), power_of_2_w, power_of_2_h);
//}

font_face_impl::font_face_impl(
    renderer&    r
  , font_libary& lib
  , string_ref   const filename
  , unsigned     const size
)
  : renderer_ {r}
  , face_ {lib.handle().as<FT_Library>(), filename, size}
  , face_size_ {size}
  , cache_ {r, face_}
  , texture_ {r.create_texture(1024, 1024)}
{
    cache_.render(renderer_, texture_, face_);
}

//------------------------------------------------------------------------------
glyph_metrics
font_face_impl::metrics(
    unicode::codepoint const cp
) {
    auto const cached = cache_[cp];

    return glyph_metrics {
        cached.width
      , cached.height
      , cached.left
      , cached.top
      , cached.advance
      , 0
    };
}

//------------------------------------------------------------------------------
font_face::texture_info
font_face_impl::get_texture(
    unicode::codepoint const cp
) {
    //TODO
    auto const cached = cache_[cp];

    return font_face::texture_info {
        &texture_
      , rect {
            static_cast<float>(cached.tex_x)
          , static_cast<float>(cached.tex_y)
          , static_cast<float>(cached.tex_x + cached.width)
          , static_cast<float>(cached.tex_y + cached.height)
        }
    };
}

//------------------------------------------------------------------------------
glyph_metrics
font_face_impl::metrics(
    unicode::codepoint const lhs
  , unicode::codepoint const rhs
) {
    auto const index_l = face_.index_of(lhs);
    auto const index_r = face_.index_of(rhs);

    auto const kerning        = face_.get_kerning(index_l, index_r);
    auto       result_metrics = metrics(rhs);

    result_metrics.left += kerning.x;
    result_metrics.top  += kerning.y;

    return result_metrics;
}

}} //namespace bkrl::detail
