/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * This file is built against both firmware sources, so it must use nothing
 * that upstream QMK added after the last merge of vial-qmk.
 */

#include "target_layer.h"

#include <stddef.h>

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
     * Whether this key keeps the target layer on: it is one of its holding
     * keys and was held when the trigger key switched the target layer on.
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
     * Otherwise it times out, counted from `pending_since`, unless the
     * caller suspends the timeout: `pending_suspended` tells that the last
     * scan loop found it suspended. They are written in other phases too,
     * and set when the phase begins.
     */
    bool     pending_used;
    bool     pending_suspended;
    uint16_t pending_since;
    /*
     * The locked target layers, one bit per layer: the target layers that
     * were locked while they were on through this module, and not unlocked
     * since. The trigger key of such a layer unlocks it, as it does for a
     * target layer that Layer Lock holds for any other reason. They are apart from
     * `phase`: the trigger key of another target layer works meanwhile.
     */
    layer_state_t locked_target_layers;
} state;

void target_layer_reset(void) {
    state.holding_keys_held    = 0;
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

/*
 * Return the entry of the holding key `keycode` and the target layer
 * `layer`, or NULL if `keycode` is no holding key of `layer`.
 */
static const target_layer_entry_t *find_entry(uint16_t keycode, uint8_t layer) {
    for (uint8_t i = 0; i < target_layer_entry_count; i++) {
        if (target_layer_entries[i].holding_key == keycode && target_layer_entries[i].target_layer == layer) {
            return &target_layer_entries[i];
        }
    }
    return NULL;
}

/* Return whether `keycode` is a holding key of the target layer `layer`. */
static bool is_holding_key_of(uint16_t keycode, uint8_t layer) {
    return find_entry(keycode, layer) != NULL;
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

bool target_layer_is_trigger_key_working(uint8_t layer) {
    return is_holding_key_held_of(layer);
}

/* Return the bit of `layer` in a set of layers. */
static layer_state_t layer_bit(uint8_t layer) {
    return (layer_state_t)1 << layer;
}

/*
 * Switch off the target layer that this module switched on, held or pending,
 * if any. The held holding keys stay held.
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
    state.locked_target_layers |= layer_bit(state.target_layer);
#ifdef LAYER_LOCK_ENABLE
    layer_lock_on(state.target_layer);
#endif
}

/*
 * Whether the target layer `layer` is locked: by this module, or by Layer
 * Lock, whichever way the layer came on before it was locked.
 */
static bool is_locked(uint8_t layer) {
#ifdef LAYER_LOCK_ENABLE
    if (is_layer_locked(layer)) {
        return true;
    }
#endif
    return (state.locked_target_layers & layer_bit(layer)) != 0;
}

enum target_layer_state_t target_layer_state_of(uint8_t layer) {
    if (!is_target_layer(layer)) {
        return TARGET_LAYER_STATE_OFF;
    }
    /*
     * Layer Lock is asked first: the hooks find out only at the next key
     * event or scan loop that it locked the target layer that is on.
     */
    if (is_locked(layer)) {
        return TARGET_LAYER_STATE_LOCKED;
    }
    if (state.phase == PHASE_OFF || state.target_layer != layer) {
        return TARGET_LAYER_STATE_OFF;
    }
    return state.phase == PHASE_PENDING ? TARGET_LAYER_STATE_PENDING : TARGET_LAYER_STATE_ON;
}

/* Unlock the locked target layer `layer` and switch it off. */
static void unlock(uint8_t layer) {
    if (state.phase != PHASE_OFF && state.target_layer == layer) {
        state.phase = PHASE_OFF;
    }
    state.locked_target_layers &= ~layer_bit(layer);
#ifdef LAYER_LOCK_ENABLE
    layer_lock_off(layer);
#else
    layer_off(layer);
#endif
}

/*
 * Take over what Layer Lock did behind this module. QMK handles the Layer
 * Lock key after the key-event hook, so this module finds out afterwards.
 *
 * - A target layer that Layer Lock locked while it was on through this
 *   module, held or pending, is a locked target layer from then on.
 * - A locked target layer that is locked no longer, unlocked by the Layer
 *   Lock key or by the idle timeout of Layer Lock, is forgotten.
 */
static void sync_with_layer_lock(void) {
#ifdef LAYER_LOCK_ENABLE
    if (state.phase != PHASE_OFF && is_layer_locked(state.target_layer)) {
        lock();
    }
    for (uint8_t i = 0; state.locked_target_layers && i < target_layer_entry_count; i++) {
        const uint8_t layer = target_layer_entries[i].target_layer;
        if (!is_layer_locked(layer)) {
            state.locked_target_layers &= ~layer_bit(layer);
        }
    }
#endif
}

/*
 * Switch the target layer `layer` on, for a trigger key pressed at the
 * key-event time `time`. The holding keys of `layer` that are held now keep
 * it on.
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
    if (is_locked(layer)) {
        /* Leave the locked target layer. */
        unlock(layer);
        return;
    }
    if (state.phase != PHASE_OFF && state.target_layer == layer) {
        /* Cancel: back to the holding layer, or off the pending target layer. */
        switch_off();
        return;
    }
    if (layer_state_is(layer)) {
        /* On for another reason than this module or Layer Lock: not ours to touch. */
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
 * Make the target layer that a holding key kept on a pending target layer,
 * at the key-event time `time` of the release of that holding key.
 */
static void make_pending(uint16_t time) {
    state.phase             = PHASE_PENDING;
    state.pending_used      = false;
    state.pending_suspended = false;
    state.pending_since     = time;
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
 * caller suspends the timeout, it does not run at all. It starts anew in the
 * first scan loop that finds the suspension ended, and so no earlier than a
 * timeout that the caller started at the key event that ended the
 * suspension.
 */
static void expire_pending(void) {
    if (state.phase != PHASE_PENDING) {
        return;
    }
    if (target_layer_is_pending_timeout_suspended()) {
        state.pending_suspended = true;
    } else if (state.pending_suspended) {
        state.pending_suspended = false;
        state.pending_since     = timer_read();
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
    /*
     * The last holding key that kept the target layer on is up. If this is
     * a quick release, the entry of this holding key says what becomes of
     * the target layer; it kept the target layer on, so it has such an entry.
     */
    uint8_t quick_release = TARGET_LAYER_QUICK_RELEASE_NOTHING;
    if (state.release_is_quick && TIMER_DIFF_16(time, state.trigger_pressed_at) < TARGET_LAYER_ONE_SHOT_TERM) {
        quick_release = find_entry(keycode, state.target_layer)->quick_release;
    }
    switch (quick_release) {
        case TARGET_LAYER_QUICK_RELEASE_PENDING:
            make_pending(time);
            break;
        case TARGET_LAYER_QUICK_RELEASE_LOCKED:
            lock();
            break;
        default:
            switch_off();
            break;
    }
}

/*
 * The key event of a key that is neither a trigger key nor a held holding
 * key.
 */
static void other_key_event(uint16_t keycode, const keyrecord_t *record) {
    if (target_layer_is_one_shot_mod_key(keycode, record)) {
        /*
         * Not a typed key. The timeout of a pending target layer starts
         * anew at its press and at its release, so that it does not end
         * before the pending one-shot mod. It counts from now, not from the
         * time of the key event: QMK may hand over the press of a tap only
         * at its release, and its own one-shot mods count from then.
         */
        state.pending_since = timer_read();
    } else if (record->event.pressed && is_key_typed(keycode, record)) {
        /*
         * A key is typed: the release of the holding key is not quick any
         * more, and a pending target layer is used.
         */
        state.release_is_quick = false;
        state.pending_used     = true;
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
    other_key_event(keycode, record);
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
