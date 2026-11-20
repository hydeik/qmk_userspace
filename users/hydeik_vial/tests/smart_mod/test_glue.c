/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "test_glue.h"

#include "quantum.h"
#include "smart_mod.h"

uint8_t smart_mod_one_shot_mod_bits(uint16_t keycode) {
    switch (keycode) {
        case COS_LCTL:
            return MOD_BIT_LCTRL;
        case COS_LSFT:
            return MOD_BIT_LSHIFT;
        case COS_LALT:
            return MOD_BIT_LALT;
        case COS_LGUI:
            return MOD_BIT_LGUI;
        case COS_RCTL:
            return MOD_BIT_RCTRL;
        case COS_RSFT:
            return MOD_BIT_RSHIFT;
        case COS_RALT:
            return MOD_BIT_RALT;
        case COS_RGUI:
            return MOD_BIT_RGUI;
        default:
            return 0;
    }
}

uint8_t smart_mod_layer(void) {
    return TEST_LAYER_MOD;
}

bool smart_mod_is_layer_key(uint16_t keycode) {
    switch (keycode) {
        case SMT_MOD:
        case SPC_MOD:
        case ENT_MOD:
            return true;
        default:
            return false;
    }
}

bool smart_mod_layer_key_taps_shift(uint16_t keycode) {
    return keycode == SMT_MOD;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return smart_mod_process_record(keycode, record);
}

void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
    smart_mod_post_process_record(keycode, record);
}

void housekeeping_task_user(void) {
    smart_mod_housekeeping_task();
}
