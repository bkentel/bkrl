#pragma once

#include <sdl/SDL.h>

#include "renderer.hpp"
#include "keyboard.hpp"
#include "config.hpp"
#include "util.hpp"

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

template <> struct sdl_deleter<SDL_Window> {
    void operator()(SDL_Window* ptr) const noexcept {
        ::SDL_DestroyWindow(ptr);
    }
};

template <> struct sdl_deleter<SDL_Renderer> {
    void operator()(SDL_Renderer* ptr) const noexcept {
        ::SDL_DestroyRenderer(ptr);
    }
};

template <> struct sdl_deleter<SDL_Texture> {
    void operator()(SDL_Texture* ptr) const noexcept {
        ::SDL_DestroyTexture(ptr);
    }
};

template <> struct sdl_deleter<SDL_Surface> {
    void operator()(SDL_Surface* ptr) const noexcept {
        ::SDL_FreeSurface(ptr);
    }
};

template <> struct sdl_deleter<SDL_PixelFormat> {
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
        using std::swap;
        swap(do_quit_, rhs.do_quit_);

        return *this;
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
    application_impl(keymap const& map, config const& cfg);

    handle_t handle() const;

    bool is_running() const;
    bool has_events() const;

    int client_width() const;
    int client_height() const;

    void do_one_event();
    void do_all_events();

    void on_command(char_sink sink)              { on_char_         = sink; }
    void on_command(command_sink sink)           { on_command_      = sink; }
    void on_close(close_sink sink)               { on_close_        = sink; }
    void on_resize(resize_sink sink)             { on_resize_       = sink; }
    void on_mouse_move(mouse_move_sink sink)     { on_mouse_move_   = sink; }
    void on_mouse_button(mouse_button_sink sink) { on_mouse_button_ = sink; }
    void on_mouse_wheel(mouse_wheel_sink sink)   { on_mouse_wheel_  = sink; }
private:
    static sdl_unique<SDL_Window> create_window_(
        optional<uint32_t> width
      , optional<uint32_t> height
      , optional<int32_t>  x
      , optional<int32_t>  y
    );

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

    char_sink         on_char_;
    command_sink      on_command_;
    close_sink        on_close_;
    resize_sink       on_resize_;
    mouse_move_sink   on_mouse_move_;
    mouse_button_sink on_mouse_button_;
    mouse_wheel_sink  on_mouse_wheel_;

    keymap const* key_map_;

    bool running_;
};

////////////////////////////////////////////////////////////////////////////////
sdl_unique<SDL_Window>
application_impl::create_window_(
    optional<uint32_t> width
  , optional<uint32_t> height
  , optional<int32_t>  x
  , optional<int32_t>  y
) {
    constexpr auto default_w = 1280;
    constexpr auto default_h = 1024;

    auto const result = SDL_CreateWindow(
        "BKRL"
      , x.get_value_or(SDL_WINDOWPOS_UNDEFINED)
      , y.get_value_or(SDL_WINDOWPOS_UNDEFINED)
      , width.get_value_or(default_w)
      , height.get_value_or(default_h)
      , SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );

    if (result == nullptr) {
        BK_TODO_FAIL();
        //BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateWindow"));
    }

    return sdl_unique<SDL_Window> {result};
}

//------------------------------------------------------------------------------
application_impl::application_impl(
    keymap const& map
  , config const& cfg
)
  : state_   {}
  , window_  {create_window_(cfg.window_w, cfg.window_h, cfg.window_x, cfg.window_y)}
  , key_map_ {&map}
  , running_ {true}
{
    on_char_         = [](char) {};
    on_command_      = [](command_type) { return true; };
    on_close_        = []() {};
    on_resize_       = [](unsigned, unsigned) {};
    on_mouse_move_   = [](mouse_move_info const&) {};
    on_mouse_button_ = [](mouse_button_info const&) {};
    on_mouse_wheel_  = [](mouse_wheel_info const&) {};
}

//------------------------------------------------------------------------------
application::handle_t
application_impl::handle() const {
    return window_;
}

//------------------------------------------------------------------------------
bool
application_impl::is_running() const {
    return running_;
}

//------------------------------------------------------------------------------
bool
application_impl::has_events() const {
    return SDL_PollEvent(nullptr) != 0;
}

