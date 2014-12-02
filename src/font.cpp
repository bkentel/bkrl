#include "font.hpp"
#include "detail/freetype_text.i.hpp"
#include "scope_exit.hpp"

#include "renderer.hpp"

using namespace bkrl; //TODO

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
int font_face::pixel_size() const noexcept { return impl_->pixel_size(); }
int font_face::ascender()   const noexcept { return impl_->ascender(); }
int font_face::descender()  const noexcept { return impl_->descender(); }
int font_face::line_gap()   const noexcept { return impl_->line_gap(); }

bkrl::argb8 bkrl::font_face::get_color() const {
    return impl_->get_color();
}

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
  , int16_t    const max_w
  , int16_t    const max_h
) {
    reset(face, string, max_w, max_h);
}

namespace {

namespace token {
    auto constexpr escape       = codepoint_t {'\\'};
    auto constexpr tag_open     = codepoint_t {'<' };
    auto constexpr tag_close    = codepoint_t {'>' };
    auto constexpr id_color     = codepoint_t {'c' };
    auto constexpr delim        = codepoint_t {':' };
    auto constexpr id_close_tag = codepoint_t {'/' };
} //namespace token

//--------------------------------------------------------------------------
template <typename Iterator>
codepoint_t peek_char(Iterator const& it, Iterator const& end) {
    return (it != end) ? *it : 0;
}

//--------------------------------------------------------------------------
template <typename Iterator>
codepoint_t consume_char(Iterator& it, Iterator const& end) {
    return (it != end) ? *it++ : 0;
}

//--------------------------------------------------------------------------
bool is_digit(codepoint_t const cp) noexcept {
    return cp >= '0' && cp <= '9';
};

//--------------------------------------------------------------------------
bool is_alpha_lower(codepoint_t const cp) noexcept {
    return cp >= 'a' && cp <= 'z';
};

//--------------------------------------------------------------------------
bool is_alpha_upper(codepoint_t const cp) noexcept {
    return cp >= 'A' && cp <= 'Z';
};

//--------------------------------------------------------------------------
template <typename Iterator>
codepoint_t expect_char(codepoint_t const expected, Iterator& it, Iterator const& end) {
    if (it == end) {
        return 0;
    }

    auto const value = *it;
    if (value != expected) {
        return 0;
    }

    ++it;

    return value;
}

//--------------------------------------------------------------------------
template <typename Iterator>
bkrl::optional<bkrl::codepoint_t>
parse_escape(Iterator& it, Iterator const& end) {
    auto result = bkrl::optional<bkrl::codepoint_t> {};

    if (!expect_char(token::escape, it, end)) {
        return result;
    }

    switch (consume_char(it, end)) {
    default               : break;
    case token::tag_open  : result = token::tag_open;  break;
    case token::tag_close : result = token::tag_close; break;
    }

    return result;
}

//--------------------------------------------------------------------------
struct format_tag {
    enum class tag_type : uint16_t {
        tag_error, tag_color
    };

    enum class tag_state : uint16_t {
        type_begin, type_end
    };

    uint32_t  data;
    tag_type  type;
    tag_state state;
};

template <typename Iterator>
format_tag parse_tag(Iterator& it, Iterator const& end) {
    auto result = format_tag {};

    if (!expect_char(token::tag_open, it, end)) {
        return result;
    }

    auto const parse_color = [&] {
        if (!expect_char(token::delim, it, end)) {
            return;
        }

        auto const id = consume_char(it, end);
        if (!id || !is_digit(id) && !is_alpha_lower(id)) {
            return;
        }

        if (!expect_char(token::tag_close, it, end)) {
            return;
        }

        result.data  = id;
        result.type  = format_tag::tag_type::tag_color;
        result.state = format_tag::tag_state::type_begin;
    };

    auto const parse_close_tag = [&] {
        if (!expect_char(token::id_color, it, end)) {
            return;
        }

        if (!expect_char(token::tag_close, it, end)) {
            return;
        }

        result.data  = 0;
        result.type  = format_tag::tag_type::tag_color;
        result.state = format_tag::tag_state::type_end;
    };


    switch (consume_char(it, end)) {
    default                  : break;
    case token::id_color     : parse_color();     break;
    case token::id_close_tag : parse_close_tag(); break;
    }

    return result;
}

}

