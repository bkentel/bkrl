#include "font.hpp"
#include "detail/freetype_text.i.hpp"

#include "renderer.hpp"
#include <utf8.h>

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
    renderer&    r
  , font_libary& lib
  , string_ref   const filename
  , unsigned     const size
)
  : impl_ {std::make_unique<detail::font_face_impl>(r, lib, filename, size)}
{
}

//------------------------------------------------------------------------------
font_face::~font_face() = default;

//------------------------------------------------------------------------------
int
font_face::line_gap() const {
    return impl_->line_gap();
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
font_face::texture_info
font_face::get_texture(
    unicode::codepoint const cp
) {
    return impl_->get_texture(cp);
}

////////////////////////////////////////////////////////////////////////////////
// transitory_text_layout
////////////////////////////////////////////////////////////////////////////////
transitory_text_layout::transitory_text_layout(
    font_face&       face
  , string_ref const string
  , int        const w
  , int        const h
)
  : w_ {w}
  , h_ {h}
{
    BK_ASSERT_DBG(
        (w == 0 && h == 0)
     || (w != 0 && h != 0)
    );

    auto beg = string.data() + 0;
    auto end = string.data() + string.size();

    utf8::utf8to32(
        std::cbegin(string)
      , std::cend(string)
      , std::back_inserter(codepoints_)
    );

    positions_.reserve(codepoints_.size());

    auto const line_gap = face.line_gap();
    int x = 0;
    int y = 20;

    auto left = unicode::codepoint {};
    for (auto const codepoint : codepoints_) {
        auto const cp = unicode::codepoint {codepoint};

        auto const metrics = face.metrics(left, cp);
        left = cp;

        if (w && x > w) {
            x = 0;
            y += line_gap;
        }

        point2d<int> const p {
            x + metrics.left
          , y - metrics.top
        };

        x += metrics.advance_x;
        y -= metrics.advance_y;

        positions_.push_back(p);
    }
}

//------------------------------------------------------------------------------
void
transitory_text_layout::render(
    renderer&  r
  , font_face& face
  , int const  x
  , int const  y
) {
    for (auto i = 0u; i < codepoints_.size(); ++i) {
        auto const cp   = unicode::codepoint {codepoints_[i]};
        auto const p    = positions_[i];
        auto const info = face.get_texture(cp);

        if (!info.t) {
            continue;
        }
        
        auto const left   = static_cast<float>(x + p.x);
        auto const top    = static_cast<float>(y + p.y);
        auto const right  = static_cast<float>(left + info.r.width());
        auto const bottom = static_cast<float>(top  + info.r.height());

        r.draw_texture(*info.t, info.r, rect {left, top, right, bottom});
    }
}
