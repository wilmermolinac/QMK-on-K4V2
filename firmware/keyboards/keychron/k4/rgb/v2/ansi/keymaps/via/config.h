// Copyright 2026 QMK-on-K4V2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// Windows base, Mac base, Fn (see ../../../config.h). Four layers do not fit
// the 1 KB emulated EEPROM, and growing it does not fit in RAM with VIA.
#define DYNAMIC_KEYMAP_LAYER_COUNT 3
