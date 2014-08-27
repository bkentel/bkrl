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
//! bsp_layout
//==============================================================================
class bsp_layout {
public:
    using index_t = unsigned;

    struct node_t {
        enum : index_t {
            index_reserved = 0
          , index_none = static_cast<unsigned>(-1)
        };

        explicit node_t(
            grid_region const region
          , index_t     const parent_index = index_none
          , index_t     const child_index  = index_none
        )
          : region {region}
          , child_index {child_index}
          , parent_index {parent_index}
        {
        }

        bool is_leaf() const { return child_index == index_none; }
        bool is_reserved() const { return child_index == index_reserved; }

        grid_region region;
        index_t     child_index  = index_none;
        index_t     parent_index = index_none;
    };
public:
    struct params_t {
        unsigned min_region_w = 4;
        unsigned min_region_h = 4;
        unsigned max_region_w = 20;
        unsigned max_region_h = 20;
        unsigned split_chance = 50;
        float    max_aspect_ratio = 16.0f / 10.0f;
    };

    using room_callback = std::function<void (grid_region bounds)>;
    using split_callback = std::function<bool (grid_region bounds)>;

    explicit bsp_layout(split_callback on_split, params_t const& params = params_t{});

    void generate(random::generator& gen);
    void generate(random::generator& gen, grid_region reserve);

    void split(
        node_t&           parent
      , index_t     const index
      , region_pair const children
    ) {
        BK_PRECONDITION(parent.is_leaf());

        node_t n0 {std::get<0>(children), index};
        node_t n1 {std::get<1>(children), index};

        auto const ok0 = on_split_(n0.region);
        auto const ok1 = on_split_(n1.region);

        if (!ok0) {
            n0.child_index = node_t::index_reserved;
        }

        if (!ok1) {
            n1.child_index = node_t::index_reserved;
        }

        parent.child_index = nodes_.size();

        nodes_.push_back(n0);
        nodes_.push_back(n1);
    }

    //vertical
    void split_y(index_t const index, grid_index const where) {
        BK_PRECONDITION(index < nodes_.size());

        auto& node = nodes_[index];
        split(node, index, generate::split_y(node.region, where));
    }

    //horizontal
    void split_x(index_t const index, grid_index const where) {
        BK_PRECONDITION(index < nodes_.size());

        auto& node = nodes_[index];
        split(node, index, generate::split_x(node.region, where));
    }


    void split(random::generator& gen);

    bool split(random::generator& gen, index_t node);

//private:
    split_callback on_split_;

    params_t params_;
    std::vector<node_t> nodes_;
};

} //namespace generate
} //namespace bkrl
