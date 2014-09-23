#pragma once
/*
  simple directmedia layer
  copyright (c) 1997-2014 sam lantinga <slouken@libsdl.org>

  this software is provided 'as-is', without any express or implied
  warranty.  in no event will the authors be held liable for any damages
  arising from the use of this software.

  permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. the origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. if you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. this notice may not be removed or altered from any source distribution.
*/

#include "enum_map.hpp"

namespace bkrl {

enum class scancode : uint16_t {
    invalid = 0,

    /**
     *  \name usage page 0x07
     *
     *  these values are from usage page 0x07 (usb keyboard page).
     */
    /* @{ */

    kb_a = 4,
    kb_b = 5,
    kb_c = 6,
    kb_d = 7,
    kb_e = 8,
    kb_f = 9,
    kb_g = 10,
    kb_h = 11,
    kb_i = 12,
    kb_j = 13,
    kb_k = 14,
    kb_l = 15,
    kb_m = 16,
    kb_n = 17,
    kb_o = 18,
    kb_p = 19,
    kb_q = 20,
    kb_r = 21,
    kb_s = 22,
    kb_t = 23,
    kb_u = 24,
    kb_v = 25,
    kb_w = 26,
    kb_x = 27,
    kb_y = 28,
    kb_z = 29,

    kb_1 = 30,
    kb_2 = 31,
    kb_3 = 32,
    kb_4 = 33,
    kb_5 = 34,
    kb_6 = 35,
    kb_7 = 36,
    kb_8 = 37,
    kb_9 = 38,
    kb_0 = 39,

    kb_return = 40,
    kb_escape = 41,
    kb_backspace = 42,
    kb_tab = 43,
    kb_space = 44,

    kb_minus = 45,
    kb_equals = 46,
    kb_leftbracket = 47,
    kb_rightbracket = 48,
    kb_backslash = 49, /**< located at the lower left of the return
                                  *   key on iso keyboards and at the right end
                                  *   of the qwerty row on ansi keyboards.
                                  *   produces reverse solidus (backslash) and
                                  *   vertical line in a us layout, reverse
                                  *   solidus and vertical line in a uk mac
                                  *   layout, number sign and tilde in a uk
                                  *   windows layout, dollar sign and pound sign
                                  *   in a swiss german layout, number sign and
                                  *   apostrophe in a german layout, grave
                                  *   accent and pound sign in a french mac
                                  *   layout, and asterisk and micro sign in a
                                  *   french windows layout.
                                  */
    kb_nonushash = 50, /**< iso usb keyboards actually use this code
                                  *   instead of 49 for the same key, but all
                                  *   oses i've seen treat the two codes
                                  *   identically. so, as an implementor, unless
                                  *   your keyboard generates both of those
                                  *   codes and your os treats them differently,
                                  *   you should generate kb_backslash
                                  *   instead of this code. as a user, you
                                  *   should not rely on this code because sdl
                                  *   will never generate it with most (all?)
                                  *   keyboards.
                                  */
    kb_semicolon = 51,
    kb_apostrophe = 52,
    kb_grave = 53, /**< located in the top left corner (on both ansi
                              *   and iso keyboards). produces grave accent and
                              *   tilde in a us windows layout and in us and uk
                              *   mac layouts on ansi keyboards, grave accent
                              *   and not sign in a uk windows layout, section
                              *   sign and plus-minus sign in us and uk mac
                              *   layouts on iso keyboards, section sign and
                              *   degree sign in a swiss german layout (mac:
                              *   only on iso keyboards), circumflex accent and
                              *   degree sign in a german layout (mac: only on
                              *   iso keyboards), superscript two and tilde in a
                              *   french windows layout, commercial at and
                              *   number sign in a french mac layout on iso
                              *   keyboards, and less-than sign and greater-than
                              *   sign in a swiss german, german, or french mac
                              *   layout on ansi keyboards.
                              */
    kb_comma = 54,
    kb_period = 55,
    kb_slash = 56,

    kb_capslock = 57,

