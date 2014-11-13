#include "font.hpp"
#include "detail/freetype_text.i.hpp"

#include "renderer.hpp"

using namespace bkrl;

////////////////////////////////////////////////////////////////////////////////
// font_libary
////////////////////////////////////////////////////////////////////////////////
font_libary::font_libary()
  : impl_ {std::make_unique<detail::font_library_impl>()}
{
}

//------------------------------------------------------------------------------
font_libary::~font_libary() = default;

//------------------------------------------------------------------------------
font_libary::handle_t
font_libary::handle() const {
    return impl_->handle();
}

////////////////////////////////////////////////////////////////////////////////
// font_face
////////////////////////////////////////////////////////////////////////////////
font_face::font_face(
    renderer&             r
  , font_libary&          lib
  , path_string_ref const filename
  , unsigned        const size
)
  : impl_ {std::make_unique<detail::font_face_impl>(r, lib, filename, size)}
{
}

//------------------------------------------------------------------------------
font_face::~font_face() = default;

//------------------------------------------------------------------------------
int font_face::pixel_size() const { return impl_->pixel_size(); }
int font_face::ascender()   const { return impl_->ascender(); }
int font_face::descender()  const { return impl_->descender(); }
int font_face::line_gap()   const { return impl_->line_gap(); }

void bkrl::font_face::set_color(rgb8 const color) {
    impl_->set_color(color);
}

void bkrl::font_face::set_color(argb8 const color) {
    impl_->set_color(color);
}

//------------------------------------------------------------------------------
glyph_metrics
font_face::metrics(
    unicode::codepoint const lhs
  , unicode::codepoint const rhs
) {
    return impl_->metrics(lhs, rhs);
}

//------------------------------------------------------------------------------
glyph_metrics
font_face::metrics(
    unicode::codepoint const cp
) {
    return impl_->metrics(cp);
}

//------------------------------------------------------------------------------
texture const&
font_face::get_texture() const {
    return impl_->get_texture();
}

////////////////////////////////////////////////////////////////////////////////
// transitory_text_layout
////////////////////////////////////////////////////////////////////////////////
transitory_text_layout::transitory_text_layout(
    font_face&       face
  , string_ref const string
  , int        const max_w
  , int        const max_h
) {
    reset(face, string, max_w, max_h);
}

//------------------------------------------------------------------------------
void transitory_text_layout::reset(
    font_face&       face
  , string_ref const string
  , int        const max_w
  , int        const max_h
) {
    constexpr auto tab_size = 20;
    
    clear();    

    //assume 2 bytes on average per codepoint.
    //overly pessimistic for mostly latin text
    codepoints_.reserve(string.size() / 2);

    utf8::utf8to32(
        std::cbegin(string)
      , std::cend(string)
      , std::back_inserter(codepoints_)
    );

    positions_.reserve(codepoints_.size());

    auto const line_gap = face.line_gap();
    auto       x        = 0;
    auto       y        = face.ascender();

    //--------------------------------------------------------------------------
    auto const next_line = [&] {
        x  = 0;
        y += line_gap;
    };

    //--------------------------------------------------------------------------
    auto const next_tab = [&] {
        auto const rem = x % tab_size;
        auto const tab = tab_size - rem;
        x += tab;
    };

    //--------------------------------------------------------------------------
    auto escape = false;
    
    auto left = unicode::codepoint {};
    for (auto const codepoint : codepoints_) {
        auto const cp = unicode::codepoint {codepoint};

        if (!escape) {
            switch (cp.value) {
            case '\\' : escape = true; break;
            case '\n' : next_line();   break;
            case '\t' : next_tab();    break;
            default : break;
            }
        } else {
            escape = false;
        }

        auto const metrics = face.metrics(left, cp);
        left = cp;

        if ((max_w != unlimited) && x > max_w) {
            next_line();
        }

        if ((max_h != unlimited) && y > max_h) {
            break;
        }

        ipoint2 const p {(x + metrics.left), (y - metrics.top)};

        actual_w_ = std::max(actual_w_, p.x + metrics.width);
        actual_h_ = std::max(actual_h_, p.y + metrics.height);

        x += metrics.advance_x;
        y -= metrics.advance_y;

        positions_.push_back(p);
    }
}

//------------------------------------------------------------------------------
void transitory_text_layout::clear() {
    codepoints_.clear();
    positions_.clear();

    actual_w_ = 0;
    actual_h_ = 0;
}

//------------------------------------------------------------------------------
bool transitory_text_layout::empty() const {
    BK_ASSERT_DBG(
        ( codepoints_.empty() &&  positions_.empty())
     || (!codepoints_.empty() && !positions_.empty())
    );

    return codepoints_.empty();
}

//------------------------------------------------------------------------------
void
transitory_text_layout::render(
    renderer&  r
  , font_face& face
  , int const  x
  , int const  y
) const {
    for (auto i = 0u; i < codepoints_.size(); ++i) {
        auto const cp = unicode::codepoint {codepoints_[i]};
        auto const p  = positions_[i];
        
        auto const& tex  = face.get_texture();
        auto const& info = face.metrics(cp);
        
        auto const w = info.width;
        auto const h = info.height;

        auto const dst_rect = make_rect_size(x + p.x, y + p.y, w, h);
        auto const src_rect = make_rect_size(info.tex_x, info.tex_y, w, h);

        r.draw_texture(tex, src_rect, dst_rect);
    }
}
