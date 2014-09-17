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
namespace detail {

////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct ft_deleter;

template <>
struct ft_deleter<FT_Library> {
    void operator()(FT_Library const ptr) const noexcept {
        auto const result = ::FT_Done_FreeType(ptr);
        if (result) {
            BK_TODO_FAIL();
        }
    }
};

template <>
struct ft_deleter<FT_Face> {
    void operator()(FT_Face const ptr) const noexcept {
        auto const result = ::FT_Done_Face(ptr);
        if (result) {
            BK_TODO_FAIL();
        }
    }
};

template <>
struct ft_deleter<FT_Glyph> {
    void operator()(FT_Glyph const ptr) const noexcept {
        ::FT_Done_Glyph(ptr);
    }
};

template <>
struct ft_deleter<FT_BitmapGlyph> {
    void operator()(FT_BitmapGlyph const ptr) const noexcept {
        ::FT_Done_Glyph(reinterpret_cast<FT_Glyph>(ptr));
    }
};

template <typename T, typename U = std::remove_pointer_t<T>>
using ft_unique = std::unique_ptr<U, ft_deleter<T>>;

////////////////////////////////////////////////////////////////////////////////
class font_library_impl {
public:
    static ft_unique<FT_Library> create_library_() {
        FT_Library library {};
        auto const result = FT_Init_FreeType(&library);
        if (result) {
            BK_TODO_FAIL();
        }

        return ft_unique<FT_Library> {library};
    }


    font_library_impl()
      : library_ {create_library_()}
    {
    }

    font_libary::handle_t handle() const {
        return library_;
    }
private:
    ft_unique<FT_Library> library_;
};

////////////////////////////////////////////////////////////////////////////////
 ft_unique<FT_Glyph>
get_glyph(FT_Face const face) {
    FT_Glyph glyph {};

    auto const error = FT_Get_Glyph(face->glyph, &glyph);
    if (error) {
        BK_TODO_FAIL();
    }

    return ft_unique<FT_Glyph> {glyph};
 }

 ////////////////////////////////////////////////////////////////////////////////
ft_unique<FT_BitmapGlyph>
glyph_to_bitmap(
    ft_unique<FT_Glyph>& original
  , bool const           destroy = true
) {
    auto glyph = original.get();

    auto const error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, destroy);
    if (error) {
        BK_TODO_FAIL();
    }

    if (destroy) {
        original.release();
    }

    auto result = ft_unique<FT_BitmapGlyph> {
        reinterpret_cast<FT_BitmapGlyph>(glyph)
    };

    if (glyph->format != FT_GLYPH_FORMAT_BITMAP) {
        BK_TODO_FAIL();
    }

    return std::move(result);
 }

////////////////////////////////////////////////////////////////////////////////
ft_unique<FT_BitmapGlyph>
load_bitmap_glyph(
    FT_Face            const face
  , unicode::codepoint const cp
) {
    auto const index = FT_Get_Char_Index(face, value_of(cp));
    if (index == 0) {
        return ft_unique<FT_BitmapGlyph> {};
    }

    FT_Error result {};

    auto const error = FT_Load_Glyph(face, index, FT_LOAD_DEFAULT);
    if (error) {
        BK_TODO_FAIL();
    }

    auto glyph     = get_glyph(face);
    auto glyph_bmp = glyph_to_bitmap(glyph, true);

    return std::move(glyph_bmp);
}

////////////////////////////////////////////////////////////////////////////////
inline int nearest_power_of_2(int value) noexcept {
    int result = 1;

    while (result < value) {
        result <<= 1;
    }

    return result;
}

void render_glyph(
    FT_BitmapGlyph const glyph
  , uint8_t*       const buffer
  , int            const pitch
) {
    auto const bitmap = glyph->bitmap;

    auto dst = buffer;
    auto src = bitmap.buffer;

    for (int y = 0; y < bitmap.rows; ++y) {
        for (int x = 0; x < bitmap.width; ++x) {
            dst[0] = *src;
            dst[1] = 0xFF;
            dst[2] = 0xFF;
            dst[3] = 0xFF;

            dst += 4;
            src += 1;
        }

        dst += pitch;
        src += bitmap.pitch; 
    }
}

////////////////////////////////////////////////////////////////////////////////
class font_code_block_cache {
public:
    using rect = axis_aligned_rect<int>;

    int load_glyphs(FT_Face const face, unicode::block_value const block) {
        BK_ASSERT_DBG(glyphs_.empty());

        auto const first = bkrl::value_of(block.first);
        auto const last  = bkrl::value_of(block.last);
        auto const size  = last - first + 1;

        glyphs_.reserve(size);

        int max_h = 0;
        int max_w = 0;

        for (auto cp = first; cp <= last; ++cp) {
            glyphs_.emplace_back(
                load_bitmap_glyph(face, unicode::codepoint {cp})
            );

            auto const glyph = glyphs_.back().get();
            if (!glyph) {
                continue;
            }

            auto const h = glyph->bitmap.rows;
            auto const w = glyph->bitmap.width;

            max_h = std::max(h, max_h);
            max_w = std::max(w, max_w);
        }

        auto const total_area = max_h * max_w * size;

        return total_area;
    }

