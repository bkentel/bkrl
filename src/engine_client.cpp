#include "engine_client.hpp"

#include "sdl.hpp"
#include "grid.hpp"
#include "command_type.hpp"
#include "random.hpp"
#include "generate.hpp"
#include "bsp_layout.hpp"

using bkrl::engine_client;
using bkrl::command_type;
namespace random = bkrl::random;

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
    //return texture_type::invalid;
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
    //return texture_type::invalid;
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

//------------------------------------------------------------------------------
//! update the texture type for the tile at @p p.
//------------------------------------------------------------------------------
void
update_texture_type(
    grid_storage&    grid
  , grid_point const p
) {
    auto const type     = grid.get(attribute::tile_type, p);
    auto const tex_type = get_texture_type(grid, p, type);

    grid.set(attribute::texture_type, p, tex_type);
}

//------------------------------------------------------------------------------
//! update the texture type for every tile in @p grid.
//------------------------------------------------------------------------------
void
update_texture_type(
    grid_storage& grid
) {
    for_each_xy(grid, [&](grid_index const x, grid_index const y) {
        update_texture_type(grid, grid_point {x, y});
    });
}

//------------------------------------------------------------------------------
//! update the texture id for the tile at @p p using the mappings in @p map.
//------------------------------------------------------------------------------
void
update_texture_id(
    grid_storage&      grid
  , texture_map const& map
  , grid_point const   p
) {
    auto const type = grid.get(attribute::texture_type, p);
    auto const id   = map[type];
    grid.set(attribute::texture_id, p, id);
}

//------------------------------------------------------------------------------
//! update the texture id for every tile in @p grid using the mappings in @p map.
//------------------------------------------------------------------------------
void
update_texture_id(
    grid_storage&     grid
 , texture_map const& map
) {
    for_each_xy(grid, [&](grid_index const x, grid_index const y) {
        update_texture_id(grid, map, grid_point {x, y});
    });
}


