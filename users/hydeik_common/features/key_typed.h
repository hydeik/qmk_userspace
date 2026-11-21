/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * key_typed: what counts as "a key typed", defined once for the common core
 * and every keymap family.
 *
 * A typed key is what consumes a pending one-shot mod and what a pending
 * target layer applies to. Keys that only hold a modifier or switch layers
 * are not typed keys, so they can be pressed in between.
 *
 * common.mk adds this module to the build of each keymap family. It knows no
 * keymap family and no firmware source.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "action.h"

/*
 * Return whether the key press in `record`, of the key `keycode`, is a key
 * typed. Every key press is one, except that of
 *
 * - an ordinary modifier key,
 * - Layer Lock,
 * - a mod-tap or layer-tap key that is held, not tapped,
 * - a key that only switches layers (`MO`, `TO`, `TG`, `TT`, `OSL`, `LM`,
 *   `DF`, `PDF`).
 *
 * Which keys are one-shot mod keys differs per keymap family, and they are
 * not typed keys either: the caller must leave them out itself. The caller
 * also passes key presses only; the answer for a release has no meaning.
 */
bool is_key_typed(uint16_t keycode, const keyrecord_t *record);
