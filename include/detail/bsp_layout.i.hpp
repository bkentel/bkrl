#pragma once
#include "bsp_layout.hpp"
#include "generate.hpp"
#include "random.hpp"

namespace bkrl { namespace detail {

//==============================================================================
//! Implementation for bkrl::bsp_layout
//==============================================================================
class bsp_layout_impl : public bsp_layout_base {
public:
    using index_t        = unsigned;

    struct node_t_ {
        enum : index_t {
            index_reserved = 0
          , index_none = static_cast<unsigned>(-1)
        };

        explicit node_t_(
            grid_region const region
          , index_t     const parent_index = index_none
          , index_t     const child_index  = index_none
        )
          : region       (region)
          , child_index  (child_index)
          , parent_index (parent_index)
        {
        }

        bool is_leaf()     const noexcept { return is_reserved() || child_index == index_none; }
        bool is_reserved() const noexcept { return child_index == index_reserved; }

        grid_region region;
        index_t     child_index  = index_none;
        index_t     parent_index = index_none;
    };

    bsp_layout_impl(
        random::generator&    gen
      , params_t       const& params
      , split_callback const& on_split
      , room_callback  const& on_room_gen
      , grid_region    const& reserve
    );

    //--------------------------------------------------------------------------
    //! split nodes_[index] along the y (vertical) axis, and add the result
    //! as children of nodes_[index].
    //--------------------------------------------------------------------------
    void split_y(index_t const index, grid_index const where);

    //--------------------------------------------------------------------------
    //! split nodes_[index] along the x (vertical) axis, and add the result
    //! as children of nodes_[index].
    //--------------------------------------------------------------------------
    void split_x(index_t const index, grid_index const where);

    //--------------------------------------------------------------------------
    //! Add @children to the node at @index.
    //--------------------------------------------------------------------------
    void add_children(index_t const index, grid_region const& child0, grid_region const& child1);

    //--------------------------------------------------------------------------
    //! Top-level split function. Recursively divide breadth-first.
    //--------------------------------------------------------------------------
    void split(random::generator& gen);

    //--------------------------------------------------------------------------
    //! Decide if a room should be generated at @p node.
    //--------------------------------------------------------------------------
    bool can_generate_room(random::generator& gen, node_t_ const& node) const;

    //--------------------------------------------------------------------------
    //! split axis.
    //--------------------------------------------------------------------------
    enum class split_type {
        split_none, split_x, split_y
    };

    //--------------------------------------------------------------------------
    //! Decide which axis to split @node along.
    //! @pre can_split(gen, node)
    //--------------------------------------------------------------------------
    split_type choose_split_type(random::generator& gen, node_t_ const& node) const;

    //--------------------------------------------------------------------------
    //! Split one node into two nodes.
    //! @returns true if the node was split, false otherwise.
    //! @pre index < nodes_.size()
    //--------------------------------------------------------------------------
    bool split(random::generator& gen, index_t const index);

    using range_t = range<unsigned>;

    void connect(random::generator& gen, connect_callback on_connect);

    range_t connect(random::generator& gen, index_t index);

    void gen_rooms(random::generator& gen, index_t index = 0);