//------------------------------------------------------------------------------
//! update the texture type and id for the tile at @p p and each of its 8 neighbors.
//------------------------------------------------------------------------------
void
update_grid(
    grid_storage&      grid
  , texture_map const& map
  , grid_point const   p
) {
    auto const x = {p.x - 1, p.x, p.x + 1};
    auto const y = {p.y - 1, p.y, p.y + 1};

    for (auto iy : y) {
        for (auto ix : x) {
            auto const ip = grid_point {ix, iy};

            if (!grid.is_valid(ip)) {
                break;
            }

            update_texture_type(grid, ip);
            update_texture_id(grid, map, ip);
        }
    }
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
      , [](tile_type const type) {
            return type == tile_type::wall || type == tile_type::door;
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

//------------------------------------------------------------------------------
//! attempt to connect @p src_room to @p dst_room with the route constrained
//! to the region given by @p bounds.
//------------------------------------------------------------------------------
bool
connect_rooms(
    random::generator& gen
  , grid_storage&      map
  , grid_region const& bounds
  , room        const& src_room
  , room        const& dst_room
) {
    constexpr auto segment_randomness = 2;
    constexpr auto min_segment_len = 2u;
    constexpr auto max_segment_len = 10u;

    auto const beg = src_room.center();
    auto const end = dst_room.center();
    auto       cur = beg;

    auto const src_id = src_room.id();
    auto const dst_id = dst_room.id();

    //
    // generate a path segment length
    //
    auto const gen_segment_len = [&](signed delta, signed n) {
        while (std::abs(delta) < n) {
            delta += random::uniform_range(
                gen, -segment_randomness, segment_randomness
            );
        }

        auto const mag = static_cast<unsigned>(std::abs(delta));

        auto const min = std::min(max_segment_len, std::max(mag, min_segment_len));
        auto const max = std::min(mag, max_segment_len);

        auto const roll = random::uniform_range(gen, min, max);

        auto const sign = (delta > 0) ? 1 : -1;

        return (roll > max_segment_len)
          ? std::make_pair(max_segment_len, sign)
          : std::make_pair(roll, sign);
    };

    //
    // choose a direction to move
    // p - delta between cur and dst
    // returns pair {cur.x or cur.y, p.x or p.y}
    //
    auto const choose_direction = [&](point2d<int> const delta) {
        auto const mag_x = std::abs(delta.x);
        auto const mag_y = std::abs(delta.y);

        if (mag_x > 2*mag_y) {
            return std::pair<grid_index&, int> {cur.x, delta.x};
        } else if (mag_y > 2*mag_x) {
            return std::pair<grid_index&, int> {cur.y, delta.y};
        }

        auto const roll = random::uniform_range(gen, 0u, 100u);
        return (roll < 50u)
           ? std::pair<grid_index&, int> {cur.x, delta.x}
           : std::pair<grid_index&, int> {cur.y, delta.y};
    };

    auto const can_tunnel = [&](grid_point const cur) {
        auto const n = check_grid_block9f(map, cur.x, cur.y, attribute::tile_type, [](tile_type const type) {
            return type == tile_type::wall || type == tile_type::door;
        });

        constexpr auto i_NW = (1<<0);
        constexpr auto i_Nx = (1<<1);
        constexpr auto i_NE = (1<<2);
        constexpr auto i_xW = (1<<3);
        constexpr auto i_xE = (1<<4);
        constexpr auto i_SW = (1<<5);
        constexpr auto i_Sx = (1<<6);
        constexpr auto i_SE = (1<<7);

        auto const c0 = (n & (i_Nx|i_NE|i_xE)) == (i_Nx|i_xE);
        auto const c1 = (n & (i_Sx|i_SW|i_xW)) == (i_Sx|i_xW);
        auto const c2 = (n & (i_Nx|i_NW|i_xW)) == (i_Nx|i_xW);
        auto const c3 = (n & (i_Sx|i_SE|i_xE)) == (i_Sx|i_xE);

        auto const c4 = (n & ~(i_SW|i_Sx|i_SE)) == (i_Nx|i_xW|i_xE);
        auto const c5 = (n & ~(i_NE|i_xE|i_SE)) == (i_Nx|i_xW|i_Sx);
        auto const c6 = (n & ~(i_NW|i_Nx|i_NE)) == (i_xW|i_xE|i_Sx);
        auto const c7 = (n & ~(i_NW|i_xW|i_SW)) == (i_Nx|i_xE|i_Sx);

        auto const c8 = n == (i_Nx|i_xW|i_xE|i_Sx);

        if (c0 || c1 || c2 || c3 || c4 || c5 || c6 || c7 || c8) {
            return false;
        }

        return true;
    };

    for (auto fail_limit = 0; fail_limit < 10;) {
        auto const delta = end - cur;
        auto const dir   = choose_direction(delta);
        auto&      out   = dir.first;
        auto const seg   = gen_segment_len(dir.second, fail_limit + 1);
        auto       len   = seg.first;
        auto const sign  = seg.second;

        for (auto i = 0u; i < len; ++i) {
            BK_ASSERT(bounds.contains(cur));

            auto const type = map.get(attribute::tile_type, cur);
            if (type == tile_type::invalid) {
                map.set(attribute::tile_type, cur, tile_type::corridor); //TODO
            } else if (type == tile_type::wall) {
                if (!can_tunnel(cur)) {
                    out -= sign;
                    fail_limit++;
                    break;
                }

                auto const doors = check_grid_block5(map, cur.x, cur.y, attribute::tile_type, tile_type::door);
                if (doors) {
                    out -= sign;
                    fail_limit++;
                    break;
                }
                
                map.set(attribute::tile_type, cur, tile_type::door); //TODO
            }

            auto const id = map.get(attribute::room_id, cur);
            if (id == dst_id) {
                return true;
            }

            out += sign;
            if (!bounds.contains(cur)) {
                out -= sign;
                fail_limit++;
                break;
            }
        }
    }

    return false;
}


class entity {
public:
    entity()
        : position {{0, 0}}
    {
    }

    bkrl::grid_point position;
};

} //namespace bkrl

struct engine_client::impl_t {
public:
    impl_t()
      : app_ {}
      , renderer_ {app_.handle()}
      , map_ {100, 100}
      , sheet_ {288, 288, 18, 18}
      , texture_map_ {bkrl::read_file("./data/texture_map.json")}
    {
        ////////////////////////////////////////////////////
        app_.on_command([&](command_type const cmd) {
            on_command(cmd);
        });

        app_.on_resize([&](unsigned const w, unsigned const h) {
            on_resize(w, h);
        });

        app_.on_mouse_move([&](signed const dx, signed const dy, std::bitset<8> const buttons) {
            on_mouse_move(dx, dy, buttons);
        });

        app_.on_mouse_button([&](signed const x, signed const y) {
            on_mouse_button(x, y);
        });

        ////////////////////////////////////////////////////
        random::generator gen {100};

        generate::simple_room room_gen {};
        generate::circle_room circle_gen {};
        
        std::vector<room> rooms;

        auto const on_split = [](grid_region const) {
            return true;
        };

        auto const on_room_gen = [&rooms, &gen, &room_gen](grid_region const bounds, unsigned const id) {
            rooms.emplace_back(room_gen.generate(gen, bounds, id));
            BK_ASSERT(id == rooms.size());
        };

        grid_region const reserve {20, 20, 40, 40};

        auto layout = bsp_layout::generate(
            gen
          , on_split
          , on_room_gen
          , bsp_layout::params_t {}
          , reserve
        );

        for (auto const& room : rooms) {
            auto const x = room.bounds().left;
            auto const y = room.bounds().top;

            map_.write(room, grid_point {x, y});
        }

        for (auto const& room : rooms) {
            merge_walls(map_, room.bounds());
        }

        layout.connect(gen, [&](grid_region const& bounds, unsigned const id0, unsigned const id1) {
            auto const connected = connect_rooms(gen, map_, bounds, rooms[id0-1], rooms[id1-1]);
            if (connected) {
                //std::cout << "connected " << id0 << " -> " << id1 << std::endl;
            } else {
                std::cout << "failed    " << id0 << " -> " << id1 << std::endl;
            }

            return true;
        });

        update_texture_type(map_);
        update_texture_id(map_, texture_map_);

        //TODO temp
        auto const& r0 = rooms[0];
        player_.position = r0.center();

        set_zoom(zoom_);

        while (app_) {
            app_.pump_events();
            render(renderer_);
        }
    }

    void render(renderer& r) {
        r.clear();

        r.set_translation(display_x_ + scroll_x_, display_y_ + scroll_y_);
        r.set_scale(zoom_, zoom_);

        auto const w = map_.width();
        auto const h = map_.height();

        for (bkrl::grid_index y = 0; y < h; ++y) {
            for (bkrl::grid_index x = 0; x < w; ++x) {
                auto const texture = map_.get(attribute::texture_id, x, y);
                r.draw_tile(sheet_, texture, x, y);
            }
        }

        r.draw_tile(sheet_, 1, player_.position.x, player_.position.y);

        r.present();
    }

    void scroll(signed const dx, signed const dy) {
        auto const delta_x = dx * (1.0f / zoom_);
        auto const delta_y = dy * (1.0f / zoom_);

        scroll_x_ += delta_x;
        scroll_y_ += delta_y;
    }

    bool can_move_to(entity const& e, signed const dx, signed const dy) {
        if (std::abs(dx) > 1 || std::abs(dy) > 1) {
            BK_TODO_FAIL();
        }

        auto const to = translate(e.position, dx, dy);

        if (!map_.is_valid(to)) {
            return false;
        }

        auto const type = map_.get(attribute::tile_type, to);

        switch (type) {
        case tile_type::floor :
        case tile_type::corridor :
            return true;
        case tile_type::door :
            return door_data {map_, to}.is_open();
        }

        return false;
    }

    void move_player(signed const dx, signed const dy) {
        if (!can_move_to(player_, dx, dy)) {
            return;
        }

        player_.position.x += dx;
        player_.position.y += dy;

        set_zoom(zoom_);

        std::cout << player_.position.x << " " << player_.position.y << std::endl;
    }

    std::bitset<8> get_doors_next_to(grid_point const p) const {
        return {
            check_grid_block9(map_, p.x, p.y, attribute::tile_type, tile_type::door)
        };
    }

    void set_door_data(grid_point const p, door_data const door) {
        map_.set(attribute::data, p, door);
        update_grid(map_, texture_map_, p);
        //update_texture_type(map_, p);
        //update_texture_id(map_, texture_map_, p);
    }

    void do_open() {
        auto const p     = player_.position;
        auto const doors = get_doors_next_to(p);
        auto const n     = doors.count();

        if (n == 1) {
            //TODO change grid_check_to_point to accept bitset
            auto const where = grid_check_to_point(p, doors.to_ulong());

            door_data door {map_, where};
            if (door.is_open()) {
                return;
            }

            door.open();
            set_door_data(where, door);
        } else if (n > 1) {
            //TODO
        }
    }

    void do_close() {
        auto const p     = player_.position;
        auto const doors = get_doors_next_to(p);
        auto const n     = doors.count();

        if (n == 1) {
            //TODO change grid_check_to_point to accept bitset
            auto const where = grid_check_to_point(p, doors.to_ulong());

            door_data door {map_, where};
            if (door.is_closed()) {
                return;
            }

            door.close();
            set_door_data(where, door);
        } else if (n > 1) {
            //TODO
        }
    }

    void on_command(command_type const cmd) {
        switch (cmd) {
        case command_type::open :
            do_open();
            break;
        case command_type::close :
            do_close();
            break;
        case command_type::scroll_n :
            scroll(0, 4);
            break;
        case command_type::scroll_s :
            scroll(0, -4);
            break;
        case command_type::scroll_e :
            scroll(-4, 0);
            break;
        case command_type::scroll_w :
            scroll(4, 0);
            break;
        case command_type::north :
            move_player(0, -1);
            break;
        case command_type::south :
            move_player(0, 1);
            break;
        case command_type::east :
            move_player(1, 0);
            break;
        case command_type::west :
            move_player(-1, 0);
            break;
        case command_type::north_west :
            move_player(-1, -1);
            break;
        case command_type::north_east :
            move_player(1, -1);
            break;
        case command_type::south_west :
            move_player(-1, 1);
            break;
        case command_type::south_east :
            move_player(1, 1);
            break;
        case command_type::zoom_in :
            set_zoom(zoom_ * 1.1f);
            break;
        case command_type::zoom_out :
            set_zoom(zoom_ * 0.9f);
            break;
        default:
            break;
        }
    }

    void on_resize(unsigned const w, unsigned const h) {
        display_w_ = static_cast<float>(w);
        display_h_ = static_cast<float>(h);

        set_zoom(zoom_);
    }

    //TODO rename this to something more meaningful
    void set_zoom(float const zoom) {
        zoom_ = (zoom < 0.1f)
          ? 0.1f
          : (zoom > 10.0f)
            ? 10.0f
            : zoom;

        auto const sw = sheet_.sprite_width;
        auto const sh = sheet_.sprite_height;

        auto const px = -static_cast<float>(player_.position.x) * sw;
        auto const py = -static_cast<float>(player_.position.y) * sh;

        display_x_ = px + (display_w_ / 2.0f / zoom_);
        display_y_ = py + (display_h_ / 2.0f / zoom_);
    }

    void on_mouse_move(signed const dx, signed const dy, std::bitset<8> const buttons) {
        if (buttons[2]) {
            scroll(dx, dy);
        } else if (buttons.any()) {
            //std::cout << "button";
        }
    }

    void on_mouse_button(signed const x, signed const y) {
        auto const q = screen_to_grid(x, y);
        if (q.x < 0 || q.y < 0) {
            return;
        }

        auto const p = grid_point {
            static_cast<grid_index>(q.x)
          , static_cast<grid_index>(q.y)
        };

        if (!map_.is_valid(p)) {
            return;
        }

        auto const tex_type      = map_.get(attribute::texture_type, p);
        auto const tex_type_str  = enum_map<texture_type>::get(tex_type);
        auto const room_id       = map_.get(attribute::room_id, p);
        auto const base_type     = map_.get(attribute::tile_type, p);
        auto const base_type_str = enum_map<tile_type>::get(base_type);
        auto const tex_id        = map_.get(attribute::texture_id, p);

        std::cout << "===================="                      << "\n";
        std::cout << "| (" << p.x << ", " << p.y << ")"          << "\n";
        std::cout << "--------------------"                      << "\n";
        std::cout << "| tile_type    = " << base_type_str.string << "\n";
        std::cout << "| texture_type = " << tex_type_str.string  << "\n";
        std::cout << "| texture_id   = " << tex_id               << "\n";
        std::cout << "| room_id      = " << room_id              << "\n";
        std::cout << "===================="                      << "\n";
        std::cout << std::endl;
    }

    point2d<int> screen_to_grid(signed const x, signed const y) const {
        auto const dx = display_x_ + scroll_x_;
        auto const dy = display_y_ + scroll_y_;

        auto const w = sheet_.sprite_width;
        auto const h = sheet_.sprite_height;

        auto const ix = static_cast<int>(std::trunc((x / zoom_ - dx) / w));
        auto const iy = static_cast<int>(std::trunc((y / zoom_ - dy) / h));

        return point2d<int> {ix, iy};
    }
private:
    application  app_;
    renderer     renderer_;
    grid_storage map_;

    sprite_sheet sheet_;
    texture_map  texture_map_;

    entity       player_;

    float display_w_ = 0.0f;
    float display_h_ = 0.0f;

    float display_x_ = 0.0f;
    float display_y_ = 0.0f;

    float scroll_x_ = 0.0f;
    float scroll_y_ = 0.0f;

    float zoom_ = 1.0f;
};

engine_client::~engine_client() = default;

engine_client::engine_client()
  : impl_ {std::make_unique<impl_t>()}
{
}

void engine_client::on_command(bkrl::command_type const cmd) {
    impl_->on_command(cmd);
}

void engine_client::render(bkrl::renderer& r) {
    impl_->render(r);
}
