#include "items.hpp"
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
#include "entity.hpp"
#include "messages.hpp"
#include "time.hpp"
#include "spatial_map.hpp"
#include "scope_exit.hpp"
#include "definitions.hpp"
#include "gui.hpp"

#include <boost/container/static_vector.hpp>
#include <boost/format.hpp>

using bkrl::engine_client;
using bkrl::command_type;
using bkrl::string_ref;
using bkrl::path_string_ref;

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
    case tt::empty     : return get_texture_type<tt::empty>(grid, p);
    case tt::floor     : return get_texture_type<tt::floor>(grid, p);
    case tt::wall      : return get_texture_type<tt::wall>(grid, p);
    case tt::door      : return get_texture_type<tt::door>(grid, p);
    case tt::stair     : return get_texture_type<tt::stair>(grid, p);
    case tt::corridor  : return get_texture_type<tt::corridor>(grid, p);
    case tt::invalid   : //fallthrough
    case tt::enum_size : //fallthrough
    default:
        break;
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
//! One level within the world.
//==============================================================================
class level {
public:
    //--------------------------------------------------------------------------
    level(
        random::generator& substantive
      , random::generator& trivial
      , data_definitions const& definitions
      , item_store&        items
      , player&            player
      , tile_sheet const&  tiles_sheet
      , tile_sheet const&  entities_sheet
      , grid_size const    width
      , grid_size const    height
    )
      : definitions_ {&definitions}
      , item_store_ {&items}
      , tiles_sheet_    {&tiles_sheet}
      , entities_sheet_ {&entities_sheet}
      , player_      {&player}
      , grid_        {width, height}
    {
        generate_(substantive, trivial);
    }
    
    //--------------------------------------------------------------------------
    void advance(random::generator& trivial) {
        for (auto& mob : mobs_) {
            auto const roll = random::percent(trivial);
            if (roll < 25) {
                continue;
            }

            auto const v = random::direction(trivial);
            if (v.x == 0 && v.y == 0) {
                continue;
            }

            if (!can_move_by(mob, v)) {
                continue;
            }

            auto const p = mob.position() + v;
            if (entity_at(p)) {
                continue;
            }

            mob.move_to(p);
            mobs_.sort();
        }

        
    }

    //--------------------------------------------------------------------------
    void draw_map(renderer& r) {
        auto const w = grid_.width();
        auto const h = grid_.height();

        auto const& sheet = *tiles_sheet_;
        
        //set to default color
        r.set_color_mod(sheet.get_texture());
        
        for (grid_index y = 0; y < h; ++y) {
            for (grid_index x = 0; x < w; ++x) {
                auto const i = grid_.get(attribute::texture_id, x, y);
                sheet.render(r, i, x, y);
            }
        }
    }

    //--------------------------------------------------------------------------
    void draw_entities(renderer& r) {
        auto const& sheet = *entities_sheet_;
        auto&       tex   = sheet.get_texture();

        auto const& entities = definitions_->get_entities();
        
        r.set_color_mod(tex, make_color(255, 255, 255));
        auto const player_pos = player_->position();
        sheet.render(r, 13, 13, player_pos.x, player_pos.y);

        for (auto const& mob : mobs_) {
            auto const p = mob.position();
            
            auto const rinfo = mob.render_info(entities);

            //auto const& e = entities.get_definition(mob.id);

            auto const tx = rinfo.tile_x;
            auto const ty = rinfo.tile_y;

            auto const color = make_color(rinfo.r, rinfo.g, rinfo.b);

            r.set_color_mod(tex, color);
            sheet.render(r, tx, ty, p.x, p.y);

            auto const health = mob.health();
            if (health.size() != 0) {
                auto const tw = sheet.tile_width();
                auto const th = sheet.tile_height();

                constexpr auto bar_border = 1;
                constexpr auto bar_size   = 2;
                constexpr auto bar_h      = bar_size + bar_border * 2;

                auto const x = p.x * tw;
                auto const y = p.y * th - bar_h;
                auto const w = tw;
                auto const h = bar_size;

                auto const percent = static_cast<float>(health.hi - health.size()) / health.hi;
                auto const w2 = static_cast<int>(std::round(percent*w));

                r.set_draw_color(make_color(100, 100, 100));
                r.draw_filled_rect(make_rect_size(
                    x - bar_border
                  , y - bar_border
                  , w + bar_border * 2
                  , h + bar_border * 2
                ));

                r.set_draw_color(make_color(255, 50, 50));
                r.draw_filled_rect(make_rect_size(
                    x, y, w2, h
                ));

            }
        }
    }

    //--------------------------------------------------------------------------
    void draw_items(renderer& r) {
        auto const& sheet = *tiles_sheet_;

        for (auto const& i : items_) {
            auto const p = i.first;
            sheet.render(r, 15, 0, p.x, p.y);
        }
    }

    //--------------------------------------------------------------------------
    void render(renderer& r) {
        draw_map(r);
        draw_items(r);
        draw_entities(r);
    }

    //--------------------------------------------------------------------------
    //! @result message_type::get_no_items      - nothing here
    //!         message_type::get_which_prompt  - more than one item
    //!         message_type::none              - ok; just one item
    //--------------------------------------------------------------------------
    message_type can_get_item(ipoint2 const p) const {
        auto const& stack = items_.at(p);
        if (!stack) {
            return message_type::get_no_items;
        }

        auto const size = stack->size();
        BK_ASSERT(size > 0);

        if (size > 1) {
            return message_type::get_which_prompt;
        }

        return message_type::none;
    }

