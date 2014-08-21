#include "engine_client.hpp"

#include "sdl.hpp"
#include "grid.hpp"
#include "command_type.hpp"
#include "random.hpp"
#include "generate.hpp"

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
    auto const n = check_grid_block5(grid, x, y, attribute::tile_type, tile_type::wall);

    switch (n) {
    case (1<<2) : return texture_type::wall_none;

    case (1<<0)|(1<<2) : return texture_type::wall_n;
    case (1<<1)|(1<<2) : return texture_type::wall_w;
    case (1<<2)|(1<<3) : return texture_type::wall_e;
    case (1<<2)|(1<<4) : return texture_type::wall_s;

    case (1<<0)|(1<<2)|(1<<4) : return texture_type::wall_ns;
    case (1<<1)|(1<<2)|(1<<3) : return texture_type::wall_ew;
    case (1<<2)|(1<<3)|(1<<4) : return texture_type::wall_se;
    case (1<<1)|(1<<2)|(1<<4) : return texture_type::wall_sw;
    case (1<<0)|(1<<2)|(1<<3) : return texture_type::wall_ne;
    case (1<<0)|(1<<1)|(1<<2) : return texture_type::wall_nw;

    case (1<<0)|(1<<2)|(1<<3)|(1<<4) : return texture_type::wall_nse;
    case (1<<0)|(1<<1)|(1<<2)|(1<<4) : return texture_type::wall_nsw;
    case (1<<0)|(1<<1)|(1<<2)|(1<<3) : return texture_type::wall_new;
    case (1<<1)|(1<<2)|(1<<3)|(1<<4) : return texture_type::wall_sew;

    case (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4) : return texture_type::wall_nsew;

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
        constexpr auto s0 = (1<<4)|(1<<0)|(1<<1)|(1<<2);
        constexpr auto s1 = (1<<4)|(1<<0)|(1<<3)|(1<<6);
        constexpr auto s2 = (1<<4)|(1<<2)|(1<<5)|(1<<8);
        constexpr auto s3 = (1<<4)|(1<<6)|(1<<7)|(1<<8);
        constexpr auto s4 = (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7);

        return (n == s4)
            || (((n & s0) == s0) && ((n & (1 << 7)) == 0))
            || (((n & s1) == s1) && ((n & (1 << 5)) == 0))
            || (((n & s2) == s2) && ((n & (1 << 3)) == 0))
            || (((n & s3) == s3) && ((n & (1 << 1)) == 0));
    };

    bkrl::for_each_edge(bounds, [&](grid_index const x, grid_index const y) {
        auto const walls = check_grid_block9(grid, x, y, attribute::tile_type, tile_type::wall);

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
        app_.on_command([&](bkrl::command_type const cmd) {
            on_command(cmd);
        });

        auto const on_split = [](grid_region const r) {
            return true;
        };

        grid_region const reserve {20, 20, 40, 40};

        bkrl::random::generator gen {100};

        generate::bsp_layout bsp {on_split};
        bsp.generate(gen, reserve);

        generate::simple_room room_gen {};
        generate::circle_room circle_gen {};
        
        std::vector<room> rooms;
        
        for (auto const& r : bsp.nodes_) {
            if (r.child_index == 0) {
                rooms.emplace_back(
                    circle_gen.generate(gen, r.region)
                );
            } else {
                if (!r.is_leaf()) continue;
                if (bkrl::random::uniform_range(gen, 0, 100) < 50) continue;

                rooms.emplace_back(
                    room_gen.generate(gen, r.region)
                );
            }

            auto& last_room = rooms.back();
            auto const x = r.region.left;
            auto const y = r.region.top;

            map_.write(last_room, bkrl::grid_point {x, y});
        }

        for (auto const& r : rooms) {
            merge_walls(map_, r.bounds());
        }

        set_texture_type(map_);
        set_texture_id(map_, texture_map_);

        while (app_) {
            app_.pump_events();
            render(renderer_);
        }
    }

    void render(renderer& r) {
        r.clear();

        auto const w = map_.width();
        auto const h = map_.height();

        for (bkrl::grid_index y = 0; y < h; ++y) {
            for (bkrl::grid_index x = 0; x < w; ++x) {

                auto const type    = map_.get(attribute::tile_type,  x, y);
                auto const texture = map_.get(attribute::texture_id, x, y);

                r.draw_tile(sheet_, texture, x, y);
            }
        }

        r.draw_tile(sheet_, 1, player_.position.x, player_.position.y);

        r.present();
    }

    void on_command(command_type const cmd) {
        switch (cmd) {
        case command_type::scroll_n :
            renderer_.set_translation_y(renderer_.get_translation_y() + 4 * renderer_.get_scale_y());
            break;
        case command_type::scroll_s :
            renderer_.set_translation_y(renderer_.get_translation_y() - 4 * renderer_.get_scale_y());
            break;
        case command_type::scroll_e :
            renderer_.set_translation_x(renderer_.get_translation_x() - 4 * renderer_.get_scale_x());
            break;
        case command_type::scroll_w :
            renderer_.set_translation_x(renderer_.get_translation_x() + 4 * renderer_.get_scale_x());
            break;
        case command_type::north :
            player_.position.y -= 1;
            std::cout << player_.position.x << " " << player_.position.y << std::endl;
            break;
        case command_type::south :
            player_.position.y += 1;
            std::cout << player_.position.x << " " << player_.position.y << std::endl;
            break;
        case command_type::east :
            player_.position.x += 1;
            std::cout << player_.position.x << " " << player_.position.y << std::endl;
            break;
        case command_type::west :
            player_.position.x -= 1;
            std::cout << player_.position.x << " " << player_.position.y << std::endl;
            break;
        case command_type::zoom_in :
            renderer_.set_scale_x(renderer_.get_scale_x() * 1.1f);
            renderer_.set_scale_y(renderer_.get_scale_y() * 1.1f);

            renderer_.set_translation_x(renderer_.get_translation_x() * 1.1f);
            renderer_.set_translation_y(renderer_.get_translation_y() * 1.1f);
            break;
        case command_type::zoom_out :
            renderer_.set_scale_x(renderer_.get_scale_x() * 0.9f);
            renderer_.set_scale_y(renderer_.get_scale_y() * 0.9f);

            renderer_.set_translation_x(renderer_.get_translation_x() * 0.9f);
            renderer_.set_translation_y(renderer_.get_translation_y() * 0.9f);
            break;
        default:
            break;
        }
    }
private:
    bkrl::application  app_;
    bkrl::renderer     renderer_;
    bkrl::grid_storage map_;

    bkrl::sprite_sheet sheet_;
    bkrl::texture_map  texture_map_;

    bkrl::entity       player_;
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