    kb_f1 = 58,
    kb_f2 = 59,
    kb_f3 = 60,
    kb_f4 = 61,
    kb_f5 = 62,
    kb_f6 = 63,
    kb_f7 = 64,
    kb_f8 = 65,
    kb_f9 = 66,
    kb_f10 = 67,
    kb_f11 = 68,
    kb_f12 = 69,

    kb_printscreen = 70,
    kb_scrolllock = 71,
    kb_pause = 72,
    kb_insert = 73, /**< insert on pc, help on some mac keyboards (but
                                   does send code 73, not 117) */
    kb_home = 74,
    kb_pageup = 75,
    kb_delete = 76,
    kb_end = 77,
    kb_pagedown = 78,
    kb_right = 79,
    kb_left = 80,
    kb_down = 81,
    kb_up = 82,

    kb_numlockclear = 83, /**< num lock on pc, clear on mac keyboards
                                     */
    kb_kp_divide = 84,
    kb_kp_multiply = 85,
    kb_kp_minus = 86,
    kb_kp_plus = 87,
    kb_kp_enter = 88,
    kb_kp_1 = 89,
    kb_kp_2 = 90,
    kb_kp_3 = 91,
    kb_kp_4 = 92,
    kb_kp_5 = 93,
    kb_kp_6 = 94,
    kb_kp_7 = 95,
    kb_kp_8 = 96,
    kb_kp_9 = 97,
    kb_kp_0 = 98,
    kb_kp_period = 99,

    kb_nonusbackslash = 100, /**< this is the additional key that iso
                                        *   keyboards have over ansi ones,
                                        *   located between left shift and y.
                                        *   produces grave accent and tilde in a
                                        *   us or uk mac layout, reverse solidus
                                        *   (backslash) and vertical line in a
                                        *   us or uk windows layout, and
                                        *   less-than sign and greater-than sign
                                        *   in a swiss german, german, or french
                                        *   layout. */
    kb_application = 101, /**< windows contextual menu, compose */
    kb_power = 102, /**< the usb document says this is a status flag,
                               *   not a physical key - but some mac keyboards
                               *   do have a power key. */
    kb_kp_equals = 103,
    kb_f13 = 104,
    kb_f14 = 105,
    kb_f15 = 106,
    kb_f16 = 107,
    kb_f17 = 108,
    kb_f18 = 109,
    kb_f19 = 110,
    kb_f20 = 111,
    kb_f21 = 112,
    kb_f22 = 113,
    kb_f23 = 114,
    kb_f24 = 115,
    kb_execute = 116,
    kb_help = 117,
    kb_menu = 118,
    kb_select = 119,
    kb_stop = 120,
    kb_again = 121,   /**< redo */
    kb_undo = 122,
    kb_cut = 123,
    kb_copy = 124,
    kb_paste = 125,
    kb_find = 126,
    kb_mute = 127,
    kb_volumeup = 128,
    kb_volumedown = 129,
/* not sure whether there's a reason to enable these */
/*     kb_lockingcapslock = 130,  */
/*     kb_lockingnumlock = 131, */
/*     kb_lockingscrolllock = 132, */
    kb_kp_comma = 133,
    kb_kp_equalsas400 = 134,

    kb_international1 = 135, /**< used on asian keyboards, see
                                            footnotes in usb doc */
    kb_international2 = 136,
    kb_international3 = 137, /**< yen */
    kb_international4 = 138,
    kb_international5 = 139,
    kb_international6 = 140,
    kb_international7 = 141,
    kb_international8 = 142,
    kb_international9 = 143,
    kb_lang1 = 144, /**< hangul/english toggle */
    kb_lang2 = 145, /**< hanja conversion */
    kb_lang3 = 146, /**< katakana */
    kb_lang4 = 147, /**< hiragana */
    kb_lang5 = 148, /**< zenkaku/hankaku */
    kb_lang6 = 149, /**< reserved */
    kb_lang7 = 150, /**< reserved */
    kb_lang8 = 151, /**< reserved */
    kb_lang9 = 152, /**< reserved */