    //--------------------------------------------------------------------------
    //! Get the item at @p index in the item stack at @p p, and remove it from the level.
    //--------------------------------------------------------------------------
    item_id get_item(ipoint2 const p, item_id const iid) {
        auto& stack = require_stack_at_(p);

        auto result = stack.remove(iid);

        if (stack.empty()) {
            items_.remove(p);
        }

        return result;
    }

    optional<item_stack const&> get_stack_at(ipoint2 const p) const {
        return items_.at(p);
    }

    void add_to_stack_(item_stack& stack, item_id const id) {
        auto const& item_defs = definitions_->get_items();
        auto const& items     = *item_store_;

        stack.insert(id, item_defs, items);
    }

    void add_to_stack_(item_stack& stack, item_stack&& other) {
        auto const& item_defs = definitions_->get_items();
        auto const& items     = *item_store_;

        stack.insert(std::move(other), item_defs, items);
    }

    //--------------------------------------------------------------------------
    message_type drop_item_at(ipoint2 const p, item_id const id) {
        add_to_stack_(make_stack_at_(p), id);
        return message_type::none;
    }

    //--------------------------------------------------------------------------
    using adjacency = boost::container::static_vector<ipoint2, 9>;

    adjacency check_adjacent(ipoint2 const p, tile_type const type) const {
        auto const& xi = x_off9;
        auto const& yi = y_off9;

        adjacency result;

        for (size_t i = 0; i < 9; ++i) {
            auto const q = ipoint2 {p.x + xi[i], p.y + yi[i]};
            if (!grid_.is_valid(q)) {
                continue;
            }

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

    //--------------------------------------------------------------------------
    optional<entity const&> entity_at(ipoint2 const p) const {
        if (player_->position() == p) {
            return static_cast<entity const&>(*player_);
        }

        return mobs_.at(p);
    }

    optional<entity&> entity_at(ipoint2 const p) {
        if (player_->position() == p) {
            return static_cast<entity&>(*player_);
        }

        return mobs_.at(p);
    }

    //--------------------------------------------------------------------------
    utf8string attack(random::generator& trivial, entity& subject, entity& object) {
        auto const p = object.position();
        BK_ASSERT_DBG(!!entity_at(p));

        auto const& entities = definitions_->get_entities();
        auto const& msgs = definitions_->get_messages();
        
        auto const& name   = object.name(entities);
        auto const  damage = 1;

        auto const killed = object.apply_damage(damage);
        if (!killed) {
            boost::format fmt {msgs[message_type::attack_regular].data()};
            fmt % name % damage;
            return boost::str(fmt);
        }
        
        if (!object.items().empty()) {
            add_to_stack_(make_stack_at_(p), std::move(object.items()));
        }

        mobs_.remove(p);

        boost::format fmt {msgs[message_type::kill_regular].data()};
        fmt % name;
        return boost::str(fmt);
    }
    //--------------------------------------------------------------------------
    bool can_move_to(entity const& e, ipoint2 const p) const {
        if (!grid_.is_valid(p)) {
            return false;
        }

        auto const tile_ok = [&] {
            auto const type = grid_.get(attribute::tile_type, p);
            switch (type) {
            case tile_type::invalid : //TODO this is just for testing

            case tile_type::floor :
            case tile_type::corridor :
            case tile_type::stair :
                return true;
            case tile_type::door :
                return door_data {grid_, p}.is_open();
            default :
                break;
            }

            return false;
        }();

        if (!tile_ok) {
            return false;
        }

        if (entity_at(p)) {
            return false;
        }

        return true;
    }
    //--------------------------------------------------------------------------
    bool can_move_to(entity const& e, int const x, int const y) const {
        return can_move_to(e, ipoint2 {x, y});
    }
    //--------------------------------------------------------------------------
    bool can_move_by(entity const& e, ivec2 const v) {
        if (std::abs(v.x) > 1 || std::abs(v.y) > 1) {
            BK_TODO_FAIL();
        }

        return can_move_to(e, e.position() + v);
    }
    //--------------------------------------------------------------------------
    bool can_move_by(entity const& e, int const dx, int const dy) {
        return can_move_by(e, ivec2 {dx, dy});
    }
    //--------------------------------------------------------------------------
    ipoint2 down_stair() const noexcept {
        return stairs_down_;
    }
    //--------------------------------------------------------------------------
    ipoint2 up_stair() const noexcept {
        return stairs_up_;
    }
    //--------------------------------------------------------------------------
    message_type open_doors(ipoint2 const p) {
        return set_doors_(p, true);
    }
    //--------------------------------------------------------------------------
    message_type open_door(ipoint2 const p) {
        return set_door_(p, true);
    }
    //--------------------------------------------------------------------------
    message_type close_doors(ipoint2 const p) {
        return set_doors_(p, false);
    }
    //--------------------------------------------------------------------------
    message_type close_door(ipoint2 const p) {
        return set_door_(p, false);
    }
    //--------------------------------------------------------------------------
    utf8string get_inspect_msg(ipoint2 const p)  const {
        if (!grid_.is_valid(p)) {
            return "";
        }

        auto const type = grid_.get(attribute::tile_type, p);
        auto result = enum_map<tile_type>::get(type).string.to_string();

        auto const stack = items_.at(p);
        if (stack) {
            auto const& items = definitions_->get_items();

            for (auto const& itm : *stack) {
                auto const& id = (*item_store_)[itm].id;
                auto const& name = items.get_locale(id).name;

                result.push_back('\n');
                result.append(name);
            }
        }
        
        auto const mob = mobs_.at(p);
        if (mob) {
            auto const& entities = definitions_->get_entities();
            auto const& id       = mob->id;
            auto const& name     = entities.get_locale(id).name;
            auto const& text     = entities.get_locale(id).text;

            result.push_back('\n');
            result.append(name.data(), name.size());
            result.append({" - "});
            result.append(text.data(), text.size());
        }

        return result;
    }
private:
    //--------------------------------------------------------------------------
    //! Return the item stack at p. If no item stack exists already, create one.
    //--------------------------------------------------------------------------
    item_stack& make_stack_at_(ipoint2 const p) {
        auto const stack = items_.at(p);
        if (!stack) {
            items_.insert(p);
            items_.sort(); //TODO
        }

        return *items_.at(p);
    }

    //--------------------------------------------------------------------------
    //! Return the item stack at p. Fail if none exists.
    //--------------------------------------------------------------------------
    item_stack& require_stack_at_(ipoint2 const p) {
        auto const stack = items_.at(p);
        if (!stack) {
            BK_TODO_FAIL();
        }

        return *stack;
    }

    item_stack const& require_stack_at_(ipoint2 const p) const {
        return const_cast<level*>(this)->require_stack_at_(p);
    }

    //--------------------------------------------------------------------------
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
    void generate_rooms_(random::generator& substantive) {
        generate::simple_room room_gen   {};

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
        
        auto params = bsp_layout::params_t {};
        params.width  = grid_.width();
        params.height = grid_.height();

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

        bsp_connector connector;

        layout_.connect(substantive, [&](grid_region const& bounds, unsigned const id0, unsigned const id1) {
            auto const connected = connector.connect(
                substantive
              , grid_
              , bounds
              , rooms[id0-1]
              , rooms[id1-1]
            );

            if (!connected) {
                connector.connect(
                    substantive
                  , grid_
                  , bounds
                  , rooms[id1-1]
                  , rooms[id0-1]
                );
            }

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
        player_->move_to(up_stair());
    }

    //--------------------------------------------------------------------------
    void place_items_(random::generator& substantive) {
        auto& items     = *item_store_;
        auto const& item_defs = definitions_->get_items();

        item_birthplace origin;
        origin.type = item_birthplace::floor;

        for (auto const& room : rooms_) {
            if (random::percent(substantive) < 70) {
                continue;
            }
            
            auto p = room.center();

            auto steps = random::uniform_range(substantive, 1, 10);
            for (; steps > 0; --steps) {
                auto const v = random::direction(substantive);

                auto const q = p + v;
                if (grid_.get(attribute::tile_type, q) == tile_type::floor) {
                    p = q;
                }
            }

            add_to_stack_(
                make_stack_at_(p)
              , generate_item(substantive, items, item_defs, loot_table {}, origin)
            );
        }

        items_.sort();
    }

    //--------------------------------------------------------------------------
    void place_entities_(random::generator& substantive) {
        spawn_table stable {};

        for (auto const& room : rooms_) {
            if (random::uniform_range(substantive, 0, 100) < 20) {
                continue;
            }
            
            auto const& items    = definitions_->get_items();
            auto const& entities = definitions_->get_entities();
           
            auto const bounds = room.bounds();
            auto            p = room.center();

            BK_ASSERT_DBG(intersects(bounds, p));

            auto mob = generate_entity(substantive, entities, items, *item_store_, stable);

            while (!can_move_to(mob, p)) {
                auto const v = random::direction(substantive);
                auto const q = p + v;
                
                if (intersects(bounds, q)) {
                    p = q;
                }
            }

            for (auto steps = random::uniform_range(substantive, 1, 10)
               ; steps > 0
               ; --steps
            ) {
                auto const v = random::direction(substantive);
                auto const q = p + v;
                
                if (intersects(bounds, q) && can_move_to(mob, q)) {
                    p = q;
                }
            }

            mob.move_to(p);
            mobs_.emplace(std::move(mob));
        }

        mobs_.sort();
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
        place_player_(substantive);
        place_entities_(substantive);
        
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
        auto const& map = get_tile_map_();

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
    tilemap const& get_tile_map_() const {
        BK_ASSERT_DBG(tiles_sheet_);
        return tiles_sheet_->map();
    }
private:
    //--------------------------------------------------------------------------
    data_definitions const* definitions_;

    item_store* item_store_ = nullptr;

    tile_sheet const* tiles_sheet_ = nullptr;
    tile_sheet const* entities_sheet_ = nullptr;

    player*           player_;

    grid_storage      grid_;
    bsp_layout        layout_;
    std::vector<room> rooms_;

    ipoint2 stairs_up_   = ipoint2 {0, 0};
    ipoint2 stairs_down_ = ipoint2 {0, 0};

    spatial_map<item_stack, int> items_;
    spatial_map<entity>          mobs_;
};

//==============================================================================
//! The "view" into the world; scaling and translation within the viewport.
//==============================================================================
class view {
public:
    using fpoint2 = point2d<float>;

    //--------------------------------------------------------------------------
    view(tile_sheet const& sheet, int const width, int const height)
      : sheet_     {&sheet}
      , display_w_ {static_cast<float>(width)}
      , display_h_ {static_cast<float>(height)}
    {
        BK_ASSERT(width  > 0.0f);
        BK_ASSERT(height > 0.0f);
    }

    //--------------------------------------------------------------------------
    void set_size(int const width, int const height) noexcept {
        BK_ASSERT_SAFE(width  > 0.0f);
        BK_ASSERT_SAFE(height > 0.0f);

        display_w_ = static_cast<float>(width);
        display_h_ = static_cast<float>(height);
    }

    //--------------------------------------------------------------------------
    int width()  const noexcept { return static_cast<int>(display_w_); }
    int height() const noexcept { return static_cast<int>(display_h_); }

    ////////////////////////////////////////////////////////////////////////////
    //--------------------------------------------------------------------------
    float zoom() const noexcept { return zoom_; }

    //--------------------------------------------------------------------------
    renderer::trans_vec translation() const noexcept {
        return {
            static_cast<renderer::trans_t>(scroll_x_)
          , static_cast<renderer::trans_t>(scroll_y_)
        };
    }

    //--------------------------------------------------------------------------
    renderer::scale_vec scale() const noexcept {
        return {zoom_, zoom_};
    }

    ////////////////////////////////////////////////////////////////////////////
    void zoom_by(float const factor) noexcept {
        zoom_to(zoom_ * factor);
    }

    void zoom_to(float const zoom) noexcept {
        constexpr auto z_min = 0.1f;
        constexpr auto z_max = 8.0f;
        
        auto const c = center();
        auto const z = zoom_;

        zoom_ = bkrl::clamp(zoom, z_min, z_max);

        auto const px = (c.x - scroll_x_ * z) / z;
        auto const py = (c.y - scroll_y_ * z) / z;

        center_on_point(px, py);
    }

    fpoint2 center() const noexcept {
        auto const w = display_w_;
        auto const h = display_h_;

        return {w / 2.0f, h / 2.0f};
    }
    
    void center_on_point(float const x, float const y) noexcept {
        auto const z = zoom_;
        auto const c = center();
        auto const p = fpoint2 {x, y};

        scroll_x_ = (c.x - z * p.x) / z;
        scroll_y_ = (c.y - z * p.y) / z;
    }

    void center_on_point(fpoint2 const p) noexcept {
        center_on_point(p.x, p.y);
    }

    void center_on_grid(int const x, int const y) noexcept {
        auto const tw = sheet_->tile_width();
        auto const th = sheet_->tile_height();

        auto const px = (x + 0.5f) * tw;
        auto const py = (y + 0.5f) * th;

        center_on_point(px, py);
    }

    void center_on_grid(ipoint2 const p) noexcept {
        center_on_grid(p.x, p.y);
    }

    void scroll_x(int const dx) noexcept {
        scroll_x_ += dx / zoom_;
    }

    void scroll_y(int const dy) noexcept {
        scroll_y_ += dy / zoom_;
    }

    void scroll(int const dx, int const dy) noexcept {
        scroll_x(dx);
        scroll_y(dy);
    }

    template <typename T>
    fpoint2 screen_to_point(point2d<T> const p) const {
        return screen_to_point(p.x, p.y);
    }

    template <typename T>
    fpoint2 screen_to_point(T const x, T const y) const {
        auto const z = zoom_;
        auto const c = center();

        auto const sx = (x - scroll_x_ * z) / z;
        auto const sy = (y - scroll_y_ * z) / z;

        return {static_cast<float>(sx), static_cast<float>(sy)};
    }

    ipoint2 screen_to_grid(int const x, int const y) const {
        auto const p = screen_to_point(x, y);

        auto const tw = sheet_->tile_width();
        auto const th = sheet_->tile_height();

        auto const gx = std::trunc(p.x / tw);
        auto const gy = std::trunc(p.y / th);

        return {static_cast<int>(gx), static_cast<int>(gy)};
    }

    ipoint2 grid_to_screen(int const x, int const y) const {
        auto const tw = sheet_->tile_width();
        auto const th = sheet_->tile_height();

        return {
            static_cast<int>(x * tw * zoom_ + scroll_x_)
          , static_cast<int>(y * th * zoom_ + scroll_y_)
        };
    }
private:
    tile_sheet const* sheet_;

    float display_w_ = 0.0f;
    float display_h_ = 0.0f;

    float scroll_x_  = 0.0f;
    float scroll_y_  = 0.0f;

    float zoom_      = 2.0f;
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

    virtual void on_char(char c) = 0;
    virtual bool on_command(command_type cmd) = 0;
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
    void on_char(char) override {
    }

    //--------------------------------------------------------------------------
    bool on_command(command_type cmd) override {
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

        return false;
    }
    
    //--------------------------------------------------------------------------
    void on_mouse_move(mouse_move_info const& ) override {
    }
    
    //--------------------------------------------------------------------------
    void on_mouse_button(mouse_button_info const& ) override {
    }
private:
    completion_handler handler_;
    ivec2 dir_ = ivec2 {0, 0};
    bool  ok_  = false;
};

//==============================================================================
//! List selection input mode.
//==============================================================================
class input_mode_selection final : public input_mode_base {
public:
    using input_mode_base::input_mode_base;

    using completion_handler = std::function<void (optional<item_id> iid)>;

    //--------------------------------------------------------------------------
    input_mode_base* enter_mode(gui::item_list& list, completion_handler handler) {
        list_    = &list;
        handler_ = std::move(handler);

        return this;
    }

    //--------------------------------------------------------------------------
    void on_char(char c) override {
        constexpr auto first = 'a';
        constexpr auto last  = 'z';

        if (c < first || c > last) {
            return;
        }

        auto const i    = c - first;
        auto const size = list_->size();
        
        if (i >= size) {
            return;
        }

        exit_mode_(optional<item_id>{list_->at(i)});
    }

    //--------------------------------------------------------------------------
    bool on_command(command_type cmd) override {
        switch (cmd) {
        default: break;
        case command_type::accept : exit_mode_(optional<item_id>{list_->selection()}); break;
        case command_type::cancel : exit_mode_(optional<item_id>{}); break;
        case command_type::north  : list_->select_prev(); break;
        case command_type::south  : list_->select_next(); break;
        }

        return true;
    }
    
    //--------------------------------------------------------------------------
    void on_mouse_move(mouse_move_info const& info) override {
        auto const i = list_->index_at(info.x, info.y);
        if (i == -1) {
            return;
        }

        list_->set_selection(i);
    }
    
    //--------------------------------------------------------------------------
    void on_mouse_button(mouse_button_info const& info) override {
        if (info.button != 1) {
            return;
        }

        auto const i = list_->index_at(info.x, info.y);
        if (i == -1) {
            return;
        }

        exit_mode_(optional<item_id>{list_->selection()});
    }
private:
    void exit_mode_(optional<item_id>&& iid) {
        handler_(iid);

        list_->clear();
        do_on_exit_();
    }

    gui::item_list*    list_ = nullptr;
    completion_handler handler_;
};

} //namespace bkrl

//==============================================================================
//==============================================================================
struct engine_client::impl_t {
public:
    enum : int {
        level_w   = 50
      , level_h   = 50
      , font_size = 20
    };

    explicit impl_t(data_definitions& defs)
      : definitions_        {&defs}
      , config_             {&defs.get_config()}
      , random_substantive_ {config_->substantive_seed}
      , random_trivial_     {config_->trivial_seed}
      , app_                {defs.get_keymap(), *config_}
      , renderer_           {app_}
      , font_lib_           {}
      , font_face_          {renderer_, font_lib_, config_->font_name, font_size}
      , last_message_       {}
      , tile_sheet_         {defs.get_tilemap(), renderer_}
      , entities_sheet_     {renderer_, entity_definitions::tile_filename(), entity_definitions::tile_size()}
      , view_               {tile_sheet_, app_.client_width(), app_.client_height()}
      , cur_level_          {nullptr}
      , player_             {}
      , input_mode_         {nullptr}
      , imode_direction_    {[&] {input_mode_ = nullptr;}}
      , imode_selection_    {[&] {input_mode_ = nullptr;}}
      , item_list_          {font_face_, defs.get_items(), item_store_}
      , msg_log_            {font_face_, defs.get_messages()}
    {
        definitions_->set_language(config_->language);
        
        ////////////////////////////////////////////////////
        init_sinks();
        init_timers();

        next_level(0);
        print_message(message_type::welcome);

        item_list_.set_position(ipoint2 {24, 128});
        ////////////////////////////////////////////////////
        main();
    }

    void main() {
        while (app_.is_running()) {
            app_.do_all_events();
            timers_.update();
            render(renderer_);
            yield();
        }
    }

    void yield() {
        if (frames_ % 5 == 0) {
            //SDL_Delay(15);
        }
    }

    void init_timers() {
        auto const fade_time = std::chrono::milliseconds {1000};
        timers_.add_timer(fade_time, [this, fade_time](timer::id_t, timer::duration_t) {
            fade_message();
            return fade_time;
        });

        auto const frame_time = std::chrono::nanoseconds {1000000000 / 60};
        timers_.add_timer(frame_time, [this, frame_time](timer::id_t, timer::duration_t) {
            render_ = true;
            return frame_time;
        });
    }

    void init_sinks() {
        app_.on_char([&](char const c) {
            on_char(c);
        });

        app_.on_command([&](command_type const cmd) {
            return on_command(cmd);
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

    void fade_message() {
        if (last_message_fade_) {
            --last_message_fade_;
        }
    }

    void print_message(string_ref const msg) {
        msg_log_.print_line(msg);
    }

    void print_message(message_type const msg) {
        if (msg == message_type::none) {
            last_message_ = transitory_text_layout {};
        } else {
            //TODO
            print_message(definitions_->get_messages()[msg]);
        }
    }

    void next_level(int level) {
        BK_ASSERT_SAFE(level >= 0);

        auto const size = static_cast<int>(levels_.size());

        if (level == size) {
            levels_.emplace_back(
                random_substantive_
              , random_trivial_
              , *definitions_
              , item_store_
              , player_
              , tile_sheet_
              , entities_sheet_
              , level_w
              , level_h
            );

            cur_level_ = &levels_.back();
        } else if (level < size) {
            cur_level_ = &levels_[level];
        } else {
            BK_TODO_FAIL();
        }
        
        level_number_ = level;
        do_zoom_reset();
    }

    void prev_level(int level) {
        if (level < 0) {
            return;
        }

        BK_ASSERT_SAFE(level < static_cast<int>(levels_.size()));

        cur_level_ = &levels_[level];

        level_number_ = level;
        player_.move_to(cur_level_->down_stair());
        do_zoom_reset();
    }

    void draw_inspect_msg(renderer& r) {
        static auto const color_text_background = make_color(50, 50, 50, 180);

        if (inspect_message_.empty()) {
            return;
        }

        auto const restore = r.restore_view();

        auto const actual_h = inspect_message_.actual_height();
        auto const line_h   = font_face_.line_gap();
        auto const diff_h   = actual_h % line_h;

        auto const msg_h = actual_h + (diff_h ? (line_h - diff_h) : 0);
        auto const msg_w = inspect_message_.actual_width();

        auto const x = mouse_pos_.x;
        auto const y = mouse_pos_.y - msg_h;
        auto const w = msg_w;
        auto const h = msg_h;

        constexpr auto border = 4;

        r.set_draw_color(color_text_background);
        r.draw_filled_rect(make_rect_size(
            x - border
          , y - border
          , w + border * 2
          , h + border * 2
        ));

        inspect_message_.render(r, font_face_, x, y);
    }

    void draw_gui(renderer& r) {
        if (!item_list_.empty()) {
            item_list_.render(r);
        }

        msg_log_.render(r, 0, 0);
    }

    void draw_level(renderer& r) {
        cur_level_->render(r);
    }

    void render(renderer& r) {
        if (!render_) {
            return;
        }

        r.set_draw_color();
        r.clear();

        auto const s = view_.scale();
        auto const t = view_.translation();

        r.set_translation(t);
        r.set_scale(s);

        draw_level(r);
        draw_inspect_msg(r);
        draw_gui(r);

        r.present();

        render_ = false;
        frames_++;
    }

    void advance() {
        turn_++;
        cur_level_->advance(random_trivial_);
    }

    void set_door_state(bool const opened) {
        auto const p = player_.position();

        auto msg = (opened)
          ? cur_level_->open_doors(p)
          : cur_level_->close_doors(p);

        if (msg != message_type::none) {
            print_message(msg);
        } else {
            //ok
            advance();
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
                ? cur_level_->open_door(p + dir)
                : cur_level_->close_door(p + dir);

            if (result_msg != message_type::none) {
                print_message(result_msg);
            } else {
                //ok
                advance();
            }
        });
    }

    void clear_inspect_message() {
        inspect_message_ = transitory_text_layout {};
    }

    void set_inspect_message(ipoint2 const p) {
        constexpr auto w = 256;
        constexpr auto h = transitory_text_layout::unlimited;

        inspect_message_.reset(
            font_face_, cur_level_->get_inspect_msg(p), w, h
        );
    }

    void do_auto_scroll_(ipoint2 const p) {
        auto const border    = config_->auto_scroll_w;
        auto const border_lo = border;
        auto const border_hi = 1.0f - border;

        auto const w = view_.width();
        auto const h = view_.height();
        auto const q = view_.grid_to_screen(p.x, p.y);
        
        auto const dist_x = w - q.x;
        auto const dist_y = h - q.y;

        auto const tw = tile_sheet_.tile_width();
        auto const th = tile_sheet_.tile_height();

        auto scroll_x = 0;
        auto scroll_y = 0;

        if (dist_x < (w * border_lo)) {
            scroll_x = -tw;
        } else if (dist_x > (w * border_hi)) {
            scroll_x = tw;
        }

        if (dist_y < (h * border_lo)) {
            scroll_y = -th;
        } else if (dist_y > (h * border_hi)) {
            scroll_y = th;
        }

        do_scroll(scroll_x, scroll_y);
    }

    void make_item_list_(string_ref const title, item_list const& items) {
        item_list_.clear();
        item_list_.insert(items);
        item_list_.set_title(title);
    }

    void make_item_list_(message_type const title, item_stack const& items) {       
        make_item_list_(definitions_->get_messages()[title], items.list());
    }

    void make_item_list_(message_type const title, item_list const& items) {       
        make_item_list_(definitions_->get_messages()[title], items);
    }

    template <typename List, typename Handler>
    void enter_item_select_mode_(
        message_type const  title
      , List         const& items
      , Handler&&           handler
    ) {
        BK_ASSERT(input_mode_ != &imode_selection_);

        make_item_list_(title, items);

        input_mode_ = imode_selection_.enter_mode(
            item_list_
          , std::forward<Handler>(handler)
        );
    }

    //--------------------------------------------------------------------------
    //! Get the item with id = iid
    //--------------------------------------------------------------------------
    void get_item_(item_id const iid) {
        auto const& p      = player_.position();
        auto const& istore = item_store_;
        auto const& idefs  = definitions_->get_items();

        cur_level_->get_item(p, iid);
        player_.items().insert(iid, idefs, istore);

        msg_log_.print_line(message_type::get_ok, get_item_name_(iid));
        
        advance();
    }

    //--------------------------------------------------------------------------
    string_ref get_item_name_(item_id const iid) const {
        auto const& istore = item_store_;
        auto const& idefs  = definitions_->get_items();

        return get_item_loc(iid, idefs, istore).name;
    }

    //--------------------------------------------------------------------------
    string_ref get_message_string_(message_type const msg) const {
        auto const& messages = definitions_->get_messages();
        return messages[msg];
    }

    //--------------------------------------------------------------------------
    void equip_item_(item_id const iid) {
        auto const& istore = item_store_;
        auto const& idefs  = definitions_->get_items();

        auto const& result = player_.equip_item(iid, idefs, istore);
        if (result.second) {
            msg_log_.print_line(message_type::wield_wear_ok, get_item_name_(iid));
            return;
        }

        auto const& conflicting = player_.equip().match_any(result.first);
        if (!conflicting) {
            BK_TODO_FAIL();
        }

        msg_log_.print_line(message_type::wield_wear_conflict, get_item_name_(iid), get_item_name_(*conflicting));
    }

    //--------------------------------------------------------------------------
    void drop_item_(item_id const iid) {
        auto const p = player_.position();

        auto const& istore = item_store_;
        auto const& idefs  = definitions_->get_items();

        player_.items().remove(iid);
        cur_level_->drop_item_at(p, iid);

        msg_log_.print_line(message_type::drop_ok, get_item_name_(iid));
    }

    ////////////////////////////////////////////////////////////////////////////
    // Commands
    ////////////////////////////////////////////////////////////////////////////
    //--------------------------------------------------------------------------
    void do_open() {
        set_door_state(true);
    }

    //--------------------------------------------------------------------------
    void do_close() {
        set_door_state(false);
    }

    //--------------------------------------------------------------------------
    void do_attack(entity& object) {
        auto& subject = player_;

        auto const v = subject.position() - object.position();
        BK_ASSERT_DBG(std::abs(v.x) <= 1);
        BK_ASSERT_DBG(std::abs(v.y) <= 1);

        auto const msg = cur_level_->attack(random_trivial_, subject, object);
        print_message(msg);

        advance();
    }

    //--------------------------------------------------------------------------
    void do_move_player(int const dx, int const dy) {
        auto& level = *cur_level_;

        auto const v = ivec2 {dx, dy};
        auto const p = player_.position() + v;
        
        if (!level.can_move_to(player_, p)) {
            auto const ent = level.entity_at(p);
            if (ent) {
                do_attack(*ent);
            }
            
            return;
        }
        
        do_auto_scroll_(p);
        player_.move_to(p);

        advance();
    }

    //--------------------------------------------------------------------------
    void do_scroll(int const dx, int const dy, int factor = 1) {
        if (factor == 0) {
            factor = config_->scroll_delta;
        }

        view_.scroll(dx * factor, dy * factor);
        clear_inspect_message();
    }

    //--------------------------------------------------------------------------
    void do_zoom_in() {
        auto const factor = 1.0f + config_->zoom_factor;

        view_.zoom_by(factor);
        clear_inspect_message();
    }

    //--------------------------------------------------------------------------
    void do_zoom_out() {
        auto const factor = 1.0f - config_->zoom_factor;

        view_.zoom_by(factor);
        clear_inspect_message();
    }

    //--------------------------------------------------------------------------
    void do_zoom_reset() {
        view_.zoom_to(1.0f);
        view_.center_on_grid(player_.position());
        clear_inspect_message();
    }

    //--------------------------------------------------------------------------
    void do_go_up() {
        auto const msg = cur_level_->go_up(player_.position());

        if (msg != message_type::none) {
            print_message(msg);
            return;
        }

        prev_level(level_number_ - 1);
        advance();

        clear_inspect_message();
    }

    //--------------------------------------------------------------------------
    void do_go_down() {
        auto const msg = cur_level_->go_down(player_.position());

        if (msg != message_type::none) {
            print_message(msg);
            return;
        }

        next_level(level_number_ + 1);
        advance();

        clear_inspect_message();
    }

    //--------------------------------------------------------------------------
    void do_wait() {
        advance();
    }

    //--------------------------------------------------------------------------
    void do_drop() {
        auto& stack = player_.items();
        if (stack.empty()) {
            msg_log_.print_line(message_type::drop_nothing);
            return;
        }

        auto const& title = message_type::title_drop;
        enter_item_select_mode_(title, stack, [this](optional<item_id> maybe_sel) {
            if (!maybe_sel) {
                return;
            }

            drop_item_(*maybe_sel);
        });
    }

    //--------------------------------------------------------------------------
    void do_get() {
        auto const& maybe_stack = cur_level_->get_stack_at(player_.position());
        if (!maybe_stack) {
            msg_log_.print_line(message_type::get_no_items);
            return;
        }

        auto const& stack = *maybe_stack;
        if (stack.size() == 1) {
            get_item_(stack.at(0));
            return;
        }

        auto const& title = message_type::title_get;
        enter_item_select_mode_(title, stack, [this](optional<item_id> maybe_sel) {
            if (!maybe_sel) {
                return;
            }

            get_item_(*maybe_sel);
        });
    }

    //--------------------------------------------------------------------------
    void do_inventory() {
        auto const& items = player_.items();
        if (items.empty()) {
            return; //TODO message
        }

        auto const& title = message_type::title_inventory;
        enter_item_select_mode_(title, items, [this](optional<item_id> maybe_sel) {
        });
    }

    //--------------------------------------------------------------------------
    void do_wield_wear() {
        auto const& istore = item_store_;
        auto const& idefs  = definitions_->get_items();

        item_stack items = player_.get_equippable(idefs, istore);

        if (items.empty()) {
            msg_log_.print_line(message_type::wield_wear_nothing);
            return;
        }

        auto const& title = message_type::title_wield_wear;
        enter_item_select_mode_(title, items, [this](optional<item_id> maybe_sel) {
            if (!maybe_sel) {
                return;
            }

            equip_item_(*maybe_sel);
        });
    }

    void do_equipment() {
        auto const& list = player_.equip().list();
        if (list.empty()) {
            return; //TODO
        }

        auto const& title = message_type::title_wield_wear;
        enter_item_select_mode_(title, list, [this](optional<item_id> maybe_sel) {
            if (!maybe_sel) {
                return;
            }
        });
    }

    ////////////////////////////////////////////////////////////////////////////
    // Sinks
    ////////////////////////////////////////////////////////////////////////////
    //--------------------------------------------------------------------------
    void on_char(char const c) {
        if (input_mode_) {
            input_mode_->on_char(c);
            return;
        }
    }

    //--------------------------------------------------------------------------
    bool on_command(command_type const cmd) {
        if (input_mode_) {
            return input_mode_->on_command(cmd);
        }

        using ct = command_type;

        switch (cmd) {
        case ct::equipment  : do_equipment();         break;
        case ct::wield_wear : do_wield_wear();        break;
        case ct::inventory  : do_inventory();         break;
        case ct::open       : do_open();              break;
        case ct::close      : do_close();             break;
        case ct::get        : do_get();               break;
        case ct::drop       : do_drop();              break;
        case ct::scroll_n   : do_scroll( 0,  1, 0);   break;
        case ct::scroll_s   : do_scroll( 0, -1, 0);   break;
        case ct::scroll_e   : do_scroll(-1,  0, 0);   break;
        case ct::scroll_w   : do_scroll( 1,  0, 0);   break;
        case ct::here       : do_wait();              break;
        case ct::north      : do_move_player (0, -1); break;
        case ct::south      : do_move_player( 0,  1); break;
        case ct::east       : do_move_player( 1,  0); break;
        case ct::west       : do_move_player(-1,  0); break;
        case ct::north_west : do_move_player(-1, -1); break;
        case ct::north_east : do_move_player( 1, -1); break;
        case ct::south_west : do_move_player(-1,  1); break;
        case ct::south_east : do_move_player( 1,  1); break;
        case ct::up         : do_go_up();             break;
        case ct::down       : do_go_down();           break;
        case ct::zoom_in    : do_zoom_in();           break;
        case ct::zoom_out   : do_zoom_out();          break;
        case ct::zoom_reset : do_zoom_reset();        break;
        case ct::cancel     :                         break;
        case ct::accept     :                         break;
        case ct::invalid    :                         break;
        default             : BK_TODO_FAIL();         break;
        }

        return false;
    }

    //--------------------------------------------------------------------------
    void on_resize(unsigned const w, unsigned const h) {
        view_.set_size(w, h);
    }

    //--------------------------------------------------------------------------
    void on_mouse_move(application::mouse_move_info const& info) {
        if (input_mode_) {
            input_mode_->on_mouse_move(info);
        }
        
        mouse_pos_ = ipoint2 {info.x, info.y};

        auto const right = (info.state & (1<<2)) != 0;
        
        if (!right) {
            auto const p = view_.screen_to_grid(info.x, info.y);

            set_inspect_message(p);

            return;
        }

        view_.scroll(info.dx, info.dy);
    }

    //--------------------------------------------------------------------------
    void on_mouse_button(application::mouse_button_info const& info) {
        if (input_mode_) {
            input_mode_->on_mouse_button(info);
        }

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
    data_definitions* definitions_;
    config const*     config_;

    random::generator random_substantive_;
    random::generator random_trivial_;

    application app_;
    renderer    renderer_;
    
    font_libary font_lib_;
    font_face   font_face_;

    timer       timers_;

    item_store  item_store_;

    transitory_text_layout last_message_;
    int                    last_message_fade_ = 5;

    transitory_text_layout inspect_message_;

    tile_sheet tile_sheet_;
    tile_sheet entities_sheet_;

    view   view_;

    std::vector<level> levels_;
    level* cur_level_    = nullptr;
    int    level_number_ = 0;

    uint64_t turn_ = 0;

    player player_;

    input_mode_base*     input_mode_;
    input_mode_direction imode_direction_;
    input_mode_selection imode_selection_;

    gui::item_list   item_list_;
    gui::message_log msg_log_;

    ipoint2 mouse_pos_ = ipoint2 {0, 0};

    int frames_ = 0;
    float fps_ = 0.0f;
    bool render_ = true;
};

engine_client::~engine_client() = default;

engine_client::engine_client(bkrl::data_definitions& defs)
  : impl_ {std::make_unique<impl_t>(defs)}
{
}
