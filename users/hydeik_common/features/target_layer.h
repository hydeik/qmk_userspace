/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * target_layer: the layer-switching state machine of the common core, used
 * unchanged by every keymap family (see
 * docs/adr/0003-layer-switching-is-one-state-machine-in-the-common-core.md).
 *
 * A holding key is a thumb key that keeps its holding layer on while it is
 * held. A trigger key, pressed while one of its holding keys is held,
 * switches its target layer on. The target layer stays on until the last of
 * its holding keys that were held at that press is released; the trigger key
 * may be released at any time. Pressing the trigger key again switches the
 * target layer off, and the trigger key of another target layer replaces it:
 * at most one target layer is on at a time, besides the locked ones.
 *
 * A quick release of the last holding key, less than the One Shot Term after
 * the press of the trigger key and with no key typed in between, does what
 * the entry of that holding key says:
 *
 * - nothing: the target layer is switched off, as by any other release;
 * - a pending target layer: it stays on for the next typed key only. Its
 *   trigger key switches it off, holding any holding key ends it, and it
 *   times out;
 * - a locked target layer: it is locked as by the Layer Lock key, until its
 *   trigger key or Layer Lock unlocks it.
 *
 * Layer Lock pressed on a target layer that is on through this module, held
 * or pending, makes it a locked target layer too. A key typed is what
 * `is_key_typed()` of the common core says, a tapped holding key and a mouse
 * key included, except the one-shot mod keys that the caller tells.
 *
 * A trigger key is the ordinary keycode `MO(target layer)`. This module
 * intercepts `MO()` of every layer that the table lists as a target layer, so
 * such a key is no momentary layer key any more: without a held holding key
 * it does nothing.
 *
 * A holding key counts as held only when QMK reports its press as a hold, not
 * as a tap. This module does not switch the holding layer: QMK does, as for
 * any layer-tap key.
 *
 * The module switches off only a target layer that it switched on itself,
 * with one exception: a trigger key unlocks and switches off its target layer
 * whenever Layer Lock holds it, whichever way the layer came on. A trigger
 * key does nothing while its target layer is on for any other reason than
 * this module or Layer Lock. A target layer should lie above its holding
 * layers, as the
 * Layer Lock key locks the highest layer that is on.
 *
 * Without Layer Lock in the build (`LAYER_LOCK_ENABLE`), a locked target
 * layer is kept on by this module alone, and only its trigger key leaves it.
 *
 * The module knows no keymap, no keymap family and no firmware source, and
 * defines no QMK hook. Its caller defines the table of entries and two
 * functions that answer its questions, and forwards three QMK hooks to it.
 * common.mk adds it to the build of each keymap family.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "action.h"

/****************************************************************************
 * Told by the caller
 ****************************************************************************/

/*
 * What a quick release of the holding key does to the target layer of an
 * entry.
 */
enum target_layer_quick_release_t {
    /* The target layer is switched off, as by any other release. */
    TARGET_LAYER_QUICK_RELEASE_NOTHING,
    /* The target layer becomes a pending target layer. */
    TARGET_LAYER_QUICK_RELEASE_PENDING,
    /* The target layer becomes a locked target layer. */
    TARGET_LAYER_QUICK_RELEASE_LOCKED,
};

/*
 * One entry of the table: while `holding_key` is held, the trigger key
 * `MO(target_layer)` works. A target layer with several holding keys has one
 * entry per holding key, and a holding key of several target layers has one
 * entry per target layer.
 *
 * `holding_key` is the keycode of the key, usually a layer-tap keycode
 * (`LT`). It must not be the trigger key of a target layer itself.
 * `quick_release` is a `target_layer_quick_release_t`. With several holding
 * keys held, the entry of the one released last decides.
 */
typedef struct {
    uint16_t holding_key;
    uint8_t  target_layer;
    uint8_t  quick_release;
} target_layer_entry_t;

/*
 * The table of entries and the number of its entries. The caller must define
 * both.
 */
extern const target_layer_entry_t target_layer_entries[];
extern const uint8_t              target_layer_entry_count;

/*
 * Return whether the key event in `record`, of the key `keycode`, is the
 * press or the release of a one-shot mod key. The caller must define it:
 * which keys are one-shot mod keys differs per keymap family.
 *
 * A one-shot mod key is not a typed key, and its press and its release each
 * restart the timeout of a pending target layer. It is asked for the presses
 * and the releases of every key but the trigger keys and the held holding
 * keys.
 */
bool target_layer_is_one_shot_mod_key(uint16_t keycode, const keyrecord_t *record);

/*
 * Return whether the timeout of a pending target layer is suspended now. The
 * caller must define it. It is asked in every scan loop while a target layer
 * is pending; the timeout starts anew when the suspension ends.
 */
bool target_layer_is_pending_timeout_suspended(void);

/****************************************************************************
 * Timing, overridable in config.h
 ****************************************************************************/

/*
 * The One Shot Term, in ms: a release of the last holding key is a quick
 * release if it comes less than this long after the press of the trigger
 * key, with no key typed in between.
 */
#ifndef TARGET_LAYER_ONE_SHOT_TERM
#    define TARGET_LAYER_ONE_SHOT_TERM 200
#endif

/*
 * How long a pending target layer lasts, in ms, counted from the release of
 * the holding key or from the last press or release of a one-shot mod key.
 * It must not be shorter than the timeout of the caller's pending one-shot
 * mods.
 */
#ifndef TARGET_LAYER_PENDING_TIMEOUT
#    define TARGET_LAYER_PENDING_TIMEOUT 2000
#endif

/****************************************************************************
 * Hooks
 ****************************************************************************/

/*
 * Key-event hook. Call from `process_record_user`, for every key event: also
 * for those that another feature of the caller handles, such as its one-shot
 * mod keys. Returns false if the key event was handled here and must be kept
 * from QMK: every press and release of a trigger key.
 */
bool target_layer_process_record(uint16_t keycode, keyrecord_t *record);

/*
 * After-report hook. Call from `post_process_record_user`, that is, once the
 * reports of the key event have been sent to the host. It switches off the
 * pending target layer that the key used.
 */
void target_layer_post_process_record(uint16_t keycode, keyrecord_t *record);

/*
 * Elapsed-time hook. Call from `housekeeping_task_user`, that is, in every
 * scan loop. It ends the One Shot Term and the timeout of a pending target
 * layer. A caller with pending one-shot mods of its own calls their
 * elapsed-time hook first, so that a pending target layer does not end
 * before them when both timeouts end in the same scan loop.
 */
void target_layer_housekeeping_task(void);

/****************************************************************************
 * State
 ****************************************************************************/

/*
 * Forget everything: the held holding keys, the target layer that is on and
 * the locked target layers. It leaves the layers and Layer Lock alone.
 *
 * The state starts out empty, so the caller does not have to call this. The
 * unit tests do, so that no state leaks from one test into the next.
 */
void target_layer_reset(void);