//------------------------------------------------------------------------------
void transitory_text_layout::reset(
    font_face&       face
  , string_ref const string
  , int16_t    const max_w
  , int16_t    const max_h
) {
    constexpr auto tab_size = 20;
    
    clear();

    //assume 2 bytes on average per codepoint.
    //overly pessimistic for mostly latin text
    data_.reserve(string.size() / 2);

    auto       it   = utf8::iterator<char const*> {cbegin(string), cbegin(string), cend(string)};
    auto const end  = utf8::iterator<char const*> {cend(string),   cbegin(string), cend(string)};

    record_t   cur_record {};
    format_tag cur_format {};

    format_t const default_format {
        make_color(255, 255, 255, 255)
      , 0
      , 0
    };

    int pos = 0;

    while (it != end) {
        auto cp = *it;

        switch (cp) {
        case token::escape :
            if (auto const result = parse_escape(it, end)) {
                cp = *result;
            } else {
                BK_TODO_FAIL();
                continue;
            }

            break;
        case token::tag_open :
            cur_format = parse_tag(it, end);
            if (cur_format.type == format_tag::tag_type::tag_color) {
                if (cur_format.state == format_tag::tag_state::type_begin) {
                    format_.emplace_back(format_t {make_color(255, 0, 0, 255), pos, pos});
                } else if (cur_format.state == format_tag::tag_state::type_end) {
                    format_.back().end = pos;
                }
            } else {
                BK_TODO_FAIL();
            }

            continue;
        default :
            ++it;
            ++pos;

            break;
        }

        cur_record.codepoint = cp;
        data_.push_back(cur_record);
    }

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
    auto const do_escape = [&](codepoint_t const cp) {
        switch (cp) {
        case '\t' : next_tab();  return true;
        case '\n' : next_line(); return true;
        }

        return false;
    };

    //--------------------------------------------------------------------------    
    auto left = unicode::codepoint {};
    for (auto& rec : data_) {
        if (do_escape(rec.codepoint)) {
            continue;
        }

        auto const cp = unicode::codepoint {rec.codepoint};

        auto const metrics = face.metrics(left, cp);
        left = cp;

        if ((max_w != unlimited) && x > max_w) {
            next_line();
        }

        if ((max_h != unlimited) && y > max_h) {
            break;
        }

        auto& p = rec.position;

        p.x = static_cast<int16_t>(x + metrics.left);
        p.y = static_cast<int16_t>(y - metrics.top);

        actual_w_ = std::max(actual_w_, static_cast<int16_t>(p.x + metrics.width));
        actual_h_ = std::max(actual_h_, static_cast<int16_t>(p.y + metrics.height));

        x += metrics.advance_x;
        y -= metrics.advance_y;
    }
}

//------------------------------------------------------------------------------
void transitory_text_layout::clear() {
    data_.clear();
    format_.clear();

    actual_w_ = 0;
    actual_h_ = 0;
}

//------------------------------------------------------------------------------
bool transitory_text_layout::empty() const {
    return data_.empty();
}

//------------------------------------------------------------------------------
void
transitory_text_layout::render(
    renderer&  r
  , font_face& face
  , int const  x
  , int const  y
) const {
    auto& tex = face.get_texture();

    argb8 old_color = face.get_color();

    on_scope_exit(
        face.set_color(old_color);
    );

    auto it  = cbegin(format_);
    auto end = cend(format_);

    int pos = 0;

    for (auto const& rec : data_) {
        if (it != end) {
            if (pos == it->beg) {
                old_color = face.get_color();
                face.set_color(it->color);
            } else if (pos == it->end) {
                face.set_color(old_color);
                ++it;
            }
        }

        auto const& p    = rec.position;        
        auto const& info = face.metrics(unicode::codepoint {rec.codepoint});
        
        auto const w = info.width;
        auto const h = info.height;

        auto const dst_rect = make_rect_size(x + p.x, y + p.y, w, h);
        auto const src_rect = make_rect_size(info.tex_x, info.tex_y, w, h);

        r.draw_texture(tex, src_rect, dst_rect);

        ++pos;
    }
}
