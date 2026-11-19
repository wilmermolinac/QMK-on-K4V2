// Copyright 2026 QMK-on-K4V2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
//
// "Zones" effect: static colors per key group (Esc, F-row, number row,
// letters, symbols, modifiers, navigation, numpad, light key).
// - Brightness follows the normal RGB brightness (VIA, F5/F6).
// - The VIA "Color" picker rotates the whole palette (hue offset), and its
//   saturation scales all zones. With the defaults (hue 0) you get the
//   palette below.
// The zone map is in flash (PROGMEM): the VIA build has almost no free RAM.

RGB_MATRIX_EFFECT(K4V2_ZONES)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

enum k4v2_zone {
    Z_NONE = 0,
    Z_ESC,   // Esc
    Z_FROW,  // F1-F12
    Z_NUM,   // ` 1-0 - =
    Z_LET,   // A-Z
    Z_SYM,   // [ ] \ ; ' , . / (and the ISO # and \ keys)
    Z_MOD,   // Tab, Caps, Shift, Ctrl, Win/Cmd, Alt/Opt, Fn, Enter, Backspace, Space
    Z_NAV,   // Del, Home, End, PgUp, PgDn, arrows
    Z_PAD,   // numpad
    Z_LIGHT, // light key (top right)
    Z_COUNT
};

#    define _N Z_NONE
#    define _E Z_ESC
#    define _F Z_FROW
#    define _1 Z_NUM
#    define _L Z_LET
#    define _S Z_SYM
#    define _M Z_MOD
#    define _V Z_NAV
#    define _P Z_PAD
#    define _G Z_LIGHT

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

// Palette: hue (0-255), saturation (0-255) per zone.
static const uint8_t PROGMEM k4v2_zone_hs[Z_COUNT][2] = {
    [Z_NONE]  = {0, 0},
    [Z_ESC]   = {0, 255},   // red
    [Z_FROW]  = {191, 255}, // purple
    [Z_NUM]   = {21, 255},  // orange
    [Z_LET]   = {128, 255}, // cyan
    [Z_SYM]   = {43, 255},  // yellow
    [Z_MOD]   = {0, 0},     // white
    [Z_NAV]   = {85, 255},  // green
    [Z_PAD]   = {170, 255}, // blue
    [Z_LIGHT] = {213, 255}, // magenta
};

static bool K4V2_ZONES(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];
            if (led == NO_LED || led < led_min || led >= led_max) continue;
            uint8_t zone = pgm_read_byte(&k4v2_zone_map[row][col]);
            if (zone == Z_NONE) continue;
            hsv_t hsv = {
                .h = pgm_read_byte(&k4v2_zone_hs[zone][0]) + rgb_matrix_config.hsv.h,
                .s = (uint16_t)pgm_read_byte(&k4v2_zone_hs[zone][1]) * rgb_matrix_config.hsv.s / 255,
                .v = rgb_matrix_config.hsv.v,
            };
            rgb_t rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
