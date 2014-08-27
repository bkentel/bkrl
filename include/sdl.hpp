#pragma once

#include <sdl/SDL.h>
#include "exception.hpp"
#include "assert.hpp"
#include "command_type.hpp"
#include "math.hpp"
#include "algorithm.hpp"

namespace bkrl {

class sprite_sheet {
public:
    sprite_sheet(unsigned width, unsigned height, unsigned sprite_width, unsigned sprite_height)
      : width {width}
      , height {height}
      , sprite_width {sprite_width}
      , sprite_height{sprite_height}
      , sprite_x {width / sprite_width}
      , sprite_y {height / sprite_height}
    {
    }

    unsigned get_x(unsigned index) const {
        return (index % sprite_x) * sprite_width;
    }

    unsigned get_y(unsigned index) const {
        return (index / sprite_x) * sprite_height;
    }

    unsigned width;
    unsigned height;

    unsigned sprite_width;
    unsigned sprite_height;

    unsigned sprite_x;
    unsigned sprite_y;
};

//==============================================================================
// errors
//==============================================================================
namespace error {
struct sdl_error : virtual exception_base {};

using sdl_error_info = boost::error_info<struct sdl_error_info_tag, char const*>;

inline sdl_error make_sdl_error(char const* api) {
    return (sdl_error {}
     << boost::errinfo_api_function {api}
     << sdl_error_info {SDL_GetError()}
    );

}

}// namespace bkrl::error

//==============================================================================
// deleters
//==============================================================================
template <typename T>
struct ft_deleter;

template <>
struct ft_deleter<FT_Library> {
    void operator()(FT_Library ptr) const noexcept {
        auto const result = ::FT_Done_FreeType(ptr);
        if (result) {
            BK_TODO_FAIL();
        }
    }
};

template <>
struct ft_deleter<FT_Face> {
    void operator()(FT_Face ptr) const noexcept {
        auto const result = ::FT_Done_Face(ptr);
        if (result) {
            BK_TODO_FAIL();
        }
    }
};

template <>
struct ft_deleter<FT_Glyph> {
    void operator()(FT_Glyph ptr) const noexcept {
        ::FT_Done_Glyph(ptr);
    }
};

template <typename T, typename U = std::remove_pointer_t<T>>
using ft_unique = std::unique_ptr<U, ft_deleter<T>>;
//////

template <typename T>
struct sdl_deleter;

template <>
struct sdl_deleter<SDL_Window> {
    void operator()(SDL_Window* ptr) const noexcept {
        ::SDL_DestroyWindow(ptr);
    }
};

template <>
struct sdl_deleter<SDL_Renderer> {
    void operator()(SDL_Renderer* ptr) const noexcept {
        ::SDL_DestroyRenderer(ptr);
    }
};

template <>
struct sdl_deleter<SDL_Texture> {
    void operator()(SDL_Texture* ptr) const noexcept {
        ::SDL_DestroyTexture(ptr);
    }
};

template <>
struct sdl_deleter<SDL_Surface> {
    void operator()(SDL_Surface* ptr) const noexcept {
        ::SDL_FreeSurface(ptr);
    }
};

template <>
struct sdl_deleter<SDL_PixelFormat> {
    void operator()(SDL_PixelFormat* ptr) const noexcept {
        ::SDL_FreeFormat(ptr);
    }
};

template <typename T>
using sdl_unique = std::unique_ptr<T, sdl_deleter<T>>;

//==============================================================================
// sdl_state
//==============================================================================
struct sdl_state {
    sdl_state(sdl_state const&) = delete;
    sdl_state& operator=(sdl_state const&) = delete;

    sdl_state() {
        auto const result = ::SDL_Init(SDL_INIT_VIDEO);
        if (result) {
            BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_Init"));
        }
    }

