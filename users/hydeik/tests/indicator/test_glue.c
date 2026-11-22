/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "test_glue.h"

#include "quantum.h"
#include "smart_mod.h"
#include "target_layer.h"

_Static_assert(TEST_LAYER_COUNT == MAX_LAYER, "the keymap of the tests must have every layer that the logic looks at");

/*****************************************************************************
 * Told to smart_mod
 *****************************************************************************/

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
    return keycode == SMT_MOD || keycode == SPC_MOD;
}

bool smart_mod_layer_key_taps_shift(uint16_t keycode) {
    return keycode == SMT_MOD;
}

/*****************************************************************************
 * Told to target_layer
 *****************************************************************************/

const target_layer_entry_t target_layer_entries[] = {
    {TAB_SYM, TEST_LAYER_NAV, TARGET_LAYER_QUICK_RELEASE_LOCKED},
    {TAB_SYM, TEST_LAYER_MOU, TARGET_LAYER_QUICK_RELEASE_LOCKED},
    {ENT_NUM, TEST_LAYER_FUN, TARGET_LAYER_QUICK_RELEASE_PENDING},
    {ENT_NUM, TEST_LAYER_NAV, TARGET_LAYER_QUICK_RELEASE_NOTHING},
};

const uint8_t target_layer_entry_count = sizeof(target_layer_entries) / sizeof(target_layer_entries[0]);

bool target_layer_is_one_shot_mod_key(uint16_t keycode, const keyrecord_t *record) {
    if (smart_mod_one_shot_mod_bits(keycode) != 0) {
        return true;
    }
    return smart_mod_is_layer_key(keycode) && smart_mod_layer_key_taps_shift(keycode) && record->tap.count != 0;
}

bool target_layer_is_pending_timeout_suspended(void) {
    return smart_mod_is_layer_key_held();
}

/*****************************************************************************
 * QMK hooks, in the order of users/hydeik/hydeik.c
 *****************************************************************************/

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    const bool target_layer_passes = target_layer_process_record(keycode, record);
    const bool smart_mod_passes    = smart_mod_process_record(keycode, record);
    return target_layer_passes && smart_mod_passes;
}

void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
    smart_mod_post_process_record(keycode, record);
    target_layer_post_process_record(keycode, record);
}

void housekeeping_task_user(void) {
    smart_mod_housekeeping_task();
    target_layer_housekeeping_task();
}
