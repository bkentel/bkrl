#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <deque>
#include <boost/container/flat_map.hpp>

#include "utf8.h"

#include "font.hpp"
#include "assert.hpp"
#include "math.hpp"
#include "util.hpp"
#include "optional.hpp"

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
class static_glyph_cache;
class glyph_cache;

//==============================================================================
//! Cached glyph info needed for rendering.
//==============================================================================
struct cached_glyph {
    unicode::codepoint codepoint;
    glyph_index        index;

    uint16_t tex_x;
    uint16_t tex_y;

    uint8_t width;
    uint8_t height;
    int8_t  left;
    int8_t  top;

    uint8_t advance;
    uint8_t reserved[3];

    operator bool() const noexcept {
        return index.value != 0;
    }
};

//==============================================================================
//! Font library; wrapper for operations involving FT_Library objects.
//==============================================================================
class library {
public:
    BK_NOCOPY(library);
    BK_DEFMOVE(library);

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
    BK_DEFMOVE(face);

    enum : unsigned { default_size = 20 };
    enum : unsigned { error_index  = 0  };

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    face(
        library&              lib
      , path_string_ref const filename
      , unsigned        const size = default_size
    )
      : face {static_cast<FT_Library>(lib), filename, size}
    {
    }
    
    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    face(
        font_libary&          lib
      , path_string_ref const filename
      , unsigned        const size = default_size
    )
      : face {lib.handle().as<FT_Library>(), filename, size}
    {
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    operator FT_Face() const noexcept {
        BK_ASSERT_DBG(face_);
        return face_.get();
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    void set_size(
        FT_UInt width
      , FT_UInt height
    );

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    void set_size(FT_UInt const size) {
        set_size(size, size);
    }
    
    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    glyph_index index_of(unicode::codepoint cp) const;

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    unique<FT_Glyph> get_glyph(glyph_index const index) {
        load_glyph_(index);
        return get_glyph_();
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    unique<FT_Glyph> get_glyph(unicode::codepoint const cp) {
        auto const i = index_of(cp);
        return get_glyph(i);
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    unique<FT_BitmapGlyph> get_bitmap_glyph(glyph_index const index) {
        load_glyph_(index);
        return get_bitmap_glyph_();
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    unique<FT_BitmapGlyph> get_bitmap_glyph(unicode::codepoint const cp) {
        auto const i = index_of(cp);
        return get_bitmap_glyph(i);
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    bool has_kerning() const noexcept {
        FT_Face const face = *this;
        return FT_HAS_KERNING(face) != 0;
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    FT_Vector get_kerning(glyph_index prev, glyph_index cur) const;
    
    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    FT_Pos pixel_size() const noexcept { return metrics_().x_ppem        ; }
    FT_Pos ascender()   const noexcept { return metrics_().ascender  >> 6; }
    FT_Pos descender()  const noexcept { return metrics_().descender >> 6; }
    FT_Pos line_gap()   const noexcept { return metrics_().height    >> 6; }
private:
    face(FT_Library lib, path_string_ref filename, unsigned size);

    FT_Size_Metrics const& metrics_() const noexcept {
        FT_Face const face = *this;
        return face->size->metrics;
    }

    void load_glyph_(glyph_index const index);

    void load_glyph_(unicode::codepoint const cp) {
        auto const i = index_of(cp);
        load_glyph_(i);
    }

    unique<FT_Glyph> get_glyph_();
    unique<FT_BitmapGlyph> get_bitmap_glyph_();

    FT_Glyph_Format glyph_format_() const noexcept {
        FT_Face const face = *this;
        return face->glyph->format;
    }
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

    glyph(face& source_face, unicode::codepoint const cp)
      : codepoint_ {cp}
      , index_     {source_face.index_of(cp)}
      , glyph_     {source_face.get_glyph(index_)}
    {
    }

    explicit operator bool() const noexcept { return !!glyph_; }
private:
    unicode::codepoint codepoint_ = unicode::codepoint {0};
    glyph_index        index_     = glyph_index {0};
    
    unique<FT_Glyph> glyph_;
};

//==============================================================================
//! Font bitmap glyph; wrapper for operations involving FT_BitmapGlyph objects.
//==============================================================================
class bitmap_glyph {
public:
    BK_NOCOPY(bitmap_glyph);
    BK_DEFMOVE(bitmap_glyph);

    bitmap_glyph() = default;

    bitmap_glyph(face& source_face, cached_glyph const& cached)
      : codepoint_ {cached.codepoint}
      , index_     {cached.index}
      , glyph_     {source_face.get_bitmap_glyph(cached.index)}
    {
    }

    bitmap_glyph(face& source_face, unicode::codepoint const cp)
      : codepoint_ {cp}
      , index_     {source_face.index_of(cp)}
      , glyph_     {source_face.get_bitmap_glyph(index_)}
    {
    }

    void render(
        uint8_t* const buffer
      , int const xoff   //!< x offset into buffer to write to
      , int const yoff   //!< y offset into buffer to write to
      , int const width  //!< buffer width in pixels
      , int const height //!< buffer height in pixels
      , int const stride //!< buffer stride in bytes per pixel
      , int const pitch  //!< buffer pitch in bytes per row
    ) const;

    int render(std::vector<uint8_t>& buffer) const {
        if (!*this) {
            return 0;
        }
           
        auto const w      = width();
        auto const h      = height();
        auto const stride = 4;
        auto const pitch  = w*stride;

        buffer.reserve(h * pitch);

        auto const out = buffer.data();
            
        render(out, 0, 0, w, h, stride, pitch);

        return pitch;
    }

    FT_Int    left()    const noexcept { return glyph_->left; }
    FT_Int    top()     const noexcept { return glyph_->top; }
    FT_Vector advance() const noexcept { return glyph_->root.advance; }
    int       width()   const noexcept { return glyph_->bitmap.width; }
    int       height()  const noexcept { return glyph_->bitmap.rows; }

    operator cached_glyph() const noexcept {
        cached_glyph result {};
    
        result.codepoint = codepoint_;
        result.index     = index_;

        if (!*this) {
            return result;
        }

        result.width   = static_cast<uint8_t>(width());
        result.height  = static_cast<uint8_t>(height());
        result.left    = static_cast<int8_t>(left());
        result.top     = static_cast<int8_t>(top());
        result.advance = static_cast<uint8_t>(advance().x >> 16);

        return result;
    }

    explicit operator bool() const noexcept {
        return (!!glyph_)
            && (index_ != glyph_index {face::error_index})
            //&& (width()  > 0)
            //&& (height() > 0)
            ;
    }
private:
    unicode::codepoint codepoint_ = unicode::codepoint {};
    glyph_index        index_     = glyph_index {};

    unique<FT_BitmapGlyph> glyph_;
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
    FT_Library      const lib
  , path_string_ref const //filename //TODO
  , unsigned        const size
) {
    BK_ASSERT(lib != nullptr);

    FT_Face face {};
    auto error = FT_New_Face(lib, R"(C:\windows\fonts\meiryo.ttc)", 0, &face);
    if (error) {
        BK_TODO_FAIL();
    }

    face_.reset(face);

    set_size(size);
}

//------------------------------------------------------------------------------
void
face::set_size(FT_UInt const width, FT_UInt const height) {
    BK_ASSERT(width > 0 && height > 0);

    auto const error = FT_Set_Pixel_Sizes(*this, width, height);
    if (error) {
        BK_TODO_FAIL();
    }
}

//------------------------------------------------------------------------------
glyph_index
face::index_of(unicode::codepoint const cp) const {
    auto const i = FT_Get_Char_Index(*this, value_of(cp));
    return glyph_index {i};
}

//------------------------------------------------------------------------------
void
face::load_glyph_(glyph_index const index) {
    auto const i = value_of(index);
    if (i == 0) {
        return;
    }

    auto const error = FT_Load_Glyph(*this, i, FT_LOAD_DEFAULT);
    if (error) {
        BK_TODO_FAIL();
    }
}

//------------------------------------------------------------------------------
unique<FT_Glyph>
face::get_glyph_() {
    if (glyph_format_() == FT_GLYPH_FORMAT_NONE) {
        return {};
    }
    
    FT_Glyph glyph {};
    auto const error = FT_Get_Glyph(face_->glyph, &glyph);
    if (error) {
        BK_TODO_FAIL();
    }

    return unique<FT_Glyph> {glyph};
}

//------------------------------------------------------------------------------
unique<FT_BitmapGlyph>
face::get_bitmap_glyph_() {
    auto glyph = get_glyph_();
    
    if (!glyph) {
        return nullptr;
    }

    auto result = glyph.get();
    if (result->format != FT_GLYPH_FORMAT_BITMAP) {
        auto const error = FT_Glyph_To_Bitmap(&result, FT_RENDER_MODE_NORMAL, nullptr, false);
        if (error) {
            BK_TODO_FAIL();
        }
    }

    BK_ASSERT_DBG(result->format == FT_GLYPH_FORMAT_BITMAP);

    return unique<FT_BitmapGlyph> {
        reinterpret_cast<FT_BitmapGlyph>(result)
    };
}

//------------------------------------------------------------------------------
FT_Vector
face::get_kerning(
    glyph_index const prev
  , glyph_index const cur
) const {
    auto const a = value_of(prev);
    auto const b = value_of(cur);

    if (!has_kerning() || (a == error_index) || (b == error_index)) {
        return FT_Vector {};
    }

    FT_Vector kerning {};
    auto const error = FT_Get_Kerning(*this, a, b, FT_KERNING_DEFAULT, &kerning);
    if (error) {
        BK_TODO_FAIL();
    }

    kerning.x >>= 6;
    kerning.y >>= 6;

    return kerning;
}

////////////////////////////////////////////////////////////////////////////////
// bitmap_glyph
////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// static_glyph_cache
////////////////////////////////////////////////////////////////////////////////
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
            auto const cp  = unicode::codepoint {i};
            auto const bmp = bitmap_glyph {f, cp};

            result.push_back(bmp);
        }

        return result;
    }

    static_glyph_cache() = default;

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

        BK_ASSERT(w > 0 && w <= max);
        BK_ASSERT(h > 0 && h <= max);

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

    void render(renderer& r, texture& tex, face& cur_face) {
        std::vector<uint8_t> buffer;

        auto const render_glyph = [&](cached_glyph const& info) {
            auto const bmp = bitmap_glyph {cur_face, info};
            if (bmp.width() == 0 || bmp.height() == 0) {
                BK_ASSERT_DBG(bmp.width() + bmp.height() == 0);
                return;
            }
            
            auto const pitch = bmp.render(buffer);
            if (pitch == 0) {
                return;
            }

            r.update_texture(tex, buffer.data(), pitch, info.tex_x, info.tex_y, bmp.width(), bmp.height());
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

////////////////////////////////////////////////////////////////////////////////
// glyph_cache
////////////////////////////////////////////////////////////////////////////////

class glyph_cache {
public:
    BK_NOCOPY(glyph_cache);
    BK_DEFMOVE(glyph_cache);

    template <typename Key, typename Value>
    using map_t = boost::container::flat_map<Key, Value>;

    using codepoint = unicode::codepoint;
    using index_t   = int;
    using value_t   = cached_glyph;

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    explicit glyph_cache(renderer& cur_renderer, face& cur_face)
      : renderer_  {cur_renderer}
      , face_      {cur_face}
      , cell_size_ {cur_face.pixel_size()}
    {
        constexpr auto tex_w = 1024;
        constexpr auto tex_h = 1024;

        static_.add_block(cur_face, unicode::basic_latin    {});
        static_.add_block(cur_face, unicode::basic_japanese {});

        auto const p = static_.update_texture_coords(tex_w, tex_h);
        tex_offset_ = p;

        cells_x_ = (tex_w)       / cell_size_;
        cells_y_ = (tex_h - p.y) / cell_size_;

        auto const cells = cells_x_ * cells_y_;
        BK_ASSERT(cells > 0);

        limit_ = static_cast<size_t>(cells);

        cached_.reserve(limit_);
        map_.reserve(limit_);

        for (index_t i = 0; i < cells; ++i) {
            lru_.push_back(i);
        }
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    value_t const& operator[](codepoint const cp) {
        //check the static cache
        auto const static_value = static_[cp];
        if (static_value) {
            cache_hit0_++;
            
            return *static_value; //ok
        }

        //then chech the dynamic cache
        auto const it = map_.find(cp);
        if (it != map_.cend()) {
            cache_hit1_++;
            
            auto const i = it->second;
            freshen(i);

            return cached_[i]; //ok
        }

        //load a value into the cache
        cache_miss_++;
        return insert(cp);
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    void set_texture(texture& cache_tex) {
        cache_tex_ = &cache_tex;
        static_.render(renderer_, cache_tex, face_);
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    tex_point_i get_required_texture_size() const {
        return {1024, 1024};
    }
private:
    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    index_t next_available() {
        auto const size = cached_.size();

        auto const i = (size >= limit_)       //if full
          ? (lru_.back())                     //then use the oldest
          : (cached_.resize(size + 1), size); //otherwise, use the "next" value.

        freshen(i);

        return i;
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    value_t load_value(codepoint const cp, int const x, int const y) {
        auto const   bmp    = bitmap_glyph {face_, cp};
        cached_glyph result = bmp;
        
        result.tex_x = static_cast<uint16_t>(x);
        result.tex_y = static_cast<uint16_t>(y);
       
        auto const pitch = bmp.render(buffer_);
        if (pitch != 0) {
            renderer_.update_texture(
                *cache_tex_
              , buffer_.data()
              , pitch
              , x
              , y
              , result.width
              , result.height
            );
        }
        
        return result;
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    void freshen(index_t const i) {
        auto const beg = std::begin(lru_);
        auto const end = std::end(lru_);
        auto const it  = std::find(beg, end, i);

        if (it == beg) { //no change needed
            return;
        } else if (it == end) { //should never happen
            BK_TODO_FAIL();
        } else { //make i the freshest value
            auto const value = *it;

            lru_.erase(it);
            lru_.push_front(value);
        }
    }

    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    value_t const& insert(codepoint const cp) {
        auto const i = next_available();
        auto& cached = cached_[i];

        auto const old_cp = unicode::codepoint {cached.codepoint};
        map_.erase(old_cp);
        map_.emplace(cp, i);

        auto const result = std::div(i, cells_x_);
        auto const x = result.rem  * cell_size_ + tex_offset_.x;
        auto const y = result.quot * cell_size_ + tex_offset_.y;

        cached = load_value(cp, x, y);

        return cached;
    }

    renderer& renderer_;
    face&     face_;

    int cell_size_;
    int cells_x_;
    int cells_y_;

    size_t                    limit_;  //!< cache size
    static_glyph_cache        static_; //!< statically cached values
    std::vector<value_t>      cached_; //!< cached values
    std::deque<index_t>       lru_;    //!< LRU eviction scheme
    map_t<codepoint, index_t> map_;    //!< map from codepoint -> cache index

    std::vector<uint8_t> buffer_;
    texture*             cache_tex_ = nullptr;
    ipoint2              tex_offset_;

    uint64_t cache_hit0_ = 0;
    uint64_t cache_hit1_ = 0;
    uint64_t cache_miss_ = 0;
};

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

    font_face_impl(renderer& r, font_libary& lib, path_string_ref filename, unsigned size);

    glyph_metrics metrics(unicode::codepoint lhs, unicode::codepoint rhs);
    glyph_metrics metrics(unicode::codepoint cp);

    texture const& get_texture() const;

    auto pixel_size() const noexcept { return face_.pixel_size(); }
    auto ascender()   const noexcept { return face_.ascender(); }
    auto descender()  const noexcept { return face_.descender(); }
    auto line_gap()   const noexcept { return face_.line_gap(); }

    argb8 get_color() const {
        return renderer_.get_color_mod(texture_);
    }

    void set_color(argb8 const color) {
        renderer_.set_color_mod(texture_, color);
    }

    void set_color(rgb8 const color) {
        renderer_.set_color_mod(texture_, color);
    }
private:
    renderer&         renderer_;
    font::face        face_;
    unsigned          face_size_;
    font::glyph_cache cache_;
    texture           texture_;
};

//------------------------------------------------------------------------------
font_face_impl::font_face_impl(
    renderer&             cur_renderer
  , font_libary&          lib
  , path_string_ref const filename
  , unsigned        const size
)
  : renderer_  {cur_renderer}
  , face_      {lib, filename, size}
  , face_size_ {size}
  , cache_     {cur_renderer, face_}
  , texture_   {cur_renderer.create_texture(cache_.get_required_texture_size())}
{
    cache_.set_texture(texture_);
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
      , cached.tex_x
      , cached.tex_y
    };
}

//------------------------------------------------------------------------------
texture const&
font_face_impl::get_texture() const {
    return texture_;
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
