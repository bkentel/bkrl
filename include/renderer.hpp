//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Application and rendering.
//##############################################################################
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
class texture;
class config;

namespace detail { class renderer_impl; }
namespace detail { class application_impl; }

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
    application(string_ref keymap, config const& cfg);
    ~application();

    handle_t handle() const;

    bool is_running() const;
    bool has_events() const;

    int client_width() const;
    int client_height() const;

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

    texture()
      : texture {handle_t {nullptr}, 0, 0, 0}
    {
    }

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
    texture create_texture(uint8_t* buffer, int width, int height);
    texture create_texture(int width, int height);

    void delete_texture(texture& tex);

    void update_texture(texture& tex, void* data, int pitch, int x, int y, int w, int h);

    ////////////////////////////////////////////////////////////////////////////

    void set_translation_x(scalar dx);
    void set_translation_y(scalar dy);

    void set_scale_x(scalar sx);
    void set_scale_y(scalar sy);

    void set_translation(scalar const dx, scalar const dy) {
        set_translation_x(dx);
        set_translation_y(dy);
    }

    void set_translation(vec2 const trans) {
        set_translation(trans.x, trans.y);
    }

    void set_scale(scalar const sx, scalar const sy) {
        set_scale_x(sx);
        set_scale_y(sy);
    }

    void set_scale(scalar const scale) {
        set_scale(scale, scale);
    }

    void set_scale(vec2 const scale) {
        set_scale(scale.x, scale.y);
    }
    
    vec2 get_scale() const;
    vec2 get_translation() const;

    ////////////////////////////////////////////////////////////////////////////

    void draw_texture(texture const& tex, scalar x, scalar y);
    void draw_texture(texture const& tex, rect src, rect dst);
private:
    std::unique_ptr<detail::renderer_impl> impl_;
};


} //namespace bkrl
