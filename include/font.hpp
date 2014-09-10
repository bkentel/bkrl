//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Fonts and text rendering.
//##############################################################################
#pragma once

#include <vector>

#include <utf8.h>

#include "types.hpp"
#include "assert.hpp"
#include "math.hpp"

#include "algorithm.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace bkrl {

//==============================================================================
// deleters
//==============================================================================
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

template <typename T, typename U = std::remove_pointer_t<T>>
using ft_unique = std::unique_ptr<U, ft_deleter<T>>;

struct glyph_load_result {
    ft_unique<FT_Glyph> glyph;
    FT_Glyph_Metrics    metrics;
    FT_UInt             index;
};

glyph_load_result load_glyph(FT_Face face, FT_ULong codepoint);

class code_block_cache {
public:
    using glyph_data = glyph_load_result;
    using rect_t = bkrl::axis_aligned_rect<int>;

    BK_NOCOPY(code_block_cache);
    BK_DEFAULT_MOVE(code_block_cache);

    code_block_cache(FT_Face const face, FT_ULong const first, FT_ULong const last);

    glyph_data const& operator[](FT_ULong const codepoint) const {
        BK_PRECONDITION(range_.contains(codepoint));
        return glyphs_[codepoint - range_.lo];
    }

    rect_t glyph_rect(FT_ULong const codepoint) const {
        BK_PRECONDITION(range_.contains(codepoint));
        return rects_[codepoint - range_.lo];
    }

    bool contains(FT_ULong const codepoint) const {
        return range_.contains(codepoint);
    }

    struct update_result {
        int final_x;
        int final_y;
        int final_h;
    };

    update_result update_rects(int size_x, int size_y, update_result last = update_result {});

    bkrl::range<FT_ULong> range() const {
        return range_;
    }
private:
    bkrl::range<FT_ULong> range_;

    std::vector<glyph_data> glyphs_;
    std::vector<rect_t>     rects_;
};




class text_renderer {
public:
    text_renderer();

    code_block_cache* get_block(FT_ULong const codepoint) {
        auto const it = std::find_if(
            std::begin(static_)
          , std::end(static_)
          , [&](code_block_cache const& block) {
                return block.contains(codepoint);
            }
        );

        return it != std::cend(static_) ? &(*it) : nullptr;
    }

    struct glyph_info {
        FT_Glyph glyph;
        FT_UInt  index;
    };

    glyph_info get_glyph(FT_ULong const codepoint) {
        auto const block = get_block(codepoint);
        if (block == nullptr) {
            BK_TODO_FAIL();
        }

        auto const& result = (*block)[codepoint];

        return glyph_info {result.glyph.get(), result.index};
    }

    axis_aligned_rect<int> get_glyph_rect(FT_ULong const codepoint) {
        auto const block = get_block(codepoint);
        if (block == nullptr) {
            BK_TODO_FAIL();
        }

        return block->glyph_rect(codepoint);
    }

    struct required_size {
        unsigned w, h;
    };

    required_size required_tex_size() const {
        //TODO
        return required_size {256, 512};
    }

    template <typename F>
    void for_each_static_codepoint(F&& function) const {
        for (auto&& block : static_) {
            auto const range = block.range();
            for (auto i = range.lo; i <= range.hi; ++i) {
                function(i);
            }
        }
    }

    struct text_layout {
        using point_t = point2d<int>;

        std::vector<point_t> pos;
        std::vector<FT_UInt> codepoint;
    };

    using rect_t = axis_aligned_rect<int>;
    text_layout layout(char const* str, rect_t bounds);

    bool has_kerning() const {
        return has_kerning_;
    }

    FT_Vector get_kerning(FT_UInt lhs, FT_UInt rhs) const {
        FT_Vector kerning {};

        if (has_kerning_ && lhs && rhs) {
            auto const error = FT_Get_Kerning(ft_face_.get(), lhs, rhs, FT_KERNING_DEFAULT, &kerning);
            if (error) {
                BK_TODO_FAIL();
            }

            kerning.x >>= 6;
            kerning.y >>= 6;
        }

        return kerning;
    }

    FT_Pos ascent() const {
        return ft_face_->size->metrics.ascender >> 6;
    }

    FT_Pos line_height() const {
        return (ft_face_->size->metrics.ascender - ft_face_->size->metrics.descender) >> 6;
    }
private:
    ft_unique<FT_Library> ft_lib_;
    ft_unique<FT_Face>    ft_face_;

    std::vector<code_block_cache> static_;

    bool has_kerning_ = false;
};

//==============================================================================
//==============================================================================

class transitory_text_layout {
public:
    using rect_t = axis_aligned_rect<int>;
    using pos_t = point2d<int>;

    transitory_text_layout(text_renderer& text, string_ref string, rect_t bounds)
      : beg_ {string.data()}
      , end_ {beg_ + string.size()}
      , cur_ {beg_}
      , bounds_ {bounds}
      , prev_index_ {}
      , line_height_ {text.line_height()}
      , pen_ {{bounds.left, bounds.top}}
    {
        pen_.y += text.ascent();
    }

    explicit operator bool() const {
        return cur_ != end_;
    }

    struct result_t {
        pos_t    pos;
        uint32_t codepoint;
        int      width;
        int      height;
    };

    result_t next(text_renderer& text) {
        BK_ASSERT_DBG(!!*this);

        auto const codepoint = utf8::next(cur_, end_);

        auto const info    = text.get_glyph(codepoint);
        auto const kerning = text.get_kerning(prev_index_, info.index);

        auto const kx = kerning.x;
        auto const ky = kerning.y;

        auto const advance_x = info.glyph->advance.x >> 16;
        auto const advance_y = info.glyph->advance.y >> 16;

        if (pen_.x + advance_x + kx > bounds_.right) {
            pen_.x =  bounds_.left;
            pen_.y += line_height_;
        }

        BK_ASSERT_DBG(info.glyph->format == FT_GLYPH_FORMAT_BITMAP);

        auto const bmp  = reinterpret_cast<FT_BitmapGlyph>(info.glyph);
        auto const left = bmp->left;
        auto const top  = bmp->top;

        auto const pos = pos_t {pen_.x + left, pen_.y - top};

        pen_.x += advance_x + kx;
        pen_.y += advance_y + ky;

        prev_index_ = info.index;
        
        return result_t {
            pos
          , codepoint
          , bmp->bitmap.width
          , bmp->bitmap.rows
        };
    }
private:
    char const* beg_;
    char const* end_;
    char const* cur_;

    rect_t    bounds_;

    FT_UInt   prev_index_;
    FT_Pos    line_height_;
    FT_Vector pen_;
};

class static_text_layout {
};

} //namespace bkrl
