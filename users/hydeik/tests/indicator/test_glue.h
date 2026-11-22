/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Test-only glue between QMK and the two state machines whose state the
 * indicators show, smart_mod and target_layer. It forwards the three QMK
 * hooks to both, in the order of users/hydeik/hydeik.c, and answers their
 * questions as hydeik.c does, with its own copy of the keycodes: the unit
 * tests are built without layout.h.
 *
 * The table of entries is test-only, and not that of the keymap family:
 *
 * | Holding key | Holding layer | Target layer | Quick release |
 * | ----------- | ------------- | ------------ | ------------- |
 * | `TAB_SYM`   | SYM           | NAV          | locked        |
 * | `TAB_SYM`   | SYM           | MOU          | locked        |
 * | `ENT_NUM`   | NUM           | FUN          | pending       |
 * | `ENT_NUM`   | NUM           | NAV          | nothing       |
 *
 * So `TAB_SYM` is the holding key of two target layers that it can lock, and
 * `ENT_NUM` is a holding key of FUN, which it can leave pending, and of NAV,
 * which `TAB_SYM` can lock.
 */

#pragma once

#include "quantum_keycodes.h"

/* The one-shot mod keys. */
enum test_keycode_t {
    COS_LCTL = QK_KB_0,
    COS_LSFT,
    COS_LALT,
    COS_LGUI,
    COS_RCTL,
    COS_RSFT,
    COS_RALT,
    COS_RGUI,
};

/*
 * The layers. MOD is the smart mod layer. No key switches OFF on: it stands
 * for a layer of the keymap in use that is off.
 */
enum test_layer_t {
    TEST_LAYER_BASE,
    TEST_LAYER_SYM,
    TEST_LAYER_NUM,
    TEST_LAYER_NAV,
    TEST_LAYER_MOU,
    TEST_LAYER_FUN,
    TEST_LAYER_MOD,
    TEST_LAYER_OFF,
    TEST_LAYER_COUNT,
};

/* The holding keys. */
#define TAB_SYM LT(TEST_LAYER_SYM, KC_TAB)
#define ENT_NUM LT(TEST_LAYER_NUM, KC_ENT)

/* The smart mod layer keys. A tap of `SMT_MOD` makes Shift pending. */
#define SMT_MOD LT(TEST_LAYER_MOD, KC_KP_0)
#define SPC_MOD LT(TEST_LAYER_MOD, KC_SPC)
