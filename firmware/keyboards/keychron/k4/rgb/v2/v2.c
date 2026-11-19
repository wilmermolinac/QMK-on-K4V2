// Copyright 2026 mgrenonville (K3 v2 board code this is based on)
// Copyright 2026 QMK-on-K4V2 contributors (K4 v2 port)
// Indicator/profile handling originally based on the Keychron K4 V2 Bluetooth
// integration by reubenjs (SonixQMK PR #486).
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Keychron K4 v2 board code: Bluetooth (ITON module via k4v2_bt.c, behaviour
// modelled on the stock firmware), Mac/Windows switch, indicators, idle sleep.

#include "quantum.h"
#include "connection.h"
#include "keymap_introspection.h"
#include "k4v2_bt.h"
#include "k4v2_zones.h"
#include <string.h>

#ifdef CONSOLE_ENABLE
#    include "print.h"
#    include "debug.h"
#endif

#define BT_PAIR_HOLD_MS      3000
#define BT_PAIR_REPEAT_MS    100
#define BT_PAIR_SENDS        3
#define BT_LED_CONNECTED_MS  2500
#define BT_BATT_SHOW_MS      5000
#define BT_BATT_FILL_STEP_MS 120
#define BT_BATT_QUERY_DELAY  2000

// Cable/BT slider: DIP index 0 on D5, pin high = Bluetooth.
#define K4V2_CONN_SWITCH_PIN D5

// Optional: give the switches time to settle after a row is selected. The
// optical K3 v2 needs this on battery power; enable it only if you see
// chattering keys in Bluetooth mode. Runs in the shared-matrix PWM ISR, so it
// busy-waits (~8 cycles/iteration).
#ifdef K4V2_MATRIX_SELECT_DELAY_US
void matrix_output_select_delay(void) {
    for (volatile uint32_t i = 0; i < (K4V2_MATRIX_SELECT_DELAY_US * (CPU_CLOCK / 8000000UL)); i++) {
    }
}
#endif

static bool     bt_ready        = false;
static bool     bt_mode         = false;
static uint8_t  bt_profile      = 0;
static uint16_t bt_held_keycode = 0;
static uint16_t bt_held_timer   = 0;
static uint8_t  bt_pair_sends   = 0;
static uint16_t bt_pair_timer   = 0;
static uint8_t  bt_led_state    = 0; // 0 idle, 1 pairing flash, 2 connected hold
static uint16_t bt_led_timer    = 0;
static uint8_t  bt_batt_level   = 0;
static bool     bt_batt_show    = false;
static uint16_t bt_batt_timer   = 0;
static bool     bt_batt_query   = false;
static uint16_t bt_batt_query_timer = 0;

// ---- Mac/Windows switch ------------------------------------------------------
// DIP index 1 on D6, active = Mac. Selects the default layer, but only when the
// keymap actually has the Mac layer.
static bool     mac_mode = false;
static uint16_t mac_fn_frow[12]; // keycodes registered by Fn + F-row in Mac mode

static void k4v2_apply_os_mode(bool mac_active) {
#ifndef K4V2_OS_SWITCH_DISABLE
    uint8_t base = mac_active ? K4V2_LAYER_MAC_BASE : K4V2_LAYER_WIN_BASE;
    if (base >= keymap_layer_count()) base = K4V2_LAYER_WIN_BASE;
    mac_mode = (base == K4V2_LAYER_MAC_BASE);
    if (bt_ready) clear_keyboard(); // release held keys on a runtime switch
    memset(mac_fn_frow, 0, sizeof(mac_fn_frow));
    layer_clear();
    default_layer_set((layer_state_t)1 << base);
#endif
}

// Mac mode: the Mac base layer has the macOS functions on the F-row, so
// Fn + F1..F12 sends whatever the Windows base layer has there (F1-F12 by
// default, editable in VIA). Returns true when the event was handled.
static bool k4v2_mac_fn_frow(keyrecord_t *record) {
    uint8_t row = record->event.key.row, col = record->event.key.col;
    if (row != 0 || col < 1 || col > 12) return false;
    uint8_t i = col - 1;
    if (record->event.pressed) {
        if (!mac_mode || !layer_state_is(K4V2_LAYER_FN)) return false;
        uint16_t kc = keymap_key_to_keycode(K4V2_LAYER_WIN_BASE, record->event.key);
        if (!IS_BASIC_KEYCODE(kc)) return false;
        mac_fn_frow[i] = kc;
        register_code(kc);
        return true;
    }
    if (mac_fn_frow[i]) {
        unregister_code(mac_fn_frow[i]);
        mac_fn_frow[i] = 0;
        return true;
    }
    return false;
}

// ---- Idle sleep (BT mode), timings from the stock firmware -------------------
// Stock counts 100 ms ticks: 400 (40 s) while not connected, 6000 (10 min)
// while connected, reset by any key, paused while pairing. The module can also
// request sleep (B6 5A 08). "Sleep" turns the LEDs off (the main power draw);
// the matrix keeps scanning so any key wakes the board.
#ifndef K4V2_IDLE_CONNECTED_MS
#    define K4V2_IDLE_CONNECTED_MS (10UL * 60UL * 1000UL)
#endif
#ifndef K4V2_IDLE_DISCONNECTED_MS
#    define K4V2_IDLE_DISCONNECTED_MS (40UL * 1000UL)
#endif

static bool k4_sleeping      = false;
static bool k4_sleep_request = false;

static void k4_indicators_off(void) {
#ifdef LED_CAPS_LOCK_PIN
    gpio_write_pin(LED_CAPS_LOCK_PIN, !LED_PIN_ON_STATE);
#endif
#ifdef LED_NUM_LOCK_PIN
    gpio_write_pin(LED_NUM_LOCK_PIN, !LED_PIN_ON_STATE);
#endif
}

static void k4_sleep_enter(void) {
    k4_sleeping      = true;
    k4_sleep_request = false;
    rgb_matrix_set_suspend_state(true);
    k4_indicators_off();
#ifdef CONSOLE_ENABLE
    uprintf("[k4v2] t=%lu SLEEP (conn=%d)\n", (unsigned long)timer_read32(), k4bt_connected);
#endif
}

static void k4_sleep_exit(void) {
    k4_sleeping = false;
    rgb_matrix_set_suspend_state(false);
    led_update_kb(host_keyboard_led_state());
#ifdef CONSOLE_ENABLE
    uprintf("[k4v2] t=%lu WAKE\n", (unsigned long)timer_read32());
#endif
}

bool led_update_kb(led_t led_state) {
    if (k4_sleeping) {
        k4_indicators_off();
        return false;
    }
    bool res = led_update_user(led_state);
    if (res) led_update_ports(led_state); // caps lock (B12) / num lock (D4) LEDs
    return res;
}

static void k4_sleep_task(bool pairing) {
    if (k4_sleeping) {
        if (!bt_mode || last_input_activity_elapsed() < 50) k4_sleep_exit();
        return;
    }
    if (!bt_mode || pairing) return;
    uint32_t limit = k4bt_connected ? K4V2_IDLE_CONNECTED_MS : K4V2_IDLE_DISCONNECTED_MS;
    if (k4_sleep_request || last_input_activity_elapsed() > limit) k4_sleep_enter();
}

// ---- Bluetooth mode ------------------------------------------------------------

// LED under the number-row key n (0 = "1" ... 9 = "0").
static inline uint8_t number_row_led(uint8_t n) {
    return g_led_config.matrix_co[1][1 + n];
}

static void k4v2_apply_conn_mode(bool cable_active) {
    bt_mode = !cable_active;
    if (bt_mode) {
        connection_set_host_noeeprom(CONNECTION_HOST_BLUETOOTH);
        k4bt_control(K4BT_GROUP_BT, K4BT_SWITCH_PROFILE + bt_profile);
    } else {
        k4bt_control(K4BT_GROUP_USB, K4BT_USB_MODE);
        connection_set_host_noeeprom(CONNECTION_HOST_USB);
    }
#ifdef CONSOLE_ENABLE
    uprintf("[k4v2] conn mode cable=%d bt_mode=%d\n", cable_active, bt_mode);
#endif
}

bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)) {
        return false;
    }
    if (index == 0) {
        // dip_switch_init() runs before bluetooth_init(): don't talk to the
        // module until keyboard_post_init_kb() has run.
        if (bt_ready) k4v2_apply_conn_mode(active);
    } else if (index == 1) {
        k4v2_apply_os_mode(active);
    }
    return true;
}

