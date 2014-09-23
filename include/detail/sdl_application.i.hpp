#pragma once

#include <sdl/SDL.h>

#include "renderer.hpp"
#include "tile_sheet.hpp"
#include "keyboard.hpp"

namespace bkrl { namespace detail {

template <typename T>
inline static SDL_Rect make_sdl_rect(axis_aligned_rect<T> const rect) noexcept {
    return SDL_Rect {
        static_cast<int>(rect.left)
      , static_cast<int>(rect.top)
      , static_cast<int>(rect.width())
      , static_cast<int>(rect.height())
    };
}

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
class sdl_state {
public:
    BK_NOCOPY(sdl_state);

    sdl_state(sdl_state&& other) noexcept {
        other.do_quit_ = false;
    }

    sdl_state& operator=(sdl_state&& rhs) noexcept {
        rhs.do_quit_ = false;
        do_quit_ = true;
    }

    sdl_state() {
        auto const result = SDL_Init(SDL_INIT_VIDEO);
        if (result) {
            BK_TODO_FAIL();
            //BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_Init"));
        }
    }

    ~sdl_state() {
        if (do_quit_) {
            SDL_Quit();
        }
    }
private:
    bool do_quit_ = true;
};

//==============================================================================
// SDL2 application_impl
//==============================================================================
class application_impl : public application_base {
public:
    explicit application_impl(string_ref keymap);

    handle_t handle() const;

    bool is_running() const;
    bool has_events() const;

    void do_one_event();
    void do_all_events();

    void on_command(command_sink sink)           { on_command_      = sink; }
    void on_close(close_sink sink)               { on_close_        = sink; }
    void on_resize(resize_sink sink)             { on_resize_       = sink; }
    void on_mouse_move(mouse_move_sink sink)     { on_mouse_move_   = sink; }
    void on_mouse_button(mouse_button_sink sink) { on_mouse_button_ = sink; }
    void on_mouse_wheel(mouse_wheel_sink sink)   { on_mouse_wheel_  = sink; }
private:
    static sdl_unique<SDL_Window> create_window_();

    void handle_event(SDL_Event const& event);
    void handle_event_quit(SDL_QuitEvent const& event);
    void handle_event_window(SDL_WindowEvent const& event);
    void handle_event_kb(SDL_KeyboardEvent const& event);
    void handle_event_mouse_move(SDL_MouseMotionEvent const& event);
    void handle_event_mouse_button(SDL_MouseButtonEvent const& event);
    void handle_event_mouse_wheel(SDL_MouseWheelEvent const& event);
private:
    sdl_state state_; //must be first
    sdl_unique<SDL_Window> window_;

    command_sink      on_command_;
    close_sink        on_close_;
    resize_sink       on_resize_;
    mouse_move_sink   on_mouse_move_;
    mouse_button_sink on_mouse_button_;
    mouse_wheel_sink  on_mouse_wheel_;

    keymap key_map_;