    //--------------------------------------------------------------------------
    //! If @p region is not 0 in size, pre-split such that @p region is a
    //! region in the final layout.
    //--------------------------------------------------------------------------
    void reserve_region(grid_region const& region);
private:
    params_t             params_;
    std::vector<node_t_> nodes_;
    room_callback        on_room_gen_;
    split_callback       on_split_;
    room_id              next_room_id_ = 0;
    std::vector<index_t> connected_nodes_;
    connect_callback     on_connect_;
};

//--------------------------------------------------------------------------
bsp_layout_impl::bsp_layout_impl(
    random::generator&    gen
  , params_t       const& params
  , split_callback const& on_split
  , room_callback  const& on_room_gen
  , grid_region    const& reserve
)
  : params_      {params}
  , on_room_gen_ {on_room_gen}
  , on_split_    {on_split}
{
    //TODO verify params
    auto const& p = params_;
    auto const  n = (p.height * p.width) / (p.min_region_w * p.min_region_h);

    nodes_.reserve(n);
    nodes_.emplace_back(grid_region {0, 0, p.width, p.height});

    reserve_region(reserve);
    split(gen);
    gen_rooms(gen);

    on_room_gen_  = room_callback  {};
    on_split_     = split_callback {};
}
//--------------------------------------------------------------------------
void
bsp_layout_impl::reserve_region(
    grid_region const& region
) {
    BK_ASSERT_DBG(nodes_.size() == 1);
    BK_ASSERT_DBG(region.right  <= params_.width);
    BK_ASSERT_DBG(region.bottom <= params_.height);

    if (region.area() == 0) {
        return;
    }

    split_y(0, region.right);
    split_x(1, region.bottom);
    split_y(3, region.left);
    split_x(6, region.top);

    nodes_[8].child_index = node_t_::index_reserved;
}

//--------------------------------------------------------------------------
//TODO unify split_x and split_y
void
bsp_layout_impl::split_y(
    index_t    const index
  , grid_index const where
) {
    BK_ASSERT_DBG(index < nodes_.size());

    auto& node = nodes_[index];
    BK_ASSERT_DBG(node.is_leaf() && !node.is_reserved());

    auto const split = bkrl::split_y(node.region, where);
    add_children(index, split.first, split.second);
}

//--------------------------------------------------------------------------
void
bsp_layout_impl::split_x(
    index_t    const index
  , grid_index const where
) {
    BK_ASSERT_DBG(index < nodes_.size());

    auto& node = nodes_[index];
    BK_ASSERT_DBG(node.is_leaf() && !node.is_reserved());

    auto const split = bkrl::split_x(node.region, where);
    add_children(index, split.first, split.second);
}

//--------------------------------------------------------------------------
void
bsp_layout_impl::add_children(
    index_t     const  parent_index
  , grid_region const& child0
  , grid_region const& child1
) {
    BK_ASSERT_DBG(parent_index < nodes_.size());

    auto& parent = nodes_[parent_index];

    BK_ASSERT_DBG(parent.is_leaf() && !parent.is_reserved());

    node_t_ n0 {child0, parent_index};
    node_t_ n1 {child1, parent_index};

    auto const ok0 = on_split_(n0.region);
    auto const ok1 = on_split_(n1.region);

    if (!ok0) {
        n0.child_index = node_t_::index_reserved;
    }

    if (!ok1) {
        n1.child_index = node_t_::index_reserved;
    }

    parent.child_index = nodes_.size();

    nodes_.push_back(n0);
    nodes_.push_back(n1);
}

//--------------------------------------------------------------------------
void
bsp_layout_impl::split(
    random::generator& gen
) {
    //
    // split all nodes from [beg, end).
    //
    auto const split_all = [&](size_t const beg, size_t const end) {
        for (auto i = beg; i < end; ++i) {
            auto const did_split = split(gen, i);
        }

        return nodes_.size();
    };

    //
    // continue to split as long as new nodes have been added.
    //
    auto beg = 0;
    auto end = nodes_.size();

    for (auto size = end - beg; size != 0; size = end - beg) {
        auto const new_end = split_all(beg, end);

        beg = end;
        end = new_end;
    }
}

//--------------------------------------------------------------------------
bool
bsp_layout_impl::can_generate_room(
    random::generator& gen
  , node_t_ const&     node
) const {
    if (!node.is_leaf()) {
        return false;
    }

    if (node.is_reserved()) {
        return true;
    }

    //can happen if a reserve room is requested
    if (node.region.width() < params_.min_region_w) {
        return false;
    }

    //can happen if a reserve room is requested
    if (node.region.height() < params_.min_region_h) {
        return false;
    }

    auto const roll = random::uniform_range(gen, 0, 100);
    return roll < params_.room_gen_chance;
}

//--------------------------------------------------------------------------
void
bsp_layout_impl::gen_rooms(
    random::generator& gen
  , index_t const      index
) {
    BK_ASSERT_DBG(index < nodes_.size());
    auto& node = nodes_[index];

    //
    // base case
    //
    if (can_generate_room(gen, node)) {
        //std::cout << "room generated at index = " << index << std::endl;
        node.child_index = node_t_::index_reserved;
        on_room_gen_(node.region, ++next_room_id_); //increment

        return;
    }

    // possible to gen, but rejected
    if (node.is_leaf()) {
        return;
    }

    //
    // recursive case
    //
    auto const lhs = node.child_index + 0;
    auto const rhs = node.child_index + 1;

    BK_ASSERT_DBG(lhs != node_t_::index_none);

    auto const& left  = nodes_[lhs];
    auto const& right = nodes_[rhs];

    //only one child
    BK_ASSERT_DBG(left.parent_index == right.parent_index);
    //if (left.parent_index != right.parent_index) {
    //    gen_rooms(gen, lhs);
    //    return;
    //}

    gen_rooms(gen, lhs);
    gen_rooms(gen, rhs);
}

//--------------------------------------------------------------------------
bsp_layout_impl::split_type
bsp_layout_impl::choose_split_type(
    random::generator& gen
  , node_t_ const&     node
) const {
    BK_ASSERT_DBG(node.is_leaf());
    BK_ASSERT_DBG(!node.is_reserved());

    auto const& p = params_;
    auto const  w = node.region.width();
    auto const  h = node.region.height();

    //
    // if one or more dimensions are too small
    //
    auto const ok_x = h >= p.min_region_h * 2;
    auto const ok_y = w >= p.min_region_w * 2;

    if (ok_x && !ok_y) {
        return split_type::split_x;
    } else if (!ok_x && ok_y) {
        return split_type::split_y;
    } else if (!ok_x && !ok_y) {
        return split_type::split_none;
    }

    //
    // if the max aspect ration is exceeded
    //
    auto const aspect = (w >= h)
        ? (static_cast<float>(w) / h)
        : (static_cast<float>(h) / w);

    if (aspect > p.max_aspect_ratio) {
        return (w >= h) ? split_type::split_y : split_type::split_x;
    }

    //
    // if one dimension is too big
    //
    auto const big_x = w > p.max_region_w;
    auto const big_y = h > p.max_region_h;

    if (big_x && !big_y) {
        return split_type::split_y;
    } else if (big_y && !big_x) {
        return split_type::split_x;
    } else if (!big_x && !big_y) {
        //
        // reject based on params_t::split_chance
        //
        auto const roll = random::uniform_range(gen, 0, 100);
        if (roll >= p.split_chance) {
            return split_type::split_none;
        }
    }

    //
    // otherwise, split randomly 50/50
    //
    auto const roll = random::uniform_range(gen, 0, 100);
    return roll < 50 ? split_type::split_y : split_type::split_x;
}

//--------------------------------------------------------------------------
bool
bsp_layout_impl::split(
    random::generator& gen
  , index_t const      index
) {
    BK_ASSERT_DBG(index < nodes_.size());
    auto& node = nodes_[index];

    //
    // don't split if reserved or an internal node
    //
    if (!node.is_leaf()) {
        return false;
    } else if (node.is_reserved()) {
        return false;
    }

    auto const split_dir = choose_split_type(gen, node);

    // no split required / possible
    if (split_dir == split_type::split_none) {
        node.child_index = node_t_::index_none;
        return false;
    }

    //
    // chose a split point
    //
    auto const& p = params_;
    auto const& r = node.region;
    auto const  w = r.width();
    auto const  h = r.height();

    auto distribution = [&](unsigned const lo, unsigned const hi) {
        return random::uniform_range(gen, lo, hi);
    };

    if (split_dir == split_type::split_x) {
        auto const where = distribution(p.min_region_h, h - p.min_region_h);
        split_x(index, r.top + where);
    } else if (split_dir == split_type::split_y) {
        auto const where = distribution(p.min_region_w, w - p.min_region_w);
        split_y(index, r.left + where);
    } else {
        // ought to be impossible
        BK_TODO_FAIL();
    }

    return true;
}

//--------------------------------------------------------------------------
void
bsp_layout_impl::connect(
    random::generator& gen
  , connect_callback   on_connect
) {
    BK_ASSERT(connected_nodes_.empty());
    connected_nodes_.reserve(next_room_id_);

    on_connect_ = on_connect;
    connect(gen, 0);
    on_connect_ = connect_callback {};
}

//--------------------------------------------------------------------------
bsp_layout_impl::range_t
bsp_layout_impl::connect(
    random::generator& gen
  , index_t const      index
) {
    BK_ASSERT_DBG(index < nodes_.size());
    auto const& node = nodes_[index];

    //
    // base case
    //
    if (node.is_leaf()) {
        auto const size0 = static_cast<unsigned>(connected_nodes_.size());

        if (node.is_reserved()) {
            connected_nodes_.emplace_back(index);
        }

        auto const size1 = static_cast<unsigned>(connected_nodes_.size());

        return range_t {size0, size1};
    }

    //
    // recursive case
    //
    auto const i0 = node.child_index + 0;
    auto const i1 = node.child_index + 1;

    BK_ASSERT(i0 != node_t_::index_none);

    auto const& left  = nodes_[i0];
    auto const& right = nodes_[i1];

    //only one child
    BK_ASSERT(left.parent_index == right.parent_index);
    //if (left.parent_index != right.parent_index) {
    //    return connect(gen, i0);
    //}

    auto const lhs = connect(gen, i0);
    auto const rhs = connect(gen, i1);

    if (lhs.size() == 0) {
        return rhs;
    } else if (rhs.size() == 0) {
        return lhs;
    }

    auto const which0 = random::uniform_range(gen, lhs.lo, lhs.hi - 1);
    auto const which1 = random::uniform_range(gen, rhs.lo, rhs.hi - 1);

    auto const id0 = static_cast<room_id>(which0 + 1);
    auto const id1 = static_cast<room_id>(which1 + 1);

    on_connect_(node.region, id0, id1);

    BK_ASSERT(lhs.hi == rhs.lo);

    return range_t {lhs.lo, rhs.hi};
}

//==============================================================================
//! Implementation for bkrl::bsp_layout
//==============================================================================
class bsp_connector_impl {
public:
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

