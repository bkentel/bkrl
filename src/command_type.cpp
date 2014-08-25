#include "command_type.hpp"
#include "util.hpp"
#include "assert.hpp"

#include "json11/json11.hpp"

////////////////////////////////////////////////////////////////////////////////
// ::bkrl::enum_map<::bkrl::command_type>
////////////////////////////////////////////////////////////////////////////////
template class ::bkrl::enum_map<::bkrl::command_type>;

namespace {
    using map_t    = ::bkrl::enum_map<::bkrl::command_type>;
    using vector_t = std::vector<map_t::value_type>;

    vector_t init_string_to_value() {
        using command = ::bkrl::command_type;

        vector_t result;

        BK_ENUMMAP_ADD_STRING(result, command, none);
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
    
        bkrl::sort(result, map_t::value_type::less_hash);

        return result;
    }

    //take a copy of string_to_value
    vector_t init_value_to_string(vector_t string_to_value) {
        bkrl::sort(string_to_value, map_t::value_type::less_enum);
        return string_to_value;
    }
}

vector_t const map_t::string_to_value_ = init_string_to_value();
vector_t const map_t::value_to_string_ = init_value_to_string(map_t::string_to_value_);
bool     const map_t::checked_         = map_t::check();
