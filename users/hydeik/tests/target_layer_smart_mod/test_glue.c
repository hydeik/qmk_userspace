/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "test_glue.h"

#include "hydeik.h"

_Static_assert((int)TEST_LAYER_BASE == (int)_BASE, "TEST_LAYER_BASE must be the BASE layer of layout.h");
_Static_assert((int)TEST_LAYER_SYM == (int)_SYM, "TEST_LAYER_SYM must be the SYM layer of layout.h");
_Static_assert((int)TEST_LAYER_NUM == (int)_NUM, "TEST_LAYER_NUM must be the NUM layer of layout.h");
_Static_assert((int)TEST_LAYER_NAV == (int)_NAV, "TEST_LAYER_NAV must be the NAV layer of layout.h");
_Static_assert((int)TEST_LAYER_FUN == (int)_FUN, "TEST_LAYER_FUN must be the FUN layer of layout.h");
_Static_assert((int)TEST_LAYER_MOD == (int)_MOD, "TEST_LAYER_MOD must be the MOD layer of layout.h");

/* clang-format off */
const uint16_t test_layout[TEST_LAYER_COUNT][TEST_LAYOUT_KEYS] = {
    [_BASE] = {BASE_LAYER},
    [_SYM]  = {SYM_LAYER},
    [_NUM]  = {NUM_LAYER},
    [_NAV]  = {NAV_LAYER},
    [_FUN]  = {FUN_LAYER},
    [_MOD]  = {MOD_LAYER},
};
/* clang-format on */
