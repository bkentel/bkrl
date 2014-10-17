//##############################################################################
//! @file
//! @author Brandon Kentel
//!
//! Public header for bsp_layout.
//##############################################################################
#pragma once

#include <memory>     //std::unique_ptr
#include <functional> //std::function

#include "types.hpp"

namespace bkrl {

class grid_storage;
class room;

namespace random { class generator; }
namespace detail { class bsp_layout_impl; }
namespace detail { class bsp_connector_impl; }

namespace detail {
struct bsp_layout_base {
    using room_id = uint16_t;
    using room_callback    = std::function<void (grid_region bounds, room_id id)>;
    using split_callback   = std::function<bool (grid_region bounds)>;
    using connect_callback = std::function<bool (grid_region bounds, room_id id0, room_id id1)>;

    struct params_t {
        grid_size width  = 50;
        grid_size height = 50;

        grid_size min_region_w = 4;
        grid_size min_region_h = 4;
        grid_size max_region_w = 20;
        grid_size max_region_h = 20;

        //! The percent chance that a given region will be split if
        //! its width < max_region_w and its height < max_region_h.
        grid_size split_chance = 80;

        //! The percent chance that a room will be generated in a given region.
        grid_size room_gen_chance = 25;

        //! The maximum aspect ratio for generated regions.
        float max_aspect_ratio = 16.0f / 10.0f;

        bool is_valid() const {
            #define BK_CHECK(x) if (!(x)) return false

            BK_CHECK(width > 0);
            BK_CHECK(width >= min_region_w);
            BK_CHECK(width >= max_region_w);
            BK_CHECK(max_region_w >= min_region_w);

            BK_CHECK(height > 0);
            BK_CHECK(height >= min_region_h);
            BK_CHECK(height >= max_region_h);
            BK_CHECK(max_region_h >= min_region_h);

            BK_CHECK(split_chance >= 0);
            BK_CHECK(split_chance <= 100);

            BK_CHECK(room_gen_chance >= 0);
            BK_CHECK(room_gen_chance <= 100);

            BK_CHECK(max_aspect_ratio >= 1.0f);

            #undef BK_CHECK

            return true;
        }
    };
};

} //namespace detail

//==============================================================================
//! Map layout generatator based on binary space partitioning.
//! @note pimpl based; moveable, but not copyable.
//==============================================================================
class bsp_layout : public detail::bsp_layout_base {
public:
    BK_NOCOPY(bsp_layout);

    static bsp_layout generate(
        random::generator&    gen
      , split_callback const& on_split
      , room_callback  const& on_room_gen
      , params_t       const& params      = params_t {}
      , grid_region    const& reserve     = grid_region {}
    );

    bsp_layout();
    bsp_layout(bsp_layout&& other);
    bsp_layout& operator=(bsp_layout&&);
    ~bsp_layout();

    //--------------------------------------------------------------------------
    //! Use @p on_connect to fully connect the generated layout.
    //--------------------------------------------------------------------------
    void connect(random::generator& gen, connect_callback on_connect);
private:
    //--------------------------------------------------------------------------
    //! Private; use bsp_layout::generate instead.
    //--------------------------------------------------------------------------
    bsp_layout(
        random::generator&    gen
      , params_t       const& params
      , split_callback const& on_split
      , room_callback  const& on_room_gen
      , grid_region    const& reserve
    );

    std::unique_ptr<detail::bsp_layout_impl> impl_;
};

//==============================================================================
// A connection algorithm for bsp_layout.
//==============================================================================
class bsp_connector {
public:
    BK_NOCOPY(bsp_connector);

    bsp_connector();
    bsp_connector(bsp_connector&&);
    bsp_connector& operator=(bsp_connector&&);
    ~bsp_connector();

    //--------------------------------------------------------------------------
    //! attempt to connect @p src_room to @p dst_room with the route constrained
    //! to the region given by @p bounds.
    //--------------------------------------------------------------------------
    bool connect(
        random::generator& gen
      , grid_storage&      grid
      , grid_region const& bounds
      , room        const& src_room
      , room        const& dst_room
    );
private:
    std::unique_ptr<detail::bsp_connector_impl> impl_;
};

} //namespace bkrl
