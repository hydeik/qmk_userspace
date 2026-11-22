/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#pragma once

#include "test_common.h"

/*
 * A test build has no keyboard, so the firmware source does not define the
 * keyboard header that typing_rules.c includes.
 */
#define QMK_KEYBOARD_H "quantum.h"

/*
 * The tests are built with QMK's stock tap-hold behaviour: a holding key
 * becomes a hold only once the tapping term has passed.
 */
#define TAPPING_TERM 200

/*
 * Mirror the value of users/hydeik/config.h that the behaviour
 * under test depends on: how long a one-shot mod of QMK's own `OSM()` keys
 * stays pending. Keep it in sync by hand.
 */
#define ONESHOT_TIMEOUT 2000
