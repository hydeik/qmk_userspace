/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/* Probe: the smart one-shot mod keymap family without Vial, on upstream QMK. */

#include "hydeik_vial.h"
#include "users/hydeik_common/keyboards/mothwing.h"

/* clang-format off */

/* A seventh layer with mouse keys, as a stand-in for the MOU layer. */
#define PROBE_MOUSE_LAYER \
    XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,   MS_WHLL,  MS_WHLD,  MS_WHLU,  MS_WHLR,  XXXXXXX,  \
    COS_LCTL, COS_LALT, COS_LGUI, COS_LSFT, XXXXXXX,   MS_LEFT,  MS_DOWN,  MS_UP,    MS_RGHT,  MS_BTN3,  \
    XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  QK_LLCK,   XXXXXXX,  MS_ACL0,  MS_ACL1,  MS_ACL2,  XXXXXXX,  \
                                  _______,  _______,   MS_BTN1,  MS_BTN2

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_HYDEIK_34(BASE_LAYER),
    [_SYM] = LAYOUT_HYDEIK_34(SYM_LAYER),
    [_NUM] = LAYOUT_HYDEIK_34(NUM_LAYER),
    [_NAV] = LAYOUT_HYDEIK_34(NAV_LAYER),
    [_FUN] = LAYOUT_HYDEIK_34(FUN_LAYER),
    [_MOD] = LAYOUT_HYDEIK_34(MOD_LAYER),
    [_MOD + 1] = LAYOUT_HYDEIK_34(PROBE_MOUSE_LAYER),
};

/* clang-format on */
