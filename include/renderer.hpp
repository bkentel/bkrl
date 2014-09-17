#pragma once

#include <memory>
#include <functional>

#include "types.hpp"
#include "math.hpp"
#include "util.hpp"

namespace bkrl {

class application;
class renderer;
class tile_sheet;
class font_manager;
class texture;

namespace detail { class renderer_impl; }
namespace detail { class application_impl; }
namespace detail { class font_manager_impl; }

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
    explicit application(string_ref keymap);
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

//==============================================================================
class texture {
public:
    using handle_t = opaque_handle<texture>;
    using rect = detail::renderer_base::rect;

    texture(handle_t handle, unsigned id, int width, int height)
      : handle_ {handle}
      , id_ {id}
      , width_ {width}
      , height_ {height}
    {
    }

    handle_t handle() const { return handle_; }

    unsigned id() const { return id_; }

    int width() const { return width_; }
    int height() const { return height_; }

    void update(rect region, void const* pixel_data, int pitch);
private:
    handle_t handle_;
    unsigned id_;

    int width_;
    int height_;
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

    texture create_texture(string_ref filename);
    void delete_texture(texture& tex);

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