    ~sdl_state() {
        ::SDL_Quit();
    }
};

//==============================================================================
// sdl_renderer
//==============================================================================
struct glyph_data {
    FT_Glyph glyph;
    FT_UInt  index;
};

class text_layout {

};


using glyph_load_result = std::tuple<glyph_data, FT_Pos, FT_Pos>;

inline glyph_load_result load_glyph(FT_Face const face, FT_ULong const codepoint) {
    auto glyph  = glyph_data {};

    if (codepoint & 0x80) {
        BK_TODO_FAIL(); //unicode
    }

    glyph.index = FT_Get_Char_Index(face, codepoint);
    if (glyph.index == 0) {
        //TODO log the error?
        return glyph_load_result {};
    }

    auto error = FT_Error {0};

    error = FT_Load_Glyph(face, glyph.index, FT_LOAD_DEFAULT);
    if (error) {
        //TODO log the error?
        return glyph_load_result {};
    }

    auto const width  = face->glyph->metrics.width  >> 6;
    auto const height = face->glyph->metrics.height >> 6;

    error = FT_Get_Glyph(face->glyph, &glyph.glyph);
    if (error) {
        BK_TODO_FAIL();
    }

    error = FT_Glyph_To_Bitmap(&glyph.glyph, FT_RENDER_MODE_NORMAL, 0, 1);
    if (error) {
        BK_TODO_FAIL();
    }

    return std::make_tuple(glyph, width, height);
}


template <size_t Bytes>
class code_block_cache {
public:
    using index_type = unsigned_int_t<Bytes>;
    using rect_t = bkrl::axis_aligned_rect<uint16_t>;

    code_block_cache(FT_Face const face, uint32_t const first, uint32_t const last)
      : first_ {first}
      , last_ {last}
    {
        BK_ASSERT(first < last);
        BK_ASSERT((last - first) <= std::numeric_limits<index_type>::max());

        glyphs_.reserve(last - first);

        constexpr auto max_width = 256;

        auto x = uint16_t {0};
        auto y = uint16_t {0};

        auto row_h = uint16_t {0};

        for (auto i = first; i < last; ++i) {
            auto result = load_glyph(face, i);

            auto const g = std::get<0>(result);
            auto const w = static_cast<uint16_t>(std::get<1>(result));
            auto const h = static_cast<uint16_t>(std::get<2>(result));

            auto l = x;
            auto r = static_cast<uint16_t>(x + w);
            auto t = y;
            auto b = static_cast<uint16_t>(y + h);

            if (r > max_width) {
                x = 0;

                l = x;
                r = x + w;

                y += row_h;
                row_h = 0;

                t = y;
                b = y + h;
            }

            if (h > row_h) {
                row_h = h;
            }

            x += w;

            auto const rect = rect_t {l, t, r, b};

            glyphs_.emplace_back(g);
            rects_.emplace_back(rect);
        }

        texture_w_ = max_width;
        texture_h_ = y + row_h;
    }

    uint16_t required_w() const {
        return texture_w_;
    }

    uint16_t required_h() const {
        return texture_h_;
    }

    glyph_data operator[](uint32_t const codepoint) const {
        if (codepoint < first_ || codepoint > last_) {
            return glyph_data {};
        }

        return glyphs_[codepoint - first_];
    }

    rect_t glyph_rect(uint32_t const codepoint) const {
        if (codepoint < first_ || codepoint > last_) {
            return rect_t {};
        }

        return rects_[codepoint - first_];
    }

    ~code_block_cache() {
        for (auto& glyph : glyphs_) {
            FT_Done_Glyph(glyph.glyph);
        }
    }
private:
    uint32_t first_;
    uint32_t last_;

    uint16_t texture_w_;
    uint16_t texture_h_;

    std::vector<glyph_data> glyphs_;
    std::vector<rect_t>     rects_;
};

class text_renderer {
public:
    text_renderer();


    FT_Glyph get_glyph(FT_ULong const codepoint) {
        if (codepoint & 0x80) {
            BK_TODO_FAIL(); //unicode
        }

        return basic_latin_[codepoint].glyph;
    }

    axis_aligned_rect<uint16_t> get_glyph_rect(FT_ULong const codepoint) {
        return basic_latin_.glyph_rect(codepoint);
    }

    uint16_t required_w() const {
        return basic_latin_.required_w();
    }

    uint16_t required_h() const {
        return basic_latin_.required_h();
    }

private:
    ft_unique<FT_Library> ft_lib_;
    ft_unique<FT_Face>    ft_face_;

