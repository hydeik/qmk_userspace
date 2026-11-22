/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * This file is built against both firmware sources, so it must use nothing
 * that upstream QMK added after the last merge of vial-qmk.
 */

#include "key_typed.h"

#include "quantum_keycodes.h"

bool is_key_typed(uint16_t keycode, const keyrecord_t *record) {
    if (IS_MODIFIER_KEYCODE(keycode) || keycode == QK_LAYER_LOCK) {
        return false;
    }
    if (IS_QK_MOD_TAP(keycode) || IS_QK_LAYER_TAP(keycode)) {
        /* Tapped, not held. */
        return record->tap.count != 0;
    }
    return !(IS_QK_TO(keycode) || IS_QK_MOMENTARY(keycode) || IS_QK_LAYER_MOD(keycode) || IS_QK_DEF_LAYER(keycode) || IS_QK_PERSISTENT_DEF_LAYER(keycode) || IS_QK_TOGGLE_LAYER(keycode) || IS_QK_ONE_SHOT_LAYER(keycode) || IS_QK_LAYER_TAP_TOGGLE(keycode));
}