    bool running_;
};

////////////////////////////////////////////////////////////////////////////////
sdl_unique<SDL_Window> application_impl::create_window_() {
    auto const result = SDL_CreateWindow(
        "BKRL"
      , SDL_WINDOWPOS_UNDEFINED
      , SDL_WINDOWPOS_UNDEFINED
      , 1024
      , 768
      , SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );

    if (result == nullptr) {
        BK_TODO_FAIL();
        //BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateWindow"));
    }

    return sdl_unique<SDL_Window> {result};
}


application_impl::application_impl(string_ref keymap)
  : state_   {}
  , window_  {create_window_()}
  , key_map_ {keymap}
  , running_ {true}
{
    on_command_      = [](command_type) {};
    on_close_        = []() {};
    on_resize_       = [](unsigned, unsigned) {};
    on_mouse_move_   = [](mouse_move_info const&) {};
    on_mouse_button_ = [](mouse_button_info const&) {};
    on_mouse_wheel_  = [](mouse_wheel_info const&) {};
}


application::handle_t
application_impl::handle() const {
    return window_;
}

bool
application_impl::is_running() const {
    return running_;
}

bool
application_impl::has_events() const {
    return SDL_PollEvent(nullptr) != 0;
}

void
application_impl::handle_event(SDL_Event const& event) {
    switch (event.type) {
    case SDL_QUIT :
        handle_event_quit(event.quit);
        break;
    case SDL_APP_TERMINATING :
    case SDL_APP_LOWMEMORY :
    case SDL_APP_WILLENTERBACKGROUND :
    case SDL_APP_DIDENTERBACKGROUND :
    case SDL_APP_WILLENTERFOREGROUND :
    case SDL_APP_DIDENTERFOREGROUND :
        break;
    case SDL_WINDOWEVENT :
        handle_event_window(event.window);
        break;
    case SDL_SYSWMEVENT :
        break;
    case SDL_KEYDOWN :
    case SDL_KEYUP :
        handle_event_kb(event.key);
        break;
    case SDL_TEXTEDITING :
    case SDL_TEXTINPUT :
        break;
    case SDL_MOUSEMOTION :
        handle_event_mouse_move(event.motion);
        break;
    case SDL_MOUSEBUTTONDOWN :
    case SDL_MOUSEBUTTONUP :
        handle_event_mouse_button(event.button);
        break;
    case SDL_MOUSEWHEEL :
        handle_event_mouse_wheel(event.wheel);
        break;
    case SDL_JOYAXISMOTION :
    case SDL_JOYBALLMOTION :
    case SDL_JOYHATMOTION :
    case SDL_JOYBUTTONDOWN :
    case SDL_JOYBUTTONUP :
    case SDL_JOYDEVICEADDED :
    case SDL_JOYDEVICEREMOVED :
        break;
    case SDL_CONTROLLERAXISMOTION :
    case SDL_CONTROLLERBUTTONDOWN :
    case SDL_CONTROLLERBUTTONUP :
    case SDL_CONTROLLERDEVICEADDED :
    case SDL_CONTROLLERDEVICEREMOVED :
    case SDL_CONTROLLERDEVICEREMAPPED :
        break;
    case SDL_FINGERDOWN :
    case SDL_FINGERUP :
    case SDL_FINGERMOTION :
    case SDL_DOLLARGESTURE :
    case SDL_DOLLARRECORD :
    case SDL_MULTIGESTURE :
        break;
    case SDL_CLIPBOARDUPDATE :
        break;
    case SDL_DROPFILE :
        break;
    case SDL_RENDER_TARGETS_RESET :
        //BK_TODO_FAIL();
        break;
    case SDL_USEREVENT :
        break;
    }
}

void
application_impl::handle_event_quit(SDL_QuitEvent const&) {
    BK_ASSERT_DBG(running_);
    running_ = false;
}

void
application_impl::handle_event_window(SDL_WindowEvent const& event) {
    switch (event.event) {
    case SDL_WINDOWEVENT_NONE :
        std::cout << "SDL_WINDOWEVENT_NONE" << std::endl;
        break;
    case SDL_WINDOWEVENT_SHOWN :
        std::cout << "SDL_WINDOWEVENT_SHOWN" << std::endl;
        break;
    case SDL_WINDOWEVENT_HIDDEN :
        std::cout << "SDL_WINDOWEVENT_HIDDEN" << std::endl;
        break;
    case SDL_WINDOWEVENT_EXPOSED :
        std::cout << "SDL_WINDOWEVENT_EXPOSED" << std::endl;
        break;
    case SDL_WINDOWEVENT_MOVED :
        std::cout << "SDL_WINDOWEVENT_MOVED" << std::endl;
        break;
    case SDL_WINDOWEVENT_RESIZED :
        std::cout << "SDL_WINDOWEVENT_RESIZED" << std::endl;
        break;
    case SDL_WINDOWEVENT_SIZE_CHANGED :
        std::cout << "SDL_WINDOWEVENT_SIZE_CHANGED" << std::endl;
        on_resize_(event.data1, event.data2);
        break;
    case SDL_WINDOWEVENT_MINIMIZED :
        std::cout << "SDL_WINDOWEVENT_MINIMIZED" << std::endl;
        break;
    case SDL_WINDOWEVENT_MAXIMIZED :
        std::cout << "SDL_WINDOWEVENT_MAXIMIZED" << std::endl;
        break;
    case SDL_WINDOWEVENT_RESTORED :
        std::cout << "SDL_WINDOWEVENT_RESTORED" << std::endl;
        break;
    case SDL_WINDOWEVENT_ENTER :
        std::cout << "SDL_WINDOWEVENT_ENTER" << std::endl;
        break;
    case SDL_WINDOWEVENT_LEAVE :
        std::cout << "SDL_WINDOWEVENT_LEAVE" << std::endl;
        break;
    case SDL_WINDOWEVENT_FOCUS_GAINED :
        std::cout << "SDL_WINDOWEVENT_FOCUS_GAINED" << std::endl;
        break;
    case SDL_WINDOWEVENT_FOCUS_LOST :
        std::cout << "SDL_WINDOWEVENT_FOCUS_LOST" << std::endl;
        break;
    case SDL_WINDOWEVENT_CLOSE :
        std::cout << "SDL_WINDOWEVENT_CLOSE" << std::endl;
        break;
    }
}

void
application_impl::handle_event_kb(SDL_KeyboardEvent const& event) {  
    if (event.type != SDL_KEYDOWN) {
        return;
    }

    key_modifier mods;

    //TODO
    auto const flags = event.keysym.mod;

    if (flags & KMOD_LCTRL) {
        mods.set(key_modifier_type::ctrl_left);
        //mods.set(key_modifier_type::ctrl);
    }
    if (flags & KMOD_RCTRL) {
        mods.set(key_modifier_type::ctrl_right);
        //mods.set(key_modifier_type::ctrl);
    }
    if (flags & KMOD_LALT) {
        mods.set(key_modifier_type::alt_left);
        //mods.set(key_modifier_type::alt);
    }
    if (flags & KMOD_RALT) {
        mods.set(key_modifier_type::alt_right);
        //mods.set(key_modifier_type::alt);
    }
    if (flags & KMOD_LSHIFT) {
        mods.set(key_modifier_type::shift_left);
        //mods.set(key_modifier_type::shift);
    }
    if (flags & KMOD_RSHIFT) {
        mods.set(key_modifier_type::shift_right);
        //mods.set(key_modifier_type::shift);
    }

    key_combo const key {
        static_cast<scancode>(event.keysym.scancode)
      , mods
    };

    auto const command = key_map_[key];
    on_command_(command);
}

void
application_impl::handle_event_mouse_move(SDL_MouseMotionEvent const& event) {
    mouse_move_info const info {
        event.x,    event.y
      , event.xrel, event.yrel
      , event.state
    };

    on_mouse_move_(info);
}

void
application_impl::handle_event_mouse_button(SDL_MouseButtonEvent const& event) {
    mouse_button_info const info {
        event.x, event.y
      , event.button
      , (event.state == SDL_PRESSED)
        ? mouse_button_info::state_t::pressed
        : mouse_button_info::state_t::released
      , event.clicks
    };

    on_mouse_button_(info);
}

void
application_impl::handle_event_mouse_wheel(SDL_MouseWheelEvent const& event) {
    mouse_wheel_info const info {
        event.x, event.y
    };

    on_mouse_wheel_(info);
}

void
application_impl::do_one_event() {
    BK_TODO_FAIL();
}

void
application_impl::do_all_events() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        handle_event(event);
    }
}

