// Copyright 2026 QMK-on-K4V2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Included after keyboard.json's settings: trims the built-in effect list so
// the light key goes through fewer, more useful effects. Dropping the
// framebuffer effects (typing heatmap, digital rain) also frees RAM, which
// the VIA build is short of.
//
// Kept: solid color, breathing, cycle all, cycle left/right, rainbow beacon,
// hue wave, raindrops, reactive simple, splash + the Zones effects.
// If you change this list, update EFFECTS in scripts/gen-via-lighting.py,
// run it, and fix the effect-ID asserts in k4v2_zones.c.

#pragma once

#undef ENABLE_RGB_MATRIX_GRADIENT_LEFT_RIGHT
#undef ENABLE_RGB_MATRIX_BAND_SAT
#undef ENABLE_RGB_MATRIX_BAND_VAL
#undef ENABLE_RGB_MATRIX_DUAL_BEACON
#undef ENABLE_RGB_MATRIX_HUE_BREATHING
#undef ENABLE_RGB_MATRIX_TYPING_HEATMAP
#undef ENABLE_RGB_MATRIX_DIGITAL_RAIN
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE
#undef ENABLE_RGB_MATRIX_MULTISPLASH
#undef ENABLE_RGB_MATRIX_SOLID_SPLASH
#undef ENABLE_RGB_MATRIX_SOLID_MULTISPLASH
#undef ENABLE_RGB_MATRIX_PIXEL_RAIN
