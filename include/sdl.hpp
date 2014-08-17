#pragma once

#include <sdl/SDL.h>
#include "exception.hpp"
#include "assert.hpp"
#include "command_type.hpp"

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
struct sdl_deleter;

template <>
struct sdl_deleter<SDL_Window> {
    void operator()(SDL_Window* ptr) const {
        ::SDL_DestroyWindow(ptr);
    }
};

template <>
struct sdl_deleter<SDL_Renderer> {
    void operator()(SDL_Renderer* ptr) const {
        ::SDL_DestroyRenderer(ptr);
    }
};

template <>
struct sdl_deleter<SDL_Texture> {
    void operator()(SDL_Texture* ptr) const {
        ::SDL_DestroyTexture(ptr);
    }
};

template <>
struct sdl_deleter<SDL_Surface> {
    void operator()(SDL_Surface* ptr) const {
        ::SDL_FreeSurface(ptr);
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
public:
    sdl_renderer(sdl_renderer&) = delete;
    sdl_renderer& operator=(sdl_renderer&) = delete;

    explicit sdl_renderer(SDL_Window* window);

    void clear() {
        auto const result = ::SDL_RenderClear(handle());
        if (result != 0) {
            BK_ASSERT(false); //TODO
        }

        ::SDL_RenderSetScale(handle(), scale_x_, scale_y_);
    }

    void present() {
        ::SDL_RenderPresent(handle());
    }


    void draw_tile(sprite_sheet& sheet, unsigned index, unsigned x, unsigned y);

    void set_scale(float scale_x, float scale_y);
    void set_scale_x(float const scale) { scale_x_ = scale; }
    void set_scale_y(float const scale) { scale_y_ = scale; }

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
private:
    sdl_unique<SDL_Renderer> renderer_;
    sdl_unique<SDL_Texture>  texture_;

    float scale_x_;
    float scale_y_;
    float trans_x_;
    float trans_y_;
};

//==============================================================================
// sdl_application
//==============================================================================
class sdl_application {
public:
    sdl_application();

    void pump_events();

    void handle_keyboard_event_(SDL_KeyboardEvent const& event);

    explicit operator bool() const { return running_; }

    SDL_Window* handle() { return window_.get(); }

    using command_sink = std::function<void (command_type)>;
    void on_command(command_sink sink) {
        command_sink_ = std::move(sink);
    }
private:
    sdl_state              state_;
    sdl_unique<SDL_Window> window_;
    SDL_Event              event_;
    bool                   running_;

    command_sink command_sink_;
};

} //namespace bkrl
