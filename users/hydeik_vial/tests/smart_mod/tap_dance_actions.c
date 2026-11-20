/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * The tap dance keys of the test keymap. In the firmware build of the
 * hydeik_vial keymap family they come from Vial instead. This file is
 * included into the keymap of the test build (INTROSPECTION_KEYMAP_C).
 */

#include "quantum.h"
#include "test_glue.h"

tap_dance_action_t tap_dance_actions[] = {
    [TEST_TAP_DANCE_X_Y] = ACTION_TAP_DANCE_DOUBLE(KC_X, KC_Y),
};
