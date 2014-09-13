#pragma once

#include <memory>
#include "types.hpp"
#include "math.hpp"

namespace bkrl {

namespace detail { class renderer_impl; }
namespace detail { class application_impl; }
namespace detail { class font_manager_impl; }

class application;
class renderer;
class tile_sheet;
class font_manager;

enum class command_type : uint16_t;

template <typename T>
struct opaque_handle {
    std::intptr_t value;
};

namespace detail {
//==============================================================================
//! common types for application
//==============================================================================
struct application_base {
    using handle_t = opaque_handle<application>;

    struct mouse_move_info {
        int x, y;
        int dx, dy;
        uint32_t state;
    };

    struct mouse_button_info {
        enum class state_t : uint8_t {
            released, pressed
        };

        int x, y;
        uint8_t button;
        state_t state;
        uint8_t clicks;
    };

    struct mouse_wheel_info {
        int dx, dy;
    };

    using command_sink      = std::function<void (command_type)>;
    using close_sink        = std::function<void ()>;
    using resize_sink       = std::function<void (unsigned w, unsigned h)>;
    using mouse_move_sink   = std::function<void (mouse_move_info const&)>;
    using mouse_button_sink = std::function<void (mouse_button_info const&)>;
    using mouse_wheel_sink  = std::function<void (mouse_wheel_info const&)>;
};

} //namespace detail

//==============================================================================
//! system window
//==============================================================================
class application : public detail::application_base {
public:
    application();
    ~application();

    handle_t handle() const; 

    bool is_running() const;
    bool has_events() const;

    void do_one_event();
    void do_all_events();

    void on_command(command_sink sink);
    void on_close(close_sink sink);
    void on_resize(resize_sink sink);
    void on_mouse_move(mouse_move_sink sink);
    void on_mouse_button(mouse_button_sink sink);
    void on_mouse_wheel(mouse_wheel_sink sink);
private:
    std::unique_ptr<detail::application_impl> impl_;
};

namespace detail {
//==============================================================================
//! common types for renderer
//==============================================================================
struct renderer_base {
    using handle_t = opaque_handle<renderer>;
    using scalar = float;
    using rect   = axis_aligned_rect<scalar>;
};

} //namesapce detail

//TODO
class tile_sheet {
public:
    using rect_t = axis_aligned_rect<int>;

    tile_sheet(int width, int height, int tile_width, int tile_height)
      : width {width}
      , height {height}
      , tile_width {tile_width}
      , tile_height{tile_height}
      , tile_x {width / tile_width}
      , tile_y {height / tile_height}
    {
    }

    rect_t at(int x, int y) const {
        BK_ASSERT_SAFE(x >= 0 && x < tile_x);
        BK_ASSERT_SAFE(y >= 0 && y < tile_y);

        auto const l = x * tile_width;
        auto const t = y * tile_height;
        auto const r = l + tile_width;
        auto const b = t + tile_height;

        return rect_t {l, t, r, b};
    }
     
    int width; //texture w
    int height; //texture h

    int tile_width;  //dimensions
    int tile_height; //dimensions

    int tile_x; //tiles per row
    int tile_y; //rows
};

//==============================================================================
//! renderer
//==============================================================================
class renderer : public detail::renderer_base {
public:
    renderer(application const& app);
    ~renderer();

    handle_t handle() const;
    ////////////////////////////////////////////////////////////////////////////

    void clear();
    void present();

    ////////////////////////////////////////////////////////////////////////////

    void set_translation_x(scalar dx);
    void set_translation_y(scalar dy);

    void set_scale_x(scalar sx);
    void set_scale_y(scalar sy);

    void set_translation(scalar const dx, scalar const dy) {
        set_translation_x(dx);
        set_translation_y(dy);
    }

    void set_scale(scalar const sx, scalar const sy) {
        set_scale_x(sx);
        set_scale_y(sy);
    }

    void set_scale(scalar const scale) {
        set_scale(scale, scale);
    }
    
    ////////////////////////////////////////////////////////////////////////////

    void draw_tile(
        tile_sheet const& sheet
      , unsigned ix
      , unsigned iy
      , scalar   x
      , scalar   y
    );

    void draw_text(string_ref string, scalar x, scalar y);
    void draw_text(string_ref string, rect bounds);
private:
    std::unique_ptr<detail::renderer_impl> impl_;
};


} //namespace bkrl
