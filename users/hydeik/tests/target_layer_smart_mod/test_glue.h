/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Test-only glue of the tests of target_layer together with smart_mod.
 *
 * These tests build the real users/hydeik/hydeik.c, so the QMK
 * hooks, the table of entries and the answers to both state machines are
 * those of the firmware. This glue adds only what the test build lacks:
 *
 * - `test_layout`, the layers of users/hydeik/layout.h as a table, from
 *   which the fixture builds its keymap. The keymap.c of a keyboard does the
 *   same through its layout adapter.
 * - The layer numbers of layout.h under `TEST_LAYER_*` names. The test
 *   sources are C++ and cannot include layout.h, whose `enum layer_t`
 *   collides with a type of the unit-test framework there. test_glue.c
 *   asserts that the numbers are the same.
 */

#pragma once

#include <stdint.h>

/* The layers, numbered as in users/hydeik/layout.h. */
enum test_layer_t {
    TEST_LAYER_BASE,
    TEST_LAYER_SYM,
    TEST_LAYER_NUM,
    TEST_LAYER_NAV,
    TEST_LAYER_MOU,
    TEST_LAYER_FUN,
    TEST_LAYER_MOD,
    TEST_LAYER_COUNT,
};

/* The number of keys of one layer of layout.h. */
#define TEST_LAYOUT_KEYS 34

/*
 * The layers of layout.h, by layer number. A layer is three rows of ten
 * finger keys, row by row, and then the four thumb keys from left to right.
 */
extern const uint16_t test_layout[TEST_LAYER_COUNT][TEST_LAYOUT_KEYS];
