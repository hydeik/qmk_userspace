/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * smart_mod: the state machine of the one-shot mods and the smart mod layer
 * of the hydeik_vial keymap family.
 *
 * These are not QMK's built-in one-shot mods, and this module does not use
 * them (see docs/adr/0002-hydeik-vial-one-shot-mods-are-an-own-state-machine.md).
 *
 * The module knows no keymap. Its caller forwards three QMK hooks to it and
 * tells it which keycodes are one-shot mod keys, which keys are smart mod
 * layer keys, and which layer is the smart mod layer.
 *
 * A one-shot mod key is an ordinary modifier while it is held. Tapped, that
 * is, released within the One Shot Term with no other key pressed in between,
 * it leaves a pending one-shot mod, which the host does not see. The next key
 * press consumes all pending one-shot mods: they are registered for that key
 * and dropped once its report has been sent. Other one-shot mod keys, ordinary
 * modifier keys and keys that only switch layers do not consume them. Pressing
 * the same one-shot mod key again cancels its pending one-shot mod, and a
 * timeout clears them all.
 *
 * A smart mod layer key is a layer-tap key. Held, it turns the smart mod layer
 * on, where one-shot mod keys lie over the home row. A one-shot mod key that
 * is pressed and released there with no key typed in between leaves a latched
 * one-shot mod: the modifier stays registered, for every key and without a
 * timeout. Once all one-shot mod keys are up the smart mod layer switches
 * itself off, so that the keys beneath can be typed with the latched one-shot
 * mods. Releasing the last held smart mod layer key releases them all. If no
 * key was typed and the release comes within the release window, counted from
 * the press of the last one-shot mod key, they become pending one-shot mods.
 *
 * Tapped, a smart mod layer key is the ordinary key of its layer-tap keycode,
 * unless it has the Shift tap rule: then a tap makes Shift a pending one-shot
 * mod, or cancels all pending one-shot mods if there are any, and a second
 * tap within the tapping term turns Caps Word on instead.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "action.h"

/****************************************************************************
 * Timing, in milliseconds. `config.h` can override each value.
 ****************************************************************************/

/*
 * One Shot Term: a one-shot mod key becomes a pending one-shot mod only if it
 * is released less than this long after its press.
 */
#ifndef SMART_MOD_ONE_SHOT_TERM
#    define SMART_MOD_ONE_SHOT_TERM 200
#endif

/*
 * Pending one-shot mods are all cleared once this long has passed since the
 * last one-shot mod tap. The value matches ONESHOT_TIMEOUT of the common core
 * on purpose.
 */
#ifndef SMART_MOD_ONE_SHOT_TIMEOUT
#    define SMART_MOD_ONE_SHOT_TIMEOUT 2000
#endif

/*
 * Release window of the smart mod layer: latched one-shot mods become pending
 * one-shot mods only if the last smart mod layer key is released less than
 * this long after the press of the last one-shot mod key, and no key was
 * typed. It is separate from the One Shot Term.
 */
#ifndef SMART_MOD_LAYER_RELEASE_WINDOW
#    define SMART_MOD_LAYER_RELEASE_WINDOW 400
#endif

/****************************************************************************
 * Told by the caller
 ****************************************************************************/

/*
 * Return the modifier bits (MOD_BIT) of a one-shot mod key, or 0 if `keycode`
 * is not a one-shot mod key. The caller must define this function.
 */
uint8_t smart_mod_one_shot_mod_bits(uint16_t keycode);

/*
 * Return the number of the smart mod layer. The caller must define this
 * function. Only this module may switch that layer on and off.
 */
uint8_t smart_mod_layer(void);

/*
 * Return whether `keycode` is a smart mod layer key. It must be a layer-tap
 * keycode (`LT`), so that QMK tells a tap from a hold; its layer is ignored,
 * because a held smart mod layer key is handled here, not by QMK. The caller
 * must define this function.
 */
bool smart_mod_is_layer_key(uint16_t keycode);

/*
 * Return whether the smart mod layer key `keycode` has the Shift tap rule.
 * The tap keycode of such a key is never sent. A tap of every other smart mod
 * layer key is an ordinary key press. The caller must define this function.
 */
bool smart_mod_layer_key_taps_shift(uint16_t keycode);

/****************************************************************************
 * Hooks
 ****************************************************************************/

/*
 * Key-event hook. Call from `process_record_user`. Returns false if the key
 * event was handled here and must not be processed further.
 */
bool smart_mod_process_record(uint16_t keycode, keyrecord_t *record);

/*
 * After-report hook. Call from `post_process_record_user`, that is, once the
 * reports of the key event have been sent to the host.
 */
void smart_mod_post_process_record(uint16_t keycode, keyrecord_t *record);

/*
 * Elapsed-time hook. Call from `housekeeping_task_user`, that is, in every
 * scan loop. It also stands in for the after-report hook, which QMK does not
 * call for a key that one of its features handled completely.
 */
void smart_mod_housekeeping_task(void);

/****************************************************************************
 * State
 ****************************************************************************/

/*
 * Forget everything: the held keys and the pending and latched one-shot mods.
 * It sends no report and leaves the registered modifiers and the layers
 * alone.
 *
 * The state starts out empty, so the caller does not have to call this. The
 * unit tests do, so that no state leaks from one test into the next.
 */
void smart_mod_reset(void);
