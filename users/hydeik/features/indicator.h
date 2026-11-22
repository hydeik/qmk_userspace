/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * indicator: the logic of the indicators. For a key position it answers the
 * one indicator state that wins there, and it answers whether an indicator
 * is shown at all.
 *
 * It knows nothing of RGB Matrix or of colours, and it reads no clock: how
 * an indicator state looks, and on which LED, is the matter of its caller
 * (see docs/adr/0005-the-master-half-decides-every-indicator-state.md).
 *
 * For a split keyboard it also builds the view, which the master half sends
 * to the other half, packs it into the bytes of a split transaction, and
 * decides when to send it and when it has expired; see "The view" below.
 * Sending and receiving themselves are the matter of its caller too.
 *
 * The logic has three steps:
 *
 * 1. `indicator_snapshot()` gathers the state of the one-shot mods and of the
 *    layer switching from smart_mod and target_layer.
 * 2. `indicator_state_at()` decides the indicator state of a key position
 *    from a snapshot and the keymap in use. It does not ask the two state
 *    machines itself.
 * 3. `indicator_is_any_shown()` answers whether any key position has an
 *    indicator state.
 *
 * The keymap in use is read with `keymap_key_to_keycode()`, on every layer
 * that the keymap can have (`MAX_LAYER`): the stored keymap in a keymap with
 * Vial, the firmware keymap in a keymap without Vial. There is no table of
 * positions, so an indicator follows its key when the keymap changes, and a
 * state whose key lies nowhere in the keymap is shown nowhere.
 *
 * Where each state is shown:
 *
 * - A pending or a latched one-shot mod: at every position, on every layer,
 *   on or off, that holds the one-shot mod key of that modifier. Left and
 *   right are different keys.
 * - A trigger key that works: at every position where a press now gives the
 *   trigger key `MO(target layer)` of a target layer whose trigger key works.
 *   What a press gives now is the keycode on the highest layer that is on
 *   and has no transparent key there, so a trigger key that lies only on a
 *   layer that is off, or under another key, is not shown.
 * - A target layer that is on, pending or locked: at every position where a
 *   press now gives the trigger key of that target layer, and at every
 *   position, on every layer, on or off, that holds the keycode of a holding
 *   key of that target layer.
 *
 * Where several meet on one position, the first of these wins:
 *
 * 1. a target layer that is on or pending,
 * 2. a locked target layer,
 * 3. a pending one-shot mod,
 * 4. a latched one-shot mod,
 * 5. a trigger key that works.
 *
 * So the trigger key of the target layer that is on shows "on", and a
 * holding key shared by a locked target layer and the one that is on shows
 * "on" too.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "action_layer.h"
#include "keyboard.h"

/*
 * What the indicator at one key position shows. The numbers are part of the
 * interface: they do not say which state wins.
 */
enum indicator_state_t {
    INDICATOR_STATE_NONE,
    INDICATOR_STATE_PENDING_ONE_SHOT_MOD,
    INDICATOR_STATE_LATCHED_ONE_SHOT_MOD,
    /* A trigger key that works now. */
    INDICATOR_STATE_TRIGGER_KEY_WORKING,
    INDICATOR_STATE_TARGET_LAYER_ON,
    INDICATOR_STATE_TARGET_LAYER_PENDING,
    INDICATOR_STATE_TARGET_LAYER_LOCKED,
};

/*
 * The state that the indicators show, at one moment. The one-shot mods are
 * sets of modifier bits (`MOD_BIT`) that tell left from right, and the
 * target layers are sets of layers, one bit per layer number.
 */
typedef struct {
    uint8_t       pending_one_shot_mods;
    uint8_t       latched_one_shot_mods;
    layer_state_t target_layers_on;
    layer_state_t target_layers_pending;
    layer_state_t target_layers_locked;
    /* The target layers whose trigger key works now. */
    layer_state_t target_layers_with_working_trigger_key;
} indicator_snapshot_t;

/* Gather the state of smart_mod and target_layer as it is now. */
indicator_snapshot_t indicator_snapshot(void);

/*
 * Return the indicator state of the key position `position`, for the state
 * in `snapshot`, the keymap in use and the layers that are on now.
 */
enum indicator_state_t indicator_state_at(const indicator_snapshot_t *snapshot, keypos_t position);