    //--------------------------------------------------------------------------
    // decide whether the tile at @p can be transformed into a corridor.
    //--------------------------------------------------------------------------
    bool can_gen_corridor(
        grid_storage const& grid
      , grid_region         bounds
      , grid_point          p
    ) const;
private:
    enum class corridor_result {
        failed
      , ok
      , ok_done
    };

    //--------------------------------------------------------------------------
    // add the positions at each cardinal direction from @p p ordered accoring
    // @p delta as candidates if the staisfy can_gen_corridor();
    //--------------------------------------------------------------------------
    void add_candidates_(
        random::generator& gen
      , grid_storage&      grid
      , grid_region        bounds
      , grid_point         p
      , ivec2              delta
    );

    //--------------------------------------------------------------------------
    // transform the tile at @p p to a corridor.
    //--------------------------------------------------------------------------
    tile_type generate_at_(
        grid_storage& grid
      , grid_point    p
    ) const;

    //--------------------------------------------------------------------------
    //! generate a corridor segment of length @p len starting at @p start.
    //--------------------------------------------------------------------------
    std::pair<grid_point, corridor_result> generate_segment_(
        grid_storage& grid
      , grid_region   bounds
      , grid_point    start
      , ivec2         dir
      , grid_size     len
      , room_id       src_id
      , room_id       dst_id
    ) const;

