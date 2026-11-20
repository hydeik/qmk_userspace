/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Test-only glue between QMK and smart_mod: a stand-in for what
 * hydeik_vial.c does in the firmware. It forwards the three hooks and tells
 * smart_mod which keycodes are one-shot mod keys.
 */

#pragma once

#include "quantum_keycodes.h"

/* The one-shot mod keys, numbered as in users/hydeik_vial/layout.h. */
enum test_keycode_t {
    COS_MOD = QK_KB_0,
    COS_LCTL,
    COS_LSFT,
    COS_LALT,
    COS_LGUI,
    COS_RCTL,
    COS_RSFT,
    COS_RALT,
    COS_RGUI,
};
