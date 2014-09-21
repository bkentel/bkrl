#include "font.hpp"
#include "detail/freetype_text.i.hpp"

using namespace bkrl;

////////////////////////////////////////////////////////////////////////////////
// font_libary
////////////////////////////////////////////////////////////////////////////////
font_libary::font_libary()
  : impl_ {std::make_unique<detail::font_library_impl>()}
{
}

font_libary::~font_libary() = default;

font_libary::handle_t
font_libary::handle() const {
    return impl_->handle();
}

////////////////////////////////////////////////////////////////////////////////
// font_face
////////////////////////////////////////////////////////////////////////////////

font_face::font_face(
    renderer&    r
  , font_libary& lib
  , string_ref   filename
  , unsigned     size
)
  : impl_ {std::make_unique<detail::font_face_impl>(r, lib, filename, size)}
{
}

font_face::~font_face() = default;

glyph_metrics
font_face::metrics(
    unicode::codepoint lhs
  , unicode::codepoint rhs
) {
    return impl_->metrics(lhs, rhs);
}

glyph_metrics
font_face::metrics(glyph_index lhs, glyph_index rhs) {
    return impl_->metrics(lhs, rhs);
}

void font_face::render(renderer& r) {
    impl_->render(r);
}

////////////////////////////////////////////////////////////////////////////////


