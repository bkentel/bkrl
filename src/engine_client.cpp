#pragma execution_character_set("utf-8")

#include "item.hpp"
#include "engine_client.hpp"
#include "font.hpp"
#include "config.hpp"
#include "renderer.hpp"
#include "grid.hpp"
#include "command_type.hpp"
#include "random.hpp"
#include "generate.hpp"
#include "bsp_layout.hpp"
#include "tile_sheet.hpp"
#include "player.hpp"
#include "messages.hpp"

#include <boost/container/static_vector.hpp>

using bkrl::engine_client;
using bkrl::command_type;
using bkrl::string_ref;

namespace random = bkrl::random;

namespace {
    static string_ref const file_key_map           {R"(./data/key_map.json)"};
    static string_ref const file_texture_map       {R"(./data/texture_map.json)"};
    static string_ref const file_materials         {R"(./data/materials.def)"};
    static string_ref const file_materials_strings {R"(./data/locale/en/materials.def)"};
    static string_ref const file_messages_en       {R"(./data/locale/en/messages.def)"}; //TODO
    static string_ref const file_messages_jp       {R"(./data/locale/jp/messages.def)"}; //TODO
}

namespace bkrl {

////////////////////////////////////////////////////////////////////////////////
// texture_transform
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// get_texture_type fallback
//------------------------------------------------------------------------------
template <tile_type Type>
texture_type
get_texture_type(
    grid_storage const& //grid
  , grid_point   const  //p
) {
    BK_TODO_FAIL();
}

//------------------------------------------------------------------------------
// get_texture_type passthrough overload
//------------------------------------------------------------------------------
template <tile_type Type>
texture_type
get_texture_type(
    grid_storage const& grid
  , grid_index   const  x
  , grid_index   const  y
) {
    return get_texture_type<Type>(grid, grid_point {x, y});
}

//------------------------------------------------------------------------------
// tile_type::floor
//------------------------------------------------------------------------------
template <>
texture_type
get_texture_type<tile_type::floor>(
    grid_storage const& //grid
  , grid_point   const  //p
) {
    return texture_type::floor;
}

//------------------------------------------------------------------------------
// tile_type::corridor
//------------------------------------------------------------------------------
template <>
texture_type
get_texture_type<tile_type::corridor>(
    grid_storage const& //grid
  , grid_point   const  //p
) {
    return texture_type::corridor;
}

//------------------------------------------------------------------------------
// tile_type::door
//------------------------------------------------------------------------------
template <>
texture_type
get_texture_type<tile_type::door>(
    grid_storage const& grid
  , grid_point   const  p
) {
    door_data const door {grid, p};

    if (door.is_open()) {
        return texture_type::door_opened;
    }

    if (door.is_closed()) {
        return texture_type::door_closed;
    }

    BK_TODO_FAIL();
}

//------------------------------------------------------------------------------
// tile_type::stair
//------------------------------------------------------------------------------
template <>
texture_type
get_texture_type<tile_type::stair>(
    grid_storage const& grid
  , grid_point   const  p
) {
    stair_data const stair {grid, p};

    if (stair.is_down()) {
        return texture_type::stair_down;
    }

    if (stair.is_up()) {
        return texture_type::stair_up;
    }

    BK_TODO_FAIL();
}

//------------------------------------------------------------------------------
// tile_type::wall
//------------------------------------------------------------------------------
template <>
texture_type
get_texture_type<tile_type::wall>(
    grid_storage const& grid
  , grid_point   const  p
) {
    constexpr auto iN = (1<<0);
    constexpr auto iW = (1<<1);
    constexpr auto iE = (1<<2);
    constexpr auto iS = (1<<3);

    auto const n = check_grid_block5f(
        grid, p.x, p.y, attribute::tile_type
      , [](tile_type const type) {
            return type == tile_type::wall
                || type == tile_type::door;
        }
    );

    switch (n) {
    case 0           : return texture_type::wall_none;
    case iN          : return texture_type::wall_n;
    case iW          : return texture_type::wall_w;
    case iE          : return texture_type::wall_e;
    case iS          : return texture_type::wall_s;
    case iN|iS       : return texture_type::wall_ns;
    case iE|iW       : return texture_type::wall_ew;
    case iS|iE       : return texture_type::wall_se;
    case iS|iW       : return texture_type::wall_sw;
    case iN|iE       : return texture_type::wall_ne;
    case iN|iW       : return texture_type::wall_nw;
    case iN|iS|iE    : return texture_type::wall_nse;
    case iN|iS|iW    : return texture_type::wall_nsw;
    case iN|iE|iW    : return texture_type::wall_new;
    case iS|iE|iW    : return texture_type::wall_sew;
    case iN|iS|iE|iW : return texture_type::wall_nsew;
    }

    BK_TODO_FAIL();
    //return texture_type::invalid;
}

//------------------------------------------------------------------------------
// get_texture_type forward to specialization
//------------------------------------------------------------------------------
texture_type
get_texture_type(
    grid_storage const& grid
  , grid_point   const  p
  , tile_type    const  type
) {
    using tt = tile_type;

    switch (type) {
    case tt::empty    : return get_texture_type<tt::empty>(grid, p);
    case tt::floor    : return get_texture_type<tt::floor>(grid, p);
    case tt::wall     : return get_texture_type<tt::wall>(grid, p);
    case tt::door     : return get_texture_type<tt::door>(grid, p);
    case tt::stair    : return get_texture_type<tt::stair>(grid, p);
    case tt::corridor : return get_texture_type<tt::corridor>(grid, p);
    }

    return texture_type::invalid;
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//! decide whether the wall at @p p can be merged (removed) with a neighbor.
//------------------------------------------------------------------------------
bool
can_merge_wall(
    grid_storage const& grid
  , grid_point          p
) {
    auto const type = grid.get(attribute::tile_type, p);
    if (type != tile_type::wall) {
        return false;
    }

    auto const n = check_grid_block9f(grid, p.x, p.y, attribute::tile_type
      , [](tile_type const t) {
            return (t == tile_type::wall) || (t == tile_type::door);
        }
    );

    constexpr auto iNW = (1<<0);
    constexpr auto iNx = (1<<1);
    constexpr auto iNE = (1<<2);
    constexpr auto ixW = (1<<3);
    constexpr auto ixE = (1<<4);
    constexpr auto iSW = (1<<5);
    constexpr auto iSx = (1<<6);
    constexpr auto iSE = (1<<7);

    constexpr auto s0 = iNW|iNx|iNE;
    constexpr auto s1 = iNW|ixW|iSW;
    constexpr auto s2 = iNE|ixE|iSE;
    constexpr auto s3 = iSW|iSx|iSE;
    constexpr auto s4 = iNW|iNx|iNE|ixW|ixE|iSW|iSx; //TODO

    if (n == s4) {
        BK_TODO_FAIL(); //TODO what!?
    }

    return (n == s4)
        || (((n & s0) == s0) && ((n & iSx) == 0))
        || (((n & s1) == s1) && ((n & ixE) == 0))
        || (((n & s2) == s2) && ((n & ixW) == 0))
        || (((n & s3) == s3) && ((n & iNx) == 0));
}

//------------------------------------------------------------------------------
//! for each wall tile in @p grid at the edges of the region give by @p bounds,
//! merge the wall tile with any neighboring wall tiles if possible.
//------------------------------------------------------------------------------
void
merge_walls(
    grid_storage&     grid
  , grid_region const bounds
) {
    for_each_edge(bounds, [&](grid_index const x, grid_index const y) {
        if (can_merge_wall(grid, grid_point {x, y})) {
            grid.set(attribute::tile_type, x, y, bkrl::tile_type::floor);
        }
    });
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//==============================================================================
//
//==============================================================================
class room_connector {
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
        random::generator&  gen
      , grid_storage&       grid
      , grid_region         bounds
      , grid_point          p
      , vector2d<int>       delta
    );

    //--------------------------------------------------------------------------
    // transform the tile at @p p to a corridor.
    //--------------------------------------------------------------------------
    void generate_at_(
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
      , vector2d<int> dir
      , grid_size     len
      , room_id       src_id
      , room_id       dst_id
    ) const;

    std::vector<grid_point> closed_;
    std::vector<grid_point> open_;
};

//------------------------------------------------------------------------------
bool
room_connector::can_gen_corridor(
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
void
room_connector::generate_at_(
    grid_storage&      grid
  , grid_point   const p
) const {
    auto const type = grid.get(attribute::tile_type, p);

    switch (type) {
    case tile_type::invalid :
    case tile_type::empty :
        grid.set(attribute::tile_type, p, tile_type::corridor);
        break;
    case tile_type::floor :
    case tile_type::door :
    case tile_type::stair :
    case tile_type::corridor :
        break;
    case tile_type::wall :
        grid.set(attribute::tile_type, p, tile_type::door);
        break;
    default :
        BK_TODO_FAIL();
    }
}

//------------------------------------------------------------------------------
std::pair<grid_point, room_connector::corridor_result>
room_connector::generate_segment_(
    grid_storage&       grid
  , grid_region   const bounds
  , grid_point    const start
  , vector2d<int> const dir
  , grid_size     const len
  , room_id       const src_id
  , room_id       const dst_id
) const {
    BK_ASSERT_DBG(len > 0);
    BK_ASSERT_DBG(intersects(bounds, start));
    //BK_ASSERT_DBG((dir.x || dir.y) && !(dir.x && dir.y)); TODO
    BK_ASSERT_DBG(src_id && dst_id && src_id != dst_id);
    BK_ASSERT_DBG(can_gen_corridor(grid, bounds, start));

    auto const step = (dir.x ? dir.x : dir.y) > 0 ? 1 : -1;

    auto  result = std::make_pair(start, corridor_result::ok);
    auto& cur    = result.first;
    auto& ok     = result.second;
    auto& p      = dir.x ? cur.x : cur.y;

    for (int i = 0; i < len; ++i) {
        p += step;

        ok = can_gen_corridor(grid, bounds, cur)
          ? corridor_result::ok
          : corridor_result::failed;

        if (ok == corridor_result::failed) {
            p -= step; //back up
            break;
        }

        generate_at_(grid, cur);

        auto const id = grid.get(attribute::room_id, cur);
        if (id == dst_id) {
            ok = corridor_result::ok_done;
            break;
        }
    }

    BK_ASSERT(can_gen_corridor(grid, bounds, cur));
    return result;
}

//------------------------------------------------------------------------------
void
room_connector::add_candidates_(
    random::generator&  gen
  , grid_storage&       grid
  , grid_region   const bounds
  , grid_point    const p
  , vector2d<int> const delta
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

    std::shuffle(dirs.data() + beg, dirs.data() + end, gen);

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
room_connector::connect(
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

    for (int failures = 0; failures < 5;) {
        if (open_.empty()) {
            return false;
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//==============================================================================
//! One level within the world.
//==============================================================================
class level {
public:
    //--------------------------------------------------------------------------
    level(random::generator& substantive, random::generator& trivial
        , tile_sheet const& sheet, grid_size width, grid_size height
    )
      : tile_sheet_ {&sheet}
      , grid_ {width, height}
    {
        generate_(substantive, trivial);
    }
    
    //--------------------------------------------------------------------------
    void render(renderer& r) {
        auto const w = grid_.width();
        auto const h = grid_.height();

        auto const& sheet = *tile_sheet_;

        for (bkrl::grid_index y = 0; y < h; ++y) {
            for (bkrl::grid_index x = 0; x < w; ++x) {
                auto const texture = grid_.get(attribute::texture_id, x, y);
                auto const tx = texture % sheet.tiles_x(); //TODO
                auto const ty = texture / sheet.tiles_x();

                sheet.render(r, tx, ty, x, y);
            }
        }
    }

    //--------------------------------------------------------------------------
    using adjacency = boost::container::static_vector<ipoint2, 9>;

    adjacency check_adjacent(ipoint2 const p, tile_type const type) const {
        auto const& xi = x_off9;
        auto const& yi = y_off9;

        adjacency result;

        for (size_t i = 0; i < 9; ++i) {
            auto const q = ipoint2 {p.x + xi[i], p.y + yi[i]};

            auto const value = grid_.get(attribute::tile_type, q);
            if (value == type) {
                result.push_back(q);
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------
    door_data get_door_at(ipoint2 const p) const {
        return {grid_, p};
    }

    //--------------------------------------------------------------------------
    stair_data get_stair_at(ipoint2 const p) const {
        return {grid_, p};
    }

    //--------------------------------------------------------------------------
    message_type go_down(ipoint2 const p) const {
        return use_stair_(p, true);
    }
    
    //--------------------------------------------------------------------------
    message_type go_up(ipoint2 const p) const {
        return use_stair_(p, false);
    }

    message_type use_stair_(ipoint2 const p, bool const down) const {
        auto const type = grid_.get(attribute::tile_type, p);
        if (type != tile_type::stair) {
            return message_type::stairs_no_stairs;
        }

        auto const stair = stair_data {grid_, p};
        
        if (down && !stair.is_down()) {
            return message_type::stairs_no_down;
        } else if(!down && !stair.is_up()) {
            return message_type::stairs_no_up;
        }
        
        return message_type::none;
    }

    //--------------------------------------------------------------------------
    bool can_move_to(entity const& e, signed const dx, signed const dy) {
        if (std::abs(dx) > 1 || std::abs(dy) > 1) {
            BK_TODO_FAIL();
        }

        auto const to = translate(e.position(), dx, dy);

        if (!grid_.is_valid(to)) {
            return false;
        }

        auto const type = grid_.get(attribute::tile_type, to);

        //TODO
        switch (type) {
        case tile_type::invalid : //TODO this is just for testing

        case tile_type::floor :
        case tile_type::corridor :
        case tile_type::stair :
            return true;
        case tile_type::door :
            return door_data {grid_, to}.is_open();
        default :
            break;
        }

        return false;
    }

    ipoint2 down_stair() const noexcept {
        return stairs_down_;
    }

    ipoint2 up_stair() const noexcept {
        return stairs_up_;
    }

    message_type open_doors(ipoint2 const p) {
        return set_doors_(p, true);
    }

    message_type open_door(ipoint2 const p) {
        return set_door_(p, true);
    }

    message_type close_doors(ipoint2 const p) {
        return set_doors_(p, false);
    }

    message_type close_door(ipoint2 const p) {
        return set_door_(p, false);
    }
private:
    //--------------------------------------------------------------------------
    void generate_rooms_(random::generator& substantive) {
        generate::simple_room room_gen   {};
        generate::circle_room circle_gen {};

        auto& rooms = rooms_;

        auto const on_split = [](grid_region const) {
            return true;
        };

        auto const on_room_gen = [&](grid_region const bounds, unsigned const id) {
            rooms.emplace_back(room_gen.generate(substantive, bounds, id));
            
            BK_ASSERT_SAFE(id == rooms.size());
        };

        //TODO temp; for testing only
        auto const reserve = grid_region {20, 20, 40, 40};
        auto const params  = bsp_layout::params_t {};

        layout_ = bsp_layout::generate(
            substantive
          , on_split
          , on_room_gen
          , params
          , reserve
        );

        for (auto const& room : rooms) {
            auto const x = room.bounds().left;
            auto const y = room.bounds().top;

            grid_.write(room, grid_point {x, y});
        }

        for (auto const& room : rooms) {
            merge_walls(grid_, room.bounds());
        }
    }

    //--------------------------------------------------------------------------
    void connect_rooms_(random::generator& substantive) {
        auto& rooms = rooms_;

        room_connector connector;

        layout_.connect(substantive, [&](grid_region const& bounds, unsigned const id0, unsigned const id1) {
            auto const connected = connector.connect(
                substantive
              , grid_
              , bounds
              , rooms[id0-1]
              , rooms[id1-1]
            );

            return true;
        });
    }

    //--------------------------------------------------------------------------
    void generate_stairs_(
        random::generator& //substantive //TODO
    ) {
        auto& rooms = rooms_;

        //TODO
        stairs_up_   = rooms.front().center();
        stairs_down_ = rooms.back().center();

        grid_.set(attribute::tile_type, stairs_up_,   tile_type::stair);
        grid_.set(attribute::tile_type, stairs_down_, tile_type::stair);

        stair_data const up   {stair_data::type::stair_up};
        stair_data const down {stair_data::type::stair_down};

        grid_.set(attribute::data, stairs_up_,   up);
        grid_.set(attribute::data, stairs_down_, down);
    }

    //--------------------------------------------------------------------------
    void place_player_(
        random::generator& //substantive //TODO
    ) {
    }

    //--------------------------------------------------------------------------
    void place_items_(
        random::generator& //substantive //TODO
    ) {
    }

    //--------------------------------------------------------------------------
    void place_entities_(
        random::generator& //substantive //TODO
    ) {
    }

    //--------------------------------------------------------------------------
    void generate_(
        random::generator& substantive
      , random::generator& //trivial //TODO
    ) {
        generate_rooms_(substantive);
        connect_rooms_(substantive);
        generate_stairs_(substantive);
        place_items_(substantive);
        place_entities_(substantive);
        place_player_(substantive);

        update_texture_type_();
        update_texture_id_();
    }

    //--------------------------------------------------------------------------
    void update_grid_(ipoint2 const p) {
        auto const& xi = x_off9;
        auto const& yi = y_off9;
        
        for (size_t i = 0; i < 9; ++i) {
            ipoint2 const q {p.x + xi[i], p.y + yi[i]};

            if (!grid_.is_valid(q)) {
                continue;
            }

            update_texture_type_(q);
            update_texture_id_(q);
        }
    }

    //------------------------------------------------------------------------------
    //! update the texture type for the tile at @p p.
    //------------------------------------------------------------------------------
    void update_texture_type_(grid_point const p) {
        auto const type     = grid_.get(attribute::tile_type, p);
        auto const tex_type = get_texture_type(grid_, p, type);

        grid_.set(attribute::texture_type, p, tex_type);
    }

    //------------------------------------------------------------------------------
    //! update the texture type for every tile in @p grid.
    //------------------------------------------------------------------------------
    void update_texture_type_() {
        for_each_xy(grid_, [&](grid_index const x, grid_index const y) {
            update_texture_type_(grid_point {x, y});
        });
    }

    //------------------------------------------------------------------------------
    //! update the texture id for the tile at @p p using the mappings in @p map.
    //------------------------------------------------------------------------------
    void update_texture_id_(grid_point const p) {
        tile_map const& map = get_tile_map_();

        auto const type = grid_.get(attribute::texture_type, p);
        auto const id   = map[type];
        grid_.set(attribute::texture_id, p, id);
    }

    //------------------------------------------------------------------------------
    //! update the texture id for every tile in @p grid using the mappings in @p map.
    //------------------------------------------------------------------------------
    void update_texture_id_() {
        for_each_xy(grid_, [&](grid_index const x, grid_index const y) {
            update_texture_id_(grid_point {x, y});
        });
    }

    //--------------------------------------------------------------------------
    message_type set_door_(ipoint2 const p, bool const opened) {
        auto const type = grid_.get(attribute::tile_type, p);
        if (type != tile_type::door) {
            return message_type::door_no_door;
        }
        
        auto door = get_door_at(p);

        if (opened) {
            if (door.is_open()) {
                return message_type::door_is_open;
            }

            door.open();
        } else {
            if (door.is_closed()) {
                return message_type::door_is_closed;
            }

            door.close();
        }

        grid_.set(attribute::data, p, door);

        update_texture_type_(p);
        update_texture_id_(p);

        return message_type::none;
    }

    //--------------------------------------------------------------------------
    message_type set_doors_(ipoint2 const p, bool const opened) {
        //get all adjacent doors
        auto adjacent = check_adjacent(p, tile_type::door);
        if (adjacent.empty()) {
            return message_type::door_no_door;
        }

        auto const beg = std::begin(adjacent);
        auto const end = std::end(adjacent);

        //remove ineligible doors
        auto it = std::remove_if(beg, end, [&] (ipoint2 const q) {
            auto const door = get_door_at(q);
            return opened ? !door.is_closed() : !door.is_open();
        });

        //the number of eligible doors
        auto const n = std::distance(beg, it);
        
        //no eligible doors
        if (n == 0) {
            return (opened)
                 ? (message_type::door_is_open)
                 : (message_type::door_is_closed);
        }
        
        //exactly one eligible door
        if (n == 1) {
            auto const q = *beg;

            if (q == p) {
                return message_type::door_blocked;
            }
            
            set_door_(q, opened);
                       
            return message_type::none;
        }

        //more than one
        return message_type::direction_prompt;
    }

    //--------------------------------------------------------------------------
    tile_map const& get_tile_map_() const {
        BK_ASSERT_DBG(tile_sheet_);
        return tile_sheet_->map();
    }

    //--------------------------------------------------------------------------
    tile_sheet const* tile_sheet_;
    grid_storage      grid_;

    bsp_layout        layout_;
    std::vector<room> rooms_;

    ipoint2 stairs_up_   = ipoint2{0, 0};
    ipoint2 stairs_down_ = ipoint2{0, 0};
};

//==============================================================================
//! The "view" into the world; scaling and translation within the viewport.
//==============================================================================
class view {
public:
    //--------------------------------------------------------------------------
    view(tile_sheet const& sheet, float const width, float const height)
      : sheet_     {&sheet}
      , display_w_ {width}
      , display_h_ {height}
    {
        BK_ASSERT_SAFE(width > 0.0f);
        BK_ASSERT_SAFE(height > 0.0f);
    }

    //--------------------------------------------------------------------------
    template <typename T>
    view(tile_sheet const& sheet, T const width, T const height)
      : view {sheet, static_cast<float>(width), static_cast<float>(height)}
    {
        static_assert(std::is_arithmetic<T>::value, "")    ;
    }

    //--------------------------------------------------------------------------
    void set_size(float const width, float const height) {
        BK_ASSERT_SAFE(width > 0.0f);
        BK_ASSERT_SAFE(height > 0.0f);

        display_w_ = width;
        display_h_ = height;
    }

    //--------------------------------------------------------------------------
    point2 center() const noexcept {
        return {display_w_ / 2.0f, display_h_ / 2.0f};
    }

    //--------------------------------------------------------------------------
    void zoom_by(float const delta, ipoint2 const center) {
        zoom_to(zoom_ + delta, center);
    }

    //--------------------------------------------------------------------------
    void zoom_to(float const z, ipoint2 const center) {
        auto constexpr min =  0.1f;
        auto constexpr max = 10.0f;

        zoom_ = clamp(z, min, max);

        scroll_x_ = 0.0f;
        scroll_y_ = 0.0f;

        center_on_grid(center);
    }

    //--------------------------------------------------------------------------
    void zoom_by(float const delta) {
        zoom_to(zoom_ + delta, screen_to_grid(center()));
    }

    //--------------------------------------------------------------------------
    void zoom_to(float const z) {
        zoom_to(z, screen_to_grid(center()));
    }

    //--------------------------------------------------------------------------
    void scroll_by_tile(int const dx, int const dy) {
        auto const z = zoom_;
        auto const w = sheet_->tile_width()  * z;
        auto const h = sheet_->tile_height() * z;

        scroll_by(-dx * w, -dy * h);
    }

    //--------------------------------------------------------------------------
    void scroll_by(float const dx, float const dy) {
        auto const z = zoom_;
        BK_ASSERT_DBG(z > 0.0f);

        auto const x = dx / zoom_;
        auto const y = dy / zoom_;

        scroll_to(scroll_x_ + x, scroll_y_ + y);
    }

    //--------------------------------------------------------------------------
    void scroll_to(float const x, float const y) {
        scroll_x_ = x;
        scroll_y_ = y;
    }

    //--------------------------------------------------------------------------
    ipoint2 screen_to_grid(float const x, float const y) const {
        auto const px = display_x_ + scroll_x_;
        auto const py = display_y_ + scroll_y_;

        auto const tw = static_cast<float>(sheet_->tile_width());
        auto const th = static_cast<float>(sheet_->tile_height());

        auto const ix = static_cast<int>(std::trunc((x / zoom_ - px) / tw));
        auto const iy = static_cast<int>(std::trunc((y / zoom_ - py) / th));

        return {ix, iy};
    }
    
    //--------------------------------------------------------------------------
    ipoint2 screen_to_grid(point2 const p) const {
        return screen_to_grid(p.x, p.y);
    }

    //--------------------------------------------------------------------------
    point2 grid_to_screen(int const x, int const y) const {
        auto const tw = sheet_->tile_width();
        auto const th = sheet_->tile_height();
        auto const z  = zoom_;

        return {
            (x + 0.5f) * tw * z - scroll_x_
          , (y + 0.5f) * th * z - scroll_y_
        };
    }

    //--------------------------------------------------------------------------
    point2 grid_to_screen(ipoint2 const p) const {
        return grid_to_screen(p.x, p.y);
    }

    //--------------------------------------------------------------------------
    void center_on_grid(int const x, int const y) {
        auto const z  = zoom_;
        auto const dw = display_w_;
        auto const dh = display_h_;

        auto const p = grid_to_screen(x, y);

        auto const dx = dw / 2.0f - p.x;
        auto const dy = dh / 2.0f - p.y;

        display_x_ = dx / z;
        display_y_ = dy / z;
    }

    //--------------------------------------------------------------------------
    void center_on_grid(ipoint2 const p) {
        center_on_grid(p.x, p.y);
    }

    //--------------------------------------------------------------------------
    float zoom() const noexcept { return zoom_; }

    //--------------------------------------------------------------------------
    vec2 translation() const noexcept {
        return {display_x_ + scroll_x_, display_y_ + scroll_y_};
    }

    //--------------------------------------------------------------------------
    vec2 scale() const noexcept {
        return {zoom_, zoom_};
    }
private:
    tile_sheet const* sheet_;

    float display_w_ = 0.0f;
    float display_h_ = 0.0f;
    float display_x_ = 0.0f;
    float display_y_ = 0.0f;

    float scroll_x_  = 0.0f;
    float scroll_y_  = 0.0f;

    float zoom_      = 1.0f;
};

//==============================================================================
//! Polymorphic base for all input modes.
//==============================================================================
class input_mode_base {
public:
    using mouse_move_info   = bkrl::application::mouse_move_info;
    using mouse_button_info = bkrl::application::mouse_button_info;

    using exit_mode_handler = std::function<void ()>;

    virtual ~input_mode_base() = default;
    input_mode_base() = delete;

    explicit input_mode_base(exit_mode_handler on_exit_mode)
      : on_exit_ {std::move(on_exit_mode)}
    {
    }

    virtual void on_command(command_type const cmd) = 0;
    virtual void on_mouse_move(mouse_move_info const& info) = 0;
    virtual void on_mouse_button(mouse_button_info const& info) = 0;
protected:
    void do_on_exit_() const {
        on_exit_();
    }
private:
    exit_mode_handler on_exit_;
};

//==============================================================================
//! Direction selection input mode.
//==============================================================================
class input_mode_direction final : public input_mode_base {
public:
    using input_mode_base::input_mode_base;

    using completion_handler = std::function<void (bool cancel, ivec2 dir)>;

    //--------------------------------------------------------------------------
    input_mode_base* enter_mode(completion_handler handler) {
        handler_ = std::move(handler);
        dir_ = ivec2 {0, 0};
        ok_  = false;

        return this;
    }

    //--------------------------------------------------------------------------
    void on_command(command_type const cmd) override {
        bool done = true;
        
        switch (cmd) {
        default:
            done = false;
            break;
        case command_type::cancel :
            break;
        case command_type::here :
            ok_ = true;
            dir_ = ivec2 {0, 0};
            break;
        case command_type::north      : dir_ = ivec2 { 0, -1}; break;
        case command_type::south      : dir_ = ivec2 { 0,  1}; break;
        case command_type::east       : dir_ = ivec2 { 1,  0}; break;
        case command_type::west       : dir_ = ivec2 {-1,  0}; break;
        case command_type::north_west : dir_ = ivec2 {-1, -1}; break;
        case command_type::north_east : dir_ = ivec2 { 1, -1}; break;
        case command_type::south_west : dir_ = ivec2 {-1,  1}; break;
        case command_type::south_east : dir_ = ivec2 { 1,  1}; break;
        }

        if (dir_.x || dir_.y) {
            ok_ = true;
        }

        if (done) {
            handler_(!ok_, dir_);
            do_on_exit_();
        }
    }
    
    //--------------------------------------------------------------------------
    void on_mouse_move(mouse_move_info const& info) override {
    }
    
    //--------------------------------------------------------------------------
    void on_mouse_button(mouse_button_info const& info) override {
    }
private:
    completion_handler handler_;
    ivec2 dir_ = ivec2 {0, 0};
    bool  ok_  = false;
};

} //namespace bkrl

//==============================================================================
//==============================================================================
struct engine_client::impl_t {
public:
    enum : int {
        level_w   = 100
      , level_h   = 100
      , font_size = 20
    };

    void init_sinks() {
        app_.on_command([&](command_type const cmd) {
            on_command(cmd);
        });

        app_.on_resize([&](unsigned const w, unsigned const h) {
            on_resize(w, h);
        });

        app_.on_mouse_move([&](application::mouse_move_info const& info) {
            on_mouse_move(info);
        });

        app_.on_mouse_button([&](application::mouse_button_info const& info) {
            on_mouse_button(info);
        });

        app_.on_mouse_wheel([&](application::mouse_wheel_info const& info) {
            if (info.dy < 0) {
                on_command(command_type::zoom_out);
            } else if (info.dy > 0) {
                on_command(command_type::zoom_in);
            }
        });
    }

    void init_map() {
        player_.move_to(level_.up_stair());
    }

    void print_message(string_ref const msg) {
        constexpr auto width  = 1024;
        constexpr auto height = 100;

        last_message_ = transitory_text_layout {font_face_, msg, width, height};
    }

    void print_message(message_type const msg, hash_t lang = 0) {
        print_message(messages_(msg, lang));
    }

    explicit impl_t(config const& cfg)
      : random_substantive_ {cfg.substantive_seed}
      , random_trivial_     {cfg.trivial_seed}
      , app_                {file_key_map, cfg}
      , renderer_           {app_}
      , font_lib_           {}
      , font_face_          {renderer_, font_lib_, "", font_size}
      , messages_           {file_messages_jp}
      , last_message_       {}
      , sheet_              {file_texture_map, renderer_}
      , view_               {sheet_, app_.client_width(), app_.client_height()}
      , level_              {random_substantive_, random_trivial_, sheet_, level_w, level_h}
      , player_             {}
      , input_mode_         {nullptr}
      , imode_direction_    {[&] {input_mode_ = nullptr;}}
      , materials_          {file_materials}
      , materials_strings_  {file_materials_strings}
    {
        ////////////////////////////////////////////////////
        init_sinks();
        init_map();

        ////////////////////////////////////////////////////
        view_.center_on_grid(player_.position());

        ////////////////////////////////////////////////////
        print_message(message_type::welcome);

        ////////////////////////////////////////////////////
        while (app_.is_running()) {
            app_.do_all_events();
            render(renderer_);
        }
    }

    void render(renderer& r) {
        r.clear();

        r.set_translation(view_.translation());
        r.set_scale(view_.scale());

        level_.render(r);

        auto const player_pos = player_.position();
        sheet_.render(r, 1, 0, player_pos.x, player_pos.y);

        last_message_.render(r, font_face_, 1, 1);

        r.present();
    }

    void do_set_door_state(bool const opened) {
        auto const p = player_.position();

        auto msg = (opened)
          ? level_.open_doors(p)
          : level_.close_doors(p);

        if (msg != message_type::none) {
            print_message(msg);
        }

        if (msg != message_type::direction_prompt) {
            return;
        }

        input_mode_ = imode_direction_.enter_mode([this, p, opened](bool const cancel, ivec2 const dir) {
            if (cancel) {
                print_message(message_type::canceled);
                return;
            }
            
            if ((dir.x == 0) && (dir.y == 0)) {
                print_message(message_type::door_blocked);
                return;
            }

            auto const result_msg = (opened)
                ? level_.open_door(p + dir)
                : level_.close_door(p + dir);

            if (result_msg != message_type::none) {
                print_message(result_msg);
            }
        });        
    }

    void do_open() {
        do_set_door_state(true);
    }

    void do_close() {
        do_set_door_state(false);
    }

    void do_move_player(int const dx, int const dy) {
        if (!level_.can_move_to(player_, dx, dy)) {
            return;
        }

        player_.move_by(dx, dy);
        view_.scroll_by_tile(dx, dy);
    }

    void do_scroll(int const dx, int const dy) {
        constexpr auto delta = 4.0f;

        auto const scroll_x = static_cast<float>(dx) * delta;
        auto const scroll_y = static_cast<float>(dy) * delta;

        view_.scroll_by(scroll_x, scroll_y);
    }

    void do_zoom_in() {
        view_.zoom_to(view_.zoom() * 1.1f);
    }

    void do_zoom_out() {
        view_.zoom_to(view_.zoom() * 0.9f);
    }

    void do_zoom_reset() {
        view_.zoom_to(1.0f, player_.position());
    }

    void do_go_up() {
        auto const msg = level_.go_up(player_.position());

        if (msg != message_type::none) {
            print_message(msg);
            return;
        }

        level_ = level {random_substantive_, random_trivial_, sheet_, level_w, level_h};

        init_map();
        do_zoom_reset();
    }

    void do_go_down() {
        auto const msg = level_.go_down(player_.position());

        if (msg != message_type::none) {
            print_message(msg);
            return;
        }

        level_ = level {random_substantive_, random_trivial_, sheet_, level_w, level_h};

        init_map();
        do_zoom_reset();
    }

    void on_command(command_type const cmd) {
        if (input_mode_) {
            input_mode_->on_command(cmd);
            return;
        }

        switch (cmd) {
        case command_type::open       : do_open(); break;
        case command_type::close      : do_close(); break;
        case command_type::scroll_n   : do_scroll( 0,  1); break;
        case command_type::scroll_s   : do_scroll( 0, -1); break;
        case command_type::scroll_e   : do_scroll(-1,  0); break;
        case command_type::scroll_w   : do_scroll( 1,  0); break;
        case command_type::north      : do_move_player (0, -1); break;
        case command_type::south      : do_move_player( 0,  1); break;
        case command_type::east       : do_move_player( 1,  0); break;
        case command_type::west       : do_move_player(-1,  0); break;
        case command_type::north_west : do_move_player(-1, -1); break;
        case command_type::north_east : do_move_player( 1, -1); break;
        case command_type::south_west : do_move_player(-1,  1); break;
        case command_type::south_east : do_move_player( 1,  1); break;
        case command_type::up         : do_go_up(); break;
        case command_type::down       : do_go_down(); break;
        case command_type::zoom_in    : do_zoom_in(); break;
        case command_type::zoom_out   : do_zoom_out(); break;
        case command_type::zoom_reset : do_zoom_reset(); break;
        default:
            break;
        }
    }

    void on_resize(unsigned const w, unsigned const h) {
        view_.set_size(static_cast<float>(w), static_cast<float>(h));
    }

    void on_mouse_move(application::mouse_move_info const& info) {
        auto const right = (info.state & (1<<2)) != 0;
        if (!right) {
            return;
        }

        view_.scroll_by(
            static_cast<float>(info.dx)
          , static_cast<float>(info.dy)
        );
    }

    void on_mouse_button(application::mouse_button_info const& info) {
        //if (info.state != application::mouse_button_info::state_t::pressed) {
        //    return;
        //}
        //
        //if (info.button != 1) {
        //    return;
        //}

        //auto const q = screen_to_grid(info.x, info.y);
        //if (q.x < 0 || q.y < 0) {
        //    return;
        //}

        //auto const p = grid_point {
        //    static_cast<grid_index>(q.x)
        //  , static_cast<grid_index>(q.y)
        //};

        //if (!map_.is_valid(p)) {
        //    return;
        //}

        //auto const tex_type      = map_.get(attribute::texture_type, p);
        //auto const tex_type_str  = enum_map<texture_type>::get(tex_type);
        //auto const room_id       = map_.get(attribute::room_id, p);
        //auto const base_type     = map_.get(attribute::tile_type, p);
        //auto const base_type_str = enum_map<tile_type>::get(base_type);
        //auto const tex_id        = map_.get(attribute::texture_id, p);

        //std::cout << "===================="                      << "\n";
        //std::cout << "| (" << p.x << ", " << p.y << ")"          << "\n";
        //std::cout << "--------------------"                      << "\n";
        //std::cout << "| tile_type    = " << base_type_str.string << "\n";
        //std::cout << "| texture_type = " << tex_type_str.string  << "\n";
        //std::cout << "| texture_id   = " << tex_id               << "\n";
        //std::cout << "| room_id      = " << room_id              << "\n";
        //std::cout << "===================="                      << "\n";
        //std::cout << std::endl;
    }
private:
    random::generator random_substantive_;
    random::generator random_trivial_;

    application app_;
    renderer    renderer_;
    
    font_libary font_lib_;
    font_face   font_face_;

    message_map messages_;

    transitory_text_layout last_message_;

    tile_sheet sheet_;

    view   view_;

    level  level_;

    player player_;

    input_mode_base* input_mode_;
    input_mode_direction imode_direction_;

    definition<item_material_def>       materials_;
    localized_string<item_material_def> materials_strings_;
};

engine_client::~engine_client() = default;

engine_client::engine_client(bkrl::config const& conf)
  : impl_ {std::make_unique<impl_t>(conf)}
{
}

void engine_client::on_command(bkrl::command_type const cmd) {
    impl_->on_command(cmd);
}

void engine_client::render(bkrl::renderer& r) {
    impl_->render(r);
}
