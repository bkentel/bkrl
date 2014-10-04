#include "util.hpp"
#include "algorithm.hpp"

#include "command_type.hpp"
#include "texture_type.hpp"
#include "tile_type.hpp"
#include "scancode.hpp"
#include "keyboard.hpp"
#include "messages.hpp"

//==============================================================================
//! convenience macro get a unique reference to a compile-time cstring.
//==============================================================================
#define BK_ENUMMAP_MAKE_STRING(ENUM, VALUE) \
[]() -> ::bkrl::string_ref { \
    static char const string[] {#VALUE}; \
    return {string, ::bkrl::string_len(string)}; \
}()

//==============================================================================
//! convenience macro to add a mapping.
//==============================================================================
#define BK_ENUMMAP_ADD_STRING(OUT, ENUM, VALUE) \
OUT.emplace_back(BK_ENUMMAP_MAKE_STRING(ENUM, VALUE), ENUM::VALUE)

//------------------------------------------------------------------------------
#define BK_DEFINE_ENUM_VARS(PREFIX, DISABLE) \
template <> PREFIX##_vector_t const PREFIX##_map_t::string_to_value_ = PREFIX##_init_string_to_value(); \
template <> PREFIX##_vector_t const PREFIX##_map_t::value_to_string_ = PREFIX##_init_value_to_string(PREFIX##_map_t::string_to_value_); \
template <> bool              const PREFIX##_map_t::checked_         = PREFIX##_map_t::check(DISABLE)
//------------------------------------------------------------------------------
#define BK_DECLARE_ENUM_TYPES(PREFIX, TYPE) \
using PREFIX##_map_t    = bkrl::enum_map<bkrl::TYPE>; \
using PREFIX##_vector_t = std::vector<PREFIX##_map_t::value_type>
//------------------------------------------------------------------------------
template class bkrl::enum_map<bkrl::key_modifier_type>;
template class bkrl::enum_map<bkrl::scancode>;
template class bkrl::enum_map<bkrl::tile_type>;
template class bkrl::enum_map<bkrl::texture_type>;
template class bkrl::enum_map<bkrl::command_type>;
template class bkrl::enum_map<bkrl::message_type>;

////////////////////////////////////////////////////////////////////////////////
// enum_map<key_modifier_type>
////////////////////////////////////////////////////////////////////////////////
namespace {
    BK_DECLARE_ENUM_TYPES(modifier, key_modifier_type);

    modifier_vector_t modifier_init_string_to_value() {
        using modifier_type = bkrl::key_modifier_type;

        modifier_vector_t result;

        BK_ENUMMAP_ADD_STRING(result, modifier_type, invalid);
        BK_ENUMMAP_ADD_STRING(result, modifier_type, ctrl_left);
        BK_ENUMMAP_ADD_STRING(result, modifier_type, ctrl_right);
        BK_ENUMMAP_ADD_STRING(result, modifier_type, alt_left);
        BK_ENUMMAP_ADD_STRING(result, modifier_type, alt_right);
        BK_ENUMMAP_ADD_STRING(result, modifier_type, shift_left);
        BK_ENUMMAP_ADD_STRING(result, modifier_type, shift_right);
        BK_ENUMMAP_ADD_STRING(result, modifier_type, ctrl);
        BK_ENUMMAP_ADD_STRING(result, modifier_type, alt);
        BK_ENUMMAP_ADD_STRING(result, modifier_type, shift);

        bkrl::sort(result, modifier_map_t::value_type::less_hash);

        return result;
    }

    //take a copy of string_to_value
    modifier_vector_t modifier_init_value_to_string(modifier_vector_t string_to_value) {
        bkrl::sort(string_to_value, modifier_map_t::value_type::less_enum);
        return string_to_value;
    }
}

////////////////////////////////////////////////////////////////////////////////
// enum_map<scancode>
////////////////////////////////////////////////////////////////////////////////
namespace {
    BK_DECLARE_ENUM_TYPES(scancode, scancode);

    scancode_vector_t scancode_init_string_to_value() {
        using scancode = bkrl::scancode;

        scancode_vector_t result;

        BK_ENUMMAP_ADD_STRING(result, scancode, invalid);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_a);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_b);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_c);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_d);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_e);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_g);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_h);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_i);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_j);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_k);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_l);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_m);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_n);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_o);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_p);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_q);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_r);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_s);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_t);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_u);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_v);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_w);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_x);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_y);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_z);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_1);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_2);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_3);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_4);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_5);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_6);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_7);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_8);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_9);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_0);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_return);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_escape);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_backspace);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_tab);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_space);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_minus);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_equals);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_leftbracket);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_rightbracket);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_backslash);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_nonushash);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_semicolon);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_apostrophe);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_grave);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_comma);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_period);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_slash);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_capslock);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f1);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f2);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f3);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f4);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f5);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f6);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f7);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f8);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f9);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f10);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f11);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f12);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_printscreen);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_scrolllock);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_pause);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_insert);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_home);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_pageup);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_delete);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_end);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_pagedown);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_right);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_left);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_down);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_up);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_numlockclear);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_divide);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_multiply);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_minus);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_plus);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_enter);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_1);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_2);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_3);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_4);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_5);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_6);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_7);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_8);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_9);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_0);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_period);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_nonusbackslash);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_application);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_power);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_equals);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f13);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f14);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f15);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f16);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f17);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f18);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f19);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f20);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f21);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f22);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f23);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_f24);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_execute);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_help);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_menu);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_select);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_stop);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_again);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_undo);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_cut);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_copy);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_paste);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_find);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_mute);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_volumeup);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_volumedown);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_comma);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_equalsas400);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_international1);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_international2);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_international3);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_international4);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_international5);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_international6);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_international7);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_international8);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_international9);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_lang1);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_lang2);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_lang3);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_lang4);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_lang5);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_lang6);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_lang7);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_lang8);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_lang9);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_alterase);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_sysreq);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_cancel);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_clear);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_prior);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_return2);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_separator);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_out);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_oper);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_clearagain);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_crsel);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_exsel);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_00);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_000);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_thousandsseparator);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_decimalseparator);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_currencyunit);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_currencysubunit);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_leftparen);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_rightparen);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_leftbrace);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_rightbrace);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_tab);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_backspace);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_a);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_b);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_c);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_d);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_e);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_f);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_xor);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_power);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_percent);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_less);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_greater);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_ampersand);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_dblampersand);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_verticalbar);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_dblverticalbar);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_colon);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_hash);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_space);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_at);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_exclam);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_memstore);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_memrecall);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_memclear);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_memadd);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_memsubtract);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_memmultiply);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_memdivide);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_plusminus);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_clear);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_clearentry);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_binary);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_octal);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_decimal);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kp_hexadecimal);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_lctrl);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_lshift);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_lalt);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_lgui);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_rctrl);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_rshift);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_ralt);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_rgui);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_mode);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_audionext);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_audioprev);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_audiostop);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_audioplay);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_audiomute);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_mediaselect);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_www);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_mail);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_calculator);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_computer);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_ac_search);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_ac_home);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_ac_back);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_ac_forward);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_ac_stop);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_ac_refresh);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_ac_bookmarks);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_brightnessdown);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_brightnessup);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_displayswitch);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kbdillumtoggle);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kbdillumdown);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_kbdillumup);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_eject);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_sleep);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_app1);
        BK_ENUMMAP_ADD_STRING(result, scancode, kb_app2);

        bkrl::sort(result, scancode_map_t::value_type::less_hash);

        return result;
    }

    //take a copy of string_to_value
    scancode_vector_t scancode_init_value_to_string(scancode_vector_t string_to_value) {
        bkrl::sort(string_to_value, scancode_map_t::value_type::less_enum);
        return string_to_value;
    }
}

