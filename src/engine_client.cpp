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

template <bkrl::tile_type>
texture_type get_texture(grid_storage& grid, grid_index const x, grid_index const y);

template <>
texture_type get_texture<tile_type::door>(
    grid_storage&    //grid
  , grid_index const //x
  , grid_index const //y
) {
    return texture_type::door_closed;
}

template <>
texture_type get_texture<tile_type::floor>(
    grid_storage&    //grid
  , grid_index const //x
  , grid_index const //y
) {
    return texture_type::floor;
}

template <>
texture_type get_texture<tile_type::wall>(
    grid_storage&    grid
  , grid_index const x
  , grid_index const y
) {
    //auto const wall = check_grid_block5(grid, x, y, attribute::tile_type, tile_type::wall);
    auto const wall = check_grid_block5f(grid, x, y, attribute::tile_type, [](tile_type const type) {
        return type == tile_type::wall || type == tile_type::door;
    });

    switch (wall) {
    case 0 : return texture_type::wall_none;

    case (1<<0) : return texture_type::wall_n;
    case (1<<1) : return texture_type::wall_w;
    case (1<<2) : return texture_type::wall_e;
    case (1<<3) : return texture_type::wall_s;

    case (1<<0)|(1<<3) : return texture_type::wall_ns;
    case (1<<1)|(1<<2) : return texture_type::wall_ew;
    case (1<<2)|(1<<3) : return texture_type::wall_se;
    case (1<<1)|(1<<3) : return texture_type::wall_sw;
    case (1<<0)|(1<<2) : return texture_type::wall_ne;
    case (1<<0)|(1<<1) : return texture_type::wall_nw;

    case (1<<0)|(1<<2)|(1<<3) : return texture_type::wall_nse;
    case (1<<0)|(1<<1)|(1<<3) : return texture_type::wall_nsw;
    case (1<<0)|(1<<1)|(1<<2) : return texture_type::wall_new;
    case (1<<1)|(1<<2)|(1<<3) : return texture_type::wall_sew;

    case (1<<0)|(1<<1)|(1<<2)|(1<<3) : return texture_type::wall_nsew;

    default: BK_TODO_FAIL(); break;
    }

    return texture_type::wall_nsew;
}

texture_type get_texture(
    grid_storage&      grid
  , grid_index const   x
  , grid_index const   y
) {
    auto const type = grid.get(attribute::tile_type, x, y);

    switch (type) {
    case tile_type::wall  : return get_texture<tile_type::wall>(grid, x, y);
    case tile_type::floor : return get_texture<tile_type::floor>(grid, x, y);
    case tile_type::door  : return get_texture<tile_type::door>(grid, x, y);
    }

    return texture_type::invalid;
}

void set_texture_type(grid_storage& grid) {
    for_each_xy(grid, [&](grid_index const x, grid_index const y) {
        auto const tex = get_texture(grid, x, y);
        grid.set(attribute::texture_type, x, y, tex);
    });
}

void set_texture_id(grid_storage& grid, texture_map const& map) {
    for_each_xy(grid, [&](grid_index const x, grid_index const y) {
        auto const type = grid.get(attribute::texture_type, x, y);
        auto const id   = map[type];
        grid.set(attribute::texture_id, x, y, id);    
    });
}

class entity {
public:
    entity()
        : position {{0, 0}}
    {
    }

    bkrl::grid_point position;
};

void merge_walls(grid_storage& grid, grid_region const bounds) {
    static auto const can_merge = [](unsigned const n) {
        constexpr auto s0 = (1<<0)|(1<<1)|(1<<2);
        constexpr auto s1 = (1<<0)|(1<<3)|(1<<5);
        constexpr auto s2 = (1<<2)|(1<<4)|(1<<7);
        constexpr auto s3 = (1<<5)|(1<<6)|(1<<7);
        constexpr auto s4 = (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6);

        return (n == s4)
            || (((n & s0) == s0) && ((n & (1 << 6)) == 0))
            || (((n & s1) == s1) && ((n & (1 << 4)) == 0))
            || (((n & s2) == s2) && ((n & (1 << 3)) == 0))
            || (((n & s3) == s3) && ((n & (1 << 1)) == 0));
    };

    bkrl::for_each_edge(bounds, [&](grid_index const x, grid_index const y) {
        auto const walls = check_grid_block9f(grid, x, y, attribute::tile_type, [](tile_type const type) {
            return type == tile_type::wall || type == tile_type::door;
        });

        //auto const walls = check_grid_block9(grid, x, y, attribute::tile_type, tile_type::wall);

        if (can_merge(walls)) {
            auto const floors = check_grid_block9(grid, x, y, attribute::tile_type, tile_type::floor);

            if (floors) {
                grid.set(attribute::tile_type, x, y, bkrl::tile_type::floor);
            } else {
                grid.set(attribute::tile_type, x, y, bkrl::tile_type::invalid);
            }
        }
    });
}

