/*
 * Throwaway probe for issue #37. Not the behaviour spec and not production
 * code: just enough of a split trigger/hold state machine to exercise, on
 * both firmware sources, everything the real one would rely on.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "action.h"

#ifndef SPLIT_PROBE_ONE_SHOT_TERM
#    define SPLIT_PROBE_ONE_SHOT_TERM 300
#endif

/* One row of the table that the caller (a keymap family) passes in. */
typedef struct {
    uint16_t holding_key;  /* a layer-tap keycode, LT(layer, kc) */
    uint16_t trigger_key;  /* any keycode the caller reserves for it */
    uint8_t  target_layer; /* the layer the trigger key moves to */
} split_probe_entry_t;

/* Told by the caller. */
extern const split_probe_entry_t split_probe_entries[];
extern const uint8_t             split_probe_entry_count;

/* Hooks, forwarded by the caller from the QMK hooks of the same name. */
bool split_probe_process_record(uint16_t keycode, keyrecord_t *record);
void split_probe_post_process_record(uint16_t keycode, keyrecord_t *record);
void split_probe_reset(void);
