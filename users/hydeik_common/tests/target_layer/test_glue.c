/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "test_glue.h"

#include "quantum.h"
#include "target_layer.h"

const target_layer_entry_t target_layer_entries[] = {
    {TAB_SYM, TEST_LAYER_NAV, TARGET_LAYER_QUICK_RELEASE_NOTHING},
    {ENT_NUM, TEST_LAYER_NAV, TARGET_LAYER_QUICK_RELEASE_NOTHING},
    {ENT_NUM, TEST_LAYER_FUN, TARGET_LAYER_QUICK_RELEASE_NOTHING},
};

const uint8_t target_layer_entry_count = sizeof(target_layer_entries) / sizeof(target_layer_entries[0]);

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return target_layer_process_record(keycode, record);
}

void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
    target_layer_post_process_record(keycode, record);
}

void housekeeping_task_user(void) {
    target_layer_housekeeping_task();
}
