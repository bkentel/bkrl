#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

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

template <>
struct deleter<FT_Library> {
    void operator()(FT_Library const ptr) const noexcept {
        auto const result = ::FT_Done_FreeType(ptr);
        if (result) {
            BK_TODO_FAIL();
        }
    }
};

template <>
struct deleter<FT_Face> {
    void operator()(FT_Face const ptr) const noexcept {
        auto const result = ::FT_Done_Face(ptr);
        if (result) {
            BK_TODO_FAIL();
        }
    }
};

template <>
struct deleter<FT_Glyph> {
    void operator()(FT_Glyph const ptr) const noexcept {
        ::FT_Done_Glyph(ptr);
    }
};

template <>
struct deleter<FT_BitmapGlyph> {
    void operator()(FT_BitmapGlyph const ptr) const noexcept {
        ::FT_Done_Glyph(reinterpret_cast<FT_Glyph>(ptr));
    }
};

template <typename T, typename U = std::remove_pointer_t<T>>
using unique = std::unique_ptr<U, deleter<T>>;

////////////////////////////////////////////////////////////////////////////////

//==============================================================================
//! Font library; wrapper for operations involving FT_Library objects.
//==============================================================================
class library {
public:
    BK_NOCOPY(library);

    library() {
        FT_Library library {};
        auto const error = FT_Init_FreeType(&library);
        if (error) {
            BK_TODO_FAIL();
        }

        lib_.reset(library);
    }

    operator FT_Library() const {
        BK_ASSERT_DBG(lib_);
        return lib_.get();
    }
private:
    unique<FT_Library> lib_;
};

//==============================================================================
//! Font face; wrapper for operations involving FT_Face objects.
//==============================================================================
class face {
public:
    BK_NOCOPY(face);

    face(FT_Library lib, string_ref const filename, unsigned size) {
        FT_Face face {};
        auto error = FT_New_Face(lib, R"(C:\windows\fonts\meiryo.ttc)", 0, &face);
        if (error) {
            BK_TODO_FAIL();
        }

        face_.reset(face);

        set_size(size);
    }

    face(library& lib, string_ref const filename, unsigned size)
      : face (static_cast<FT_Library>(lib), filename, size)
    {
    }

    operator FT_Face() const {
        BK_ASSERT_DBG(face_);
        return face_.get();
    }

    void set_size(FT_UInt const height) {
        set_size(0, height);
    }

    void set_size(FT_UInt const width, FT_UInt const height) {
        auto const error = FT_Set_Pixel_Sizes(*this, width, height);
        if (error) {
            BK_TODO_FAIL();
        }
    }

    glyph_index index_of(unicode::codepoint const cp) {
        return glyph_index {
            FT_Get_Char_Index(*this, value_of(cp))
        };
    }

    void load_glyph(glyph_index const index) {
        auto const i = value_of(index);
        if (!i) {
            return;
        }

        auto const error = FT_Load_Glyph(*this, i, FT_LOAD_DEFAULT);
        if (error) {
            BK_TODO_FAIL();
        }
    }

    void load_glyph(unicode::codepoint const cp) {
        load_glyph(index_of(cp));
    }

    unique<FT_Glyph> get_glyph(glyph_index const index) {
        load_glyph(index);
        return get_glyph();
    }

    unique<FT_Glyph> get_glyph(unicode::codepoint const cp) {
        return get_glyph(index_of(cp));
    }

    FT_Glyph_Format glyph_format() const noexcept {
        return face_->glyph->format;
    }

