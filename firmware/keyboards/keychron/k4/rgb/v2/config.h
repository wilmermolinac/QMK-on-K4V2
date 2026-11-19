// Copyright 2026 QMK-on-K4V2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// Keyboard-level keycode: show the battery level on the number row (Bluetooth only).
#define KC_BT_BAT QK_KB_0

// Layer layout used by the Mac/Windows switch (D6). Keymaps with fewer layers
// simply stay on the Windows base layer.
#define K4V2_LAYER_WIN_BASE 0
#define K4V2_LAYER_WIN_FN   1
#define K4V2_LAYER_MAC_BASE 2
#define K4V2_LAYER_MAC_FN   3