////////////////////////////////////////////////////////////////////////////////

class renderer_impl : public renderer_base {
public:
    renderer_impl(application const& app);

    handle_t handle() const;
    ////////////////////////////////////////////////////////////////////////////

    void clear();
    void present();

    ////////////////////////////////////////////////////////////////////////////

    texture create_texture(string_ref filename);
    texture create_texture(uint8_t* buffer, int width, int height);
    void delete_texture(texture& tex);

    ////////////////////////////////////////////////////////////////////////////

    void set_translation_x(scalar dx);
    void set_translation_y(scalar dy);

    void set_scale_x(scalar sx);
    void set_scale_y(scalar sy);
    
    ////////////////////////////////////////////////////////////////////////////

    void draw_texture(texture const& tex, scalar x, scalar y) {
        auto const sdl_tex = tex.handle().as<SDL_Texture*>();
        
        SDL_SetTextureBlendMode(sdl_tex, SDL_BLENDMODE_BLEND);
        SDL_SetTextureColorMod(sdl_tex, 0xFF, 0, 0);
        
        SDL_Rect r {static_cast<int>(x), static_cast<int>(y), 0, 0};
        SDL_QueryTexture(sdl_tex, nullptr, nullptr, &r.w, &r.h);

        SDL_RenderCopy(renderer_.get(), sdl_tex, nullptr, &r);
    }