    unique<FT_Glyph> get_glyph() {
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

    bool has_kerning() const noexcept {
        return !!FT_HAS_KERNING(face_.get());
    }

    FT_Vector get_kerning(glyph_index const lhs, glyph_index const rhs) const {
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

        return kerning;
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
    BK_DEFAULT_MOVE(glyph);

    glyph(face& source_face, unicode::codepoint const cp)
      : glyph(source_face, source_face.index_of(cp))
    {
    }

    glyph(face& source_face, glyph_index const index) {
        glyph_ = source_face.get_glyph(index);
    }

    operator FT_Glyph() const {
        BK_ASSERT_DBG(glyph_);
        return glyph_.get();
    }

    explicit operator bool() const noexcept {
        return !!glyph_;
    }
private:
    unique<FT_Glyph> glyph_;
};

//==============================================================================
//! Font bitmap glyph; wrapper for operations involving FT_BitmapGlyph objects.
//==============================================================================
class bitmap_glyph {
public:
    BK_NOCOPY(bitmap_glyph);
    BK_DEFAULT_MOVE(bitmap_glyph);

    bitmap_glyph(glyph& source) {
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

    void render(
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

        BK_ASSERT_DBG(xoff + src_w < width);
        BK_ASSERT_DBG(yoff + src_h < height);

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

    FT_Int    left()    const noexcept { return glyph_->left; }
    FT_Int    top()     const noexcept { return glyph_->top; }
    FT_Vector advance() const noexcept { return glyph_->root.advance; }
    int       width()   const noexcept { return glyph_->bitmap.width; }
    int       height()  const noexcept { return glyph_->bitmap.rows; }

    explicit operator bool() const noexcept {
        return !!glyph_;
    }
private:
    unique<FT_BitmapGlyph> glyph_;
};

//==============================================================================
//! Unicode block cache.
//==============================================================================
class block_cache {
public:
    BK_NOCOPY(block_cache);
    BK_DEFAULT_MOVE(block_cache);

    block_cache(face& font_face, unicode::block_value const block)
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

            BK_ASSERT_DBG(!w || w && h);
            BK_ASSERT_DBG(!h || w && h);
        }

        required_area_ = max_h * max_w * count;
    }

    void render(
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
            pos_[i++] = FT_Vector {xoff, yoff};

            if (!g) {
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

            row_height = std::max(row_height, h);
            g.render(buffer, xoff, yoff, width, height, stride, pitch);

            xoff += w + padding;
        }
    }

    void set_position(unicode::codepoint const cp, FT_Vector const p) {
        auto const i = block_.offset(cp);
        pos_[i] = p;
    }

    FT_Vector get_position(unicode::codepoint const cp) const {
        auto const i = block_.offset(cp);
        return pos_[i];
    }

    int required_area() const noexcept {
        return required_area_;
    }
private:
    unicode::block_value      block_;
    int                       required_area_;
    std::vector<bitmap_glyph> glyphs_;
    std::vector<FT_Vector>    pos_;
};

////////////////////////////////////////////////////////////////////////////////
//class font_code_block_cache {
//public:
//    using rect = axis_aligned_rect<int>;
//
//    int load_glyphs(FT_Face const face, unicode::block_value const block) {
//        BK_ASSERT_DBG(glyphs_.empty());
//
//        auto const first = bkrl::value_of(block.first);
//        auto const last  = bkrl::value_of(block.last);
//        auto const size  = last - first + 1;
//
//        glyphs_.reserve(size);
//
//        int max_h = 0;
//        int max_w = 0;
//
//        for (auto cp = first; cp <= last; ++cp) {
//            glyphs_.emplace_back(
//                load_bitmap_glyph(face, unicode::codepoint {cp})
//            );
//
//            auto const glyph = glyphs_.back().get();
//            if (!glyph) {
//                continue;
//            }
//
//            auto const h = glyph->bitmap.rows;
//            auto const w = glyph->bitmap.width;
//
//            max_h = std::max(h, max_h);
//            max_w = std::max(w, max_w);
//        }
//
//        auto const total_area = max_h * max_w * size;
//
//        return total_area;
//    }
//
//    font_code_block_cache(
//        FT_Face const face
//      , unicode::block_value const block
//    )
//      : block_ {block}
//    {
//        total_area_ = load_glyphs(face, block);
//    }
//
//    struct render_result {
//        int x;
//        int y;
//        int h;
//    };
//
//    render_result render(
//        uint8_t* const buffer
//      , int      const buffer_w
//      , int      const buffer_h
//      , int      const xoff  = 0
//      , int      const yoff  = 0
//      , int      const row_h = 0
//    ) {
//        positions_.reserve(glyphs_.size());
//
//        auto cur_row_h = row_h;
//        auto x         = xoff;
//        auto y         = yoff;
//
//        for (auto const& glyph : glyphs_) {
//            if (!glyph) {
//                positions_.emplace_back(FT_Vector {});
//                continue;
//            }
//
//            auto const w = glyph->bitmap.width;
//            auto const h = glyph->bitmap.rows;
//
//            if (x + w > buffer_w) {
//                x = 0;
//                y += cur_row_h;
//                cur_row_h = h;
//            }
//
//            auto const dst = buffer + (xoff * 4) + (yoff * buffer_w * 4);
//            render_glyph(glyph.get(), dst, buffer_w * 4);
//
//            positions_.emplace_back(FT_Vector {x, y});
//
//            cur_row_h = std::max(h, cur_row_h);
//            x += w;
//        }
//
//        return render_result {x, y, cur_row_h};
//    }
//
//    int required_area() const {
//        return total_area_;
//    }
//
//    unicode::block_value block() const {
//        return block_;
//    }
//
//    unicode::block_value block_;
//    std::vector<ft_unique<FT_BitmapGlyph>> glyphs_;
//    std::vector<FT_Vector> positions_;
//    int total_area_;
//};
//
//bool operator<(font_code_block_cache const& lhs, font_code_block_cache const& rhs) {
//    //TODO
//    return lhs.block_.first < rhs.block_.first;
//}
//

} //namespace font

namespace detail {
//==============================================================================
//!
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

////////////////////////////////////////////////////////////////////////////////
inline int nearest_power_of_2(int value) noexcept {
    int result = 1;

    while (result < value) {
        result <<= 1;
    }

    return result;
}



////////////////////////////////////////////////////////////////////////////////
class font_face_impl {
public:
    BK_NOCOPY(font_face_impl);