/*
 * Return whether an indicator is shown: whether at least one key position of
 * the matrix has an indicator state for the state in `snapshot`. A state
 * whose key lies nowhere in the keymap in use does not count.
 */
bool indicator_is_any_shown(const indicator_snapshot_t *snapshot);

/*****************************************************************************
 * The view
 *
 * The view is what the master half of a split keyboard decides and both
 * halves draw: the indicator state of every key position of the matrix, of
 * both halves, and the bit "an indicator is shown". The phase of the blink is
 * not part of it.
 *****************************************************************************/

/*
 * The view. A view that is all zero is the empty view: no indicator state,
 * and the bit off. It is what a half draws from before the first view has
 * arrived and after the view has expired.
 */
typedef struct {
    /* An `enum indicator_state_t` per key position, by row and column. */
    uint8_t states[MATRIX_ROWS][MATRIX_COLS];
    /* Whether an indicator is shown: whether any position has a state. */
    bool any_shown;
} indicator_view_t;

/*
 * Fill `view` for the state of smart_mod and target_layer as it is now, the
 * keymap in use and the layers that are on now. It decides every key
 * position once, and takes the bit from what it has decided.
 */
void indicator_view_build(indicator_view_t *view);

/* Return the indicator state that `view` holds for the key position `position`. */
enum indicator_state_t indicator_view_state_at(const indicator_view_t *view, keypos_t position);

/*
 * The number of bytes of a packed view: one byte for the bit, and half a
 * byte per key position of the matrix.
 */
#define INDICATOR_VIEW_PACKED_SIZE (1 + (MATRIX_ROWS * MATRIX_COLS + 1) / 2)

/*
 * The number of bytes that one split transaction carries from the master
 * half to the other half: QMK's `RPC_M2S_BUFFER_SIZE`, which is 32 unless a
 * `config.h` sets it. A build in which a packed view does not fit into it
 * fails at compile time, in indicator.c.
 */
#ifndef INDICATOR_VIEW_TRANSACTION_SIZE
#    ifdef RPC_M2S_BUFFER_SIZE
#        define INDICATOR_VIEW_TRANSACTION_SIZE RPC_M2S_BUFFER_SIZE
#    else
#        define INDICATOR_VIEW_TRANSACTION_SIZE 32
#    endif
#endif

/*
 * Pack `view` into the `INDICATOR_VIEW_PACKED_SIZE` bytes at `packed`, to
 * send them to the other half. Every bit of the bytes is set from the view
 * alone, so two views are the same exactly when their packed bytes are, and
 * the empty view packs to bytes that are all zero.
 */
void indicator_view_pack(const indicator_view_t *view, uint8_t *packed);

/*
 * Fill `view` from the `INDICATOR_VIEW_PACKED_SIZE` bytes at `packed`, as
 * `indicator_view_pack()` wrote them. Bytes that are all zero give the empty
 * view.
 */
void indicator_view_unpack(const uint8_t *packed, indicator_view_t *view);

/*
 * The time in milliseconds after which the master half sends the view again
 * although it did not change, so that the other half catches up after it
 * restarted or after a transaction was lost. A `config.h` can set it.
 */
#ifndef INDICATOR_VIEW_SEND_INTERVAL
#    define INDICATOR_VIEW_SEND_INTERVAL 500
#endif

/*
 * The time in milliseconds after which a view that was not renewed has
 * expired. A `config.h` can set it.
 */
#ifndef INDICATOR_VIEW_TIMEOUT
#    define INDICATOR_VIEW_TIMEOUT 2000
#endif

/*
 * Return whether the master half sends the view now: when the view has
 * `changed` since the last send that succeeded, when the last send failed
 * (`last_send_failed`), and when `INDICATOR_VIEW_SEND_INTERVAL` has passed
 * since the last send. `elapsed_since_last_send` is that time in
 * milliseconds.
 */
bool indicator_view_should_send(bool changed, bool last_send_failed, uint32_t elapsed_since_last_send);

/*
 * Return whether the view that a half received has expired: before the first
 * view has arrived (`has_arrived` is false), and when `INDICATOR_VIEW_TIMEOUT`
 * has passed since the last view arrived. `elapsed_since_arrival` is that
 * time in milliseconds. A half draws the empty view in place of one that has
 * expired.
 */
bool indicator_view_has_expired(bool has_arrived, uint32_t elapsed_since_arrival);
