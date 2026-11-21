/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Test-only glue between QMK and target_layer. It forwards the three QMK
 * hooks to target_layer and defines a test-only table of entries, which is
 * that of no keymap family.
 *
 * The table, with `MO(TEST_LAYER_NAV)` and `MO(TEST_LAYER_FUN)` as the
 * trigger keys:
 *
 * | Holding key | Holding layer | Target layer | Quick release |
 * | ----------- | ------------- | ------------ | ------------- |
 * | `TAB_SYM`   | SYM           | NAV          | nothing       |
 * | `ENT_NUM`   | NUM           | NAV          | nothing       |
 * | `ENT_NUM`   | NUM           | FUN          | nothing       |
 * | `ENT_NUM`   | NUM           | PEN          | pending       |
 * | `TAB_SYM`   | SYM           | PEN          | pending       |
 * | `TAB_SYM`   | SYM           | LCK          | locked        |
 * | `ENT_NUM`   | NUM           | LCK          | nothing       |
 *
 * So NAV has two holding keys, `ENT_NUM` is the holding key of two target
 * layers, and `TAB_SYM` is no holding key of FUN. PEN and LCK are the target
 * layers of the quick release, with `MO(TEST_LAYER_PEN)` and
 * `MO(TEST_LAYER_LCK)` as their trigger keys; the two holding keys of LCK
 * differ in what a quick release does.
 *
 * The two answers of the caller: a one-shot mod key is one of QMK's own
 * `OSM()` keys, and the timeout of a pending target layer is suspended while
 * a test says so through `test_pending_timeout_suspended`.
 */

#pragma once

#include <stdbool.h>

#include "quantum_keycodes.h"

/* The layers. */
enum test_layer_t {
    TEST_LAYER_BASE,
    TEST_LAYER_SYM,
    TEST_LAYER_NUM,
    TEST_LAYER_NAV,
    TEST_LAYER_FUN,
    TEST_LAYER_PEN,
    TEST_LAYER_LCK,
    TEST_LAYER_COUNT,
};

/* The holding keys. */
#define TAB_SYM LT(TEST_LAYER_SYM, KC_TAB)
#define ENT_NUM LT(TEST_LAYER_NUM, KC_ENT)

/*
 * Whether the timeout of a pending target layer is suspended now. A test
 * sets it; the fixture clears it.
 */
extern bool test_pending_timeout_suspended;
