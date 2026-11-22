/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * The hydeik keymap family on the crkbd rev4.1 mini, without Vial.
 */

#include "hydeik.h"

#include "keyboards/crkbd_rev4_1_mini.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_HYDEIK_34(BASE_LAYER),
    [_SYM]  = LAYOUT_HYDEIK_34(SYM_LAYER),
    [_NUM]  = LAYOUT_HYDEIK_34(NUM_LAYER),
    [_NAV]  = LAYOUT_HYDEIK_34(NAV_LAYER),
    [_FUN]  = LAYOUT_HYDEIK_34(FUN_LAYER),
    [_MOD]  = LAYOUT_HYDEIK_34(MOD_LAYER),
};
