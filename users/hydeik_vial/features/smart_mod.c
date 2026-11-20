/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "smart_mod.h"

bool smart_mod_process_record(uint16_t keycode, keyrecord_t *record) {
    const uint8_t mods = smart_mod_one_shot_mod_bits(keycode);
    if (!mods) {
        return true;
    }

    /* A one-shot mod key is an ordinary modifier while held. */
    if (record->event.pressed) {
        register_mods(mods);
    } else {
        unregister_mods(mods);
    }
    return false;
}

void smart_mod_post_process_record(uint16_t keycode, keyrecord_t *record) {}

void smart_mod_housekeeping_task(void) {}
