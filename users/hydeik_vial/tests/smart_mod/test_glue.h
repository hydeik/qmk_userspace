/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Test-only glue between QMK and smart_mod. It forwards the three QMK hooks
 * to smart_mod and tells it which keycodes are one-shot mod keys, which keys
 * are smart mod layer keys, which of them has the tap rule of `SMT_MOD`, and
 * which layer is the smart mod layer. That is what hydeik_vial.c will do once
 * smart_mod is part of the firmware build.
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

/* The layers, numbered as in users/hydeik_vial/layout.h. */
enum test_layer_t {
    TEST_LAYER_BASE,
    TEST_LAYER_SYM,
    TEST_LAYER_NUM,
    TEST_LAYER_NAV,
    TEST_LAYER_FUN,
    TEST_LAYER_MOD,
};

/* The smart mod layer keys, defined as in users/hydeik_vial/layout.h. */
#define SMT_MOD LT(TEST_LAYER_MOD, KC_KP_0)
#define SPC_MOD LT(TEST_LAYER_MOD, KC_SPC)
#define ENT_MOD LT(TEST_LAYER_MOD, KC_ENT)

/* The tap dance keys of tap_dance_actions.c. */
enum test_tap_dance_t {
    TEST_TAP_DANCE_X_Y,
};