void keyboard_post_init_kb(void) {
    bt_ready = true;
    // Like stock: select the host from the slider, send nothing to the module.
    bt_mode = gpio_read_pin(K4V2_CONN_SWITCH_PIN);
    connection_set_host_noeeprom(bt_mode ? CONNECTION_HOST_BLUETOOTH : CONNECTION_HOST_USB);
#ifdef CONSOLE_ENABLE
    debug_enable = true;
    uprintf("[k4v2] post_init bt_mode=%d\n", bt_mode);
#endif
    k4v2_zones_init();
    keyboard_post_init_user();
}

void k4bt_on_notification(uint8_t group, uint8_t code) {
#ifdef CONSOLE_ENABLE
    uprintf("[k4v2] t=%lu NOTIF %02X %02X\n", (unsigned long)timer_read32(), group, code);
#endif
    if (group == K4BT_NOTIF_BT) {
        switch (code) {
            case K4BT_BT_PAIRING:
                bt_led_state  = 1;
                bt_led_timer  = timer_read();
                bt_pair_sends = 0;
                break;
            case K4BT_BT_CONNECTED:
                bt_led_state  = 2;
                bt_led_timer  = timer_read();
                bt_pair_sends = 0;
                // Learn the battery level shortly after connecting so the
                // low-battery reminder works without pressing Fn+B.
                bt_batt_query       = true;
                bt_batt_query_timer = timer_read();
                break;
            case K4BT_BT_DISCONNECTED:
                bt_led_state = 0;
                break;
        }
    } else if (group == K4BT_NOTIF_BATTERY) {
        if (code == K4BT_BATT_ABOVE_70 || code == K4BT_BATT_30_70 || code == K4BT_BATT_BELOW_30) bt_batt_level = code;
        if (code == K4BT_BATT_LOW || code == K4BT_BATT_SHUTDOWN) bt_batt_level = K4BT_BATT_BELOW_30;
        if (code == K4BT_BATT_SLEEP_REQ) k4_sleep_request = true; // module asks the MCU to sleep
    }
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }

    if (k4v2_mac_fn_frow(record)) {
        return false;
    }

    if (keycode == KC_BT_BAT) {
        if (record->event.pressed && bt_mode) {
            bt_batt_show  = true;
            bt_batt_timer = timer_read();
            k4bt_control(K4BT_GROUP_BT, K4BT_QUERY_BATTERY);
        }
        return false;
    }

    switch (keycode) {
        case BT_PRF1:
        case BT_PRF2:
        case BT_PRF3:
            if (!bt_mode) return false;
            if (record->event.pressed) {
                bt_profile      = keycode - BT_PRF1;
                bt_held_keycode = keycode;
                bt_held_timer   = timer_read();
                bt_pair_sends   = 0;
                k4bt_control(K4BT_GROUP_BT, K4BT_SWITCH_PROFILE + bt_profile);
            } else {
                bt_held_keycode = 0;
            }
            return false;
    }
    return true;
}