//------------------------------------------------------------------------------
int
application_impl::client_width() const {
    auto w = int {};
    SDL_GetWindowSize(window_.get(), &w, nullptr);

    BK_ASSERT_DBG(w > 0);
    return w;
}

//------------------------------------------------------------------------------
int
application_impl::client_height() const {
    auto h = int {};
    SDL_GetWindowSize(window_.get(), nullptr, &h);

    BK_ASSERT_DBG(h > 0);
    return h;
}

//------------------------------------------------------------------------------
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
    default :
        break;
    }
}

//------------------------------------------------------------------------------
void
application_impl::handle_event_quit(SDL_QuitEvent const&) {
    BK_ASSERT_DBG(running_);
    running_ = false;
}

//------------------------------------------------------------------------------
void
application_impl::handle_event_window(SDL_WindowEvent const& event) {
    switch (event.event) {
    case SDL_WINDOWEVENT_NONE :
        break;
    case SDL_WINDOWEVENT_SHOWN :
        break;
    case SDL_WINDOWEVENT_HIDDEN :
        break;
    case SDL_WINDOWEVENT_EXPOSED :
        break;
    case SDL_WINDOWEVENT_MOVED :
        break;
    case SDL_WINDOWEVENT_RESIZED :
        break;
    case SDL_WINDOWEVENT_SIZE_CHANGED :
        on_resize_(event.data1, event.data2);
        break;
    case SDL_WINDOWEVENT_MINIMIZED :
        break;
    case SDL_WINDOWEVENT_MAXIMIZED :
        break;
    case SDL_WINDOWEVENT_RESTORED :
        break;
    case SDL_WINDOWEVENT_ENTER :
        break;
    case SDL_WINDOWEVENT_LEAVE :
        break;
    case SDL_WINDOWEVENT_FOCUS_GAINED :
        break;
    case SDL_WINDOWEVENT_FOCUS_LOST :
        break;
    case SDL_WINDOWEVENT_CLOSE :
        break;
    default :
        break;
    }
}

//------------------------------------------------------------------------------
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
    }
    if (flags & KMOD_RCTRL) {
        mods.set(key_modifier_type::ctrl_right);
    }
    if (flags & KMOD_LALT) {
        mods.set(key_modifier_type::alt_left);
    }
    if (flags & KMOD_RALT) {
        mods.set(key_modifier_type::alt_right);
    }
    if (flags & KMOD_LSHIFT) {
        mods.set(key_modifier_type::shift_left);
    }
    if (flags & KMOD_RSHIFT) {
        mods.set(key_modifier_type::shift_right);
    }

    key_combo const key {
        static_cast<scancode>(event.keysym.scancode)
      , mods
    };

    auto const command = (*key_map_)[key];
    if (!on_command_(command)) {
        return;
    }

    auto const keycode = event.keysym.sym;
    if (keycode == (keycode & 0x7F)) {
        on_char_(static_cast<char>(keycode));
    }
}

//------------------------------------------------------------------------------
void
application_impl::handle_event_mouse_move(SDL_MouseMotionEvent const& event) {
    mouse_move_info const info {
        event.x,    event.y
      , event.xrel, event.yrel
      , event.state
    };

    on_mouse_move_(info);
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void
application_impl::handle_event_mouse_wheel(SDL_MouseWheelEvent const& event) {
    mouse_wheel_info const info {
        event.x, event.y
    };

    on_mouse_wheel_(info);
}

//------------------------------------------------------------------------------
void
application_impl::do_one_event() {
    BK_TODO_FAIL();
}

//------------------------------------------------------------------------------
void
application_impl::do_all_events() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        handle_event(event);
    }
}

////////////////////////////////////////////////////////////////////////////////
//! SDL implementation of renderer.
////////////////////////////////////////////////////////////////////////////////
class renderer_impl : public renderer_base {
public:
    static SDL_Texture* get_texture(texture& t) {
        return t.handle().as<SDL_Texture*>();
    }

    static SDL_Texture* get_texture(texture const& t) {
        //HACK TODO sdl is broken wrt to constness?
        return t.handle().as<SDL_Texture*>();
    }

    renderer_impl(application const& app);

    handle_t handle() const;
    ////////////////////////////////////////////////////////////////////////////

