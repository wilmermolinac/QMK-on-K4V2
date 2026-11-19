// Copyright 2026 QMK-on-K4V2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Editable "Zones" palette: kept in RAM for the effect, stored in the
// keyboard's EEPROM data block, and exposed to VIA as one color picker per
// key group (custom channel 0, value IDs = enum k4v2_zone).

#include "quantum.h"
#include "eeconfig.h"
#include "k4v2_zones.h"
#include <string.h>

#ifdef VIA_ENABLE
#    include "via.h"
#endif

#define K4V2_ZONES_MAGIC 0x5A

typedef struct {
    uint8_t magic;
    uint8_t hs[K4V2_ZONE_COUNT - 1][2];
} k4v2_zones_eeprom_t;

_Static_assert(sizeof(k4v2_zones_eeprom_t) <= EECONFIG_KB_DATA_SIZE, "EECONFIG_KB_DATA_SIZE too small for the Zones palette");

// The effect dropdown in via/*.json uses these numbers
// (scripts/gen-via-lighting.py). If the effect list changes, update both.
_Static_assert(RGB_MATRIX_SOLID_REACTIVE_SIMPLE == 8, "effect IDs changed: update scripts/gen-via-lighting.py");
_Static_assert(RGB_MATRIX_SPLASH == 9, "effect IDs changed: update scripts/gen-via-lighting.py");
_Static_assert(RGB_MATRIX_CUSTOM_K4V2_ZONES == 10, "effect IDs changed: update scripts/gen-via-lighting.py");
_Static_assert(RGB_MATRIX_CUSTOM_K4V2_ZONES_ICE == 17, "effect IDs changed: update scripts/gen-via-lighting.py");
_Static_assert(RGB_MATRIX_EFFECT_MAX == 18, "effect IDs changed: update scripts/gen-via-lighting.py");

// Default = "Classic" palette.
static const uint8_t k4v2_zone_default[K4V2_ZONE_COUNT][2] = {
    [K4V2_ZONE_NONE]  = {0, 0},
    [K4V2_ZONE_ESC]   = {0, 255},   // red
    [K4V2_ZONE_FROW]  = {191, 255}, // purple
    [K4V2_ZONE_NUM]   = {21, 255},  // orange
    [K4V2_ZONE_LET]   = {128, 255}, // cyan
    [K4V2_ZONE_SYM]   = {43, 255},  // yellow
    [K4V2_ZONE_MOD]   = {0, 0},     // white
    [K4V2_ZONE_NAV]   = {85, 255},  // green
    [K4V2_ZONE_PAD]   = {170, 255}, // blue
    [K4V2_ZONE_LIGHT] = {213, 255}, // magenta
};

uint8_t k4v2_zone_custom[K4V2_ZONE_COUNT][2];

// VIA sends a stream of values while a color picker is dragged: write the
// EEPROM once things settle (or on VIA's explicit save).
#define K4V2_ZONES_SAVE_DELAY_MS 1500
static bool     zones_dirty;
static uint16_t zones_dirty_timer;

static void k4v2_zones_save(void) {
    k4v2_zones_eeprom_t e;
    e.magic = K4V2_ZONES_MAGIC;
    memcpy(e.hs, &k4v2_zone_custom[1][0], sizeof(e.hs));
    eeconfig_update_kb_datablock(&e, 0, sizeof(e));
    zones_dirty = false;
}

void k4v2_zones_task(void) {
    if (zones_dirty && timer_elapsed(zones_dirty_timer) >= K4V2_ZONES_SAVE_DELAY_MS) {
        k4v2_zones_save();
    }
}

void k4v2_zones_reset(void) {
    memcpy(k4v2_zone_custom, k4v2_zone_default, sizeof(k4v2_zone_custom));
    k4v2_zones_save();
}

void k4v2_zones_init(void) {
    k4v2_zones_eeprom_t e;
    eeconfig_read_kb_datablock(&e, 0, sizeof(e));
    if (e.magic == K4V2_ZONES_MAGIC) {
        k4v2_zone_custom[0][0] = 0;
        k4v2_zone_custom[0][1] = 0;
        memcpy(&k4v2_zone_custom[1][0], e.hs, sizeof(e.hs));
    } else {
        memcpy(k4v2_zone_custom, k4v2_zone_default, sizeof(k4v2_zone_custom));
    }
}

// Called by QMK when the EEPROM is (re)initialised, e.g. after a new VIA build.
void eeconfig_init_kb(void) {
    k4v2_zones_reset();
    eeconfig_init_user();
}

#ifdef VIA_ENABLE
// data = [ command_id, channel_id, value_id, value_data... ]
void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    uint8_t *command_id = &data[0];
    uint8_t  channel_id = data[1];
    uint8_t  value_id   = data[2];
    uint8_t *value_data = &data[3];

    if (channel_id != id_custom_channel || value_id == K4V2_ZONE_NONE || value_id >= K4V2_ZONE_COUNT) {
        *command_id = id_unhandled;
        return;
    }
    switch (*command_id) {
        case id_custom_set_value:
            k4v2_zone_custom[value_id][0] = value_data[0];
            k4v2_zone_custom[value_id][1] = value_data[1];
            zones_dirty       = true;
            zones_dirty_timer = timer_read();
            break;
        case id_custom_get_value:
            value_data[0] = k4v2_zone_custom[value_id][0];
            value_data[1] = k4v2_zone_custom[value_id][1];
            break;
        case id_custom_save:
            k4v2_zones_save();
            break;
        default:
            *command_id = id_unhandled;
            break;
    }
}
#endif
