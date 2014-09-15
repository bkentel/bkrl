//#include "sdl.hpp"
//
//template <typename T>
//struct scope_exit_t {
//    scope_exit_t(T&& f) : f_{std::move(f)} {}
//    ~scope_exit_t() { f_(); }
//private:
//    std::remove_reference_t<T> f_;
//};
//
//template <typename T>
//scope_exit_t<T> scope_exit(T&& f) {
//    return scope_exit_t<T> {std::forward<T>(f)};
//}
//
//#define BK_CONCAT_(x, y) x ## y
//#define BK_CONCAT(x, y) BK_CONCAT_(x, y)
//
//#define BK_SCOPE_EXIT(F) auto BK_CONCAT(bk_scope_exit_, __LINE__) {scope_exit(F)}
//
//using bkrl::sdl_renderer;
//using bkrl::sdl_application;
//using bkrl::sdl_unique;
//using bkrl::ft_unique;
//namespace error = bkrl::error;
//
//////////////////////////////////////////////////////////////////////////////////
//// sdl_renderer
//////////////////////////////////////////////////////////////////////////////////
//namespace {
//
//static sdl_unique<SDL_Renderer> create_renderer(SDL_Window* window) {
//    auto const result = SDL_CreateRenderer(
//        window
//      , -1
//      , SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
//    );
//
//    if (result == nullptr) {
//        BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateRenderer"));
//    }
//
//    return sdl_unique<SDL_Renderer> {result};
//}
//
//static sdl_unique<SDL_Texture> create_texture(SDL_Renderer* renderer) {
//    auto const result_bmp = SDL_LoadBMP("./data/Myne.bmp");
//    if (result_bmp == nullptr) {
//        BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_LoadBMP"));
//    }
//
//    sdl_unique<SDL_Surface> surface {result_bmp};
//
//    auto const result_texture = SDL_CreateTextureFromSurface(renderer, surface.get());
//    if (result_bmp == nullptr) {
//        BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateTextureFromSurface"));
//    }
//
//    return sdl_unique<SDL_Texture> {result_texture};
//}
//
//static sdl_unique<SDL_Window> create_window() {
//    auto const result = SDL_CreateWindow(
//        "BKRL"
//      , SDL_WINDOWPOS_UNDEFINED
//      , SDL_WINDOWPOS_UNDEFINED
//      , 1024
//      , 768
//      , SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
//    );
//
//    if (result == nullptr) {
//        BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateWindow"));
//    }
//
//    return sdl_unique<SDL_Window> {result};
//}
//
//static sdl_unique<SDL_PixelFormat> create_pixel_format() {
//    auto const result = ::SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
//    if (result == nullptr) {
//        BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_AllocFormat"));
//    }
//
//    return sdl_unique<SDL_PixelFormat> {result};
//}
//
//} //namespace
//
//
////////////////////
//
//
//
///////
//
//sdl_renderer::sdl_renderer(SDL_Window* const window)
//    : renderer_ {create_renderer(window)}
//    , texture_  {create_texture(renderer_.get())}
//    , text_renderer_ {}
//    , pixel_format_ {create_pixel_format()}
//    , scale_x_ {1.0f}
//    , scale_y_ {1.0f}
//    , trans_x_ {0.0f}
//    , trans_y_ {0.0f}
//{
//    //TODO
//    auto const size = text_renderer_.required_tex_size();
//
//    auto texture = ::SDL_CreateTexture(
//        renderer_.get()
//      , SDL_PIXELFORMAT_RGBA8888
//      , SDL_TEXTUREACCESS_STREAMING
//      , size.w
//      , size.h
//    );
//
//    if (texture == nullptr) {
//        BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateTexture"));
//    }
//
//    glyph_texture_.reset(texture);
//
//    auto const result = ::SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
//    if (result) {
//        BK_TODO_FAIL();
//    }
//
//    text_renderer_.for_each_static_codepoint([&](unsigned const codepoint) {
//        auto info = text_renderer_.get_glyph(codepoint);
//        auto glyph = info.glyph;
//
//        if (glyph == nullptr) {
//            return;
//        }
//
//        BK_ASSERT_DBG(glyph->format == FT_GLYPH_FORMAT_BITMAP);
//
//        auto const bmp = reinterpret_cast<FT_BitmapGlyph>(glyph);
//        auto const r   = text_renderer_.get_glyph_rect(codepoint);
//        auto const dst = SDL_Rect {r.left, r.top, r.width(), r.height()};
//
//        write_glyph_(texture, dst, bmp->bitmap);
//    });
//}
//
//void sdl_renderer::clear() {
//    auto const result = ::SDL_RenderClear(handle());
//    if (result != 0) {
//        BK_ASSERT(false); //TODO
//    }
//
//    ::SDL_RenderSetScale(handle(), 1.0f, 1.0f);
//}
//
//SDL_Texture* sdl_renderer::get_glyph_texture_(int w, int h) {
//    if (!glyph_texture_ || glyph_texture_w_ < w || glyph_texture_h_ < h) {
//        if (glyph_texture_w_ < w) { glyph_texture_w_ = w; }
//        if (glyph_texture_h_ < h) { glyph_texture_h_ = h; }
//
//        auto texture = ::SDL_CreateTexture(
//            renderer_.get()
//		  , SDL_PIXELFORMAT_RGBA8888
//		  , SDL_TEXTUREACCESS_STREAMING
//		  , glyph_texture_w_
//		  , glyph_texture_h_
//        );
//
//        if (texture == nullptr) {
//            BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateTexture"));
//        }
//
//        glyph_texture_.reset(texture);
//
//        ::SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
//    }
//
//    return glyph_texture_.get();
//}
//
//void sdl_renderer::write_glyph_(SDL_Texture* texture, SDL_Rect const& dst, FT_Bitmap const& bitmap) {
//    auto const format  = pixel_format_.get();
//
//    uint32_t*  out         = nullptr;
//    int        pitch       = 0;
//    auto const lock_result = ::SDL_LockTexture(texture, &dst, reinterpret_cast<void**>(&out), &pitch);
//    if (lock_result) {
//        BK_TODO_FAIL();
//    }
//
//    BK_SCOPE_EXIT([&texture] {
//        ::SDL_UnlockTexture(texture);
//    });
//
//    for (int y = 0; y < bitmap.rows; ++y) {
//        for (int x = 0; x < bitmap.width; ++x) {
//            auto const src_i = y * bitmap.pitch + x;
//            auto const dst_i = y * (pitch / 4) + x;
//
//            auto const a     = bitmap.buffer[src_i];
//            auto const value = SDL_MapRGBA(format, 200, 100, 100, a);
//
//            out[dst_i] = value;
//        }
//    }
//}
//
//void sdl_renderer::draw_text(bkrl::string_ref const string, text_rect const rect) {
//    auto const renderer = renderer_.get();
//
//    axis_aligned_rect<int> const bounds {
//        static_cast<int>(rect.left   + trans_x_)
//      , static_cast<int>(rect.top    + trans_y_)
//      , static_cast<int>(rect.right  + trans_x_)
//      , static_cast<int>(rect.bottom + trans_y_)
//    };
//
//    transitory_text_layout layout {text_renderer_, string, bounds};
//
//    while (!!layout) {
//        auto const result    = layout.next(text_renderer_);
//        auto const codepoint = result.codepoint;
//        auto const pos       = result.pos;
//        auto const w         = result.width;
//        auto const h         = result.height;
//
//        auto const info       = text_renderer_.get_glyph(codepoint);
//        auto const glyph      = info.glyph;
//        auto const glyph_rect = text_renderer_.get_glyph_rect(codepoint);
//
//        auto const src_rect = SDL_Rect {glyph_rect.left, glyph_rect.top, glyph_rect.width(), glyph_rect.height()};
//        auto const dst_rect = SDL_Rect {pos.x, pos.y, w, h};
//
//        SDL_RenderCopy(renderer, glyph_texture_.get(), &src_rect, &dst_rect);
//    }
//}
//
//void sdl_renderer::draw_tile(
//    bkrl::sprite_sheet& sheet
//  , unsigned index
//  , unsigned x
//  , unsigned y
//) {
//    auto const src_x = static_cast<int>(sheet.get_x(index));
//    auto const src_y = static_cast<int>(sheet.get_y(index));
//
//    auto const w = static_cast<int>(sheet.sprite_width);
//    auto const h = static_cast<int>(sheet.sprite_height);
//
//    auto const dst_x = static_cast<int>(trans_x_) + static_cast<int>(x * w);
//    auto const dst_y = static_cast<int>(trans_y_) + static_cast<int>(y * h);
//
//    SDL_Rect const src {src_x, src_y, w, h};
//    SDL_Rect const dst {dst_x, dst_y, w, h};
//
//    auto const result = SDL_RenderCopy(handle(), texture_.get(), &src, &dst);
//    if (result != 0) {
//        BK_ASSERT(false); //TODO
//    }
//}
//
//////////////////////////////////////////////////////////////////////////////////
//// sdl_application
//////////////////////////////////////////////////////////////////////////////////
//sdl_application::sdl_application()
//  : state_ {}
//  , window_ {create_window()}
//  , event_ {}
//  , running_ {false}
//{
//    running_ = true;
//}
//
//void sdl_application::pump_events() {
//    while (SDL_PollEvent(&event_)) {
//        switch (event_.type) {
//        default: break;
//        case SDL_QUIT :
//            running_ = false;
//            break;
//        case SDL_WINDOWEVENT :
//            handle_window_event_(event_.window);
//            break;
//        case SDL_SYSWMEVENT  : break;
//        case SDL_KEYDOWN :
//            handle_keyboard_event_(event_.key);
//            break;
//        case SDL_KEYUP :
//            break;
//        case SDL_TEXTEDITING : break;
//        case SDL_TEXTINPUT : break;
//        case SDL_MOUSEMOTION :
//            handle_mousemotion_event_(event_.motion);
//            break;
//        case SDL_MOUSEBUTTONDOWN :
//        case SDL_MOUSEBUTTONUP :
//            handle_mouse_button_event_(event_.button);
//            break;
//        case SDL_MOUSEWHEEL :
//            if (event_.wheel.y > 0) {
//                command_sink_(command_type::zoom_in);
//            } else if (event_.wheel.y < 0) {
//                command_sink_(command_type::zoom_out);
//            }
//
//            break;
//        }
//    }
//}
//
//void sdl_application::handle_mouse_button_event_(SDL_MouseButtonEvent const& event) {
//    if (!mouse_button_sink_) {
//        return;
//    }
//
//    mouse_button_sink_(event.x, event.y);
//}
//
//void sdl_application::handle_mousemotion_event_(SDL_MouseMotionEvent const& event) {
//    if (!mouse_move_sink_) {
//        return;
//    }
//
//    std::bitset<8> const buttons {event.state};
//    auto const dx = event.xrel;
//    auto const dy = event.yrel;
//
//    mouse_move_sink_(dx, dy, buttons);
//}
//
//void sdl_application::handle_window_event_(SDL_WindowEvent const& event) {
//    switch (event.event) {
//    case SDL_WINDOWEVENT_NONE :
//        std::cout << "SDL_WINDOWEVENT_NONE" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_SHOWN :
//        std::cout << "SDL_WINDOWEVENT_SHOWN" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_HIDDEN :
//        std::cout << "SDL_WINDOWEVENT_HIDDEN" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_EXPOSED :
//        std::cout << "SDL_WINDOWEVENT_EXPOSED" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_MOVED :
//        std::cout << "SDL_WINDOWEVENT_MOVED" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_RESIZED :
//        std::cout << "SDL_WINDOWEVENT_RESIZED" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_SIZE_CHANGED :
//        if (resize_sink_) {
//            resize_sink_(event.data1, event.data2);
//        }
//        std::cout << "SDL_WINDOWEVENT_SIZE_CHANGED" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_MINIMIZED :
//        std::cout << "SDL_WINDOWEVENT_MINIMIZED" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_MAXIMIZED :
//        std::cout << "SDL_WINDOWEVENT_MAXIMIZED" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_RESTORED :
//        std::cout << "SDL_WINDOWEVENT_RESTORED" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_ENTER :
//        std::cout << "SDL_WINDOWEVENT_ENTER" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_LEAVE :
//        std::cout << "SDL_WINDOWEVENT_LEAVE" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_FOCUS_GAINED :
//        std::cout << "SDL_WINDOWEVENT_FOCUS_GAINED" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_FOCUS_LOST :
//        std::cout << "SDL_WINDOWEVENT_FOCUS_LOST" << std::endl;
//        break;
//    case SDL_WINDOWEVENT_CLOSE :
//        std::cout << "SDL_WINDOWEVENT_CLOSE" << std::endl;
//        break;
//    }
//}
//
////TODO
////struct key_binding {
////    unsigned           keycode;
////    std::bitset<8>     modifiers;
////    bkrl::command_type command;
////
////    bool operator<(key_binding const& rhs) const {
////        return keycode < rhs.keycode && modifiers.to_ulong() < rhs.modifiers.to_ulong();
////    }
////};
//
//void sdl_application::handle_keyboard_event_(SDL_KeyboardEvent const& event) {
//    if (!command_sink_) {
//        return; //TODO
//    }
//
//    auto vkey = event.keysym.sym;
//
//    switch (event.keysym.scancode) {
//    case SDL_SCANCODE_KP_0 : vkey = SDLK_KP_0; break;
//    case SDL_SCANCODE_KP_1 : vkey = SDLK_KP_1; break;
//    case SDL_SCANCODE_KP_2 : vkey = SDLK_KP_2; break;
//    case SDL_SCANCODE_KP_3 : vkey = SDLK_KP_3; break;
//    case SDL_SCANCODE_KP_4 : vkey = SDLK_KP_4; break;
//    case SDL_SCANCODE_KP_5 : vkey = SDLK_KP_5; break;
//    case SDL_SCANCODE_KP_6 : vkey = SDLK_KP_6; break;
//    case SDL_SCANCODE_KP_7 : vkey = SDLK_KP_7; break;
//    case SDL_SCANCODE_KP_8 : vkey = SDLK_KP_8; break;
//    case SDL_SCANCODE_KP_9 : vkey = SDLK_KP_9; break;
//    }
//
//    auto const no_mods = [&event] {
//        return (event.keysym.mod & (
//            KMOD_LSHIFT | KMOD_RSHIFT
//          | KMOD_LCTRL  | KMOD_RCTRL
//          | KMOD_LALT   | KMOD_RALT
//        )) == 0;
//    };
//
//    switch (vkey) {
//    case SDLK_w :
//        if (no_mods()) { command_sink_(command_type::scroll_n); }
//        break;
//    case SDLK_a :
//        if (no_mods()) { command_sink_(command_type::scroll_w); }
//        break;
//    case SDLK_s :
//        if (no_mods()) { command_sink_(command_type::scroll_s); }
//        break;
//    case SDLK_d :
//        if (no_mods()) { command_sink_(command_type::scroll_e); }
//        break;
//    case SDLK_o :
//        if (no_mods()) { command_sink_(command_type::open); }
//        break;
//    case SDLK_c :
//        if (no_mods()) { command_sink_(command_type::close); }
//        break;
//    case SDLK_KP_7 :
//        if (no_mods()) { command_sink_(command_type::north_west); }
//        break;
//    case SDLK_KP_9 :
//        if (no_mods()) { command_sink_(command_type::north_east); }
//        break;
//    case SDLK_KP_1 :
//        if (no_mods()) { command_sink_(command_type::south_west); }
//        break;
//    case SDLK_KP_3 :
//        if (no_mods()) { command_sink_(command_type::south_east); }
//        break;
//    case SDLK_KP_8 :
//    case SDLK_UP :
//        if (no_mods()) { command_sink_(command_type::north); }
//        break;
//    case SDLK_KP_2 :
//    case SDLK_DOWN :
//        if (no_mods()) { command_sink_(command_type::south); }
//        break;
//    case SDLK_KP_4 :
//    case SDLK_LEFT :
//        if (no_mods()) { command_sink_(command_type::west); }
//        break;
//    case SDLK_KP_6 :
//    case SDLK_RIGHT :
//        if (no_mods()) { command_sink_(command_type::east); }
//        break;
//    case SDLK_KP_PLUS :
//    case SDLK_PLUS  : command_sink_(command_type::zoom_in); break;
//    case SDLK_KP_MINUS :
//    case SDLK_MINUS : command_sink_(command_type::zoom_out); break;
//    }
//}
