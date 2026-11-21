/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#pragma once

#include "test_common.h"

/*
 * A test build has no keyboard, so the firmware source does not define the
 * keyboard header that the common core includes.
 */
#define QMK_KEYBOARD_H "quantum.h"

/*
 * The tests are built with QMK's stock tap-hold behaviour: a holding key
 * becomes a hold only once the tapping term has passed.
 */
#define TAPPING_TERM 200
