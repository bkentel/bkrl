#include "command_type.hpp"
#include "tiles.hpp"
#include "scancode.hpp"
#include "keyboard.hpp"
#include "messages.hpp"

template <> bkrl::key_modifier_type
bkrl::from_hash(hash_t const hash) {
    using mapping_t = string_ref_mapping<key_modifier_type> const;
    using mt = key_modifier_type;

    static mapping_t mappings[] = {
        {"invalid",     mt::invalid}
      , {"ctrl_left",   mt::ctrl_left}
      , {"ctrl_right",  mt::ctrl_right}
      , {"alt_left",    mt::alt_left}
      , {"alt_right",   mt::alt_right}
      , {"shift_left",  mt::shift_left}
      , {"shift_right", mt::shift_right}
      , {"ctrl",        mt::ctrl}
      , {"alt",         mt::alt}
      , {"shift",       mt::shift}
    };

    return find_mapping(mappings, hash, mt::invalid);
}

template <> bkrl::scancode
bkrl::from_hash(hash_t const hash) {
    using mapping_t = string_ref_mapping<scancode> const;
    using sc = scancode;

    static mapping_t mappings[] = {
        {"invalid", sc::invalid}
      , {"kb_a", sc::kb_a}
      , {"kb_b", sc::kb_b}
      , {"kb_c", sc::kb_c}
      , {"kb_d", sc::kb_d}
      , {"kb_e", sc::kb_e}
      , {"kb_f", sc::kb_f}
      , {"kb_g", sc::kb_g}
      , {"kb_h", sc::kb_h}
      , {"kb_i", sc::kb_i}
      , {"kb_j", sc::kb_j}
      , {"kb_k", sc::kb_k}
      , {"kb_l", sc::kb_l}
      , {"kb_m", sc::kb_m}
      , {"kb_n", sc::kb_n}
      , {"kb_o", sc::kb_o}
      , {"kb_p", sc::kb_p}
      , {"kb_q", sc::kb_q}
      , {"kb_r", sc::kb_r}
      , {"kb_s", sc::kb_s}
      , {"kb_t", sc::kb_t}
      , {"kb_u", sc::kb_u}
      , {"kb_v", sc::kb_v}
      , {"kb_w", sc::kb_w}
      , {"kb_x", sc::kb_x}
      , {"kb_y", sc::kb_y}
      , {"kb_z", sc::kb_z}
      , {"kb_1", sc::kb_1}
      , {"kb_2", sc::kb_2}
      , {"kb_3", sc::kb_3}
      , {"kb_4", sc::kb_4}
      , {"kb_5", sc::kb_5}
      , {"kb_6", sc::kb_6}
      , {"kb_7", sc::kb_7}
      , {"kb_8", sc::kb_8}
      , {"kb_9", sc::kb_9}
      , {"kb_0", sc::kb_0}
      , {"kb_return", sc::kb_return}
      , {"kb_escape", sc::kb_escape}
      , {"kb_backspace", sc::kb_backspace}
      , {"kb_tab", sc::kb_tab}
      , {"kb_space", sc::kb_space}
      , {"kb_minus", sc::kb_minus}
      , {"kb_equals", sc::kb_equals}
      , {"kb_leftbracket", sc::kb_leftbracket}
      , {"kb_rightbracket", sc::kb_rightbracket}
      , {"kb_backslash", sc::kb_backslash}
      , {"kb_nonushash", sc::kb_nonushash}
      , {"kb_semicolon", sc::kb_semicolon}
      , {"kb_apostrophe", sc::kb_apostrophe}
      , {"kb_grave", sc::kb_grave}
      , {"kb_comma", sc::kb_comma}
      , {"kb_period", sc::kb_period}
      , {"kb_slash", sc::kb_slash}
      , {"kb_capslock", sc::kb_capslock}
      , {"kb_f1", sc::kb_f1}
      , {"kb_f2", sc::kb_f2}
      , {"kb_f3", sc::kb_f3}
      , {"kb_f4", sc::kb_f4}
      , {"kb_f5", sc::kb_f5}
      , {"kb_f6", sc::kb_f6}
      , {"kb_f7", sc::kb_f7}
      , {"kb_f8", sc::kb_f8}
      , {"kb_f9", sc::kb_f9}
      , {"kb_f10", sc::kb_f10}
      , {"kb_f11", sc::kb_f11}
      , {"kb_f12", sc::kb_f12}
      , {"kb_printscreen", sc::kb_printscreen}
      , {"kb_scrolllock", sc::kb_scrolllock}
      , {"kb_pause", sc::kb_pause}
      , {"kb_insert", sc::kb_insert}
      , {"kb_home", sc::kb_home}
      , {"kb_pageup", sc::kb_pageup}
      , {"kb_delete", sc::kb_delete}
      , {"kb_end", sc::kb_end}
      , {"kb_pagedown", sc::kb_pagedown}
      , {"kb_right", sc::kb_right}
      , {"kb_left", sc::kb_left}
      , {"kb_down", sc::kb_down}
      , {"kb_up", sc::kb_up}
      , {"kb_numlockclear", sc::kb_numlockclear}
      , {"kb_kp_divide", sc::kb_kp_divide}
      , {"kb_kp_multiply", sc::kb_kp_multiply}
      , {"kb_kp_minus", sc::kb_kp_minus}
      , {"kb_kp_plus", sc::kb_kp_plus}
      , {"kb_kp_enter", sc::kb_kp_enter}
      , {"kb_kp_1", sc::kb_kp_1}
      , {"kb_kp_2", sc::kb_kp_2}
      , {"kb_kp_3", sc::kb_kp_3}
      , {"kb_kp_4", sc::kb_kp_4}
      , {"kb_kp_5", sc::kb_kp_5}
      , {"kb_kp_6", sc::kb_kp_6}
      , {"kb_kp_7", sc::kb_kp_7}
      , {"kb_kp_8", sc::kb_kp_8}
      , {"kb_kp_9", sc::kb_kp_9}
      , {"kb_kp_0", sc::kb_kp_0}
      , {"kb_kp_period", sc::kb_kp_period}
      , {"kb_nonusbackslash", sc::kb_nonusbackslash}
      , {"kb_application", sc::kb_application}
      , {"kb_power", sc::kb_power}
      , {"kb_kp_equals", sc::kb_kp_equals}
      , {"kb_f13", sc::kb_f13}
      , {"kb_f14", sc::kb_f14}
      , {"kb_f15", sc::kb_f15}
      , {"kb_f16", sc::kb_f16}
      , {"kb_f17", sc::kb_f17}
      , {"kb_f18", sc::kb_f18}
      , {"kb_f19", sc::kb_f19}
      , {"kb_f20", sc::kb_f20}
      , {"kb_f21", sc::kb_f21}
      , {"kb_f22", sc::kb_f22}
      , {"kb_f23", sc::kb_f23}
      , {"kb_f24", sc::kb_f24}
      , {"kb_execute", sc::kb_execute}
      , {"kb_help", sc::kb_help}
      , {"kb_menu", sc::kb_menu}
      , {"kb_select", sc::kb_select}
      , {"kb_stop", sc::kb_stop}
      , {"kb_again", sc::kb_again}
      , {"kb_undo", sc::kb_undo}
      , {"kb_cut", sc::kb_cut}
      , {"kb_copy", sc::kb_copy}
      , {"kb_paste", sc::kb_paste}
      , {"kb_find", sc::kb_find}
      , {"kb_mute", sc::kb_mute}
      , {"kb_volumeup", sc::kb_volumeup}
      , {"kb_volumedown", sc::kb_volumedown}
      , {"kb_kp_comma", sc::kb_kp_comma}
      , {"kb_kp_equalsas400", sc::kb_kp_equalsas400}
      , {"kb_international1", sc::kb_international1}
      , {"kb_international2", sc::kb_international2}
      , {"kb_international3", sc::kb_international3}
      , {"kb_international4", sc::kb_international4}
      , {"kb_international5", sc::kb_international5}
      , {"kb_international6", sc::kb_international6}
      , {"kb_international7", sc::kb_international7}
      , {"kb_international8", sc::kb_international8}
      , {"kb_international9", sc::kb_international9}
      , {"kb_lang1", sc::kb_lang1}
      , {"kb_lang2", sc::kb_lang2}
      , {"kb_lang3", sc::kb_lang3}
      , {"kb_lang4", sc::kb_lang4}
      , {"kb_lang5", sc::kb_lang5}
      , {"kb_lang6", sc::kb_lang6}
      , {"kb_lang7", sc::kb_lang7}
      , {"kb_lang8", sc::kb_lang8}
      , {"kb_lang9", sc::kb_lang9}
      , {"kb_alterase", sc::kb_alterase}
      , {"kb_sysreq", sc::kb_sysreq}
      , {"kb_cancel", sc::kb_cancel}
      , {"kb_clear", sc::kb_clear}
      , {"kb_prior", sc::kb_prior}
      , {"kb_return2", sc::kb_return2}
      , {"kb_separator", sc::kb_separator}
      , {"kb_out", sc::kb_out}
      , {"kb_oper", sc::kb_oper}
      , {"kb_clearagain", sc::kb_clearagain}
      , {"kb_crsel", sc::kb_crsel}
      , {"kb_exsel", sc::kb_exsel}
      , {"kb_kp_00", sc::kb_kp_00}
      , {"kb_kp_000", sc::kb_kp_000}
      , {"kb_thousandsseparator", sc::kb_thousandsseparator}
      , {"kb_decimalseparator", sc::kb_decimalseparator}
      , {"kb_currencyunit", sc::kb_currencyunit}
      , {"kb_currencysubunit", sc::kb_currencysubunit}
      , {"kb_kp_leftparen", sc::kb_kp_leftparen}
      , {"kb_kp_rightparen", sc::kb_kp_rightparen}
      , {"kb_kp_leftbrace", sc::kb_kp_leftbrace}
      , {"kb_kp_rightbrace", sc::kb_kp_rightbrace}
      , {"kb_kp_tab", sc::kb_kp_tab}
      , {"kb_kp_backspace", sc::kb_kp_backspace}
      , {"kb_kp_a", sc::kb_kp_a}
      , {"kb_kp_b", sc::kb_kp_b}
      , {"kb_kp_c", sc::kb_kp_c}
      , {"kb_kp_d", sc::kb_kp_d}
      , {"kb_kp_e", sc::kb_kp_e}
      , {"kb_kp_f", sc::kb_kp_f}
      , {"kb_kp_xor", sc::kb_kp_xor}
      , {"kb_kp_power", sc::kb_kp_power}
      , {"kb_kp_percent", sc::kb_kp_percent}
      , {"kb_kp_less", sc::kb_kp_less}
      , {"kb_kp_greater", sc::kb_kp_greater}
      , {"kb_kp_ampersand", sc::kb_kp_ampersand}
      , {"kb_kp_dblampersand", sc::kb_kp_dblampersand}
      , {"kb_kp_verticalbar", sc::kb_kp_verticalbar}
      , {"kb_kp_dblverticalbar", sc::kb_kp_dblverticalbar}
      , {"kb_kp_colon", sc::kb_kp_colon}
      , {"kb_kp_hash", sc::kb_kp_hash}
      , {"kb_kp_space", sc::kb_kp_space}
      , {"kb_kp_at", sc::kb_kp_at}
      , {"kb_kp_exclam", sc::kb_kp_exclam}
      , {"kb_kp_memstore", sc::kb_kp_memstore}
      , {"kb_kp_memrecall", sc::kb_kp_memrecall}
      , {"kb_kp_memclear", sc::kb_kp_memclear}
      , {"kb_kp_memadd", sc::kb_kp_memadd}
      , {"kb_kp_memsubtract", sc::kb_kp_memsubtract}
      , {"kb_kp_memmultiply", sc::kb_kp_memmultiply}
      , {"kb_kp_memdivide", sc::kb_kp_memdivide}
      , {"kb_kp_plusminus", sc::kb_kp_plusminus}
      , {"kb_kp_clear", sc::kb_kp_clear}
      , {"kb_kp_clearentry", sc::kb_kp_clearentry}
      , {"kb_kp_binary", sc::kb_kp_binary}
      , {"kb_kp_octal", sc::kb_kp_octal}
      , {"kb_kp_decimal", sc::kb_kp_decimal}
      , {"kb_kp_hexadecimal", sc::kb_kp_hexadecimal}
      , {"kb_lctrl", sc::kb_lctrl}
      , {"kb_lshift", sc::kb_lshift}
      , {"kb_lalt", sc::kb_lalt}
      , {"kb_lgui", sc::kb_lgui}
      , {"kb_rctrl", sc::kb_rctrl}
      , {"kb_rshift", sc::kb_rshift}
      , {"kb_ralt", sc::kb_ralt}
      , {"kb_rgui", sc::kb_rgui}
      , {"kb_mode", sc::kb_mode}
      , {"kb_audionext", sc::kb_audionext}
      , {"kb_audioprev", sc::kb_audioprev}
      , {"kb_audiostop", sc::kb_audiostop}
      , {"kb_audioplay", sc::kb_audioplay}
      , {"kb_audiomute", sc::kb_audiomute}
      , {"kb_mediaselect", sc::kb_mediaselect}
      , {"kb_www", sc::kb_www}
      , {"kb_mail", sc::kb_mail}
      , {"kb_calculator", sc::kb_calculator}
      , {"kb_computer", sc::kb_computer}
      , {"kb_ac_search", sc::kb_ac_search}
      , {"kb_ac_home", sc::kb_ac_home}
      , {"kb_ac_back", sc::kb_ac_back}
      , {"kb_ac_forward", sc::kb_ac_forward}
      , {"kb_ac_stop", sc::kb_ac_stop}
      , {"kb_ac_refresh", sc::kb_ac_refresh}
      , {"kb_ac_bookmarks", sc::kb_ac_bookmarks}
      , {"kb_brightnessdown", sc::kb_brightnessdown}
      , {"kb_brightnessup", sc::kb_brightnessup}
      , {"kb_displayswitch", sc::kb_displayswitch}
      , {"kb_kbdillumtoggle", sc::kb_kbdillumtoggle}
      , {"kb_kbdillumdown", sc::kb_kbdillumdown}
      , {"kb_kbdillumup", sc::kb_kbdillumup}
      , {"kb_eject", sc::kb_eject}
      , {"kb_sleep", sc::kb_sleep}
      , {"kb_app1", sc::kb_app1}
      , {"kb_app2", sc::kb_app2}
    };

    return find_mapping(mappings, hash, sc::invalid);
}

