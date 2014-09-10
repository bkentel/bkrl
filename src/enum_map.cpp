#include "util.hpp"
#include "assert.hpp"

#include "command_type.hpp"
#include "texture_type.hpp"
#include "tile_type.hpp"

////////////////////////////////////////////////////////////////////////////////
// enum_map<tile_type>
////////////////////////////////////////////////////////////////////////////////
template class bkrl::enum_map<bkrl::tile_type>;

namespace {
    using tile_map_t    = bkrl::enum_map<bkrl::tile_type>;
    using tile_vector_t = std::vector<tile_map_t::value_type>;

    tile_vector_t tile_init_string_to_value() {
        using tile_type = bkrl::tile_type;

        tile_vector_t result;

        BK_ENUMMAP_ADD_STRING(result, tile_type, invalid);
        BK_ENUMMAP_ADD_STRING(result, tile_type, empty);
        BK_ENUMMAP_ADD_STRING(result, tile_type, floor);
        BK_ENUMMAP_ADD_STRING(result, tile_type, wall);
        BK_ENUMMAP_ADD_STRING(result, tile_type, door);
        BK_ENUMMAP_ADD_STRING(result, tile_type, stair);
        BK_ENUMMAP_ADD_STRING(result, tile_type, corridor);

        bkrl::sort(result, tile_map_t::value_type::less_hash);

        return result;
    }

    //take a copy of string_to_value
    tile_vector_t tile_init_value_to_string(tile_vector_t string_to_value) {
        bkrl::sort(string_to_value, tile_map_t::value_type::less_enum);
        return string_to_value;
    }
}

tile_vector_t const tile_map_t::string_to_value_ = tile_init_string_to_value();
tile_vector_t const tile_map_t::value_to_string_ = tile_init_value_to_string(tile_map_t::string_to_value_);
bool          const tile_map_t::checked_         = tile_map_t::check();

////////////////////////////////////////////////////////////////////////////////
// enum_map<texture_type>
////////////////////////////////////////////////////////////////////////////////
template class bkrl::enum_map<bkrl::texture_type>;

namespace {
    using texture_map_t    = bkrl::enum_map<bkrl::texture_type>;
    using texture_vector_t = std::vector<texture_map_t::value_type>;

    texture_vector_t texture_init_string_to_value() {
        using texture_type = bkrl::texture_type;

        texture_vector_t result;

        BK_ENUMMAP_ADD_STRING(result, texture_type, invalid);

        BK_ENUMMAP_ADD_STRING(result, texture_type, floor);

        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_none);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_n);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_s);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_e);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_w);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_ns);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_ew);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_se);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_sw);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_ne);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_nw);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_nse);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_nsw);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_sew);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_new);
        BK_ENUMMAP_ADD_STRING(result, texture_type, wall_nsew);

        BK_ENUMMAP_ADD_STRING(result, texture_type, door_closed);
        BK_ENUMMAP_ADD_STRING(result, texture_type, door_opened);

        BK_ENUMMAP_ADD_STRING(result, texture_type, corridor);

        BK_ENUMMAP_ADD_STRING(result, texture_type, stair_up);
        BK_ENUMMAP_ADD_STRING(result, texture_type, stair_down);

        bkrl::sort(result, texture_map_t::value_type::less_hash);

        return result;
    }

    //take a copy of string_to_value
    texture_vector_t texture_init_value_to_string(texture_vector_t string_to_value) {
        bkrl::sort(string_to_value, texture_map_t::value_type::less_enum);
        return string_to_value;
    }
}

texture_vector_t const texture_map_t::string_to_value_ = texture_init_string_to_value();
texture_vector_t const texture_map_t::value_to_string_ = texture_init_value_to_string(texture_map_t::string_to_value_);
bool             const texture_map_t::checked_         = texture_map_t::check();

////////////////////////////////////////////////////////////////////////////////
// enum_map<command_type>
////////////////////////////////////////////////////////////////////////////////
template class bkrl::enum_map<bkrl::command_type>;

namespace {
    using command_map_t    = bkrl::enum_map<bkrl::command_type>;
    using command_vector_t = std::vector<command_map_t::value_type>;

    command_vector_t command_init_string_to_value() {
        using command = bkrl::command_type;

        command_vector_t result;

        BK_ENUMMAP_ADD_STRING(result, command, invalid);
        BK_ENUMMAP_ADD_STRING(result, command, cancel);
        BK_ENUMMAP_ADD_STRING(result, command, accept);

        BK_ENUMMAP_ADD_STRING(result, command, here);
        BK_ENUMMAP_ADD_STRING(result, command, north);
        BK_ENUMMAP_ADD_STRING(result, command, south);
        BK_ENUMMAP_ADD_STRING(result, command, east);
        BK_ENUMMAP_ADD_STRING(result, command, west);
        BK_ENUMMAP_ADD_STRING(result, command, north_west);
        BK_ENUMMAP_ADD_STRING(result, command, north_east);
        BK_ENUMMAP_ADD_STRING(result, command, south_west);
        BK_ENUMMAP_ADD_STRING(result, command, south_east);

        BK_ENUMMAP_ADD_STRING(result, command, zoom_in);
        BK_ENUMMAP_ADD_STRING(result, command, zoom_out);

        BK_ENUMMAP_ADD_STRING(result, command, scroll_n);
        BK_ENUMMAP_ADD_STRING(result, command, scroll_s);
        BK_ENUMMAP_ADD_STRING(result, command, scroll_e);
        BK_ENUMMAP_ADD_STRING(result, command, scroll_w);

        BK_ENUMMAP_ADD_STRING(result, command, open);
        BK_ENUMMAP_ADD_STRING(result, command, close);

        bkrl::sort(result, command_map_t::value_type::less_hash);

        return result;
    }

    //take a copy of string_to_value
    command_vector_t command_init_value_to_string(command_vector_t string_to_value) {
        bkrl::sort(string_to_value, command_map_t::value_type::less_enum);
        return string_to_value;
    }
}

command_vector_t const command_map_t::string_to_value_ = command_init_string_to_value();
command_vector_t const command_map_t::value_to_string_ = command_init_value_to_string(command_map_t::string_to_value_);
bool             const command_map_t::checked_         = command_map_t::check();
