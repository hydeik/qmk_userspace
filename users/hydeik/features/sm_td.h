/* Copyright 2024 Stanislav Markin (https://github.com/stasmarkin)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Version: 0.3.1
 * Date: 2024-03-07
 */
#pragma once

#include QMK_KEYBOARD_H
#include "deferred_exec.h"
#ifdef SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS
#include "timer.h"
#endif


/* ************************************* *
 *         GLOBAL CONFIGURATION          *
 * ************************************* */

#ifndef SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS
#    define SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS 0
#endif

#if SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS > 0
    #define SMTD_SIMULTANEOUS_PRESSES_DELAY wait_ms(SMTD_GLOBAL_SIMULTANEOUS_PRESSES_DELAY_MS)
#else
    #define SMTD_SIMULTANEOUS_PRESSES_DELAY
#endif

#ifndef SMTD_GLOBAL_TAP_TERM
#    define SMTD_GLOBAL_TAP_TERM TAPPING_TERM
#endif

#ifndef SMTD_GLOBAL_SEQUENCE_TERM
#    define SMTD_GLOBAL_SEQUENCE_TERM TAPPING_TERM / 2
#endif

#ifndef SMTD_GLOBAL_FOLLOWING_TAP_TERM
#    define SMTD_GLOBAL_FOLLOWING_TAP_TERM TAPPING_TERM
#endif

#ifndef SMTD_GLOBAL_RELEASE_TERM
#    define SMTD_GLOBAL_RELEASE_TERM 50
#endif

#ifndef SMTD_GLOBAL_MODS_RECALL
#    define SMTD_GLOBAL_MODS_RECALL true
#endif

#ifndef SMTD_GLOBAL_AGGREGATE_TAPS
#    define SMTD_GLOBAL_AGGREGATE_TAPS false
#endif

/* ************************************* *
 *      USER SMTD STATE DECLARATION      *
 * ************************************* */

#define SMTD(keycode)                                \
    {                                                \
        .freeze            = false,                  \
            .macro_keycode = keycode,                \
            .tap_mods      = 0,                      \
            .following_key = MAKE_KEYPOS(0, 0),      \
            .stage         = SMTD_STAGE_NONE,        \
            .timeout       = INVALID_DEFERRED_TOKEN, \
            .sequence_len  = 0,                      \
    }

/* ************************************* *
 *       USER TIMEOUT DEFINITIONS        *
 * ************************************* */

typedef enum {
    SMTD_TIMEOUT_TAP,
    SMTD_TIMEOUT_SEQUENCE,
    SMTD_TIMEOUT_FOLLOWING_TAP,
    SMTD_TIMEOUT_RELEASE,
} smtd_timeout;

__attribute__((weak)) uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout);

uint32_t get_smtd_timeout_default(smtd_timeout timeout);
uint32_t get_smtd_timeout_or_default(uint16_t keycode, smtd_timeout timeout);

/* ************************************* *
 *    USER FEATURE FLAGS DEFINITIONS     *
 * ************************************* */

typedef enum {
    SMTD_FEATURE_MODS_RECALL,
    SMTD_FEATURE_AGGREGATE_TAPS,
} smtd_feature;

__attribute__((weak)) bool smtd_feature_enabled(uint16_t keycode, smtd_feature feature);

bool smtd_feature_enabled_default(smtd_feature feature);
bool smtd_feature_enabled_or_default(uint16_t keycode, smtd_feature feature);

/* ************************************* *
 *       USER ACTION DEFINITIONS         *
 * ************************************* */

typedef enum {
    SMTD_ACTION_TOUCH,
    SMTD_ACTION_TAP,
    SMTD_ACTION_HOLD,
    SMTD_ACTION_RELEASE,
} smtd_action;

void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t sequence_len);


/* ************************************* *
 *       USER STATES DEFINITIONS         *
 * ************************************* */

typedef enum {
    SMTD_STAGE_NONE,
    SMTD_STAGE_TOUCH,
    SMTD_STAGE_SEQUENCE,
    SMTD_STAGE_FOLLOWING_TOUCH,
    SMTD_STAGE_HOLD,
    SMTD_STAGE_RELEASE,
} smtd_stage;

typedef struct {
    uint16_t       macro_keycode;
    uint8_t        tap_mods;
    uint8_t        sequence_len;
    keypos_t       following_key;
    deferred_token timeout;
    smtd_stage     stage;
    bool           freeze;
} smtd_state;

extern smtd_state smtd_states[];
extern size_t     smtd_states_size;


/* ************************************* *
 *      CORE LOGIC IMPLEMENTATION        *
 * ************************************* */

extern smtd_state  *smtd_active_states[10];
extern uint8_t      smtd_active_states_next_idx;

#define DO_ACTION_TAP(state)                                                                                                  \
    uint8_t current_mods = get_mods();                                                                                        \
    if (smtd_feature_enabled_or_default(state->macro_keycode, SMTD_FEATURE_MODS_RECALL) && state->tap_mods != current_mods) { \
        set_mods(state->tap_mods);                                                                                            \
        send_keyboard_report();                                                                                               \
        SMTD_SIMULTANEOUS_PRESSES_DELAY;                                                                        \
    }                                                                                                                         \
    on_smtd_action(state->macro_keycode, SMTD_ACTION_TAP, state->sequence_len);                                               \
    if (smtd_feature_enabled_or_default(state->macro_keycode, SMTD_FEATURE_MODS_RECALL) && state->tap_mods != current_mods) { \
        SMTD_SIMULTANEOUS_PRESSES_DELAY;                                                                        \
        set_mods(current_mods);                                                                                               \
        send_keyboard_report();                                                                                               \
    }

#define PUSH_STATE(state)                                    \
    smtd_active_states[smtd_active_states_next_idx] = state; \
    smtd_active_states_next_idx++;

#define REMOVE_STATE(state)                                                 \
    for (uint8_t i = 0; i < smtd_active_states_next_idx; i++) {             \
        if (smtd_active_states[i] == state) {                               \
            for (uint8_t j = i; j < smtd_active_states_next_idx - 1; j++) { \
                smtd_active_states[j] = smtd_active_states[j + 1];          \
            }                                                               \
            smtd_active_states_next_idx--;                                  \
            smtd_active_states[smtd_active_states_next_idx] = NULL;         \
            break;                                                          \
        }                                                                   \
    }

void smtd_press_following_key(smtd_state *state, bool release);
void smtd_next_stage(smtd_state *state, smtd_stage next_stage);
bool process_smtd_state(uint16_t keycode, keyrecord_t *record, smtd_state *state);

/* ************************************* *
 *      ENTRY POINT IMPLEMENTATION       *
 * ************************************* */

bool process_smtd(uint16_t keycode, keyrecord_t *record);
