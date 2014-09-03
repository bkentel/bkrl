#pragma once

#include <memory>     //std::unique_ptr
#include <functional> //std::function

#include "types.hpp"

namespace bkrl {

namespace random { class generator; }
namespace detail { class bsp_layout_impl; }

namespace detail {
struct bsp_layout_base {
    using room_id = uint16_t;
    using room_callback    = std::function<void (grid_region bounds, room_id id)>;
    using split_callback   = std::function<bool (grid_region bounds)>;
    using connect_callback = std::function<bool (grid_region bounds, room_id id0, room_id id1)>;

    struct params_t {
        unsigned width  = 50;
        unsigned height = 50;

        unsigned min_region_w = 4;
        unsigned min_region_h = 4;
        unsigned max_region_w = 20;
        unsigned max_region_h = 20;

        //! The percent chance that a given region will be split if
        //! its width < max_region_w and its height < max_region_h.
        unsigned split_chance = 20;

        //! The percent chance that a room will be generated in a given region.
        unsigned room_gen_chance = 200;

        //! The maximum aspect ratio for generated regions.
        float max_aspect_ratio = 16.0f / 10.0f;
    };
};

} //namespace detail

//==============================================================================
//! Map layout generatator based on binary space partitioning.
//! @note pimpl based; moveable, but not copyable.
//==============================================================================
class bsp_layout : public detail::bsp_layout_base {
public:
    static bsp_layout generate(
        random::generator&    gen
      , split_callback const& on_split
      , room_callback  const& on_room_gen
      , params_t       const& params      = params_t {}
      , grid_region    const& reserve     = grid_region {}
    );

    bsp_layout();
    bsp_layout(bsp_layout const&) = delete;
    bsp_layout& operator=(bsp_layout const&) = delete;
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

} //namespace bkrl
