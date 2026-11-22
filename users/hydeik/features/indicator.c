/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * The logic of the indicators. indicator.h describes the behaviour.
 */

#include "indicator.h"

#include "keymap_common.h"
#include "quantum_keycodes.h"

#include "smart_mod.h"
#include "target_layer.h"

static layer_state_t layer_bit(uint8_t layer) {
    return (layer_state_t)1 << layer;
}

/*****************************************************************************
 * Step 1: the snapshot
 *****************************************************************************/

indicator_snapshot_t indicator_snapshot(void) {
    indicator_snapshot_t snapshot = {
        .pending_one_shot_mods = smart_mod_pending_mods(),
        .latched_one_shot_mods = smart_mod_latched_mods(),
    };

    for (uint8_t i = 0; i < target_layer_entry_count; i++) {
        const uint8_t layer = target_layer_entries[i].target_layer;

        switch (target_layer_state_of(layer)) {
            case TARGET_LAYER_STATE_ON:
                snapshot.target_layers_on |= layer_bit(layer);
                break;
            case TARGET_LAYER_STATE_PENDING:
                snapshot.target_layers_pending |= layer_bit(layer);
                break;
            case TARGET_LAYER_STATE_LOCKED:
                snapshot.target_layers_locked |= layer_bit(layer);
                break;
            case TARGET_LAYER_STATE_OFF:
                break;
        }
        if (target_layer_is_trigger_key_working(layer)) {
            snapshot.target_layers_with_working_trigger_key |= layer_bit(layer);
        }
    }
    return snapshot;
}

/*****************************************************************************
 * Step 2: the indicator state of a key position
 *****************************************************************************/

/*
 * Which of two indicator states wins on one position: the one with the
 * higher number here.
 */
static uint8_t priority_of(enum indicator_state_t state) {
    switch (state) {
        case INDICATOR_STATE_TARGET_LAYER_ON:
        case INDICATOR_STATE_TARGET_LAYER_PENDING:
            return 5;
        case INDICATOR_STATE_TARGET_LAYER_LOCKED:
            return 4;
        case INDICATOR_STATE_PENDING_ONE_SHOT_MOD:
            return 3;
        case INDICATOR_STATE_LATCHED_ONE_SHOT_MOD:
            return 2;
        case INDICATOR_STATE_TRIGGER_KEY_WORKING:
            return 1;
        case INDICATOR_STATE_NONE:
            break;
    }
    return 0;
}

static enum indicator_state_t winner_of(enum indicator_state_t a, enum indicator_state_t b) {
    return priority_of(b) > priority_of(a) ? b : a;
}

/* The indicator state that the state of the target layer `layer` gives. */
static enum indicator_state_t state_of_target_layer(const indicator_snapshot_t *snapshot, uint8_t layer) {
    const layer_state_t bit = layer_bit(layer);

    if ((snapshot->target_layers_on & bit) != 0) {
        return INDICATOR_STATE_TARGET_LAYER_ON;
    }
    if ((snapshot->target_layers_pending & bit) != 0) {
        return INDICATOR_STATE_TARGET_LAYER_PENDING;
    }
    if ((snapshot->target_layers_locked & bit) != 0) {
        return INDICATOR_STATE_TARGET_LAYER_LOCKED;
    }
    return INDICATOR_STATE_NONE;
}

/*
 * What `keycode` shows as the key that a press gives now: if it is the
 * trigger key of a target layer, the state of that target layer, or that the
 * trigger key works.
 */
static enum indicator_state_t state_of_pressed_key(const indicator_snapshot_t *snapshot, uint16_t keycode) {
    if (!IS_QK_MOMENTARY(keycode)) {
        return INDICATOR_STATE_NONE;
    }
    const uint8_t layer = QK_MOMENTARY_GET_LAYER(keycode);
    if (layer >= MAX_LAYER) {
        /* A stored keymap can name a layer that does not exist. */
        return INDICATOR_STATE_NONE;
    }
    const enum indicator_state_t state = state_of_target_layer(snapshot, layer);

    if (state == INDICATOR_STATE_NONE && (snapshot->target_layers_with_working_trigger_key & layer_bit(layer)) != 0) {
        return INDICATOR_STATE_TRIGGER_KEY_WORKING;
    }
    return state;
}

/*
 * What `keycode` shows wherever it lies in the keymap, on a layer that is on
 * or off: as a holding key, the states of its target layers, and as a
 * one-shot mod key, that its one-shot mod is pending or latched.
 */
static enum indicator_state_t state_of_key_on_any_layer(const indicator_snapshot_t *snapshot, uint16_t keycode) {
    enum indicator_state_t state = INDICATOR_STATE_NONE;