    void clear();
    void present();

    ////////////////////////////////////////////////////////////////////////////

    texture create_texture(path_string_ref filename);
    texture create_texture(void const* buffer, tex_coord_i width, tex_coord_i height);
    texture create_texture(tex_coord_i width, tex_coord_i height);
    void delete_texture(texture& tex);

    void update_texture(texture& tex, void const* data, int pitch, int x, int y, int w, int h);

    void set_color_mod(texture& tex, uint8_t r, uint8_t g, uint8_t b) {
        auto const t = get_texture(tex);
        
        auto const result = SDL_SetTextureColorMod(t, r, g, b);
        if (result) {
            BK_TODO_FAIL();
        }
    }

    void set_color_mod(
        texture& tex
      , uint8_t r, uint8_t g, uint8_t b
      , uint8_t //a //TODO
    ) {
        set_color_mod(tex, r, g, b);
    }
    
    void set_alpha_mod(texture& tex, uint8_t a) {
        auto const t = get_texture(tex);
        
        auto const result = SDL_SetTextureAlphaMod(t, a);
        if (result) {
            BK_TODO_FAIL();
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    void set_translation_x(trans_t dx);
    void set_translation_y(trans_t dy);

    void set_scale_x(scale_t sx);
    void set_scale_y(scale_t sy);
    
    void set_scale(scale_t sx, scale_t sy);

    scale_vec get_scale() const;
    trans_vec get_translation() const;

    ////////////////////////////////////////////////////////////////////////////

    void set_draw_alpha(uint8_t const) {
    }

    void set_draw_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        auto const result = SDL_SetRenderDrawColor(renderer_.get(), r, g, b, a);
        if (result) {
            BK_TODO_FAIL();
        }
    }

    void set_draw_color(uint8_t r, uint8_t g, uint8_t b) {
        set_draw_color(r, g, b, 255);
    }

    void draw_filled_rect(rect_t const bounds) {
        auto dst = make_sdl_rect(bounds);
        dst.x += translation_.x;
        dst.y += translation_.y;

        auto const result = SDL_RenderFillRect(renderer_.get(), &dst);
        if (result) {
            BK_TODO_FAIL();
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    void draw_texture(texture const& tex, pos_t x, pos_t y);
    void draw_texture(texture const& tex, rect_t src, rect_t dst);
private:
    texture create_texture_(SDL_Surface* surface);

    static sdl_unique<SDL_Renderer> create_renderer_(application const& app);

    sdl_unique<SDL_Renderer> renderer_;

    std::vector<sdl_unique<SDL_Texture>> textures_;

    trans_vec translation_;
    scale_vec scale_;
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
renderer_impl::renderer_impl(application const& app)
  : renderer_    {create_renderer_(app)}
  , translation_ (trans_vec {0, 0})
  , scale_       (scale_vec {1.0f, 1.0f})
{
    auto const result = SDL_SetRenderDrawBlendMode(renderer_.get(), SDL_BLENDMODE_BLEND);
    if (result) {
        BK_TODO_FAIL();
    }
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

    auto result = SDL_RenderClear(r);
    if (result != 0) {
        BK_TODO_FAIL(); //TODO
    }
}

//------------------------------------------------------------------------------
void
renderer_impl::present() {
    auto const r = renderer_.get();

    SDL_RenderPresent(r);
}

//------------------------------------------------------------------------------
texture
renderer_impl::create_texture_(SDL_Surface* const surface) {
    BK_ASSERT_DBG(surface);

    //SDL_SetColorKey(surface, SDL_TRUE, 0);

    auto const result = SDL_CreateTextureFromSurface(renderer_.get(), surface);
    if (result == nullptr) {
        BK_TODO_FAIL();
        //BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateTextureFromSurface"));
    }

    auto const handle = texture::handle_t {result};
    auto const id     = static_cast<unsigned>(textures_.size());

    textures_.emplace_back(
        sdl_unique<SDL_Texture> {result}
    );

    auto const w = static_cast<tex_coord_i>(surface->w);
    auto const h = static_cast<tex_coord_i>(surface->h);

    return texture {handle, id, w, h};
}

texture
renderer_impl::create_texture(path_string_ref const filename) {
    //TODO make this work for not just bmp.
    
    //TODO add a second version for bin data
    auto const buffer = bkrl::read_file(filename);
    auto const result = SDL_LoadBMP_RW(
        SDL_RWFromConstMem(buffer.data(), buffer.size())
      , 1
    );

    if (result == nullptr) {
        BK_TODO_FAIL();
        //BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_LoadBMP"));
    }

    sdl_unique<SDL_Surface> surface {result};

    return create_texture_(surface.get());
}

texture
renderer_impl::create_texture(
    tex_coord_i const width
  , tex_coord_i const height
) {
    auto const result = SDL_CreateTexture(
        renderer_.get()
      , SDL_PIXELFORMAT_ARGB8888
      , SDL_TEXTUREACCESS_STATIC
      , width
      , height
    );

    if (result == nullptr) {
        BK_TODO_FAIL();
    }

    SDL_SetTextureBlendMode(result, SDL_BLENDMODE_BLEND);

    auto const handle = texture::handle_t {result};
    auto const id     = static_cast<unsigned>(textures_.size());

    textures_.emplace_back(
        sdl_unique<SDL_Texture> {result}
    );

    return texture {handle, id, width, height};
}

//------------------------------------------------------------------------------
texture
renderer_impl::create_texture(
    void const* const buffer
  , tex_coord_i const width
  , tex_coord_i const height
) {
    //HACK TODO is SDL broken here wrt to constness?
    auto const result = SDL_CreateRGBSurfaceFrom(
        const_cast<void*>(buffer)
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
void
renderer_impl::set_translation_x(trans_t const dx) {
    translation_.x = dx;
}

//------------------------------------------------------------------------------
void
renderer_impl::set_translation_y(trans_t const dy) {
    translation_.y = dy;
}

//------------------------------------------------------------------------------
void
renderer_impl::set_scale_x(scale_t const sx) {
    set_scale(sx, scale_.y);
}

void
renderer_impl::set_scale_y(scale_t const sy) {
    set_scale(scale_.x, sy);
}

void
renderer_impl::set_scale(scale_t const sx, scale_t const sy) {
    auto const r = renderer_.get();

    scale_.x = sx;
    scale_.y = sy;

    auto const result = SDL_RenderSetScale(r, scale_.x, scale_.y);
    if (result) {
        BK_TODO_FAIL();
    }
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
renderer_impl::scale_vec
renderer_impl::get_scale() const {
    return scale_;
}

//------------------------------------------------------------------------------
renderer_impl::trans_vec
renderer_impl::get_translation() const {
    return translation_;
}

//------------------------------------------------------------------------------
void
renderer_impl::draw_texture(
    texture const& tex
  , pos_t   const  x
  , pos_t   const  y
) {
    auto const sdl_tex = tex.handle().as<SDL_Texture*>();
        
    SDL_SetTextureBlendMode(sdl_tex, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(sdl_tex, 0xFF, 0, 0); //TODO

    SDL_Rect r {x, y, 0, 0};
    SDL_QueryTexture(sdl_tex, nullptr, nullptr, &r.w, &r.h);

    SDL_RenderCopy(renderer_.get(), sdl_tex, nullptr, &r);
}

//------------------------------------------------------------------------------
void
renderer_impl::draw_texture(
    texture const& tex
  , rect_t  const  src
  , rect_t  const  dst
) {
    auto const sdl_tex = get_texture(tex);

    auto const src_rect = make_sdl_rect(src);
    auto       dst_rect = make_sdl_rect(dst);
    
    dst_rect.x += translation_.x;
    dst_rect.y += translation_.y;

    auto const result = SDL_RenderCopy(renderer_.get(), sdl_tex, &src_rect, &dst_rect);
    if (result) {
        BK_TODO_FAIL();
    }
}

void
renderer_impl::update_texture(
    texture& tex
  , void const* const data
  , int         const pitch
  , int         const x
  , int         const y
  , int         const w
  , int         const h
) {
    auto const sdl_tex = get_texture(tex);

    SDL_Rect const rect {x, y, w, h};

    auto const result = SDL_UpdateTexture(sdl_tex, &rect, data, pitch);
    if (result) {
        BK_TODO_FAIL();
    }
}

}} //namespace bkrl::detail