//static ft_unique<FT_Library> create_freetype() {
//    FT_Library library;
//	auto const result = FT_Init_FreeType(&library);
//    if (result) {
//        BK_TODO_FAIL();
//    }
//
//    return ft_unique<FT_Library> {library};
//}
//
//static ft_unique<FT_Face> create_fontface(FT_Library library) {
//    FT_Face face;
//	auto result = FT_New_Face(library, R"(C:\windows\fonts\meiryo.ttc)", 0, &face);
//    if (result) {
//        BK_TODO_FAIL();
//    }
//
//    result = FT_Set_Pixel_Sizes(face, 0, 24);
//    if (result) {
//        BK_TODO_FAIL();
//    }
//
//    return ft_unique<FT_Face> {face};
//}
//
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//glyph_load_result
//bkrl::load_glyph(
//    FT_Face const face
//  , FT_ULong const codepoint
//) {
//    auto result = glyph_load_result {};
//
//    result.index = FT_Get_Char_Index(face, codepoint);
//    if (result.index == 0) {
//        //TODO log the error?
//        return glyph_load_result {};
//    }
//
//    auto error = FT_Error {0};
//
//    error = FT_Load_Glyph(face, result.index, FT_LOAD_DEFAULT);
//    if (error) {
//        //TODO log the error?
//        return glyph_load_result {};
//    }
//    
//    result.metrics = face->glyph->metrics;
//
//    auto glyph = [&] {
//        FT_Glyph result {};
//        error = FT_Get_Glyph(face->glyph, &result);
//        if (error) {
//            BK_TODO_FAIL();
//        }
//
//        return ft_unique<FT_Glyph> {result};
//    }();
//
//    auto ptr = glyph.get();
//    error = FT_Glyph_To_Bitmap(&ptr, FT_RENDER_MODE_NORMAL, 0, 0);
//    if (error) {
//        BK_TODO_FAIL();
//    }
//
//    result.glyph.reset(ptr);
//
//    return result;
//}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//
//bkrl::text_renderer::text_renderer()
//  : ft_lib_       {create_freetype()}
//  , ft_face_      {create_fontface(ft_lib_.get())}
//  , has_kerning_  {FT_HAS_KERNING(ft_face_.get()) != 0}
//{
//    auto const face = ft_face_.get();
//
//    static_.emplace_back(face, 0x0000, 0x007F); //latin
//    static_.emplace_back(face, 0x3000, 0x30FF); //kana
//
//    auto const size   = required_tex_size();
//    auto       result = code_block_cache::update_result {};
//
//    for (auto& block : static_) {
//        result = block.update_rects(size.w, size.h, result);
//    }
//}
//
//
//text_renderer::text_layout
//text_renderer::layout(char const* str, rect_t bounds) {
//    BK_ASSERT_DBG(str != nullptr);
//
//    auto const face        = ft_face_.get();
//    auto const line_height = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;
//
//    auto const get_kerning = [&](FT_UInt const index_l, FT_UInt const index_r) {
//        FT_Vector kerning {};
//
//        if (has_kerning_ && index_l && index_r) {
//            auto const error = FT_Get_Kerning(face, index_l, index_r, FT_KERNING_DEFAULT, &kerning);
//            if (error) {
//                BK_TODO_FAIL();
//            }
//        }
//
//        return kerning;
//    };
//
//    text_layout result;
//    
//    FT_Vector pen {bounds.left, bounds.top + (face->size->metrics.ascender >> 6)};
//
//    //TODO fix for utf8
//
//    auto index      = FT_UInt {};
//    auto prev_index = FT_UInt {};
//    auto codepoint  = FT_UInt {};
//
//    while (auto c = *str++) {
//        codepoint = c;
//
//        auto const info = get_glyph(codepoint);
//
//        prev_index = index;
//        index = info.index;
//
//        auto const glyph   = info.glyph;
//        auto const kerning = get_kerning(prev_index, index);
//
//        auto const advance_x = glyph->advance.x >> 16;
//        auto const advance_y = glyph->advance.y >> 16;
//
//        auto const kx = kerning.x;
//        auto const ky = kerning.y;
//
//        if (pen.x + advance_x + kx > bounds.right) {
//            pen.x =  bounds.left;
//            pen.y += line_height;
//        }
//
//        auto const left = reinterpret_cast<FT_BitmapGlyph>(glyph)->left;
//        auto const top  = reinterpret_cast<FT_BitmapGlyph>(glyph)->top;
//
//        result.pos.emplace_back(point2d<int> {pen.x + left, pen.y - top});
//        result.codepoint.emplace_back(codepoint);
//
//        pen.x += advance_x + kx;
//        pen.y += advance_y + ky;
//    }
//
//    return result;
//}
//
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//
//code_block_cache::code_block_cache(
//    FT_Face  const face
//  , FT_ULong const first
//  , FT_ULong const last
//)
//  : range_ {{first, last}}
//{
//    BK_ASSERT_SAFE(!!range_);
//
//    glyphs_.reserve(last - first);
//
//    for (auto i = first; i <= last; ++i) {
//        auto glyph = load_glyph(face, i);
//
//        glyphs_.emplace_back(std::move(glyph));
//    }
//}
//
//code_block_cache::update_result
//code_block_cache::update_rects(
//    int const size_x
//  , int const size_y
//  , update_result const last
//) {
//    BK_ASSERT_DBG(size_x > 0);
//    BK_ASSERT_DBG(size_y > 0);
//    
//    BK_ASSERT_DBG(last.final_h >= 0);
//
//    BK_ASSERT_DBG(last.final_x < size_x);
//    BK_ASSERT_DBG(last.final_y < size_y);
//    BK_ASSERT_DBG(last.final_h < size_y);
//
//    rects_.clear();
//    rects_.reserve(glyphs_.size());
//
//    auto x     = FT_Pos {last.final_x};
//    auto y     = FT_Pos {last.final_y};
//    auto row_h = FT_Pos {last.final_h};
//    
//    auto rect  = rect_t {0, 0, 0, 0};
//
//    auto const set_rect = [&](FT_Pos x, FT_Pos y, FT_Pos w, FT_Pos h) {
//        rect.left   = x;
//        rect.right  = x + w;
//        rect.top    = y;
//        rect.bottom = y + h;
//    };
//
//    for (auto const& glyph : glyphs_) {
//        auto const w = glyph.metrics.width  >> 6;
//        auto const h = glyph.metrics.height >> 6;
//
//        if (x + w > size_x) {
//            x      = 0;
//            y     += row_h;
//            row_h  = 0;
//        }
//
//        if (y + h > size_y) {
//            BK_TODO_FAIL();
//        }
//
//        if (h > row_h) {
//            row_h = h;
//        }
//
//        set_rect(x, y, w, h);
//
//        x += w;
//
//        rects_.emplace_back(rect);
//    }
//
//    return update_result {x, y, row_h};
//}
//