    code_block_cache<1> basic_latin_;

    bool has_kerning_ = false;
};

class sdl_renderer {
//TODO should use pimpl
public:
    sdl_renderer(sdl_renderer&) = delete;
    sdl_renderer& operator=(sdl_renderer&) = delete;

    explicit sdl_renderer(SDL_Window* window);

    void clear() {
        auto const result = ::SDL_RenderClear(handle());
        if (result != 0) {
            BK_ASSERT(false); //TODO
        }

        ::SDL_RenderSetScale(handle(), 1.0f, 1.0f);
        draw_text("Hello", text_rect{20.0f, 20.0f, 100.0f, 100.0f});
    }

    void present() {
        ::SDL_RenderPresent(handle());
    }


    void draw_tile(sprite_sheet& sheet, unsigned index, unsigned x, unsigned y);

    void set_scale(float const scale_x, float const scale_y) {
        BK_ASSERT(scale_x > 0.0f && scale_y > 0.0f);

        scale_x_ = scale_x;
        scale_y_ = scale_y;

        ::SDL_RenderSetScale(handle(), scale_x_, scale_y_);
    }

    void set_scale_x(float const scale) { set_scale(scale, scale_y_); }
    void set_scale_y(float const scale) { set_scale(scale_x_, scale); }

    void set_translation(float const x, float const y) {
        set_translation_x(x);
        set_translation_y(y);
    }

    void set_translation_x(float const x) {
        trans_x_ = x;
    }

    void set_translation_y(float const y) {
        trans_y_ = y;
    }

    float get_translation_x() const { return trans_x_; }
    float get_translation_y() const { return trans_y_; }

    float get_scale_x() const { return scale_x_; }
    float get_scale_y() const { return scale_y_; }

    SDL_Renderer* handle() {
        return renderer_.get();
    }

    using text_rect = axis_aligned_rect<float>;
    void draw_text(string_ref string, text_rect rect);
private:
    sdl_unique<SDL_Renderer> renderer_;
    sdl_unique<SDL_Texture>  texture_;
    sdl_unique<SDL_PixelFormat> pixel_format_;
    text_renderer            text_renderer_;

    sdl_unique<SDL_Texture>  glyph_texture_;
    int glyph_texture_w_ = 0;
    int glyph_texture_h_ = 0;

    SDL_Texture* get_glyph_texture_(int w, int h);
    void write_glyph_(SDL_Texture* texture, SDL_Rect const& dst, FT_Bitmap const& bitmap);

    float scale_x_ = 1.0f;
    float scale_y_ = 1.0f;
    float trans_x_ = 0.0f;
    float trans_y_ = 0.0f;
private:

};

//==============================================================================
// sdl_application
//==============================================================================
class sdl_application {
public:
    sdl_application();

    void pump_events();

    void handle_keyboard_event_(SDL_KeyboardEvent const& event);
    void handle_window_event_(SDL_WindowEvent const& event);
    void handle_mousemotion_event_(SDL_MouseMotionEvent const& event);

    explicit operator bool() const { return running_; }

    SDL_Window* handle() { return window_.get(); }

    using command_sink = std::function<void (command_type)>;
    void on_command(command_sink sink) {
        command_sink_ = std::move(sink);
    }

    using resize_sink = std::function<void (unsigned w, unsigned h)>;
    void on_resize(resize_sink sink) {
        resize_sink_ = std::move(sink);

        int w = 0;
        int h = 0;
        SDL_GetWindowSize(window_.get(), &w, &h);

        resize_sink_(w, h);
    }

    using mouse_move_sink = std::function<void (signed dx, signed dy, std::bitset<8> buttons)>;
    void on_mouse_move(mouse_move_sink sink) {
        mouse_move_sink_ = std::move(sink);
    }
private:
    sdl_state              state_;
    sdl_unique<SDL_Window> window_;
    SDL_Event              event_;
    bool                   running_;

    command_sink command_sink_;
    resize_sink  resize_sink_;
    mouse_move_sink mouse_move_sink_;
};

} //namespace bkrl
