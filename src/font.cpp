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
int font_face::pixel_size() const { return impl_->pixel_size(); }
int font_face::ascender()   const { return impl_->ascender(); }
int font_face::descender()  const { return impl_->descender(); }
int font_face::line_gap()   const { return impl_->line_gap(); }

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
  , int        const w
  , int        const h
)
  : w_ {w}
  , h_ {h}
{
    BK_ASSERT_DBG(
        (w == 0 && h == 0) || (w != 0 && h != 0)
    );

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
    int x = 0;
    int y = face.ascender();

    auto left = unicode::codepoint {};
    for (auto const codepoint : codepoints_) {
        auto const cp = unicode::codepoint {codepoint};

        auto const metrics = face.metrics(left, cp);
        left = cp;

        if (w && x > w) {
            x = 0;
            y += line_gap;
        }

        ipoint2 const p {
            x + metrics.left
          , y - metrics.top
        };

        x += metrics.advance_x;
        y -= metrics.advance_y;

        positions_.push_back(p);
    }
}

//------------------------------------------------------------------------------
namespace {

template <typename T0, typename T1, typename T2, typename T3>
inline bkrl::rect make_rect(
    T0 const left
  , T1 const top
  , T2 const right
  , T3 const bottom
) noexcept {
    return bkrl::rect {
        static_cast<float>(left)
      , static_cast<float>(top)
      , static_cast<float>(right)
      , static_cast<float>(bottom)
    };
}

} //namespace

void
transitory_text_layout::render(
    renderer&  r
  , font_face& face
  , int const  x
  , int const  y
) {
    auto const scale = r.get_scale();
    auto const trans = r.get_translation();

    r.set_scale(1.0f);
    r.set_translation(0.0f, 0.0f);

    for (auto i = 0u; i < codepoints_.size(); ++i) {
        auto const cp = unicode::codepoint {codepoints_[i]};
        auto const p  = positions_[i];
        
        auto const& tex  = face.get_texture();
        auto const& info = face.metrics(cp);
        
        auto const w = info.width;
        auto const h = info.height;

        auto const left   = x + p.x;
        auto const top    = y + p.y;
        auto const right  = left + w;
        auto const bottom = top  + h;

        rect const dst_rect = make_rect(left, top, right, bottom);
        rect const src_rect = make_rect(info.tex_x, info.tex_y, info.tex_x + w, info.tex_y + h);

        r.draw_texture(tex, src_rect, dst_rect);
    }

    r.set_scale(scale);
    r.set_translation(trans);
}
