/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#pragma once

#include "test_common.h"

/*
 * A test build has no keyboard, so the firmware source does not define the
 * keyboard header that hydeik_vial.h and the common core include.
 */
#define QMK_KEYBOARD_H "quantum.h"

/*
 * Mirror the values of users/hydeik_vial/config.h that the behaviour under
 * test depends on. Keep them in sync by hand.
 *
 * The tests are built without Vial, so with QMK's stock tap-hold behaviour:
 * a holding key or a smart mod layer key becomes a hold only once the
 * tapping term has passed. In the firmware, Hold On Other Key Press makes it
 * a hold as soon as another key is pressed.
 */
#define TAPPING_TERM 200