    std::vector<grid_point> closed_;
    std::vector<grid_point> open_;
};

//------------------------------------------------------------------------------
bool
bsp_connector_impl::can_gen_corridor(
    grid_storage const& grid
  , grid_region  const  bounds
  , grid_point   const  p
) const {
    if (!intersects(p, bounds)) {
        return false;
    }

    auto const type = grid.get(attribute::tile_type, p);

    switch (type) {
    case tile_type::invalid :
    case tile_type::empty :
    case tile_type::floor :
    case tile_type::door :
    case tile_type::stair :
    case tile_type::corridor :
        return true;
    case tile_type::wall :
        break;
    default :
        BK_TODO_FAIL();
    }

    BK_ASSERT(type == tile_type::wall);

    auto const doors = check_grid_block5(grid, p.x, p.y, attribute::tile_type, tile_type::door);
    if (doors) {
        return false;
    }

    auto const walls = check_grid_block9(grid, p.x, p.y, attribute::tile_type, tile_type::wall);

    constexpr auto i_NW = (1<<0);
    constexpr auto i_Nx = (1<<1);
    constexpr auto i_NE = (1<<2);
    constexpr auto i_xW = (1<<3);
    constexpr auto i_xE = (1<<4);
    constexpr auto i_SW = (1<<5);
    constexpr auto i_Sx = (1<<6);
    constexpr auto i_SE = (1<<7);

    auto const c0 = (walls & (i_Nx|i_NE|i_xE)) == (i_Nx|i_xE);
    auto const c1 = (walls & (i_Sx|i_SW|i_xW)) == (i_Sx|i_xW);
    auto const c2 = (walls & (i_Nx|i_NW|i_xW)) == (i_Nx|i_xW);
    auto const c3 = (walls & (i_Sx|i_SE|i_xE)) == (i_Sx|i_xE);

    auto const c4 = (walls & ~(i_SW|i_Sx|i_SE)) == (i_Nx|i_xW|i_xE);
    auto const c5 = (walls & ~(i_NE|i_xE|i_SE)) == (i_Nx|i_xW|i_Sx);
    auto const c6 = (walls & ~(i_NW|i_Nx|i_NE)) == (i_xW|i_xE|i_Sx);
    auto const c7 = (walls & ~(i_NW|i_xW|i_SW)) == (i_Nx|i_xE|i_Sx);

    auto const c8 = walls == (i_Nx|i_xW|i_xE|i_Sx);

    if (c0 || c1 || c2 || c3 || c4 || c5 || c6 || c7 || c8) {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
tile_type
bsp_connector_impl::generate_at_(
    grid_storage&      grid
  , grid_point   const p
) const {
    auto const type = grid.get(attribute::tile_type, p);

    switch (type) {
    case tile_type::invalid :
    case tile_type::empty :
        grid.set(attribute::tile_type, p, tile_type::corridor);
        return tile_type::corridor;
    case tile_type::floor :
    case tile_type::door :
    case tile_type::stair :
    case tile_type::corridor :
        break;
    case tile_type::wall :
        grid.set(attribute::tile_type, p, tile_type::door);
        return tile_type::door;
    default :
        BK_TODO_FAIL();
    }

    return type;
}

static inline bool can_place_door_at(
    grid_storage const& grid
  , grid_point   const  p
) {
    auto const is_ok = [&](ipoint2 const q) {
        if (!grid.is_valid(q)) {
            return false;
        }

        auto const type = grid.get(attribute::tile_type, q);

        switch (type) {
        case tile_type::corridor :
        case tile_type::floor :
        case tile_type::stair :
            return true;
        }

        return false;
    };

    auto const n = ipoint2 {p.x,     p.y - 1};
    auto const s = ipoint2 {p.x,     p.y + 1};
    auto const w = ipoint2 {p.x - 1, p.y    };
    auto const e = ipoint2 {p.x + 1, p.y    };

    return (is_ok(n) && is_ok(s)) || (is_ok(e) && is_ok(w));
}

//------------------------------------------------------------------------------
std::pair<grid_point, bsp_connector_impl::corridor_result>
bsp_connector_impl::generate_segment_(
    grid_storage&       grid
  , grid_region   const bounds
  , grid_point    const start
  , ivec2         const dir
  , grid_size     const len
  , room_id       const src_id
  , room_id       const dst_id
) const {
    BK_ASSERT_DBG(len > 0);
    BK_ASSERT_DBG(intersects(bounds, start));
    BK_ASSERT_DBG(src_id && dst_id && src_id != dst_id);
    BK_ASSERT_DBG(dir.x || dir.y);

    //a list of doors that have been generated along this segment
    boost::container::static_vector<ipoint2, 8> doors;

    auto const v = 
        (dir.x > 0) ? ivec2 { 1,  0}
      : (dir.x < 0) ? ivec2 {-1,  0}
      : (dir.y > 0) ? ivec2 { 0,  1}
      :               ivec2 { 0, -1};

    auto  result = std::make_pair(start, corridor_result::ok);
    auto& p      = result.first;
    auto& ok     = result.second;

    //
    // try to transform the grid at q
    //
    auto const gen_at = [&](ipoint2 const q) {
        if (!can_gen_corridor(grid, bounds, q)) {
            return false;
        }

        auto const type = generate_at_(grid, q);
        if (type == tile_type::door) {
            doors.push_back(q);
        }

        return true;
    };

    BK_ASSERT_DBG(gen_at(p));

    for (int i = 0; i < len; ++i) {
        p += v;

        //
        // can't gen here
        //
        if (!gen_at(p)) {
            ok = corridor_result::failed;
            p -= v; //back up
            break;
        }

        //
        // reached out destination
        //
        auto const id = grid.get(attribute::room_id, p);
        if (id == dst_id) {
            ok = corridor_result::ok_done;
            break;
        }

        grid.set(attribute::room_id, p, src_id);
    }

    for (auto const d : doors) {
        if (!can_place_door_at(grid, d)) {
            grid.set(attribute::tile_type, d, tile_type::wall);
        }
    }

    BK_ASSERT(can_gen_corridor(grid, bounds, p));
    return result;
}

//------------------------------------------------------------------------------
void
bsp_connector_impl::add_candidates_(
    random::generator&  gen
  , grid_storage&       grid
  , grid_region   const bounds
  , grid_point    const p
  , ivec2         const delta
) {
    constexpr auto weight = 1.1f;

    constexpr auto iN = 0u;
    constexpr auto iS = 1u;
    constexpr auto iW = 2u;
    constexpr auto iE = 3u;

    std::array<grid_point, 4> dirs {
        grid_point {p.x + 0, p.y - 1} //north
      , grid_point {p.x + 0, p.y + 1} //south
      , grid_point {p.x - 1, p.y + 0} //west
      , grid_point {p.x + 1, p.y + 0} //east
    };

    auto const mag_x = static_cast<float>(std::abs(delta.x));
    auto const mag_y = static_cast<float>(std::abs(delta.y));

    auto beg = 0u;
    auto end = 4u;

    if (mag_x > weight*mag_y) {
        if (delta.x >= 0) {
            std::swap(dirs[0], dirs[iE]);
        } else {
            std::swap(dirs[0], dirs[iW]);
        }

        beg++;
    } else if (mag_y > weight*mag_x) {
        if (delta.y >= 0) {
            std::swap(dirs[0], dirs[iS]);
        } else {
            std::swap(dirs[0], dirs[iN]);
        }

        beg++;
    }

    //std::shuffle is no good here because it uses the std:: distributions,
    //not boost. which means results will be inconsistant across platforms
    std::random_shuffle(dirs.data() + beg, dirs.data() + end, [&](size_t const i) {
        return bkrl::random::uniform_range(gen, size_t {0}, i - 1);
    });

    std::copy_if(
        std::crbegin(dirs), std::crend(dirs), std::back_inserter(open_)
      , [&](grid_point const gp) {
            auto const it = std::find_if(
                std::cbegin(closed_), std::cend(closed_)
              , [&](grid_point const q) {
                    return gp == q;
                }
            );

            if (it != std::cend(closed_)) {
                return false;
            }

            return can_gen_corridor(grid, bounds, gp);
        }
    );
}

//------------------------------------------------------------------------------
bool
bsp_connector_impl::connect(
    random::generator& gen
  , grid_storage&      grid
  , grid_region const& bounds
  , room        const& src_room
  , room        const& dst_room
) {
    open_.clear();
    closed_.clear();

    auto const beg = src_room.center();
    auto const end = dst_room.center();
    auto       cur = beg;

    auto const src_id = src_room.id();
    auto const dst_id = dst_room.id();

    BK_ASSERT_DBG(intersects(bounds, beg));
    BK_ASSERT_DBG(intersects(bounds, end));

    //TODO
    BK_ASSERT_DBG(grid.get(attribute::tile_type, beg) != tile_type::wall);
    BK_ASSERT_DBG(grid.get(attribute::tile_type, end) != tile_type::wall);

    add_candidates_(gen, grid, bounds, cur, end - cur);

    for (int failures = 0; failures < 3;) {
        if (open_.empty()) {
            break;
        }

        auto const p = open_.back();
        open_.pop_back();
        closed_.push_back(p);

        if (p == cur) {
            continue;
        }

        auto const dir    = p - cur;
        auto const len    = random::uniform_range(gen, 1, 10);
        auto const result = generate_segment_(grid, bounds, cur, dir, len, src_id, dst_id);

        if (result.second == corridor_result::failed) {
            failures++;
        } else if (result.second == corridor_result::ok_done) {
            return true;
        }

        cur = result.first;
        add_candidates_(gen, grid, bounds, cur, end - cur);
    }

    return false;
}

}} //namespace bkrl::detail
