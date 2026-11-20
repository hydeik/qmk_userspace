/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Test-only glue between QMK and smart_mod. It forwards the three QMK hooks
 * to smart_mod and tells it which keycodes are one-shot mod keys, which is
 * what hydeik_vial.c will do once smart_mod is part of the firmware build.
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
