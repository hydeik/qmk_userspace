/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * The tap dance keys of the test keymap. In the firmware build of a keymap
 * with Vial they come from Vial instead. This file is included into the
 * keymap of the test build (see test_keymap_tables.c).
 */

#include "quantum.h"
#include "test_glue.h"

tap_dance_action_t tap_dance_actions[] = {
    [TEST_TAP_DANCE_X_Y] = ACTION_TAP_DANCE_DOUBLE(KC_X, KC_Y),
};
