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
 * switches its target layer on. The target layer stays on until the last
 * holding key of that activation is released; the trigger key may be released
 * at any time. Pressing the trigger key again switches the target layer off,
 * and the trigger key of another target layer replaces it: at most one target
 * layer is on at a time.
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
 * The module switches off only a target layer that it switched on itself. A
 * trigger key does nothing while its target layer is on for another reason,
 * Layer Lock included.
 *
 * The module knows no keymap, no keymap family and no firmware source, and
 * defines no QMK hook. Its caller defines the table of entries and forwards
 * three QMK hooks to it. common.mk adds it to the build of each keymap
 * family.
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
 * entry. Not acted on yet: releasing the last holding key always switches
 * the target layer off.
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
 * `quick_release` is a `target_layer_quick_release_t`.
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

/****************************************************************************
 * Hooks
 ****************************************************************************/

/*
 * Key-event hook. Call from `process_record_user`. Returns false if the key
 * event was handled here and must not be processed further: every press and
 * release of a trigger key.
 */
bool target_layer_process_record(uint16_t keycode, keyrecord_t *record);

/*
 * After-report hook. Call from `post_process_record_user`, that is, once the
 * reports of the key event have been sent to the host. It does nothing yet.
 */
void target_layer_post_process_record(uint16_t keycode, keyrecord_t *record);

/*
 * Elapsed-time hook. Call from `housekeeping_task_user`, that is, in every
 * scan loop. It does nothing yet.
 */
void target_layer_housekeeping_task(void);

/****************************************************************************
 * State
 ****************************************************************************/

/*
 * Forget everything: the held holding keys and the target layer that is on.
 * It leaves the layers alone.
 *
 * The state starts out empty, so the caller does not have to call this. The
 * unit tests do, so that no state leaks from one test into the next.
 */
void target_layer_reset(void);
