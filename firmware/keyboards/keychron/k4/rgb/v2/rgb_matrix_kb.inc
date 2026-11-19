// Copyright 2026 QMK-on-K4V2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
//
// "Zones" effects: static colors per key group (Esc, F-row, number row,
// letters, symbols, modifiers, navigation, numpad, light key).
//
// - K4V2_ZONES: your own colors. Edit them in VIA (Lighting -> Zones colors);
//   they're saved in the keyboard. Default = Classic.
// - K4V2_ZONES_<NAME>: fixed palettes. The global hue (Fn + numpad -, or VIA's
//   Color) rotates them and the global saturation fades them.
// Brightness always follows the normal RGB brightness.
// Tables are const (flash): the VIA build has very little free RAM.

RGB_MATRIX_EFFECT(K4V2_ZONES)
RGB_MATRIX_EFFECT(K4V2_ZONES_CLASSIC)
RGB_MATRIX_EFFECT(K4V2_ZONES_OCEAN)
RGB_MATRIX_EFFECT(K4V2_ZONES_SUNSET)
RGB_MATRIX_EFFECT(K4V2_ZONES_NEON)
RGB_MATRIX_EFFECT(K4V2_ZONES_FOREST)
RGB_MATRIX_EFFECT(K4V2_ZONES_CANDY)
RGB_MATRIX_EFFECT(K4V2_ZONES_ICE)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#    include "k4v2_zones.h"

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

// Same matrix for ANSI and ISO: positions without an LED on a layout are skipped.
static const uint8_t PROGMEM k4v2_zone_map[6][19] = {
    // 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
    {_E, _F, _F, _F, _F, _F, _F, _F, _F, _F, _F, _F, _F, _V, _V, _V, _V, _V, _G}, // Esc F1-F12 Del Home End PgUp PgDn Light
    {_1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _M, _N, _P, _P, _P, _P}, // ` 1-0 - = Bksp | NumLk / * -
    {_M, _L, _L, _L, _L, _L, _L, _L, _L, _L, _L, _S, _S, _S, _N, _P, _P, _P, _P}, // Tab Q-P [ ] \ | 7 8 9 +
    {_M, _L, _L, _L, _L, _L, _L, _L, _L, _L, _S, _S, _S, _M, _N, _P, _P, _P, _N}, // Caps A-L ; ' (ISO #) Enter | 4 5 6
    {_M, _S, _L, _L, _L, _L, _L, _L, _L, _S, _S, _S, _N, _M, _V, _P, _P, _P, _P}, // LShift (ISO \) Z-M , . / RShift Up | 1 2 3 Ent
    {_M, _M, _M, _N, _N, _N, _M, _N, _N, _N, _M, _M, _M, _V, _V, _V, _P, _P, _N}, // Ctrl Win Alt Space RAlt Fn RCtrl arrows | 0 .
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

// Fixed palettes: {hue, saturation} (0-255) per zone, in enum k4v2_zone order:
//   none, Esc, F-row, numbers, letters, symbols, modifiers, navigation, numpad, light key
// Hue: 0 red, 21 orange, 43 yellow, 64 lime, 85 green, 128 cyan, 170 blue,
// 191 purple, 213 magenta, 234 pink. Saturation 0 = white.
typedef uint8_t k4v2_palette_t[K4V2_ZONE_COUNT][2];

static const k4v2_palette_t k4v2_pal_classic = {{0, 0}, {0, 255}, {191, 255}, {21, 255}, {128, 255}, {43, 255}, {0, 0}, {85, 255}, {170, 255}, {213, 255}};
static const k4v2_palette_t k4v2_pal_ocean   = {{0, 0}, {8, 200}, {150, 255}, {128, 255}, {170, 255}, {110, 255}, {150, 60}, {100, 255}, {185, 220}, {8, 200}};
static const k4v2_palette_t k4v2_pal_sunset  = {{0, 0}, {0, 255}, {245, 255}, {10, 255}, {21, 255}, {36, 255}, {28, 120}, {225, 220}, {200, 230}, {245, 255}};
static const k4v2_palette_t k4v2_pal_neon    = {{0, 0}, {0, 255}, {213, 255}, {43, 255}, {128, 255}, {85, 255}, {191, 255}, {234, 255}, {170, 255}, {64, 255}};
static const k4v2_palette_t k4v2_pal_forest  = {{0, 0}, {21, 255}, {64, 255}, {40, 230}, {85, 255}, {110, 230}, {30, 110}, {128, 220}, {75, 180}, {21, 255}};
static const k4v2_palette_t k4v2_pal_candy   = {{0, 0}, {0, 150}, {200, 140}, {21, 150}, {128, 120}, {43, 150}, {0, 0}, {85, 130}, {170, 120}, {230, 150}};
static const k4v2_palette_t k4v2_pal_ice     = {{0, 0}, {170, 255}, {160, 200}, {150, 170}, {0, 0}, {140, 150}, {170, 90}, {128, 210}, {165, 255}, {128, 255}};

// adjust = true: apply the global hue offset and saturation scale.
static bool k4v2_zones_render(effect_params_t *params, const uint8_t (*pal)[2], bool adjust) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];
            if (led == NO_LED || led < led_min || led >= led_max) continue;
            uint8_t zone = pgm_read_byte(&k4v2_zone_map[row][col]);
            if (zone == K4V2_ZONE_NONE) continue;
            hsv_t hsv = {.h = pal[zone][0], .s = pal[zone][1], .v = rgb_matrix_config.hsv.v};
            if (adjust) {
                hsv.h += rgb_matrix_config.hsv.h;
                hsv.s = (uint16_t)hsv.s * rgb_matrix_config.hsv.s / 255;
            }
            rgb_t rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static bool K4V2_ZONES(effect_params_t *params) {
    return k4v2_zones_render(params, (const uint8_t (*)[2])k4v2_zone_custom, false);
}
static bool K4V2_ZONES_CLASSIC(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_pal_classic, true);
}
static bool K4V2_ZONES_OCEAN(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_pal_ocean, true);
}
static bool K4V2_ZONES_SUNSET(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_pal_sunset, true);
}
static bool K4V2_ZONES_NEON(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_pal_neon, true);
}
static bool K4V2_ZONES_FOREST(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_pal_forest, true);
}
static bool K4V2_ZONES_CANDY(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_pal_candy, true);
}
static bool K4V2_ZONES_ICE(effect_params_t *params) {
    return k4v2_zones_render(params, k4v2_pal_ice, true);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
