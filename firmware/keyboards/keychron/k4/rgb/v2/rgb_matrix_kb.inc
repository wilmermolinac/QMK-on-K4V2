// Copyright 2026 QMK-on-K4V2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
//
// "Zones" effects: static colors per key group.
//
// - K4V2_ZONES: your own colors. Edit them in VIA (Lighting -> Zones colors);
//   they're saved in the keyboard. Default = Classic.
// - K4V2_ZONES_CLASSIC / OCEAN / SUNSET / NEON / ICE: fixed palettes over the
//   general groups (Esc, F-row, numbers, letters, symbols, modifiers,
//   navigation, numpad, light key).
// - K4V2_ZONES_GAMING: WASD and the usual game keys stand out, the rest is dim.
// - K4V2_ZONES_CODE: keys grouped by what they do when programming
//   (brackets, operators, punctuation, editing, navigation, shortcuts, debug F-keys).
// The fixed ones follow the global hue (rotation) and saturation; all follow
// the global brightness. Tables are const (flash): the VIA build is short on RAM.

RGB_MATRIX_EFFECT(K4V2_ZONES)
RGB_MATRIX_EFFECT(K4V2_ZONES_CLASSIC)
RGB_MATRIX_EFFECT(K4V2_ZONES_GAMING)
RGB_MATRIX_EFFECT(K4V2_ZONES_CODE)
RGB_MATRIX_EFFECT(K4V2_ZONES_OCEAN)
RGB_MATRIX_EFFECT(K4V2_ZONES_SUNSET)
RGB_MATRIX_EFFECT(K4V2_ZONES_NEON)
RGB_MATRIX_EFFECT(K4V2_ZONES_ICE)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#    include "k4v2_zones.h"

// Matrix reference (same for ANSI and ISO; positions without an LED are skipped):
//  row 0: Esc F1..F12 Del Home End PgUp PgDn Light
//  row 1: ` 1..0 - = Bksp | NumLk / * -
//  row 2: Tab Q..P [ ] \ | 7 8 9 +
//  row 3: Caps A..L ; ' (ISO #) Enter | 4 5 6
//  row 4: LShift (ISO \) Z..M , . / RShift Up | 1 2 3 Ent
//  row 5: Ctrl Win Alt Space RAlt Fn RCtrl Left Down Right | 0 .

// ---- General groups (Custom and the color palettes) ------------------------
#    define _N K4V2_ZONE_NONE
#    define _E K4V2_ZONE_ESC
#    define _F K4V2_ZONE_FROW
#    define _1 K4V2_ZONE_NUM
#    define _L K4V2_ZONE_LET
#    define _S K4V2_ZONE_SYM
#    define _M K4V2_ZONE_MOD
#    define _V K4V2_ZONE_NAV
#    define _P K4V2_ZONE_PAD
#    define _G K4V2_ZONE_LIGHT
static const uint8_t PROGMEM k4v2_map_general[6][19] = {
    // 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
    {_E, _F, _F, _F, _F, _F, _F, _F, _F, _F, _F, _F, _F, _V, _V, _V, _V, _V, _G},
    {_1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _M, _N, _P, _P, _P, _P},
    {_M, _L, _L, _L, _L, _L, _L, _L, _L, _L, _L, _S, _S, _S, _N, _P, _P, _P, _P},
    {_M, _L, _L, _L, _L, _L, _L, _L, _L, _L, _S, _S, _S, _M, _N, _P, _P, _P, _N},
    {_M, _S, _L, _L, _L, _L, _L, _L, _L, _S, _S, _S, _N, _M, _V, _P, _P, _P, _P},
    {_M, _M, _M, _N, _N, _N, _M, _N, _N, _N, _M, _M, _M, _V, _V, _V, _P, _P, _N},
};
#    undef _N
#    undef _E
#    undef _F
#    undef _1
#    undef _L
#    undef _S
#    undef _M
#    undef _V
#    undef _P
#    undef _G

