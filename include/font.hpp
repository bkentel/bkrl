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

class font_libary;
class font_cache;
class transitory_text_layout;
class static_text_layout;

namespace detail { class font_cache_impl; }
namespace detail { class font_library_impl; }

namespace unicode {

using codepoint = tagged_type<uint32_t, struct tag_codepoint>;

template <uint32_t First, uint32_t Last>
struct block {
    static_assert(First < Last, "");

    enum : uint32_t {
        first = First
      , last  = Last
      , size  = Last - First + 1 //inclusive of end points
    };

};

struct block_value {
    template <uint32_t First, uint32_t Last>
    block_value(block<First, Last>)
        : first {First}, last {Last}
    {
    }

    codepoint first;
    codepoint last;
};

using basic_latin = block<0x00,     0x7F>;
using cjk_symbols = block<0x3000, 0x303F>;
using hiragana    = block<0x3040, 0x309F>;
using katakana    = block<0x30A0, 0x30FF>;

using basic_japanese = block<cjk_symbols::first, katakana::last>;

} //unicode

using glyph_index = tagged_type<uint32_t, struct tag_glyph_index>;
using text_rect   = axis_aligned_rect<int>;

//==============================================================================
//==============================================================================
struct glyph_info {
    unicode::codepoint codepoint;
    glyph_index        index;
};

//==============================================================================
//==============================================================================
class font_libary {
public:
    using handle_t = opaque_handle<font_libary>;

    BK_NOCOPY(font_libary);

    font_libary();
    ~font_libary();

    handle_t handle() const;
private:
    std::unique_ptr<detail::font_library_impl> impl_;
};

//==============================================================================
//==============================================================================
class font_cache {
public:
    BK_NOCOPY(font_cache);

    explicit font_cache(font_libary& lib, string_ref filename, unsigned size);
    ~font_cache();

    void cache(unicode::codepoint cp);
    void cache(unicode::codepoint first, unicode::codepoint last);
    void chache(std::initializer_list<unicode::block_value> blocks);

    void evict(unicode::codepoint cp);
    void evict(unicode::codepoint first, unicode::codepoint last);

    glyph_info operator[](unicode::codepoint cp);
    glyph_info operator[](glyph_index i);
private:
    std::unique_ptr<detail::font_cache_impl> impl_;
};

//==============================================================================
//==============================================================================
class transitory_text_layout {
public:
    transitory_text_layout(font_cache& font, string_ref string);
    transitory_text_layout(font_cache& font, string_ref string, text_rect bounds);


private:
    text_rect bounds_;

    std::vector<unicode::codepoint> codepoints_;
    std::vector<point2d<int>>       positions_;
};

//==============================================================================
//==============================================================================
class static_text_layout {
public:
    explicit static_text_layout(string_ref string);
    static_text_layout(string_ref string, text_rect bounds);
private:
};

////////////////////////////////////////////////////////////////////////////////



} //namespace bkrl
