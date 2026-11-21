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
#include "timer.h"

#include "key_typed.h"

#ifdef LAYER_LOCK_ENABLE
#    include "layer_lock.h"
#endif

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
    /*
     * A pending target layer: this module switched it on, and it stays on
     * for the next typed key after a quick release of the holding key.
     */
    PHASE_PENDING,
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
    /*
     * While PHASE_HELD: whether a release of the last holding key would be a
     * quick release now. No key was typed since the trigger press, at the
     * key-event time `trigger_pressed_at`, and the One Shot Term has not
     * ended.
     */
    bool     release_is_quick;
    uint16_t trigger_pressed_at;
    /*
     * While PHASE_PENDING: whether a typed key has used the pending target
     * layer, which is switched off once the report of that key is sent.
     * Otherwise it times out, counted from `pending_since`.
     */
    bool     pending_used;
    uint16_t pending_since;
    /*
     * The locked target layers, one bit per layer: the target layers that
     * were locked while they were on through this module, and not unlocked
     * since. The trigger key of such a layer unlocks it.
     */
    layer_state_t locked_target_layers;
} state;

void target_layer_reset(void) {
    state.holding_keys_held = 0;
    state.phase                = PHASE_OFF;
    state.locked_target_layers = 0;
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
 * Make the target layer that is on through this module a locked target
 * layer. It is locked as by the Layer Lock key, and no longer held or
 * pending.
 */
static void lock(void) {
    state.phase = PHASE_OFF;
    state.locked_target_layers |= (layer_state_t)1 << state.target_layer;
#ifdef LAYER_LOCK_ENABLE
    layer_lock_on(state.target_layer);
#endif
}

/* Unlock the locked target layer `layer` and switch it off. */
static void unlock(uint8_t layer) {
    state.locked_target_layers &= ~((layer_state_t)1 << layer);
#ifdef LAYER_LOCK_ENABLE
    layer_lock_off(layer);
#else
    layer_off(layer);
#endif
}

/*
 * Take over what Layer Lock did behind this module: forget the locked target
 * layers that are locked no longer, unlocked by the Layer Lock key or by the
 * idle timeout of Layer Lock.
 */
static void sync_with_layer_lock(void) {
#ifdef LAYER_LOCK_ENABLE
    for (uint8_t i = 0; state.locked_target_layers && i < target_layer_entry_count; i++) {
        const uint8_t layer = target_layer_entries[i].target_layer;
        if (!is_layer_locked(layer)) {
            state.locked_target_layers &= ~((layer_state_t)1 << layer);
        }
    }
#endif
}

/*
 * Return what a quick release of the holding key `keycode` does to the
 * target layer `layer`.
 */
static uint8_t quick_release_of(uint16_t keycode, uint8_t layer) {
    for (uint8_t i = 0; i < target_layer_entry_count; i++) {
        if (target_layer_entries[i].holding_key == keycode && target_layer_entries[i].target_layer == layer) {
            return target_layer_entries[i].quick_release;
        }
    }
    return TARGET_LAYER_QUICK_RELEASE_NOTHING;
}

/*
 * Switch the target layer `layer` on, for a trigger key pressed at the
 * key-event time `time`. The holding keys of `layer` that are held now are
 * the holding keys of this activation.
 */
static void switch_on(uint8_t layer, uint16_t time) {
    for (uint8_t i = 0; i < state.holding_keys_held; i++) {
        held_holding_key_t *key    = &state.held_holding_keys[i];
        key->keeps_target_layer_on = is_holding_key_of(key->keycode, layer);
    }
    state.phase              = PHASE_HELD;
    state.target_layer       = layer;
    state.release_is_quick   = true;
    state.trigger_pressed_at = time;
    layer_on(layer);
}

/*
 * The trigger key of the target layer `layer` was pressed, at the key-event
 * time `time`.
 */
static void trigger_pressed(uint8_t layer, uint16_t time) {
    if (state.locked_target_layers & ((layer_state_t)1 << layer)) {
        unlock(layer);
        return;
    }
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
    switch_on(layer, time);
}

/*
 * Switch off the pending target layer that a typed key has used; that key
 * has been sent.
 */
static void switch_off_used(void) {
    if (state.phase == PHASE_PENDING && state.pending_used) {
        switch_off();
    }
}

/*
 * Switch off the pending target layer if its timeout has ended. While the
 * caller suspends the timeout, it does not run at all: it starts anew when
 * the suspension ends.
 */
static void expire_pending(void) {
    if (state.phase != PHASE_PENDING) {
        return;
    }
    if (target_layer_is_pending_timeout_suspended()) {
        state.pending_since = timer_read();
    } else if (timer_elapsed(state.pending_since) >= TARGET_LAYER_PENDING_TIMEOUT) {
        switch_off();
    }
}

/* The holding key `keycode` at `position` was pressed and is held. */
static void holding_key_held(uint16_t keycode, keypos_t position) {
    if (state.phase == PHASE_PENDING) {
        /* A new gesture starts. */
        switch_off();
    }
    if (state.holding_keys_held == MAX_HOLDING_KEYS_HELD) {
        return;
    }
    held_holding_key_t *key    = &state.held_holding_keys[state.holding_keys_held++];
    key->keycode               = keycode;
    key->position              = position;
    key->keeps_target_layer_on = false;
}

/*
 * End the wait for a quick release if the One Shot Term has ended. The
 * release finds that out too, but only as long as the 16-bit time of the key
 * events has not wrapped around since the trigger press.
 */
static void expire_quick_release(void) {
    if (state.phase == PHASE_HELD && state.release_is_quick && timer_elapsed(state.trigger_pressed_at) >= TARGET_LAYER_ONE_SHOT_TERM) {
        state.release_is_quick = false;
    }
}

/*
 * The holding key `keycode` at `position` was released, at the key-event
 * time `time`. Does nothing if it was not held: its press was a tap.
 */
static void holding_key_released(uint16_t keycode, keypos_t position, uint16_t time) {
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
    if (!kept_target_layer_on || state.phase != PHASE_HELD) {
        return;
    }
    for (uint8_t i = 0; i < state.holding_keys_held; i++) {
        if (state.held_holding_keys[i].keeps_target_layer_on) {
            return;
        }
    }
    /* The last holding key of the activation is up. */
    const bool    quick         = state.release_is_quick && TIMER_DIFF_16(time, state.trigger_pressed_at) < TARGET_LAYER_ONE_SHOT_TERM;
    const uint8_t quick_release = quick ? quick_release_of(keycode, state.target_layer) : TARGET_LAYER_QUICK_RELEASE_NOTHING;
    if (quick_release == TARGET_LAYER_QUICK_RELEASE_PENDING) {
        state.phase         = PHASE_PENDING;
        state.pending_used  = false;
        state.pending_since = time;
    } else if (quick_release == TARGET_LAYER_QUICK_RELEASE_LOCKED) {
        lock();
    } else {
        switch_off();
    }
}

bool target_layer_process_record(uint16_t keycode, keyrecord_t *record) {
    sync_with_layer_lock();
    if (IS_QK_MOMENTARY(keycode) && is_target_layer(QK_MOMENTARY_GET_LAYER(keycode))) {
        /* A trigger key. Its release means nothing. */
        if (record->event.pressed) {
            trigger_pressed(QK_MOMENTARY_GET_LAYER(keycode), record->event.time);
        }
        return false;
    }
    if (is_holding_key(keycode)) {
        if (!record->event.pressed) {
            holding_key_released(keycode, record->event.key, record->event.time);
            return true;
        }
        if (record->tap.count == 0) {
            holding_key_held(keycode, record->event.key);
            return true;
        }
        /* Tapped, it is a typed key like any other. */
    }
    if (!record->event.pressed) {
        return true;
    }
    if (target_layer_is_one_shot_mod_key(keycode, record)) {
        /* Not a typed key. A pending target layer outlasts its tap. */
        state.pending_since = record->event.time;
    } else if (is_key_typed(keycode, record)) {
        state.release_is_quick = false;
        state.pending_used     = true;
    }
    return true;
}

void target_layer_post_process_record(uint16_t keycode, keyrecord_t *record) {
    switch_off_used();
}

void target_layer_housekeeping_task(void) {
    /*
     * QMK skips the after-report hook for a key that one of its features
     * handled completely, so the switch-off is caught up on here.
     */
    switch_off_used();
    sync_with_layer_lock();
    expire_quick_release();
    expire_pending();
}