// ---- Gaming groups -----------------------------------------------------------
enum {
    GZ_NONE = 0,
    GZ_REST, // everything else, dim
    GZ_MOVE, // W A S D
    GZ_ACT,  // Q E R F G: abilities, interact, reload
    GZ_SLOT, // 1-5: weapons / items
    GZ_BODY, // Space, Shift, Ctrl, Alt: jump, sprint, crouch
    GZ_UTIL, // Esc, Tab, T, Z X C V B, M, arrows: menus, chat, map, extra binds
    GZ_LIGHT,
    GZ_COUNT
};
#    define __ GZ_NONE
#    define _r GZ_REST
#    define _W GZ_MOVE
#    define _A GZ_ACT
#    define _1 GZ_SLOT
#    define _B GZ_BODY
#    define _U GZ_UTIL
#    define _G GZ_LIGHT
static const uint8_t PROGMEM k4v2_map_gaming[6][19] = {
    // 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
    {_U, _r, _r, _r, _r, _r, _r, _r, _r, _r, _r, _r, _r, _r, _r, _r, _r, _r, _G},
    {_r, _1, _1, _1, _1, _1, _r, _r, _r, _r, _r, _r, _r, _r, __, _r, _r, _r, _r},
    {_U, _A, _W, _A, _A, _U, _r, _r, _r, _r, _r, _r, _r, _r, __, _r, _r, _r, _r},
    {_r, _W, _W, _W, _A, _A, _r, _r, _r, _r, _r, _r, _r, _r, __, _r, _r, _r, __},
    {_B, _r, _U, _U, _U, _U, _U, _r, _U, _r, _r, _r, __, _r, _U, _r, _r, _r, _r},
    {_B, _r, _B, __, __, __, _B, __, __, __, _r, _r, _r, _U, _U, _U, _r, _r, __},
};
#    undef __
#    undef _r
#    undef _W
#    undef _A
#    undef _1
#    undef _B
#    undef _U
#    undef _G

// ---- Code groups ------------------------------------------------------------
enum {
    CZ_NONE = 0,
    CZ_LET,   // letters, soft
    CZ_DIG,   // 1-8, soft
    CZ_BRKT,  // [ ] (and { }), 9 0 for ( )
    CZ_OPER,  // - = / and numpad / * - +
    CZ_PUNCT, // ; ' , . \ ` (and the ISO # and \ keys)
    CZ_EDIT,  // Backspace, Del, Enter, Tab, Space, numpad Enter
    CZ_NAV,   // arrows, Home, End, PgUp, PgDn
    CZ_MOD,   // Ctrl, Win/Cmd, Alt/Opt, Shift, Fn, Caps: shortcuts
    CZ_DEBUG, // Esc, F5 (run), F9-F12 (breakpoint, step)
    CZ_FKEY,  // other F-keys, dim
    CZ_PAD,   // numpad digits, dim
    CZ_LIGHT,
    CZ_COUNT
};
#    define __ CZ_NONE
#    define _l CZ_LET
#    define _d CZ_DIG
#    define _b CZ_BRKT
#    define _o CZ_OPER
#    define _p CZ_PUNCT
#    define _e CZ_EDIT
#    define _n CZ_NAV
#    define _m CZ_MOD
#    define _x CZ_DEBUG
#    define _f CZ_FKEY
#    define _k CZ_PAD
#    define _G CZ_LIGHT
static const uint8_t PROGMEM k4v2_map_code[6][19] = {
    // 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
    {_x, _f, _f, _f, _f, _x, _f, _f, _f, _x, _x, _x, _x, _e, _n, _n, _n, _n, _G},
    {_p, _d, _d, _d, _d, _d, _d, _d, _d, _b, _b, _o, _o, _e, __, _k, _o, _o, _o},
    {_e, _l, _l, _l, _l, _l, _l, _l, _l, _l, _l, _b, _b, _p, __, _k, _k, _k, _o},
    {_m, _l, _l, _l, _l, _l, _l, _l, _l, _l, _p, _p, _p, _e, __, _k, _k, _k, __},
    {_m, _p, _l, _l, _l, _l, _l, _l, _l, _p, _p, _o, __, _m, _n, _k, _k, _k, _e},
    {_m, _m, _m, __, __, __, _e, __, __, __, _m, _m, _m, _n, _n, _n, _k, _k, __},
};
#    undef __
#    undef _l
#    undef _d
#    undef _b
#    undef _o
#    undef _p
#    undef _e
#    undef _n
#    undef _m
#    undef _x
#    undef _f
#    undef _k
#    undef _G

// ---- Palettes: {hue, saturation, value} per group (0-255) ---------------------
// Hue: 0 red, 21 orange, 43 yellow, 64 lime, 85 green, 128 cyan, 170 blue,
// 191 purple, 213 magenta, 234 pink. Saturation 0 = white. Value scales the
// global brightness (255 = full, lower = dimmer).
typedef uint8_t k4v2_hsv3_t[3];

