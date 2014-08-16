#include "sdl.hpp"

using bkrl::sdl_renderer;
using bkrl::sdl_application;
using bkrl::sdl_unique;
namespace error = bkrl::error;

////////////////////////////////////////////////////////////////////////////////
// sdl_renderer
////////////////////////////////////////////////////////////////////////////////
namespace {

static sdl_unique<SDL_Renderer> create_renderer(SDL_Window* window) {
    auto const result = SDL_CreateRenderer(
        window
        , -1
        , SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (result == nullptr) {
        BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateRenderer"));
    }

    return sdl_unique<SDL_Renderer> {result};
}

static sdl_unique<SDL_Texture> create_texture(SDL_Renderer* renderer) {
    auto const result_bmp = SDL_LoadBMP("./data/Myne.bmp");
    if (result_bmp == nullptr) {
        BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_LoadBMP"));
    }

    sdl_unique<SDL_Surface> surface {result_bmp};

    auto const result_texture = SDL_CreateTextureFromSurface(renderer, surface.get());
    if (result_bmp == nullptr) {
        BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateTextureFromSurface"));
    }

    return sdl_unique<SDL_Texture> {result_texture};
}

static sdl_unique<SDL_Window> create_window() {
    auto const result = SDL_CreateWindow(
        "BKRL"
      , SDL_WINDOWPOS_UNDEFINED
      , SDL_WINDOWPOS_UNDEFINED
      , 1024
      , 768
      , SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );

    if (result == nullptr) {
        BOOST_THROW_EXCEPTION(error::make_sdl_error("SDL_CreateWindow"));
    }

    return sdl_unique<SDL_Window> {result};
}

} //namespace

sdl_renderer::sdl_renderer(SDL_Window* const window)
    : renderer_ {create_renderer(window)}
    , texture_  {create_texture(renderer_.get())}
    , scale_x_ {1.0f}
    , scale_y_ {1.0f}
    , trans_x_ {0.0f}
    , trans_y_ {0.0f}
{
}

void sdl_renderer::draw_tile(
    bkrl::sprite_sheet& sheet
  , unsigned index
  , unsigned x
  , unsigned y
) {
    auto const src_x = static_cast<int>(sheet.get_x(index));
    auto const src_y = static_cast<int>(sheet.get_y(index));

    auto const w = static_cast<int>(sheet.sprite_width);
    auto const h = static_cast<int>(sheet.sprite_height);

    auto const dst_x = static_cast<int>(trans_x_) + static_cast<int>(x * w);
    auto const dst_y = static_cast<int>(trans_y_) + static_cast<int>(y * h);

    SDL_Rect const src {src_x, src_y, w, h};
    SDL_Rect const dst {dst_x, dst_y, w, h};

    auto const result = SDL_RenderCopy(handle(), texture_.get(), &src, &dst);
    if (result != 0) {
        BK_ASSERT(false); //TODO
    }
}

////////////////////////////////////////////////////////////////////////////////
// sdl_application
////////////////////////////////////////////////////////////////////////////////
sdl_application::sdl_application()
  : state_ {}
  , window_ {create_window()}
  , event_ {}
  , running_ {false}
{
    running_ = true;
}

void sdl_application::pump_events() {
    while (SDL_PollEvent(&event_)) {
        switch (event_.type) {
        default: break;
        case SDL_QUIT :
            running_ = false;
            break;
        case SDL_WINDOWEVENT : break;
        case SDL_SYSWMEVENT  : break;
        case SDL_KEYDOWN :
            handle_keyboard_event_(event_.key);
            break;
        case SDL_KEYUP :
            //handle_keyboard_event_(event_.key);
            break;
        case SDL_TEXTEDITING : break;
        case SDL_TEXTINPUT : break;
        case SDL_MOUSEMOTION : break;
        case SDL_MOUSEBUTTONDOWN : break;
        case SDL_MOUSEBUTTONUP : break;
        case SDL_MOUSEWHEEL :
            if (event_.wheel.y > 0) {
                command_sink_(command::zoom_in);
            } else if (event_.wheel.y < 0) {
                command_sink_(command::zoom_out);
            }

            break;
        }
    }
}

void sdl_application::handle_keyboard_event_(SDL_KeyboardEvent const& event) {
    if (!command_sink_) {
        return; //TODO
    }

    switch (event.keysym.sym) {
    case SDLK_w :
        if (event.keysym.mod == 0) { command_sink_(command::scroll_n); }
        break;
    case SDLK_a :
        if (event.keysym.mod == 0) { command_sink_(command::scroll_w); }
        break;
    case SDLK_s :
        if (event.keysym.mod == 0) { command_sink_(command::scroll_s); }
        break;
    case SDLK_d :
        if (event.keysym.mod == 0) { command_sink_(command::scroll_e); }
        break;
    case SDLK_UP :
        if (event.keysym.mod == 0) { command_sink_(command::north); }
        break;
    case SDLK_DOWN :
        if (event.keysym.mod == 0) { command_sink_(command::south); }
        break;
    case SDLK_LEFT :
        if (event.keysym.mod == 0) { command_sink_(command::west); }
        break;
    case SDLK_RIGHT :
        if (event.keysym.mod == 0) { command_sink_(command::east); }
        break;
    case SDLK_KP_PLUS :
    case SDLK_PLUS  : command_sink_(command::zoom_in); break;
    case SDLK_KP_MINUS :
    case SDLK_MINUS : command_sink_(command::zoom_out); break;
    }
}
