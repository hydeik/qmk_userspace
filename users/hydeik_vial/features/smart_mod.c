/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "smart_mod.h"

#include "action_util.h"

/*
 * The state machine. Every field is a set of modifier bits (MOD_BIT), so a
 * modifier is in at most one of these states at a time.
 */
static struct {
    /* Pending one-shot mods: tapped, and not visible to the host. */
    uint8_t pending;
    /*
     * Pending one-shot mods that were registered for the key press that
     * consumed them, and are dropped once the report of that key is sent.
     */
    uint8_t applied;
} state;

void smart_mod_reset(void) {
    state.pending = 0;
    state.applied = 0;
}

/* Drop the consumed one-shot mods; the key they applied to has been sent. */
static void drop_applied(void) {
    if (state.applied) {
        const uint8_t mods = state.applied;
        state.applied      = 0;
        unregister_mods(mods);
    }
}

/* Register the pending one-shot mods for the key press that consumes them. */
static void consume_pending(void) {
    if (state.pending) {
        state.applied = state.pending;
        state.pending = 0;
        register_mods(state.applied);
    }
}

static void process_one_shot_mod_key(uint8_t mods, const keyrecord_t *record) {
    if (record->event.pressed) {
        /* A one-shot mod key is an ordinary modifier while held. */
        register_mods(mods);
    } else {
        unregister_mods(mods);
        state.pending |= mods;
    }
}

bool smart_mod_process_record(uint16_t keycode, keyrecord_t *record) {
    const uint8_t mods = smart_mod_one_shot_mod_bits(keycode);
    if (mods) {
        process_one_shot_mod_key(mods, record);
        return false;
    }

    if (record->event.pressed) {
        consume_pending();
    }
    return true;
}

void smart_mod_post_process_record(uint16_t keycode, keyrecord_t *record) {
    drop_applied();
}

void smart_mod_housekeeping_task(void) {}