// General groups, in enum k4v2_zone order:
//   none, Esc, F-row, numbers, letters, symbols, modifiers, navigation, numpad, light key
static const k4v2_hsv3_t k4v2_pal_classic[K4V2_ZONE_COUNT] = {{0, 0, 0}, {0, 255, 255}, {191, 255, 255}, {21, 255, 255}, {128, 255, 255}, {43, 255, 255}, {0, 0, 255}, {85, 255, 255}, {170, 255, 255}, {213, 255, 255}};
static const k4v2_hsv3_t k4v2_pal_ocean[K4V2_ZONE_COUNT]   = {{0, 0, 0}, {8, 200, 255}, {150, 255, 255}, {128, 255, 255}, {170, 255, 255}, {110, 255, 255}, {150, 60, 255}, {100, 255, 255}, {185, 220, 255}, {8, 200, 255}};
static const k4v2_hsv3_t k4v2_pal_sunset[K4V2_ZONE_COUNT]  = {{0, 0, 0}, {0, 255, 255}, {245, 255, 255}, {10, 255, 255}, {21, 255, 255}, {36, 255, 255}, {28, 120, 255}, {225, 220, 255}, {200, 230, 255}, {245, 255, 255}};
static const k4v2_hsv3_t k4v2_pal_neon[K4V2_ZONE_COUNT]    = {{0, 0, 0}, {0, 255, 255}, {213, 255, 255}, {43, 255, 255}, {128, 255, 255}, {85, 255, 255}, {191, 255, 255}, {234, 255, 255}, {170, 255, 255}, {64, 255, 255}};
static const k4v2_hsv3_t k4v2_pal_ice[K4V2_ZONE_COUNT]     = {{0, 0, 0}, {170, 255, 255}, {160, 200, 255}, {150, 170, 255}, {0, 0, 255}, {140, 150, 255}, {170, 90, 255}, {128, 210, 255}, {165, 255, 255}, {128, 255, 255}};

// Gaming: hues far apart so neighbouring groups don't blend on the LEDs.
static const k4v2_hsv3_t k4v2_pal_gaming[GZ_COUNT] = {
    {0, 0, 0},       // none
    {191, 255, 28},  // rest: very dim purple
    {0, 255, 255},   // W A S D: red
    {43, 255, 255},  // Q E R F G: yellow
    {85, 255, 255},  // 1-5: green
    {128, 255, 255}, // Space, Shift, Ctrl, Alt: cyan
    {172, 255, 255}, // Esc, Tab, T, Z X C V B, M, arrows: blue
    {213, 255, 255}, // light key
};

// Code: none, letters, digits, brackets, operators, punctuation, editing,
// navigation, modifiers, debug, other F-keys, numpad, light key
static const k4v2_hsv3_t k4v2_pal_code[CZ_COUNT] = {
    {0, 0, 0},       // none
    {28, 50, 120},   // letters: soft warm white
    {21, 255, 120},  // digits: soft orange
    {213, 255, 255}, // brackets: magenta
    {43, 255, 255},  // operators: yellow
    {85, 255, 255},  // punctuation: green
    {0, 0, 255},     // editing: white
    {170, 255, 255}, // navigation: blue
    {191, 255, 220}, // modifiers: purple
    {0, 255, 255},   // debug + Esc: red
    {191, 120, 80},  // other F-keys: dim lavender
    {150, 200, 90},  // numpad digits: dim blue
    {213, 255, 255}, // light key
};

// custom != NULL: use the editable palette (hue, sat; full value, no global
// hue/sat adjustment). Otherwise use the fixed 3-byte palette and apply the
// global hue offset and saturation scale.
static bool k4v2_zones_render(effect_params_t *params, const uint8_t (*map)[19], const k4v2_hsv3_t *pal, const uint8_t (*custom)[2]) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];
            if (led == NO_LED || led < led_min || led >= led_max) continue;
            uint8_t zone = pgm_read_byte(&map[row][col]);
            if (zone == 0) continue;
            hsv_t hsv;
            if (custom) {
                hsv.h = custom[zone][0];
                hsv.s = custom[zone][1];
                hsv.v = rgb_matrix_config.hsv.v;
            } else {
                hsv.h = pal[zone][0] + rgb_matrix_config.hsv.h;
                hsv.s = (uint16_t)pal[zone][1] * rgb_matrix_config.hsv.s / 255;
                hsv.v = (uint16_t)pal[zone][2] * rgb_matrix_config.hsv.v / 255;
            }
            rgb_t rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static bool K4V2_ZONES(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_map_general, NULL, (const uint8_t (*)[2])k4v2_zone_custom);
}
static bool K4V2_ZONES_CLASSIC(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_map_general, k4v2_pal_classic, NULL);
}
static bool K4V2_ZONES_GAMING(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_map_gaming, k4v2_pal_gaming, NULL);
}
static bool K4V2_ZONES_CODE(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_map_code, k4v2_pal_code, NULL);
}
static bool K4V2_ZONES_OCEAN(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_map_general, k4v2_pal_ocean, NULL);
}
static bool K4V2_ZONES_SUNSET(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_map_general, k4v2_pal_sunset, NULL);
}
static bool K4V2_ZONES_NEON(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_map_general, k4v2_pal_neon, NULL);
}
static bool K4V2_ZONES_ICE(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_map_general, k4v2_pal_ice, NULL);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