    font_face_impl(renderer& r, font_libary& lib, string_ref filename, unsigned size);

    glyph_metrics metrics(unicode::codepoint lhs, unicode::codepoint rhs);
    glyph_metrics metrics(glyph_index lhs, glyph_index rhs);

    void render(renderer& r) {
        r.draw_texture(block_texture_);
    }
private:
    font::face face_;

    std::vector<font::block_cache> blocks_;

    texture block_texture_;

    unsigned face_size_;

    bool has_kerning_;
};

////////////////////////////////////////////////////////////////////////////////
font_face_impl::font_face_impl(
    renderer&    r
  , font_libary& lib
  , string_ref   filename
  , unsigned     size
)
  : face_ {lib.handle().as<FT_Library>(), filename, size}
  , face_size_ {size}
{   
    blocks_.reserve(4);

    blocks_.emplace_back(face_, unicode::block_value {unicode::basic_latin {}});
    blocks_.emplace_back(face_, unicode::block_value {unicode::basic_japanese {}});

    //TODO a bit wasteful with texture space (up to 2x what is needed).

    int total_area = 0;
    for (auto const& block : blocks_) {
        total_area += block.required_area();
    }

    auto const ideal_dim = static_cast<int>(
        std::ceil(std::sqrt(total_area))
    );

    auto const power_of_2_w = nearest_power_of_2(ideal_dim);
    auto const power_of_2_h = nearest_power_of_2(total_area / power_of_2_w);

    std::vector<uint8_t> buffer;
    buffer.resize(power_of_2_w * power_of_2_h * 4, 0);

    int xoff  = 0;
    int yoff  = 0;
    int row_h = 0;

    for (auto& block : blocks_) {
        block.render(buffer.data(), xoff, yoff, power_of_2_w, power_of_2_h, 4, power_of_2_w*4, row_h);
    }

    block_texture_ = r.create_texture(buffer.data(), power_of_2_w, power_of_2_h);
}

glyph_metrics
font_face_impl::metrics(
    unicode::codepoint const lhs
  , unicode::codepoint const rhs
) {
    //auto const index_l = FT_Get_Char_Index(face_.get(), value_of(lhs));
    //auto const index_r = FT_Get_Char_Index(face_.get(), value_of(rhs));

    //return metrics(
    //    glyph_index {index_l}
    //  , glyph_index {index_r}
    //);

    return glyph_metrics {};
}

glyph_metrics
font_face_impl::metrics(
    glyph_index const lhs
  , glyph_index const rhs
) {
    return glyph_metrics {};
}

////==============================================================================
//// deleters
////==============================================================================
//template <typename T>
//struct ft_deleter;
//
//template <>
//struct ft_deleter<FT_Library> {
//    void operator()(FT_Library const ptr) const noexcept {
//        auto const result = ::FT_Done_FreeType(ptr);
//        if (result) {
//            BK_TODO_FAIL();
//        }
//    }
//};
//
//template <>
//struct ft_deleter<FT_Face> {
//    void operator()(FT_Face const ptr) const noexcept {
//        auto const result = ::FT_Done_Face(ptr);
//        if (result) {
//            BK_TODO_FAIL();
//        }
//    }
//};
//
//template <>
//struct ft_deleter<FT_Glyph> {
//    void operator()(FT_Glyph const ptr) const noexcept {
//        ::FT_Done_Glyph(ptr);
//    }
//};
//
//template <typename T, typename U = std::remove_pointer_t<T>>
//using ft_unique = std::unique_ptr<U, ft_deleter<T>>;
//
//struct glyph_load_result {
//    ft_unique<FT_Glyph> glyph;
//    FT_Glyph_Metrics    metrics;
//    FT_UInt             index;
//};
//
//glyph_load_result load_glyph(FT_Face face, FT_ULong codepoint);
//
//class code_block_cache {
//public:
//    using glyph_data = glyph_load_result;
//    using rect_t = bkrl::axis_aligned_rect<int>;
//
//    BK_NOCOPY(code_block_cache);
//    BK_DEFAULT_MOVE(code_block_cache);
//
//    code_block_cache(FT_Face const face, FT_ULong const first, FT_ULong const last);
//
//    glyph_data const& operator[](FT_ULong const codepoint) const {
//        BK_PRECONDITION(range_.contains(codepoint));
//        return glyphs_[codepoint - range_.lo];
//    }
//
//    rect_t glyph_rect(FT_ULong const codepoint) const {
//        BK_PRECONDITION(range_.contains(codepoint));
//        return rects_[codepoint - range_.lo];
//    }
//
//    bool contains(FT_ULong const codepoint) const {
//        return range_.contains(codepoint);
//    }
//
//    struct update_result {
//        int final_x;
//        int final_y;
//        int final_h;
//    };
//
//    update_result update_rects(int size_x, int size_y, update_result last = update_result {});
//
//    bkrl::range<FT_ULong> range() const {
//        return range_;
//    }
//private:
//    bkrl::range<FT_ULong> range_;
//
//    std::vector<glyph_data> glyphs_;
//    std::vector<rect_t>     rects_;
//};
//
//
//
//
//class text_renderer {
//public:
//    text_renderer();
//
//    code_block_cache* get_block(FT_ULong const codepoint) {
//        auto const it = std::find_if(
//            std::begin(static_)
//          , std::end(static_)
//          , [&](code_block_cache const& block) {
//                return block.contains(codepoint);
//            }
//        );
//
//        return it != std::cend(static_) ? &(*it) : nullptr;
//    }
//
//    struct glyph_info {
//        FT_Glyph glyph;
//        FT_UInt  index;
//    };
//
//    glyph_info get_glyph(FT_ULong const codepoint) {
//        auto const block = get_block(codepoint);
//        if (block == nullptr) {
//            BK_TODO_FAIL();
//        }
//
//        auto const& result = (*block)[codepoint];
//
//        return glyph_info {result.glyph.get(), result.index};
//    }
//
//    axis_aligned_rect<int> get_glyph_rect(FT_ULong const codepoint) {
//        auto const block = get_block(codepoint);
//        if (block == nullptr) {
//            BK_TODO_FAIL();
//        }
//
//        return block->glyph_rect(codepoint);
//    }
//
//    struct required_size {
//        unsigned w, h;
//    };
//
//    required_size required_tex_size() const {
//        //TODO
//        return required_size {256, 512};
//    }
//
//    template <typename F>
//    void for_each_static_codepoint(F&& function) const {
//        for (auto&& block : static_) {
//            auto const range = block.range();
//            for (auto i = range.lo; i <= range.hi; ++i) {
//                function(i);
//            }
//        }
//    }
//
//    struct text_layout {
//        using point_t = point2d<int>;
//
//        std::vector<point_t> pos;
//        std::vector<FT_UInt> codepoint;
//    };
//
//    using rect_t = axis_aligned_rect<int>;
//    text_layout layout(char const* str, rect_t bounds);
//
//    bool has_kerning() const {
//        return has_kerning_;
//    }
//
//    FT_Vector get_kerning(FT_UInt lhs, FT_UInt rhs) const {
//        FT_Vector kerning {};
//
//        if (has_kerning_ && lhs && rhs) {
//            auto const error = FT_Get_Kerning(ft_face_.get(), lhs, rhs, FT_KERNING_DEFAULT, &kerning);
//            if (error) {
//                BK_TODO_FAIL();
//            }
//
//            kerning.x >>= 6;
//            kerning.y >>= 6;
//        }
//
//        return kerning;
//    }
//
//    FT_Pos ascent() const {
//        return ft_face_->size->metrics.ascender >> 6;
//    }
//
//    FT_Pos line_height() const {
//        return (ft_face_->size->metrics.ascender - ft_face_->size->metrics.descender) >> 6;
//    }
//private:
//    ft_unique<FT_Library> ft_lib_;
//    ft_unique<FT_Face>    ft_face_;
//
//    std::vector<code_block_cache> static_;
//
//    bool has_kerning_ = false;
//};
//
////==============================================================================
////==============================================================================
//
//class transitory_text_layout {
//public:
//    using rect_t = axis_aligned_rect<int>;
//    using pos_t = point2d<int>;
//
//    transitory_text_layout(text_renderer& text, string_ref string, rect_t bounds)
//      : beg_ {string.data()}
//      , end_ {beg_ + string.size()}
//      , cur_ {beg_}
//      , bounds_ {bounds}
//      , prev_index_ {}
//      , line_height_ {text.line_height()}
//      , pen_ {{bounds.left, bounds.top}}
//    {
//        pen_.y += text.ascent();
//    }
//
//    explicit operator bool() const {
//        return cur_ != end_;
//    }
//
//    struct result_t {
//        pos_t    pos;
//        uint32_t codepoint;
//        int      width;
//        int      height;
//    };
//
//    result_t next(text_renderer& text) {
//        BK_ASSERT_DBG(!!*this);
//
//        auto const codepoint = utf8::next(cur_, end_);
//
//        auto const info    = text.get_glyph(codepoint);
//        auto const kerning = text.get_kerning(prev_index_, info.index);
//
//        auto const kx = kerning.x;
//        auto const ky = kerning.y;
//
//        auto const advance_x = info.glyph->advance.x >> 16;
//        auto const advance_y = info.glyph->advance.y >> 16;
//
//        if (pen_.x + advance_x + kx > bounds_.right) {
//            pen_.x =  bounds_.left;
//            pen_.y += line_height_;
//        }
//
//        BK_ASSERT_DBG(info.glyph->format == FT_GLYPH_FORMAT_BITMAP);
//
//        auto const bmp  = reinterpret_cast<FT_BitmapGlyph>(info.glyph);
//        auto const left = bmp->left;
//        auto const top  = bmp->top;
//
//        auto const pos = pos_t {pen_.x + left, pen_.y - top};
//
//        pen_.x += advance_x + kx;
//        pen_.y += advance_y + ky;
//
//        prev_index_ = info.index;
//        
//        return result_t {
//            pos
//          , codepoint
//          , bmp->bitmap.width
//          , bmp->bitmap.rows
//        };
//    }
//private:
//    char const* beg_;
//    char const* end_;
//    char const* cur_;
//
//    rect_t    bounds_;
//
//    FT_UInt   prev_index_;
//    FT_Pos    line_height_;
//    FT_Vector pen_;
//};
//
//class static_text_layout {
//};


}} //namespace bkrl::detail