void housekeeping_task_kb(void) {
    k4v2_zones_task();

    // Hold a profile key: like stock, send enter_pairing a few times.
    if (bt_mode && bt_held_keycode && timer_elapsed(bt_held_timer) >= BT_PAIR_HOLD_MS) {
        bt_held_keycode = 0;
        bt_pair_sends   = BT_PAIR_SENDS;
        bt_pair_timer   = timer_read() - BT_PAIR_REPEAT_MS;
    }
    if (bt_pair_sends && timer_elapsed(bt_pair_timer) >= BT_PAIR_REPEAT_MS) {
        bt_pair_timer = timer_read();
        bt_pair_sends--;
        k4bt_control(K4BT_GROUP_BT, K4BT_ENTER_PAIRING);
    }

    if (bt_mode && bt_batt_query && timer_elapsed(bt_batt_query_timer) >= BT_BATT_QUERY_DELAY) {
        bt_batt_query = false;
        k4bt_control(K4BT_GROUP_BT, K4BT_QUERY_BATTERY);
    }

    // Pairing in progress: profile key held, enter_pairing pending, or module pairing.
    k4_sleep_task(bt_held_keycode || bt_pair_sends || bt_led_state == 1);
}

// Pairing: blank the matrix and flash the profile key blue (only after the
// module confirms), hold it solid on connect. Fn+B: battery gauge on 1..0.
bool rgb_matrix_indicators_kb(void) {
    if (!rgb_matrix_indicators_user()) {
        return false;
    }
    if (!bt_mode) {
        return true;
    }
    if (bt_led_state) {
        if (bt_led_state == 2 && timer_elapsed(bt_led_timer) >= BT_LED_CONNECTED_MS) {
            bt_led_state = 0;
            return true;
        }
        rgb_matrix_set_color_all(0, 0, 0);
        if (bt_led_state == 2 || (timer_elapsed(bt_led_timer) % 500) < 250) {
            rgb_matrix_set_color(number_row_led(bt_profile), 0, 0, 255);
        }
        return true;
    }
    if (bt_batt_show) {
        uint16_t elapsed = timer_elapsed(bt_batt_timer);
        if (elapsed < BT_BATT_SHOW_MS) {
            uint8_t leds = 10, r = 0, g = 0, b = 0;
            switch (bt_batt_level) {
                case K4BT_BATT_BELOW_30:
                    leds = 3;
                    r    = 180;
                    break;
                case K4BT_BATT_30_70:
                    leds = 7;
                    r    = 180;
                    g    = 72;
                    break;
                case K4BT_BATT_ABOVE_70:
                    g = 180;
                    break;
                default:
                    b = 80; // unknown
                    break;
            }
            uint8_t lit = elapsed / BT_BATT_FILL_STEP_MS + 1;
            rgb_matrix_set_color_all(1, 1, 1);
            for (uint8_t i = 0; i < 10; i++) {
                bool on = i < leds && i < lit;
                rgb_matrix_set_color(number_row_led(i), on ? r : 0, on ? g : 0, on ? b : 0);
            }
            return true;
        }
        bt_batt_show = false;
    }
    if (bt_batt_level == K4BT_BATT_BELOW_30 && (timer_read() % 4000) < 600) {
        rgb_matrix_set_color(number_row_led(bt_profile), 255, 0, 0);
    }
    return true;
}