    font_code_block_cache(
        FT_Face const face
      , unicode::block_value const block
    )
      : block_ {block}
    {
        total_area_ = load_glyphs(face, block);
    }

    struct render_result {
        int x;
        int y;
        int h;
    };

    render_result render(
        uint8_t* const buffer
      , int      const buffer_w
      , int      const buffer_h
      , int      const xoff  = 0
      , int      const yoff  = 0
      , int      const row_h = 0
    ) {
        positions_.reserve(glyphs_.size());

        auto cur_row_h = row_h;
        auto x         = xoff;
        auto y         = yoff;

        for (auto const& glyph : glyphs_) {
            if (!glyph) {
                positions_.emplace_back(FT_Vector {});
                continue;
            }

            auto const w = glyph->bitmap.width;
            auto const h = glyph->bitmap.rows;

            if (x + w > buffer_w) {
                x = 0;
                y += cur_row_h;
                cur_row_h = h;
            }

            auto const dst = buffer + (xoff * 4) + (yoff * buffer_w * 4);
            render_glyph(glyph.get(), dst, buffer_w * 4);

            positions_.emplace_back(FT_Vector {x, y});

            cur_row_h = std::max(h, cur_row_h);
            x += w;
        }

        return render_result {x, y, cur_row_h};
    }

    int required_area() const {
        return total_area_;
    }

    unicode::block_value block() const {
        return block_;
    }

    unicode::block_value block_;
    std::vector<ft_unique<FT_BitmapGlyph>> glyphs_;
    std::vector<FT_Vector> positions_;
    int total_area_;
};

bool operator<(font_code_block_cache const& lhs, font_code_block_cache const& rhs) {
    //TODO
    return lhs.block_.first < rhs.block_.first;
}


////////////////////////////////////////////////////////////////////////////////
class font_cache_impl {
public:
    BK_NOCOPY(font_cache_impl);

    explicit font_cache_impl(renderer& r, font_libary& lib, string_ref filename, unsigned size);

    void cache(unicode::codepoint cp);
    void cache(unicode::codepoint first, unicode::codepoint last);
    void cache(std::initializer_list<unicode::block_value> blocks);

    void evict(unicode::codepoint cp);
    void evict(unicode::codepoint first, unicode::codepoint last);

    glyph_info operator[](unicode::codepoint cp);
    glyph_info operator[](glyph_index i);
private:
    static ft_unique<FT_Face> create_face_(FT_Library const lib, unsigned size) {
        //TODO proper font locating

        FT_Face face {};
        auto error = FT_New_Face(lib, R"(C:\windows\fonts\meiryo.ttc)", 0, &face);
        if (error) {
            BK_TODO_FAIL();
        }

        auto result = ft_unique<FT_Face> {face};

        error = FT_Set_Pixel_Sizes(face, 0, size);
        if (error) {
            BK_TODO_FAIL();
        }

        return std::move(result);
    }

    ft_unique<FT_Face> face_;

    std::vector<font_code_block_cache> blocks_;

    texture block_texture_;

    unsigned face_size_;

    bool has_kerning_;
};

////////////////////////////////////////////////////////////////////////////////
font_cache_impl::font_cache_impl(
    renderer&    r
  , font_libary& lib
  , string_ref   filename
  , unsigned     size
)
  : face_ {create_face_(lib.handle().as<FT_Library>(), size)}
  , face_size_ {size}
{   
    blocks_.reserve(4);

    auto const face = face_.get();

    blocks_.emplace_back(face, unicode::block_value {unicode::basic_latin {}});
    blocks_.emplace_back(face, unicode::block_value {unicode::basic_japanese {}});

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
    buffer.resize(power_of_2_w*power_of_2_h*4, 0);

    font_code_block_cache::render_result result {};

    for (auto& block : blocks_) {
        result = block.render(buffer.data(), power_of_2_w, power_of_2_h, result.x, result.y, result.y);
    }

    block_texture_ = r.create_texture(buffer.data(), power_of_2_w, power_of_2_h);
}

void font_cache_impl::cache(unicode::codepoint cp) {
}

void font_cache_impl::cache(unicode::codepoint first, unicode::codepoint last) {
}

void font_cache_impl::cache(std::initializer_list<unicode::block_value> blocks) {
}

void font_cache_impl::evict(unicode::codepoint cp) {
}

void font_cache_impl::evict(unicode::codepoint first, unicode::codepoint last) {
}

glyph_info font_cache_impl::operator[](unicode::codepoint cp) {
    for (auto const& b : blocks_) {
        auto const block = b.block();

        if (cp < block) {
            continue;
        } else if (!block.contains(cp)) {
            break;
        } else {
            //TODO
        }
    }

    return {};
}

glyph_info font_cache_impl::operator[](glyph_index i) {
    return {};
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
