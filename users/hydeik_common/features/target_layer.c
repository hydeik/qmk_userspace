/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * This file must not branch on the keymap family or the firmware source, and
 * must use nothing that upstream QMK added after the last merge of vial-qmk.
 */

#include "target_layer.h"

#include "action_layer.h"
#include "quantum_keycodes.h"

/*
 * How many holding keys can be held at once. One more is ignored: it makes
 * no trigger key work.
 */
#define MAX_HOLDING_KEYS_HELD 4

/* What the state machine is doing with `state.target_layer`. */
enum phase_t {
    /* No target layer is on through this module. */
    PHASE_OFF,
    /* This module switched the target layer on, and a holding key keeps it on. */
    PHASE_HELD,
};

/*
 * A holding key that is held, not tapped. It is remembered by its keycode
 * and its position together: results of combos and tap dances all arrive at
 * one fixed position, and one keycode may lie at several positions.
 */
typedef struct {
    uint16_t keycode;
    keypos_t position;
    /*
     * Whether this key is a holding key of the activation: it was held when
     * the trigger key switched the target layer on, and keeps it on.
     */
    bool keeps_target_layer_on;
} held_holding_key_t;

/* The state machine. */
static struct {
    /* The holding keys that are held: their number and the keys. */
    uint8_t            holding_keys_held;
    held_holding_key_t held_holding_keys[MAX_HOLDING_KEYS_HELD];
    /*
     * The target layer that this module switched on and not off again, if
     * `phase` is not PHASE_OFF. This is the module's own record, not the
     * layer state of QMK.
     */
    uint8_t phase;
    uint8_t target_layer;
} state;

void target_layer_reset(void) {
    state.holding_keys_held = 0;
    state.phase             = PHASE_OFF;
}

/* Return whether the table lists `layer` as a target layer. */
static bool is_target_layer(uint8_t layer) {
    for (uint8_t i = 0; i < target_layer_entry_count; i++) {
        if (target_layer_entries[i].target_layer == layer) {
            return true;
        }
    }
    return false;
}

/* Return whether `keycode` is the holding key of any entry. */
static bool is_holding_key(uint16_t keycode) {
    for (uint8_t i = 0; i < target_layer_entry_count; i++) {
        if (target_layer_entries[i].holding_key == keycode) {
            return true;
        }
    }
    return false;
}

/* Return whether `keycode` is a holding key of the target layer `layer`. */
static bool is_holding_key_of(uint16_t keycode, uint8_t layer) {
    for (uint8_t i = 0; i < target_layer_entry_count; i++) {
        if (target_layer_entries[i].holding_key == keycode && target_layer_entries[i].target_layer == layer) {
            return true;
        }
    }
    return false;
}

/* Return whether a holding key of the target layer `layer` is held. */
static bool is_holding_key_held_of(uint8_t layer) {
    for (uint8_t i = 0; i < state.holding_keys_held; i++) {
        if (is_holding_key_of(state.held_holding_keys[i].keycode, layer)) {
            return true;
        }
    }
    return false;
}

/*
 * Switch off the target layer that this module switched on, if any. The held
 * holding keys stay held.
 */
static void switch_off(void) {
    if (state.phase == PHASE_OFF) {
        return;
    }
    state.phase = PHASE_OFF;
    layer_off(state.target_layer);
}

/*
 * Switch the target layer `layer` on. The holding keys of `layer` that are
 * held now are the holding keys of this activation.
 */
static void switch_on(uint8_t layer) {
    for (uint8_t i = 0; i < state.holding_keys_held; i++) {
        held_holding_key_t *key    = &state.held_holding_keys[i];
        key->keeps_target_layer_on = is_holding_key_of(key->keycode, layer);
    }
    state.phase        = PHASE_HELD;
    state.target_layer = layer;
    layer_on(layer);
}

/* The trigger key of the target layer `layer` was pressed. */
static void trigger_pressed(uint8_t layer) {
    if (state.phase != PHASE_OFF && state.target_layer == layer) {
        /* Cancel: back to the holding layer. */
        switch_off();
        return;
    }
    if (layer_state_is(layer)) {
        /* On for another reason, Layer Lock included: not ours to touch. */
        return;
    }
    if (!is_holding_key_held_of(layer)) {
        return;
    }
    /* At most one target layer is on through this module. */
    switch_off();
    switch_on(layer);
}

/* The holding key `keycode` at `position` was pressed and is held. */
static void holding_key_held(uint16_t keycode, keypos_t position) {
    if (state.holding_keys_held == MAX_HOLDING_KEYS_HELD) {
        return;
    }
    held_holding_key_t *key    = &state.held_holding_keys[state.holding_keys_held++];
    key->keycode               = keycode;
    key->position              = position;
    key->keeps_target_layer_on = false;
}

/*
 * The holding key `keycode` at `position` was released. Does nothing if it
 * was not held: its press was a tap.
 */
static void holding_key_released(uint16_t keycode, keypos_t position) {
    uint8_t released = 0;
    for (;; released++) {
        if (released == state.holding_keys_held) {
            return;
        }
        const held_holding_key_t *key = &state.held_holding_keys[released];
        if (key->keycode == keycode && key->position.col == position.col && key->position.row == position.row) {
            break;
        }
    }
    const bool kept_target_layer_on = state.held_holding_keys[released].keeps_target_layer_on;
    /* Forget it: the key remembered last takes its place. */
    state.held_holding_keys[released] = state.held_holding_keys[--state.holding_keys_held];
    if (!kept_target_layer_on) {
        return;
    }
    for (uint8_t i = 0; i < state.holding_keys_held; i++) {
        if (state.held_holding_keys[i].keeps_target_layer_on) {
            return;
        }
    }
    /* The last holding key of the activation is up. */
    switch_off();
}

bool target_layer_process_record(uint16_t keycode, keyrecord_t *record) {
    if (IS_QK_MOMENTARY(keycode) && is_target_layer(QK_MOMENTARY_GET_LAYER(keycode))) {
        /* A trigger key. Its release means nothing. */
        if (record->event.pressed) {
            trigger_pressed(QK_MOMENTARY_GET_LAYER(keycode));
        }
        return false;
    }
    if (is_holding_key(keycode)) {
        if (!record->event.pressed) {
            holding_key_released(keycode, record->event.key);
        } else if (record->tap.count == 0) {
            holding_key_held(keycode, record->event.key);
        }
    }
    return true;
}

void target_layer_post_process_record(uint16_t keycode, keyrecord_t *record) {}

void target_layer_housekeeping_task(void) {}
