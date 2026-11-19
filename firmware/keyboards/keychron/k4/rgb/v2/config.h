// Copyright 2026 QMK-on-K4V2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// Keyboard-level keycode: show the battery level on the number row (Bluetooth only).
#define KC_BT_BAT QK_KB_0

// Layer layout used by the Mac/Windows switch (D6). Three layers so the VIA
// keymap fits the SN32F248B's 1 KB emulated EEPROM / 8 KB RAM:
//   0 = Windows base, 1 = Mac base, 2 = Fn (shared, above both bases).
// In Mac mode Fn + F1..F12 sends the Windows base layer's F-row keys.
#define K4V2_LAYER_WIN_BASE 0
#define K4V2_LAYER_MAC_BASE 1
#define K4V2_LAYER_FN       2

// Start with the per-group "Zones" lighting (see rgb_matrix_kb.inc). The light
// key cycles through it, the fixed palettes and the other effects.
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_CUSTOM_K4V2_ZONES

// EEPROM space for the editable Zones palette (k4v2_zones.c).
#define EECONFIG_KB_DATA_SIZE 20
