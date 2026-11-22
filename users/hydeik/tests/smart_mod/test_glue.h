/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Test-only glue between QMK and smart_mod. It forwards the three QMK hooks
 * to smart_mod and tells it which keycodes are one-shot mod keys, which keys
 * are smart mod layer keys, which of them has the tap rule of `SMT_MOD`, and
 * which layer is the smart mod layer.
 *
 * test_glue.c is the copy of the smart_mod part of
 * users/hydeik/hydeik.c that the unit tests build. The two must
 * configure smart_mod the same way. They differ only in this:
 *
 * - target_layer is not part of this test build. The
 *   tests of the two state machines together are in
 *   tests/target_layer_smart_mod/, which builds hydeik.c itself.
 * - The unit tests are built without layout.h, so this file has its own copy
 *   of the keycodes and layer numbers used there, under `TEST_LAYER_*` names
 *   for the layers. The assertions in test_glue.c and in hydeik.c pin
 *   the numbers that the copy repeats.
 * - The unit tests are built without Vial. In the firmware, Vial's tap-hold
 *   settings decide whether a smart mod layer key is tapped or held, and the
 *   tapping term of the double tap of `SMT_MOD` is the one set in Vial, not
 *   the `TAPPING_TERM` of config.h.
 */

#pragma once

#include "quantum_keycodes.h"

/*
 * The custom keycodes: the same entries in the same order as in
 * users/hydeik/layout.h. `COS_MOD` is the reserved slot with no
 * behaviour; the others are the one-shot mod keys.
 */
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

/* The layers, numbered as in users/hydeik/layout.h. */
enum test_layer_t {
    TEST_LAYER_BASE,
    TEST_LAYER_SYM,
    TEST_LAYER_NUM,
    TEST_LAYER_NAV,
    TEST_LAYER_MOU,
    TEST_LAYER_FUN,
    TEST_LAYER_MOD,
};

/* The smart mod layer keys, defined as in users/hydeik/layout.h. */
#define SMT_MOD LT(TEST_LAYER_MOD, KC_KP_0)
#define SPC_MOD LT(TEST_LAYER_MOD, KC_SPC)
#define ENT_MOD LT(TEST_LAYER_MOD, KC_ENT)

/* The tap dance keys of tap_dance_actions.c. */
enum test_tap_dance_t {
    TEST_TAP_DANCE_X_Y,
};

/* The combos of combos.c. */
enum test_combo_t {
    TEST_COMBO_J_K_Q,
};
