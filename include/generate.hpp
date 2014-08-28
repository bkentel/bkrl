//##############################################################################
//! @author Brandon Kentel
//!
//! Procedural generation.
//##############################################################################
#pragma once
#include "types.hpp"
#include "grid.hpp"
#include "random.hpp"

namespace bkrl {
namespace generate {

//==============================================================================
//==============================================================================
using region_pair = std::tuple<grid_region, grid_region>;

inline region_pair split_y(grid_region const region, grid_index const split) {
    BK_PRECONDITION(split >= region.left);
    BK_PRECONDITION(split <= region.right);

    auto const l0 = region.left;
    auto const t0 = region.top;
    auto const r0 = split;
    auto const b0 = region.bottom;

    auto const l1 = r0;
    auto const t1 = region.top;
    auto const r1 = region.right;
    auto const b1 = region.bottom;

    return std::make_tuple(
        grid_region {l0, t0, r0, b0}
      , grid_region {l1, t1, r1, b1}
    );
}

inline region_pair split_x(grid_region const region, grid_index const split) {
    BK_PRECONDITION(split >= region.top);
    BK_PRECONDITION(split <= region.bottom);

    auto const l0 = region.left;
    auto const t0 = region.top;
    auto const r0 = region.right;
    auto const b0 = split;

    auto const l1 = region.left;
    auto const t1 = b0;
    auto const r1 = region.right;
    auto const b1 = region.bottom;

    return std::make_tuple(
        grid_region {l0, t0, r0, b0}
      , grid_region {l1, t1, r1, b1}
    );
}

//==============================================================================
//! simple_room
//==============================================================================
class simple_room {
public:
    room generate(random::generator& gen, grid_region bounds);
private:
};

//==============================================================================
//! simple_room
//==============================================================================
class circle_room {
public:
    room generate(random::generator& gen, grid_region bounds);
private:
};

//==============================================================================
//! BSP based map generation.
//! PIMPL based.
//==============================================================================
class bsp_layout {
public:
    using room_callback  = std::function<void (grid_region bounds)>;
    using split_callback = std::function<bool (grid_region bounds)>;

    struct params_t {
        unsigned width  = 100;
        unsigned height = 100;

        unsigned min_region_w = 4;
        unsigned min_region_h = 4;
        unsigned max_region_w = 20;
        unsigned max_region_h = 20;

        unsigned split_chance    = 50;
        unsigned room_gen_change = 50;

        float max_aspect_ratio = 16.0f / 10.0f;
    };
public:
    static bsp_layout generate(
        random::generator&    gen
      , params_t       const& params
      , split_callback const& on_split
      , room_callback  const& on_room_gen
      , grid_region    const& reserve = grid_region {}
    );

    BK_NO_COPY(bsp_layout);

    bsp_layout(bsp_layout&& other);

    ~bsp_layout();
private:
    class impl_t;
    std::unique_ptr<impl_t> impl_;

    explicit bsp_layout(
        random::generator&    gen
      , params_t       const& params
      , split_callback const& on_split
      , room_callback  const& on_room_gen
      , grid_region    const& reserve
    );
};

} //namespace generate
} //namespace bkrl