////////////////////////////////////////////////////////////////////////////////
// enum_map<tile_type>
////////////////////////////////////////////////////////////////////////////////
namespace {
    BK_DECLARE_ENUM_TYPES(tile, tile_type);

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

////////////////////////////////////////////////////////////////////////////////
// enum_map<texture_type>
////////////////////////////////////////////////////////////////////////////////
namespace {
    BK_DECLARE_ENUM_TYPES(texture, texture_type);

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

////////////////////////////////////////////////////////////////////////////////
// enum_map<command_type>
////////////////////////////////////////////////////////////////////////////////
namespace {
    BK_DECLARE_ENUM_TYPES(command, command_type);

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

        BK_ENUMMAP_ADD_STRING(result, command, up);
        BK_ENUMMAP_ADD_STRING(result, command, down);

        BK_ENUMMAP_ADD_STRING(result, command, zoom_in);
        BK_ENUMMAP_ADD_STRING(result, command, zoom_out);
        BK_ENUMMAP_ADD_STRING(result, command, zoom_reset);

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

////////////////////////////////////////////////////////////////////////////////
// enum_map<message_type>
////////////////////////////////////////////////////////////////////////////////
namespace {
    BK_DECLARE_ENUM_TYPES(message, message_type);

    message_vector_t message_init_string_to_value() {
        using message = bkrl::message_type;

        message_vector_t result;

        BK_ENUMMAP_ADD_STRING(result, message, invalid);

        BK_ENUMMAP_ADD_STRING(result, message, welcome);
        BK_ENUMMAP_ADD_STRING(result, message, direction_prompt);
        BK_ENUMMAP_ADD_STRING(result, message, canceled);
        BK_ENUMMAP_ADD_STRING(result, message, door_no_door);
        BK_ENUMMAP_ADD_STRING(result, message, door_is_open);
        BK_ENUMMAP_ADD_STRING(result, message, door_is_closed);
        BK_ENUMMAP_ADD_STRING(result, message, door_blocked);

        bkrl::sort(result, message_map_t::value_type::less_hash);

        return result;
    }

    //take a copy of string_to_value
    message_vector_t message_init_value_to_string(message_vector_t string_to_value) {
        bkrl::sort(string_to_value, message_map_t::value_type::less_enum);
        return string_to_value;
    }
}

//------------------------------------------------------------------------------
BK_DEFINE_ENUM_VARS(modifier, false);
BK_DEFINE_ENUM_VARS(scancode, true);
BK_DEFINE_ENUM_VARS(tile, false);
BK_DEFINE_ENUM_VARS(texture, false);
BK_DEFINE_ENUM_VARS(command, false);
BK_DEFINE_ENUM_VARS(message, false);

//------------------------------------------------------------------------------
#undef BK_DEFINE_ENUM_VARS
#undef BK_DECLARE_ENUM_TYPES
#undef BK_ENUMMAP_ADD_STRING
#undef BK_ENUMMAP_MAKE_STRING
