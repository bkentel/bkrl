//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Application and rendering.
//##############################################################################
#pragma once

#include <memory>
#include <functional>

#include "math.hpp"
#include "util.hpp"
#include "render_types.hpp"
#include "enum_forward.hpp"
#include "keyboard.hpp"

namespace bkrl {

class application;
class renderer;
class tile_sheet;
class texture;
class config;
class keymap;

//TODO move elsewhere
enum mouse_button : unsigned {
    left, middle, right, x1
};

namespace detail { class renderer_impl; }
namespace detail { class application_impl; }

namespace detail {
//==============================================================================
//! common types for application
//==============================================================================
struct application_base {
    using handle_t = opaque_handle<application>;

    struct mouse_button_state {
        mouse_button_state(uint32_t const state = 0) noexcept
          : state {state}
        {
        }
        
        uint32_t state;

        bool is_down(unsigned const button) const noexcept {
            return (state & (1 << button)) != 0;
        }

        bool is_down_ex(unsigned const button) const noexcept {
            return ((state & (1 << button)) == state) && (state != 0);
        }

        bool is_down_any() const noexcept {
            return state != 0;
        }
    };

    struct mouse_move_info {
        int x, y;
        int dx, dy;
        mouse_button_state buttons;

        operator ipoint2() const noexcept { return {x, y}; }
        operator ivec2()   const noexcept { return {dx, dy}; }
    };

    struct mouse_button_info {
        int x, y;
        uint8_t button;
        uint8_t pressed;
        uint8_t clicks;

        operator ipoint2() const noexcept { return {x, y}; }
    };

    struct mouse_wheel_info {
        int dx, dy;
    };

    using char_sink         = std::function<void (char)>;
    using command_sink      = std::function<bool (command_type)>;
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
    application(keymap const& map, config const& cfg);
    ~application();

    handle_t handle() const;

    bool is_running() const;
    bool has_events() const;

    int client_width() const;
    int client_height() const;

    void do_one_event();
    void do_all_events();

    void sleep(int ms) const;

    key_modifier get_kb_mods() const;

    void on_char(char_sink sink);
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

    using trans_t = int;
    using scale_t = float;
    using size_t  = int;
    using pos_t   = int;

    using rect_t  = axis_aligned_rect<pos_t>;

    using trans_vec = vector2d<trans_t>;
    using scale_vec = vector2d<scale_t>;

    using color3b = color3<uint8_t>;
    using color4b = color4<uint8_t>;
};

} //namesapce detail

//==============================================================================
class texture {
public:
    using handle_t = opaque_handle<texture>;

    texture()
      : texture {handle_t {nullptr}, 0, 0, 0}
    {
    }

    texture(handle_t handle, unsigned id, tex_coord_i width, tex_coord_i height)
      : handle_ {handle}
      , id_     {id}
      , width_  {width}
      , height_ {height}
    {
        BK_ASSERT_DBG(width  >= 0);
        BK_ASSERT_DBG(height >= 0);
    }

    handle_t handle() const { return handle_; }

    unsigned id() const { return id_; }

    tex_coord_i width()  const { return width_; }
    tex_coord_i height() const { return height_; }
private:
    handle_t handle_;
    unsigned id_;

    tex_coord_i width_;
    tex_coord_i height_;
};

//==============================================================================
//! renderer
//==============================================================================
class renderer : public detail::renderer_base {
public:
    struct restore_view_t {
        BK_NOCOPY(restore_view_t);

        restore_view_t(renderer& r, scale_vec const scale, trans_vec const trans)
          : r_ {&r}
          , old_scale_(r.get_scale())
          , old_trans_(r.get_translation())
        {
            r.set_scale(scale);
            r.set_translation(trans);
        }

        restore_view_t(restore_view_t&& other) noexcept
          : r_ {other.r_}
          , old_scale_(other.old_scale_)
          , old_trans_(other.old_trans_)
        {
            other.r_ = nullptr;
        }

        restore_view_t& operator=(restore_view_t&& rhs) noexcept {
            if (this == &rhs) {
                return *this;
            }

            using std::swap;

            swap(r_, rhs.r_);
            swap(old_scale_, rhs.old_scale_);
            swap(old_trans_, rhs.old_trans_);

            rhs.r_ = nullptr;

            return *this;
        }

        ~restore_view_t() {
            if (!r_) {
                return;
            }

            r_->set_scale(old_scale_);
            r_->set_translation(old_trans_);
        }

        renderer* r_;
        scale_vec old_scale_;
        trans_vec old_trans_;
    };

    restore_view_t restore_view() {
        return restore_view_t {*this, scale_vec {1.0f, 1.0f}, trans_vec {0, 0}};
    }

    renderer(application const& app);
    ~renderer();

    handle_t handle() const;
    ////////////////////////////////////////////////////////////////////////////

    //--------------------------------------------------------------------------
    void set_translation_x(trans_t dx);
    void set_translation_y(trans_t dy);

    void set_translation(trans_t const dx, trans_t const dy) {
        set_translation_x(dx);
        set_translation_y(dy);
    }

    void set_translation(trans_vec const trans) {
        set_translation(trans.x, trans.y);
    }

    //--------------------------------------------------------------------------
    void set_scale_x(scale_t sx);
    void set_scale_y(scale_t sy);

    void set_scale(scale_t sx, scale_t sy);

    void set_scale(scale_t const scale) {
        set_scale(scale, scale);
    }

    void set_scale(scale_vec const scale) {
        set_scale(scale.x, scale.y);
    }
    
    //--------------------------------------------------------------------------
    scale_vec get_scale() const;
    trans_vec get_translation() const;

    ////////////////////////////////////////////////////////////////////////////
    void clear();
    void present();

    ////////////////////////////////////////////////////////////////////////////

    texture create_texture(path_string_ref filename);
    
    texture create_texture(void const* buffer, tex_coord_i width, tex_coord_i height);
    
    texture create_texture(tex_coord_i width, tex_coord_i height);
    
    texture create_texture(tex_point_i const size) {
        return create_texture(size.x, size.y);
    }

    void delete_texture(texture& tex);

    void update_texture(texture& tex, void const* data, int pitch, int x, int y, int w, int h);
    
    void update_texture(texture& tex, void const* const data, int const pitch, rect_t const rect) {
        update_texture(tex, data, pitch, rect.left, rect.top, rect.width(), rect.height());
    }

    argb8 get_color_mod(texture const& tex) const;

    void set_color_mod(texture& tex);
    void set_color_mod(texture& tex, argb8 color);
    void set_color_mod(texture& tex, rgb8  color);
    
    void set_alpha_mod(texture& tex, uint8_t a = 255);

    void draw_texture(texture const& tex, pos_t x, pos_t y);
    void draw_texture(texture const& tex, rect_t src, rect_t dst);
    ////////////////////////////////////////////////////////////////////////////
    void set_draw_color();
    void set_draw_color(argb8 color);
    void set_draw_color(rgb8 color);
    
    void set_draw_alpha(uint8_t a = 255);

    void draw_filled_rect(rect_t bounds);

    ////////////////////////////////////////////////////////////////////////////

private:
    std::unique_ptr<detail::renderer_impl> impl_;
};


} //namespace bkrl
