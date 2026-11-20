/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "smart_mod.h"

#include "action_layer.h"
#include "action_tapping.h"
#include "action_util.h"
#include "modifiers.h"
#include "quantum_keycodes.h"
#include "timer.h"

#ifdef CAPS_WORD_ENABLE
#    include "caps_word.h"
#endif

/*
 * The state machine. `held`, `tapping`, `pending`, `applied`, `latching` and
 * `latched` are sets of modifier bits (MOD_BIT); the times are those of key
 * events, 16 bits wide.
 */
static struct {
    /* The one-shot mod keys that are down. */
    uint8_t held;
    /*
     * The held one-shot mod key that can still become a tap: no other key has
     * been pressed since its press, and the One Shot Term has not ended.
     */
    uint8_t  tapping;
    uint16_t tapping_since;
    /*
     * Pending one-shot mods: tapped, and not visible to the host. They time
     * out together, counted from the last one-shot mod tap.
     */
    uint8_t  pending;
    uint16_t pending_since;
    /*
     * Pending one-shot mods that were registered for the key press that
     * consumed them, and are dropped once the report of that key is sent.
     */
    uint8_t applied;
    /* The number of smart mod layer keys that are held, not tapped. */
    uint8_t layer_keys_held;
    /*
     * Whether the smart mod layer is on. It is switched on when the first
     * smart mod layer key is held, and off as soon as all one-shot mod keys
     * are up again, or else with the last smart mod layer key.
     */
    bool layer_on;
    /*
     * The held one-shot mod keys that latch on release: pressed on the smart
     * mod layer, with no key typed since.
     */
    uint8_t latching;
    /*
     * Latched one-shot mods: registered, and kept so until the last smart mod
     * layer key is released.
     */
    uint8_t latched;
    /*
     * Whether the release window is open: the latched one-shot mods become
     * pending if the last smart mod layer key is released now. It opens at
     * the press of a one-shot mod key on the smart mod layer, and closes
     * when a key is typed or the window ends.
     */
    bool     window_open;
    uint16_t window_since;
    /*
     * Whether the only pending one-shot mod is the Shift of a Shift tap, with
     * no key pressed since: a second Shift tap within the tapping term of
     * the first one turns Caps Word on.
     */
    bool     shift_tapped;
    uint16_t shift_tapped_since;
} state;

void smart_mod_reset(void) {
    state.held            = 0;
    state.tapping         = 0;
    state.pending         = 0;
    state.applied         = 0;
    state.layer_keys_held = 0;
    state.layer_on        = false;
    state.latching        = 0;
    state.latched         = 0;
    state.window_open     = false;
    state.shift_tapped    = false;
}

/* Drop the consumed one-shot mods; the key they applied to has been sent. */
static void drop_applied(void) {
    if (state.applied) {
        const uint8_t mods = state.applied;
        state.applied      = 0;
        unregister_mods(mods);
    }
}

/*
 * End the tap of the held one-shot mod key if the One Shot Term has ended.
 * The release finds that out too, but only as long as the 16-bit time of the
 * key events has not wrapped around since the press.
 */
static void expire_tapping(void) {
    if (state.tapping && timer_elapsed(state.tapping_since) >= SMART_MOD_ONE_SHOT_TERM) {
        state.tapping = 0;
    }
}

/*
 * Close the release window if it has ended. As with the One Shot Term, the
 * release of the smart mod layer key finds that out by itself only as long
 * as the time of the key events has not wrapped around.
 */
static void expire_window(void) {
    if (state.window_open && timer_elapsed(state.window_since) >= SMART_MOD_LAYER_RELEASE_WINDOW) {
        state.window_open = false;
    }
}

/* Clear the pending one-shot mods if their timeout has ended. */
static void expire_pending(void) {
    if (state.pending && timer_elapsed(state.pending_since) >= SMART_MOD_ONE_SHOT_TIMEOUT) {
        state.pending      = 0;
        state.shift_tapped = false;
    }
}

/* Register the pending one-shot mods for the key press that consumes them. */
static void consume_pending(void) {
    if (state.pending) {
        /*
         * A modifier that a held key has registered already is that key's to
         * release, so it is not among those dropped afterwards.
         */
        state.applied = state.pending & ~get_mods();
        state.pending = 0;
        register_mods(state.applied);
    }
}

static void switch_layer_on(void) {
    state.layer_on = true;
    layer_on(smart_mod_layer());
}

static void switch_layer_off(void) {
    if (state.layer_on) {
        state.layer_on = false;
        layer_off(smart_mod_layer());
    }
}

static void process_one_shot_mod_key(uint8_t mods, const keyrecord_t *record) {
    if (record->event.pressed) {
        /* A one-shot mod key is an ordinary modifier while held. */
        register_mods(mods);
        state.held |= mods;
        if (state.pending & mods) {
            /*
             * The same key again: cancel it. This press is never a tap, and
             * it does not latch.
             */
            state.pending &= ~mods;
            state.tapping = 0;
        } else {
            state.tapping       = mods;
            state.tapping_since = record->event.time;
            if (state.layer_on) {
                state.latching |= mods;
                state.window_open  = true;
                state.window_since = record->event.time;
            }
        }
    } else {
        state.held &= ~mods;
        if (state.latching & mods) {
            /* Latch it: the modifier stays registered. */
            state.latching &= ~mods;
            state.latched |= mods;
            state.tapping &= ~mods;
        } else {
            /* A latched one-shot mod cannot be released on its own. */
            unregister_mods(mods & ~state.latched);
            if (state.tapping == mods) {
                state.tapping = 0;
                if (TIMER_DIFF_16(record->event.time, state.tapping_since) < SMART_MOD_ONE_SHOT_TERM) {
                    state.pending |= mods;
                    state.pending_since = record->event.time;
                }
            }
        }
        if (!state.held) {
            /* The one-shot mods are chosen. Back to the keys beneath. */
            switch_layer_off();
        }
    }
}