//MSB first
template <int Bit, int... Bits>
struct binary_number {
    enum : unsigned {
        value = (Bit << sizeof...(Bits)) | binary_number<Bits...>::value
    };
};

template <> struct binary_number<0> {
    enum : unsigned { value = 0 };
};

template <> struct binary_number<1> {
    enum : unsigned { value = 1 };
};

void connect_rooms(
    random::generator& gen
  , grid_storage& map
  , grid_region const& bounds
  , room const& src_room
  , room const& dst_room
) {
    constexpr auto segment_randomness = 2;
    constexpr auto min_segment_len = 1u;
    constexpr auto max_segment_len = 10u;

    auto const beg = src_room.center();
    auto const end = dst_room.center();
    auto       cur = beg;

    auto const src_id = src_room.id();
    auto const dst_id = dst_room.id();

    //
    // generate a path segment length
    //
    auto const gen_segment_len = [&](signed delta) {
        while (delta == 0) {
            delta = random::uniform_range(
                gen, -segment_randomness, segment_randomness
            );
        }

        auto const min = min_segment_len;
        auto const max = static_cast<unsigned>(std::abs(delta));

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
    auto const choose_direction = [&](point2d<int> const p) {
        auto const roll = random::uniform_range(gen, 0u, 100u);
        return (roll < 50u)
          ? std::pair<grid_index&, int> {cur.x, p.x}
          : std::pair<grid_index&, int> {cur.y, p.y};
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

        auto const c0 = (n & i_Nx|i_NE|i_xE) == (i_Nx|i_xE);
        auto const c1 = (n & i_Sx|i_SW|i_xW) == (i_Sx|i_xW);
        auto const c2 = (n & i_Nx|i_NW|i_xW) == (i_Nx|i_xW);
        auto const c3 = (n & i_Sx|i_SE|i_xE) == (i_Sx|i_xE);

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

    for (auto limit = 0; limit < 100; ++limit) {
        auto const delta = end - cur;
        auto const dir   = choose_direction(delta);
        auto&      out   = dir.first;
        auto const seg   = gen_segment_len(dir.second);
        auto const len   = seg.first;
        auto const sign  = seg.second;

        for (auto i = 0u; i < len; ++i) {
            if (cur.x <  bounds.left
             || cur.x >= bounds.right
             || cur.y <  bounds.top
             || cur.y >= bounds.bottom
            ) {
                break;
            }

            auto const type = map.get(attribute::tile_type, cur);
            if (type == tile_type::invalid) {
                map.set(attribute::tile_type, cur, tile_type::floor); //TODO
            } else if (type == tile_type::wall) {
                if (!can_tunnel(cur)) {
                    break;   
                }

                auto const doors = check_grid_block9(map, cur.x, cur.y, attribute::tile_type, tile_type::door);
                if (doors) {
                    break;
                }
                
                map.set(attribute::tile_type, cur, tile_type::door); //TODO
            }

            auto const id = map.get(attribute::room_id, cur);
            if (id == dst_id) {
                return;
            }

            out += sign;
        }
    }
}


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
        app_.on_command([&](command_type const cmd) {
            on_command(cmd);
        });

        app_.on_resize([&](unsigned const w, unsigned const h) {
            on_resize(w, h);
        });

        app_.on_mouse_move([&](signed const dx, signed const dy, std::bitset<8> const buttons) {
            on_mouse_move(dx, dy, buttons);
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

        layout.connect(gen, [&](grid_region const& bounds, unsigned const id0, unsigned const id1) {
            //std::cout << "connect\t" << id0 << "\t-> " << id1 << std::endl;
            
            //if (id0 == 1) {
                connect_rooms(gen, map_, bounds, rooms[id0-1], rooms[id1-1]);
            //}
            return true;
        });

        for (auto const& room : rooms) {
            merge_walls(map_, room.bounds());
        }

        set_texture_type(map_);
        set_texture_id(map_, texture_map_);

        //TODO temp
        auto const& r0 = rooms[0];
        auto const& bounds = r0.bounds();
        player_.position.x = bounds.left + 1;
        player_.position.y = bounds.top + 1;


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

                //auto const type    = map_.get(attribute::tile_type,  x, y);
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
        case tile_type::invalid :
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

    void set_door_data(grid_point const p, door_data const door, texture_type const texture) {
        map_.set(attribute::data, p, door);
        map_.set(attribute::texture_type, p, texture);
        map_.set(attribute::texture_id, p, texture_map_[texture]);
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
            set_door_data(where, door, texture_type::door_opened);
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
            set_door_data(where, door, texture_type::door_closed);
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