    void draw_texture(texture const& tex, rect src, rect dst);

    void draw_tile(
        tile_sheet const& sheet
      , unsigned ix
      , unsigned iy
      , scalar   x
      , scalar   y
    );

private:
    texture create_texture_(SDL_Surface* surface);

    static sdl_unique<SDL_Renderer> create_renderer_(application const& app);

    sdl_unique<SDL_Renderer> renderer_;

    std::vector<sdl_unique<SDL_Texture>> textures_;

    vector2d<float> translation_;
    vector2d<float> scale_;
};

//------------------------------------------------------------------------------
sdl_unique<SDL_Renderer>
renderer_impl::create_renderer_(application const& app) {
    auto const window = reinterpret_cast<SDL_Window*>(app.handle().value);

    auto const result = SDL_CreateRenderer(
        window
      , -1
      , SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (result == nullptr) {
        BK_TODO_FAIL();
        //BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateRenderer"));
    }

    return sdl_unique<SDL_Renderer> {result};
}

//------------------------------------------------------------------------------
//sdl_unique<SDL_Texture>
//renderer_impl::create_tile_sheet_texture_(SDL_Renderer& renderer) {
//    //TODO
//    auto const result_bmp = SDL_LoadBMP("./data/Myne.bmp");
//    if (result_bmp == nullptr) {
//        BK_TODO_FAIL();
//        //BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_LoadBMP"));
//    }
//
//    sdl_unique<SDL_Surface> surface {result_bmp};
//
//    auto const result_texture = SDL_CreateTextureFromSurface(&renderer, surface.get());
//    if (result_bmp == nullptr) {
//        BK_TODO_FAIL();
//        //BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateTextureFromSurface"));
//    }
//
//    return sdl_unique<SDL_Texture> {result_texture};
//}

//------------------------------------------------------------------------------
renderer_impl::renderer_impl(application const& app)
  : renderer_           {create_renderer_(app)}
  , translation_ {{0.0f, 0.0f}}
  , scale_       {{1.0f, 1.0f}}
{
}

//------------------------------------------------------------------------------
renderer_impl::handle_t
renderer_impl::handle() const {
    return renderer_;
}

//------------------------------------------------------------------------------
void
renderer_impl::clear() {
    auto const r = renderer_.get();

    auto const result = SDL_RenderClear(r);
    if (result != 0) {
        BK_TODO_FAIL(); //TODO
    }

    SDL_RenderSetScale(r, 1.0f, 1.0f);
}

//------------------------------------------------------------------------------
void
renderer_impl::present() {
    auto const r = renderer_.get();

    SDL_RenderPresent(r);
}

//------------------------------------------------------------------------------
texture
renderer_impl::create_texture_(SDL_Surface* surface) {
    BK_ASSERT_DBG(surface);

    auto const result = SDL_CreateTextureFromSurface(renderer_.get(), surface);
    if (result == nullptr) {
        BK_TODO_FAIL();
        //BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateTextureFromSurface"));
    }

    auto const handle = texture::handle_t {result};
    auto const id     = textures_.size();

    textures_.emplace_back(
        sdl_unique<SDL_Texture> {result}
    );

    return texture {handle, id, surface->w, surface->h};
}

texture
renderer_impl::create_texture(string_ref filename) {
    //TODO make this work for not just bmp.

    auto const result = SDL_LoadBMP(filename.data());
    if (result == nullptr) {
        BK_TODO_FAIL();
        //BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_LoadBMP"));
    }

    sdl_unique<SDL_Surface> surface {result};

    return create_texture_(surface.get());
}

//------------------------------------------------------------------------------
texture
renderer_impl::create_texture(
    uint8_t* const buffer
  , int      const width
  , int      const height
) {
    auto const result = SDL_CreateRGBSurfaceFrom(
        buffer
      , width
      , height
      , 32
      , width * 4
      , 0xFFu << 16
      , 0xFFu <<  8
      , 0xFFu <<  0
      , 0xFFu << 24
    );

    if (!result) {
        BK_TODO_FAIL();
    }

    sdl_unique<SDL_Surface> surface {result};

    return create_texture_(surface.get());
}

//------------------------------------------------------------------------------
void
renderer_impl::delete_texture(texture& tex) {
    auto const handle  = tex.handle();
    auto const sdl_tex = reinterpret_cast<SDL_Texture*>(handle.value);

    auto const it = std::find_if(
        std::cbegin(textures_)
      , std::cend(textures_)
      , [&](sdl_unique<SDL_Texture> const& t) {
            return t.get() == sdl_tex;
        }
    );

    if (it == std::cend(textures_)) {
        BK_TODO_FAIL();
        return;
    }

    textures_.erase(it);
}

//------------------------------------------------------------------------------
void renderer_impl::set_translation_x(scalar const dx) {
    translation_.x = dx;
}

//------------------------------------------------------------------------------
void renderer_impl::set_translation_y(scalar const dy) {
    translation_.y = dy;
}

//------------------------------------------------------------------------------
void renderer_impl::set_scale_x(scalar const sx) {
    scale_.x = sx;
}

//------------------------------------------------------------------------------
void renderer_impl::set_scale_y(scalar const sy) {
    scale_.y = sy;
}

//------------------------------------------------------------------------------
void
renderer_impl::draw_tile(
    tile_sheet const& sheet
  , unsigned const ix
  , unsigned const iy
  , scalar   const x
  , scalar   const y
) {
    //auto const src = make_sdl_rect(sheet.at(ix, iy));

    //auto const w = src.w;
    //auto const h = src.h;

    //auto const dst_x = static_cast<int>(scale_.x * (x*w + translation_.x));
    //auto const dst_y = static_cast<int>(scale_.y * (y*h + translation_.y));

    //auto const dst = SDL_Rect {
    //    dst_x
    //  , dst_y
    //  , static_cast<int>(w * scale_.x)
    //  , static_cast<int>(h * scale_.y)
    //};

    //auto const tex = reinterpret_cast<SDL_Texture*>(
    //    sheet.tile_texture.handle().value
    //);

    //auto const result = SDL_RenderCopy(
    //    renderer_.get()
    //  , tex
    //  , &src
    //  , &dst
    //);

    //if (result != 0) {
    //    BK_TODO_FAIL(); //TODO
    //}
}

//------------------------------------------------------------------------------
void
renderer_impl::draw_texture(texture const& tex, rect const src, rect const dst) {
    auto const sdl_tex  = tex.handle().as<SDL_Texture*>();
    auto const src_rect = make_sdl_rect(src);

    auto dst_rect = make_sdl_rect(dst);
    dst_rect.x = static_cast<int>(scale_.x * (dst_rect.x + translation_.x));
    dst_rect.y = static_cast<int>(scale_.y * (dst_rect.y + translation_.y));
    dst_rect.w = static_cast<int>(scale_.x * dst_rect.w);
    dst_rect.h = static_cast<int>(scale_.y * dst_rect.h);

    auto const result = SDL_RenderCopy(renderer_.get(), sdl_tex, &src_rect, &dst_rect);
    if (result) {
        BK_TODO_FAIL();
    }
}

}} //namespace bkrl::detail
