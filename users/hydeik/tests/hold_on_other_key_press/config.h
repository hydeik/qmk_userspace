/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#pragma once

#include "test_common.h"

/*
 * A test build has no keyboard, so the firmware source does not define the
 * keyboard header that hydeik.h and typing_rules.c include.
 */
#define QMK_KEYBOARD_H "quantum.h"

/*
 * Mirror the values of users/hydeik/config.h that the behaviour under
 * test depends on. Keep them in sync by hand.
 *
 * HOLD_ON_OTHER_KEY_PRESS is the setting under test: a holding key or a
 * smart mod layer key becomes a hold as soon as another key is pressed. The
 * test is built without Vial, so the global define takes effect, as in a
 * keymap without Vial.
 */
#define TAPPING_TERM 200
#define HOLD_ON_OTHER_KEY_PRESS
