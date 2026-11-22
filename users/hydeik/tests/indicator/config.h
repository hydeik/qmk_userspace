/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#pragma once

#include "test_common.h"

/*
 * The tests are built with QMK's stock tap-hold behaviour: a holding key or a
 * smart mod layer key becomes a hold only once the tapping term has passed.
 */
#define TAPPING_TERM 200

/*
 * Eight layers, as a keymap with Vial has them. The logic looks at every
 * layer that the keymap can have (`MAX_LAYER`), and the keymap of a test
 * fixture has to define every key that is looked up.
 */
#define LAYER_STATE_8BIT
