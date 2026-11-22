/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * The QMK hooks that exist only with Vial. They forward to firmware_default,
 * the firmware defaults of the Vial settings (features/firmware_default.h
 * describes the behaviour).
 *
 * rules.mk adds this file to the build of a keymap with Vial only, so a
 * keymap without Vial has none of these hooks, and neither have the unit
 * tests: those of firmware_default call it directly.
 *
 * The one behaviour of its own that this file holds is in
 * `eeconfig_init_user`, which also does what the default of QMK does there:
 * it clears the user settings in the EEPROM.
 */

#include QMK_KEYBOARD_H

#include "features/firmware_default.h"

/*
 * `via_init_kb()` is a keyboard-level hook that no keyboard of this keymap
 * family defines, and vial-qmk has no user-level hook at that point.
 */
void via_init_kb(void) {
    firmware_default_via_init();
}

void eeconfig_init_user(void) {
    /* What the default `eeconfig_init_user()` of QMK does. */
    eeconfig_update_user(0);
    firmware_default_eeconfig_init();
}

void keyboard_post_init_user(void) {
    firmware_default_keyboard_post_init();
}