template <> bkrl::command_type
bkrl::from_hash(hash_t const hash) {
    using mapping_t = string_ref_mapping<command_type> const;
    using ct = command_type;

    static mapping_t mappings[] = {
        {"invalid",    ct::invalid}
      , {"cancel",     ct::cancel}
      , {"accept",     ct::accept}
      , {"here",       ct::here}
      , {"north",      ct::north}
      , {"south",      ct::south}
      , {"east",       ct::east}
      , {"west",       ct::west}
      , {"north_west", ct::north_west}
      , {"north_east", ct::north_east}
      , {"south_west", ct::south_west}
      , {"south_east", ct::south_east}
      , {"up",         ct::up}
      , {"down",       ct::down}
      , {"zoom_in",    ct::zoom_in}
      , {"zoom_out",   ct::zoom_out}
      , {"zoom_reset", ct::zoom_reset}
      , {"scroll_n",   ct::scroll_n}
      , {"scroll_s",   ct::scroll_s}
      , {"scroll_e",   ct::scroll_e}
      , {"scroll_w",   ct::scroll_w}
      , {"open",       ct::open}
      , {"close",      ct::close}
      , {"get",        ct::get}
      , {"drop",       ct::drop}
      , {"inventory",  ct::inventory}
      , {"wield_wear", ct::wield_wear}
      , {"take_off",   ct::take_off}
      , {"equipment",  ct::equipment}
    };

    return find_mapping(mappings, hash, ct::invalid);
}