    for (uint8_t i = 0; i < target_layer_entry_count; i++) {
        if (target_layer_entries[i].holding_key == keycode) {
            state = winner_of(state, state_of_target_layer(snapshot, target_layer_entries[i].target_layer));
        }
    }

    const uint8_t mods = smart_mod_one_shot_mod_bits(keycode);
    if ((mods & snapshot->pending_one_shot_mods) != 0) {
        state = winner_of(state, INDICATOR_STATE_PENDING_ONE_SHOT_MOD);
    }
    if ((mods & snapshot->latched_one_shot_mods) != 0) {
        state = winner_of(state, INDICATOR_STATE_LATCHED_ONE_SHOT_MOD);
    }
    return state;
}

enum indicator_state_t indicator_state_at(const indicator_snapshot_t *snapshot, keypos_t position) {
    const layer_state_t    layers_on    = layer_state | default_layer_state;
    enum indicator_state_t state        = INDICATOR_STATE_NONE;
    bool                   pressed_seen = false;

    /* From the highest layer down, as QMK looks a pressed key up. */
    for (int8_t layer = MAX_LAYER - 1; layer >= 0; layer--) {
        const uint16_t keycode = keymap_key_to_keycode(layer, position);

        if (keycode == KC_TRANSPARENT) {
            continue;
        }
        if (!pressed_seen && (layers_on & layer_bit(layer)) != 0) {
            pressed_seen = true;
            state        = winner_of(state, state_of_pressed_key(snapshot, keycode));
        }
        state = winner_of(state, state_of_key_on_any_layer(snapshot, keycode));
    }
    return state;
}

/*****************************************************************************
 * Step 3: whether an indicator is shown
 *****************************************************************************/

bool indicator_is_any_shown(const indicator_snapshot_t *snapshot) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (indicator_state_at(snapshot, (keypos_t){.col = col, .row = row}) != INDICATOR_STATE_NONE) {
                return true;
            }
        }
    }
    return false;
}

/*****************************************************************************
 * The view
 *****************************************************************************/

void indicator_view_build(indicator_view_t *view) {
    const indicator_snapshot_t snapshot = indicator_snapshot();

    view->any_shown = false;
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            const enum indicator_state_t state = indicator_state_at(&snapshot, (keypos_t){.col = col, .row = row});

            view->states[row][col] = state;
            if (state != INDICATOR_STATE_NONE) {
                view->any_shown = true;
            }
        }
    }
}

enum indicator_state_t indicator_view_state_at(const indicator_view_t *view, keypos_t position) {
    return (enum indicator_state_t)view->states[position.row][position.col];
}

/*
 * The packed view. Byte 0 holds the bit "an indicator is shown" in its
 * lowest bit. The bytes after it hold the indicator states, four bits each,
 * by row and then by column: an even position in the low half of its byte,
 * the odd position after it in the high half. Every other bit is zero.
 */
_Static_assert(INDICATOR_VIEW_PACKED_SIZE <= INDICATOR_VIEW_TRANSACTION_SIZE, "The packed view of the indicators does not fit into one split transaction");
_Static_assert(INDICATOR_STATE_TARGET_LAYER_LOCKED <= 0x0F, "An indicator state does not fit into four bits of the packed view");

void indicator_view_pack(const indicator_view_t *view, uint8_t *packed) {
    for (uint8_t i = 0; i < INDICATOR_VIEW_PACKED_SIZE; i++) {
        packed[i] = 0;
    }
    packed[0] = view->any_shown ? 1 : 0;

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            const uint8_t index = row * MATRIX_COLS + col;

            packed[1 + index / 2] |= (uint8_t)((view->states[row][col] & 0x0F) << (index % 2 * 4));
        }
    }
}

void indicator_view_unpack(const uint8_t *packed, indicator_view_t *view) {
    view->any_shown = (packed[0] & 1) != 0;

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            const uint8_t index = row * MATRIX_COLS + col;

            view->states[row][col] = (packed[1 + index / 2] >> (index % 2 * 4)) & 0x0F;
        }
    }
}

bool indicator_view_should_send(bool changed, bool last_send_failed, uint32_t elapsed_since_last_send) {
    return changed || last_send_failed || elapsed_since_last_send >= INDICATOR_VIEW_SEND_INTERVAL;
}

bool indicator_view_has_expired(bool has_arrived, uint32_t elapsed_since_arrival) {
    return !has_arrived || elapsed_since_arrival >= INDICATOR_VIEW_TIMEOUT;
}