/*
 * A smart mod layer key that is held, not tapped. QMK does not get to see
 * it: the smart mod layer is switched on and off here alone.
 */
static void process_held_layer_key(const keyrecord_t *record) {
    if (record->event.pressed) {
        if (state.layer_keys_held++ == 0) {
            switch_layer_on();
        }
    } else if (state.layer_keys_held && --state.layer_keys_held == 0) {
        /*
         * The last one is released, and with it every latched one-shot mod.
         * A one-shot mod key that is still down goes on as an ordinary
         * modifier, or as a tap, outside the smart mod layer.
         */
        const uint8_t mods = state.latched;
        state.latched      = 0;
        state.latching     = 0;
        switch_layer_off();
        unregister_mods(mods & ~state.held);
        if (mods && state.window_open && TIMER_DIFF_16(record->event.time, state.window_since) < SMART_MOD_LAYER_RELEASE_WINDOW) {
            /* No key was typed, and the release is quick: keep them. */
            state.pending |= mods;
            state.pending_since = record->event.time;
        }
        state.window_open = false;
    }
}

/*
 * The press of a tapped smart mod layer key with the Shift tap rule. It is a
 * toggle: it makes Shift pending, or cancels all pending one-shot mods.
 * `again` tells that the Shift of such a tap is the only pending one-shot
 * mod and no key has been pressed since.
 */
static void process_shift_tap(uint16_t keycode, keyrecord_t *record, bool again) {
    if (again && TIMER_DIFF_16(record->event.time, state.shift_tapped_since) < GET_TAPPING_TERM(keycode, record)) {
        /* A double tap: Caps Word instead of the pending Shift. */
        state.pending = 0;
#ifdef CAPS_WORD_ENABLE
        caps_word_on();
#endif
    } else if (state.pending) {
        state.pending = 0;
    } else {
        state.pending            = MOD_BIT_LSHIFT;
        state.pending_since      = record->event.time;
        state.shift_tapped       = true;
        state.shift_tapped_since = record->event.time;
    }
}

/*
 * Whether the press in `record` leaves the pending one-shot mods alone: the
 * key is an ordinary modifier or only switches layers. Every other key press
 * consumes them. One-shot mod keys never get here.
 */
static bool lets_pending_pass(uint16_t keycode, const keyrecord_t *record) {
    if (IS_MODIFIER_KEYCODE(keycode) || keycode == QK_LAYER_LOCK) {
        return true;
    }
    if (IS_QK_MOD_TAP(keycode) || IS_QK_LAYER_TAP(keycode)) {
        /* Held, not tapped. */
        return record->tap.count == 0;
    }
    return IS_QK_TO(keycode) || IS_QK_MOMENTARY(keycode) || IS_QK_LAYER_MOD(keycode) || IS_QK_DEF_LAYER(keycode) || IS_QK_PERSISTENT_DEF_LAYER(keycode) || IS_QK_TOGGLE_LAYER(keycode) || IS_QK_ONE_SHOT_LAYER(keycode) || IS_QK_LAYER_TAP_TOGGLE(keycode);
}

bool smart_mod_process_record(uint16_t keycode, keyrecord_t *record) {
    const uint8_t mods = smart_mod_one_shot_mod_bits(keycode);

    /* This key event may come before the elapsed-time hook notices. */
    expire_pending();

    /* Any key press ends the wait for the second Shift tap. */
    const bool shift_tapped = state.shift_tapped;
    if (record->event.pressed) {
        state.shift_tapped = false;
    }

    if (mods) {
        process_one_shot_mod_key(mods, record);
        return false;
    }

    if (smart_mod_is_layer_key(keycode) && record->tap.count == 0) {
        if (record->event.pressed) {
            state.tapping = 0;
        }
        process_held_layer_key(record);
        return false;
    }

    if (smart_mod_is_layer_key(keycode) && smart_mod_layer_key_taps_shift(keycode)) {
        if (record->event.pressed) {
            state.tapping = 0;
            process_shift_tap(keycode, record, shift_tapped);
        }
        return false;
    }

    if (record->event.pressed) {
        /* Another key in between: the held one-shot mod key is not a tap. */
        state.tapping = 0;
        if (!lets_pending_pass(keycode, record)) {
            /*
             * A key is typed: the one-shot mod keys that are held on the
             * smart mod layer are ordinary modifiers that were used.
             */
            state.latching    = 0;
            state.window_open = false;
            consume_pending();
        }
    }
    return true;
}

void smart_mod_post_process_record(uint16_t keycode, keyrecord_t *record) {
    drop_applied();
}

void smart_mod_housekeeping_task(void) {
    /*
     * QMK skips the after-report hook for a key that one of its features
     * handled completely, so the drop is caught up on here.
     */
    drop_applied();
    expire_tapping();
    expire_window();
    expire_pending();
}