template <> bkrl::tile_type
bkrl::from_hash(hash_t const hash) {
    using mapping_t = string_ref_mapping<tile_type> const;
    using tt = tile_type;

    static mapping_t mappings[] = {
        {"invalid",  tt::invalid}
      , {"empty",    tt::empty}
      , {"floor",    tt::floor}
      , {"wall",     tt::wall}
      , {"door",     tt::door}
      , {"stair",    tt::stair}
      , {"corridor", tt::corridor}
    };

    return find_mapping(mappings, hash, tt::invalid);
}

template <> bkrl::texture_type
bkrl::from_hash(hash_t const hash) {
    using mapping_t = string_ref_mapping<texture_type> const;
    using tt = texture_type;

    static mapping_t mappings[] = {
        {"invalid",     tt::invalid}
      , {"floor",       tt::floor}

      , {"wall_none",   tt::wall_none}

      , {"wall_n",      tt::wall_n}
      , {"wall_s",      tt::wall_s}
      , {"wall_e",      tt::wall_e}
      , {"wall_w",      tt::wall_w}

      , {"wall_ns",     tt::wall_ns}
      , {"wall_ew",     tt::wall_ew}
      , {"wall_se",     tt::wall_se}
      , {"wall_sw",     tt::wall_sw}
      , {"wall_ne",     tt::wall_ne}
      , {"wall_nw",     tt::wall_nw}

      , {"wall_nse",    tt::wall_nse}
      , {"wall_nsw",    tt::wall_nsw}
      , {"wall_sew",    tt::wall_sew}
      , {"wall_new",    tt::wall_new}
      
      , {"wall_nsew",   tt::wall_nsew}

      , {"door_closed", tt::door_closed}
      , {"door_opened", tt::door_opened}

      , {"corridor",    tt::corridor}

      , {"stair_up",    tt::stair_up}
      , {"stair_down",  tt::stair_down}
    };

    return find_mapping(mappings, hash, tt::invalid);
}
