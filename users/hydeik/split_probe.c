/* Throwaway probe for issue #37. See split_probe.h. */

#include "split_probe.h"

#include "action_layer.h"
#include "quantum_keycodes.h"
#include "timer.h"

#ifdef LAYER_LOCK_ENABLE
#    include "layer_lock.h"
#endif

#define MAX_ENTRIES 8
#define BIT(i) ((uint8_t)(1U << (i)))

/* `held`, `target_on`, `one_shot` and `one_shot_used` are sets of entries. */
static struct {
    uint8_t  held;                 /* the holding key is held, not tapped */
    keypos_t held_at[MAX_ENTRIES]; /* where that holding key is */
    uint8_t  target_on;            /* this module switched the target layer on */
    uint8_t  one_shot;             /* the target layer stays on for one key */
    uint8_t  one_shot_used;        /* that key has been pressed */
    bool     key_typed;            /* a key was pressed since the last trigger key */
    uint16_t triggered_at;         /* event time of the last trigger key press */
} state;

void split_probe_reset(void) {
    state.held          = 0;
    state.target_on     = 0;
    state.one_shot      = 0;
    state.one_shot_used = 0;
}

static void target_off(uint8_t entry) {
#ifdef LAYER_LOCK_ENABLE
    if (is_layer_locked(split_probe_entries[entry].target_layer)) {
        return;
    }
#endif
    layer_off(split_probe_entries[entry].target_layer);
}

bool split_probe_process_record(uint16_t keycode, keyrecord_t *record) {
    for (uint8_t i = 0; i < split_probe_entry_count && i < MAX_ENTRIES; i++) {
        const split_probe_entry_t *entry = &split_probe_entries[i];

        if (keycode == entry->holding_key) {
            if (record->event.pressed && record->tap.count == 0) {
                /* Held, not tapped. QMK switches its own layer on. */
                state.held |= BIT(i);
                state.held_at[i] = record->event.key;
            } else if (!record->event.pressed && (state.held & BIT(i)) && KEYEQ(state.held_at[i], record->event.key)) {
                state.held &= ~BIT(i);
                if (state.target_on & BIT(i)) {
                    state.target_on &= ~BIT(i);
                    if (!state.key_typed && TIMER_DIFF_16(record->event.time, state.triggered_at) < SPLIT_PROBE_ONE_SHOT_TERM) {
                        state.one_shot |= BIT(i); /* the target layer stays on for the next key */
                    } else {
                        target_off(i);
                    }
                }
            }
            return true; /* QMK goes on to handle the layer-tap key itself */
        }

        if (keycode == entry->trigger_key) {
            if (record->event.pressed && (state.held & BIT(i))) {
                if (state.target_on & BIT(i)) {
                    state.target_on &= ~BIT(i); /* the same key again: cancel */
                    target_off(i);
                } else {
                    state.target_on |= BIT(i);
                    state.key_typed    = false;
                    state.triggered_at = record->event.time;
                    layer_on(entry->target_layer);
                }
            }
            return false;
        }
    }

    if (record->event.pressed) {
        state.key_typed     = true;
        state.one_shot_used = state.one_shot; /* switched off once this key is handled */
    }
    return true;
}

void split_probe_post_process_record(uint16_t keycode, keyrecord_t *record) {
    for (uint8_t i = 0; state.one_shot_used; i++) {
        if (state.one_shot_used & BIT(i)) {
            state.one_shot_used &= ~BIT(i);
            state.one_shot &= ~BIT(i);
            target_off(i);
        }
    }
}
