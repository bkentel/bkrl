//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Fonts and text rendering.
//##############################################################################
#pragma once

#include <memory>

#include "types.hpp"
#include "assert.hpp"
#include "math.hpp"
#include "util.hpp"

namespace bkrl {

class renderer;
class texture;

class font_libary;
class font_face;
class transitory_text_layout;
class static_text_layout;

namespace detail { class font_face_impl; }
namespace detail { class font_library_impl; }

////////////////////////////////////////////////////////////////////////////////
namespace unicode {

using codepoint = tagged_type<codepoint_t, struct tag_codepoint>;

template <codepoint_t First, codepoint_t Last>
struct block {
    static_assert(First < Last, "");

    enum : uint32_t {
        first = First
      , last  = Last
      , size  = Last - First + 1 //inclusive of end points
    };

};

struct block_value {
    explicit block_value(codepoint const cp)
      : first {cp}, last {cp}
    {
    }
    
    template <codepoint_t First, codepoint_t Last>
    block_value(block<First, Last>)
        : first {First}, last {Last}
    {
        BK_ASSERT_DBG(Last >= First);
    }

    bool contains(codepoint const cp) const noexcept {
        return cp >= first && cp <= last;
    }

    int size() const noexcept {
        return value_of(last) - value_of(first) + 1;
    }

    int offset(codepoint const cp) const {
        BK_ASSERT_DBG(contains(cp));
        return value_of(cp) - value_of(first);
    }

    codepoint first;
    codepoint last;
};

inline bool operator<(codepoint const lhs, block_value const rhs) {
    return lhs < rhs.first;
}

inline bool operator<(block_value const lhs, codepoint const rhs) {
    return lhs.last < rhs;
}

inline bool operator<(block_value const lhs, block_value const rhs) {
    return lhs.last < rhs.first;
}

using basic_latin     = block<0x00,     0x7F>;
using cjk_symbols     = block<0x3000, 0x303F>;
using hiragana        = block<0x3040, 0x309F>;
using katakana        = block<0x30A0, 0x30FF>;
using half_full_forms = block<0xFF01, 0xFFEE>;

using basic_japanese = block<cjk_symbols::first, katakana::last>;

} //namespace unicode
////////////////////////////////////////////////////////////////////////////////

using glyph_index = tagged_type<uint32_t, struct tag_glyph_index>;
using text_rect   = axis_aligned_rect<int>;

//==============================================================================
//! glyph_metrics
//==============================================================================
struct glyph_metrics {
    int width;
    int height;
    int left;
    int top;

    int advance_x;
    int advance_y;

    int tex_x;
    int tex_y;
};

//==============================================================================
//! font_libary
//==============================================================================
class font_libary {
public:
    BK_NOCOPY(font_libary);

    using handle_t = opaque_handle<font_libary>;

    font_libary();
    ~font_libary();

    handle_t handle() const;
private:
    std::unique_ptr<detail::font_library_impl> impl_;
};

//==============================================================================
//! font_face
//==============================================================================
class font_face {
public:
    BK_NOCOPY(font_face);

    font_face(
        renderer&    render
      , font_libary& lib
      , string_ref   filename
      , unsigned     size
    );

    ~font_face();

    glyph_metrics metrics(unicode::codepoint cp);
    glyph_metrics metrics(unicode::codepoint lhs, unicode::codepoint rhs);
       
    texture const& get_texture() const;

    int line_gap() const;
    int size() const;
private:
    std::unique_ptr<detail::font_face_impl> impl_;
};

//==============================================================================
//! transitory_text_layout
//==============================================================================
class transitory_text_layout {
public:
    transitory_text_layout() = default;
    transitory_text_layout(font_face& face, string_ref string, int w = 0, int h = 0);

    void render(renderer& r, font_face& face, int x, int y);
private:
    int w_ = 0;
    int h_ = 0;

    std::vector<codepoint_t> codepoints_;
    std::vector<ipoint2>     positions_;
};

//==============================================================================
//! static_text_layout
//==============================================================================
class static_text_layout {
};

////////////////////////////////////////////////////////////////////////////////



} //namespace bkrl
