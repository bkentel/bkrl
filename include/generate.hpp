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
using split_result = std::tuple<bool, grid_region, grid_region>;

//TODO combine these somehow?
split_result split_vertical(
    random::range_generator const& gen
  , grid_region                    region
  , unsigned                       minimum = 0
);

split_result split_horizontal(
    random::range_generator const& gen
  , grid_region                    region
  , unsigned                       minimum = 0
);

//==============================================================================
//! simple_room
//==============================================================================
class simple_room {
public:
    room generate(random::generator& gen, bkrl::grid_region bounds);
private:
};

//==============================================================================
//! bsp_layout
//==============================================================================
class bsp_layout {
    struct node_t {
        node_t(grid_region const region)
          : region {region}
        {
        }

        bool is_leaf()  const { return child_index == 0; }
        bool is_empty() const { return room == nullptr; }

        bkrl::grid_region region;
        room*             room = nullptr;
        unsigned          child_index = 0;
    };
public:
    struct params_t {
        unsigned min_region_w = 4;
        unsigned min_region_h = 4;
        unsigned max_region_w = 20;
        unsigned max_region_h = 20;
        unsigned split_chance = 50;
    };

    using room_callback = std::function<void (grid_region bounds)>;

    explicit bsp_layout(params_t const& params = params_t{});
  
    void generate(random::generator& gen);

    void split(random::generator& gen);

    bool split(random::generator& gen, node_t const node);
//private:
    params_t params_;
    std::vector<node_t> nodes_;
};

} //namespace generate
} //namespace bkrl