    kb_alterase = 153, /**< erase-eaze */
    kb_sysreq = 154,
    kb_cancel = 155,
    kb_clear = 156,
    kb_prior = 157,
    kb_return2 = 158,
    kb_separator = 159,
    kb_out = 160,
    kb_oper = 161,
    kb_clearagain = 162,
    kb_crsel = 163,
    kb_exsel = 164,

    kb_kp_00 = 176,
    kb_kp_000 = 177,
    kb_thousandsseparator = 178,
    kb_decimalseparator = 179,
    kb_currencyunit = 180,
    kb_currencysubunit = 181,
    kb_kp_leftparen = 182,
    kb_kp_rightparen = 183,
    kb_kp_leftbrace = 184,
    kb_kp_rightbrace = 185,
    kb_kp_tab = 186,
    kb_kp_backspace = 187,
    kb_kp_a = 188,
    kb_kp_b = 189,
    kb_kp_c = 190,
    kb_kp_d = 191,
    kb_kp_e = 192,
    kb_kp_f = 193,
    kb_kp_xor = 194,
    kb_kp_power = 195,
    kb_kp_percent = 196,
    kb_kp_less = 197,
    kb_kp_greater = 198,
    kb_kp_ampersand = 199,
    kb_kp_dblampersand = 200,
    kb_kp_verticalbar = 201,
    kb_kp_dblverticalbar = 202,
    kb_kp_colon = 203,
    kb_kp_hash = 204,
    kb_kp_space = 205,
    kb_kp_at = 206,
    kb_kp_exclam = 207,
    kb_kp_memstore = 208,
    kb_kp_memrecall = 209,
    kb_kp_memclear = 210,
    kb_kp_memadd = 211,
    kb_kp_memsubtract = 212,
    kb_kp_memmultiply = 213,
    kb_kp_memdivide = 214,
    kb_kp_plusminus = 215,
    kb_kp_clear = 216,
    kb_kp_clearentry = 217,
    kb_kp_binary = 218,
    kb_kp_octal = 219,
    kb_kp_decimal = 220,
    kb_kp_hexadecimal = 221,

    kb_lctrl = 224,
    kb_lshift = 225,
    kb_lalt = 226, /**< alt, option */
    kb_lgui = 227, /**< windows, command (apple), meta */
    kb_rctrl = 228,
    kb_rshift = 229,
    kb_ralt = 230, /**< alt gr, option */
    kb_rgui = 231, /**< windows, command (apple), meta */

    kb_mode = 257,    /**< i'm not sure if this is really not covered
                                 *   by any of the above, but since there's a
                                 *   special kmod_mode for it i'm adding it here
                                 */

    /* @} *//* usage page 0x07 */

    /**
     *  \name usage page 0x0c
     *
     *  these values are mapped from usage page 0x0c (usb consumer page).
     */
    /* @{ */

    kb_audionext = 258,
    kb_audioprev = 259,
    kb_audiostop = 260,
    kb_audioplay = 261,
    kb_audiomute = 262,
    kb_mediaselect = 263,
    kb_www = 264,
    kb_mail = 265,
    kb_calculator = 266,
    kb_computer = 267,
    kb_ac_search = 268,
    kb_ac_home = 269,
    kb_ac_back = 270,
    kb_ac_forward = 271,
    kb_ac_stop = 272,
    kb_ac_refresh = 273,
    kb_ac_bookmarks = 274,

    /* @} *//* usage page 0x0c */

    /**
     *  \name walther keys
     *
     *  these are values that christian walther added (for mac keyboard?).
     */
    /* @{ */

    kb_brightnessdown = 275,
    kb_brightnessup = 276,
    kb_displayswitch = 277, /**< display mirroring/dual display
                                           switch, video mode switch */
    kb_kbdillumtoggle = 278,
    kb_kbdillumdown = 279,
    kb_kbdillumup = 280,
    kb_eject = 281,
    kb_sleep = 282,

    kb_app1 = 283,
    kb_app2 = 284,

    /* @} *//* walther keys */

    /* add any other keys here. */

    enum_size /**< not a key, just marks the number of scancodes
                                 for array bounds */
};

extern template class enum_map<scancode>;

} //namespace bkrl
