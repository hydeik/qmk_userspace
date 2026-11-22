/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * firmware_default: the firmware defaults of the Vial settings of the hydeik
 * keymap family. It exists only with Vial: rules.mk adds it to the build of a
 * keymap with Vial only.
 *
 * Vial keeps its settings in the EEPROM and lets the user change them in the
 * Vial GUI. When vial-qmk resets them, it sets every tap-hold option off.
 * This module then switches Hold On Other Key Press back on, because layer
 * switching needs it: a holding key that is still down while a trigger key is
 * tapped has to count as held, even inside the tapping term.
 *
 * - After a settings reset that the firmware does while it boots, Hold On
 *   Other Key Press is on, and stored, so the Vial GUI shows it as on.
 * - It is a default and nothing more: the user can switch it off in the Vial
 *   GUI, and that choice is kept until the next settings reset.
 * - No other setting is touched.
 *
 * The firmware resets the settings while it boots in two cases: on the first
 * boot of a newly built firmware (every build has a new build id, which makes
 * the stored data not valid) and after the EEPROM was cleared (`EE_CLR`).
 *
 * Not covered: a reset asked for from the host while the keyboard runs (the
 * Vial command that resets the settings and the VIA command that resets the
 * keymap). vial-qmk calls no hook there, so Hold On Other Key Press stays off
 * until the user switches it on again or until the next settings reset at
 * boot.
 *
 * vial_hooks.c, the owner of the QMK hooks that exist only with Vial,
 * forwards three of them here.
 */

#pragma once

/*
 * Call from `via_init_kb()`. VIA calls that hook right before it checks
 * whether the stored data is valid, and resets the settings if it is not.
 */
void firmware_default_via_init(void);

/* Call from `eeconfig_init_user()`, which runs after a reset of the EEPROM. */
void firmware_default_eeconfig_init(void);

/* Call from `keyboard_post_init_user()`. */
void firmware_default_keyboard_post_init(void);
