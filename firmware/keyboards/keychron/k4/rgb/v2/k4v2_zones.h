// Copyright 2026 QMK-on-K4V2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
//
// "Zones" lighting: one color per key group. The colors of the "Zones" effect
// can be edited from VIA and are saved in the keyboard; the "Zones <name>"
// effects are fixed palettes (see rgb_matrix_kb.inc).

#pragma once

#include <stdint.h>

// Key groups. The numbers are also the VIA value IDs of the color pickers
// (channel id_custom_channel), so don't reorder them.
enum k4v2_zone {
    K4V2_ZONE_NONE = 0,
    K4V2_ZONE_ESC,   // Esc
    K4V2_ZONE_FROW,  // F1-F12
    K4V2_ZONE_NUM,   // ` 1-0 - =
    K4V2_ZONE_LET,   // A-Z
    K4V2_ZONE_SYM,   // [ ] \ ; ' , . / (and the ISO # and \ keys)
    K4V2_ZONE_MOD,   // Tab, Caps, Shift, Ctrl, Win/Cmd, Alt/Opt, Fn, Enter, Backspace, Space
    K4V2_ZONE_NAV,   // Del, Home, End, PgUp, PgDn, arrows
    K4V2_ZONE_PAD,   // numpad
    K4V2_ZONE_LIGHT, // light key (top right)
    K4V2_ZONE_COUNT
};

// Hue/saturation (0-255) per zone, index 0 unused. Lives in RAM, loaded from
// EEPROM at boot.
extern uint8_t k4v2_zone_custom[K4V2_ZONE_COUNT][2];

void k4v2_zones_init(void);
void k4v2_zones_task(void);  // call from housekeeping: saves pending VIA edits
void k4v2_zones_reset(void); // back to the "Classic" colors (RAM + EEPROM)
