#pragma once

#include <sdl/SDL.h>
#include "exception.hpp"
#include "assert.hpp"
#include "command_type.hpp"
#include "math.hpp"
#include "algorithm.hpp"

#include "font.hpp"

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


class sdl_renderer {
//TODO should use pimpl
public:
    sdl_renderer(sdl_renderer&) = delete;
    sdl_renderer& operator=(sdl_renderer&) = delete;

    explicit sdl_renderer(SDL_Window* window);

    void clear();

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
    void handle_mouse_button_event_(SDL_MouseButtonEvent const& event);

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

    using mouse_button_sink = std::function<void (signed x, signed y)>;
    void on_mouse_button(mouse_button_sink sink) {
        mouse_button_sink_ = std::move(sink);
    }
private:
    sdl_state              state_;
    sdl_unique<SDL_Window> window_;
    SDL_Event              event_;
    bool                   running_;

    command_sink command_sink_;
    resize_sink  resize_sink_;
    mouse_move_sink mouse_move_sink_;
    mouse_button_sink mouse_button_sink_;
};

} //namespace bkrl
