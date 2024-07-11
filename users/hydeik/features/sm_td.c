#include "sm_td.h"

/* ************************************* *
 *       USER TIMEOUT DEFINITIONS        *
 * ************************************* */

uint32_t get_smtd_timeout_default(smtd_timeout timeout) {
    switch (timeout) {
        case SMTD_TIMEOUT_TAP:
            return SMTD_GLOBAL_TAP_TERM;
        case SMTD_TIMEOUT_SEQUENCE:
            return SMTD_GLOBAL_SEQUENCE_TERM;
        case SMTD_TIMEOUT_FOLLOWING_TAP:
            return SMTD_GLOBAL_FOLLOWING_TAP_TERM;
        case SMTD_TIMEOUT_RELEASE:
            return SMTD_GLOBAL_RELEASE_TERM;
    }
    return 0;
}

uint32_t get_smtd_timeout_or_default(uint16_t keycode, smtd_timeout timeout) {
    if (get_smtd_timeout) {
        return get_smtd_timeout(keycode, timeout);
    }
    return get_smtd_timeout_default(timeout);
}

/* ************************************* *
 *    USER FEATURE FLAGS DEFINITIONS     *
 * ************************************* */

bool smtd_feature_enabled_default(smtd_feature feature) {
    switch (feature) {
        case SMTD_FEATURE_MODS_RECALL:
            return SMTD_GLOBAL_MODS_RECALL;
        case SMTD_FEATURE_AGGREGATE_TAPS:
            return SMTD_GLOBAL_AGGREGATE_TAPS;
    }
    return false;
}

bool smtd_feature_enabled_or_default(uint16_t keycode, smtd_feature feature) {
    if (smtd_feature_enabled) {
        return smtd_feature_enabled(keycode, feature);
    }
    return smtd_feature_enabled_default(feature);
}

/* ************************************* *
 *      CORE LOGIC IMPLEMENTATION        *
 * ************************************* */
smtd_state  *smtd_active_states[10]      = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
uint8_t      smtd_active_states_next_idx = 0;

void smtd_press_following_key(smtd_state *state, bool release) {
    state->freeze            = true;
    keyevent_t  event_press  = MAKE_KEYEVENT(state->following_key.row, state->following_key.col, true);
    keyrecord_t record_press = {.event = event_press};
    process_record(&record_press);
    if (release) {
        keyevent_t  event_release  = MAKE_KEYEVENT(state->following_key.row, state->following_key.col, false);
        keyrecord_t record_release = {.event = event_release};
        SMTD_SIMULTANEOUS_PRESSES_DELAY;
        process_record(&record_release);
    }
    state->freeze = false;
}

uint32_t timeout_reset_seq(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state   = (smtd_state *)cb_arg;
    state->sequence_len = 0;
    return 0;
}

uint32_t timeout_touch(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *)cb_arg;
    on_smtd_action(state->macro_keycode, SMTD_ACTION_HOLD, state->sequence_len);
    smtd_next_stage(state, SMTD_STAGE_HOLD);
    return 0;
}

uint32_t timeout_sequence(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *)cb_arg;
    if (smtd_feature_enabled_or_default(state->macro_keycode, SMTD_FEATURE_AGGREGATE_TAPS)) {
        DO_ACTION_TAP(state);
    }
    state->sequence_len = 0;
    smtd_next_stage(state, SMTD_STAGE_NONE);
    return 0;
}

uint32_t timeout_join(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *)cb_arg;
    on_smtd_action(state->macro_keycode, SMTD_ACTION_HOLD, state->sequence_len);
    smtd_next_stage(state, SMTD_STAGE_HOLD);
    SMTD_SIMULTANEOUS_PRESSES_DELAY;
    smtd_press_following_key(state, false);
    return 0;
}

uint32_t timeout_release(uint32_t trigger_time, void *cb_arg) {
    smtd_state *state = (smtd_state *)cb_arg;
    DO_ACTION_TAP(state);
    SMTD_SIMULTANEOUS_PRESSES_DELAY;
    smtd_press_following_key(state, false);
    state->sequence_len = 0;
    smtd_next_stage(state, SMTD_STAGE_NONE);
    return 0;
}

void smtd_next_stage(smtd_state *state, smtd_stage next_stage) {
    if (state->stage == SMTD_STAGE_NONE) {
        state->tap_mods = get_mods();
        PUSH_STATE(state);
    }
    if (next_stage == SMTD_STAGE_NONE) {
        state->tap_mods = 0;
        REMOVE_STATE(state);
    }

    deferred_token prev_token = state->timeout;
    state->timeout            = INVALID_DEFERRED_TOKEN;
    state->stage              = next_stage;

    switch (state->stage) {
        case SMTD_STAGE_NONE:
            state->following_key = MAKE_KEYPOS(0, 0);
            if (state->sequence_len != 0) {
                state->timeout = defer_exec(get_smtd_timeout_or_default(state->macro_keycode, SMTD_TIMEOUT_SEQUENCE), timeout_reset_seq, state);
            }
            break;
        case SMTD_STAGE_TOUCH:
            state->timeout = defer_exec(get_smtd_timeout_or_default(state->macro_keycode, SMTD_TIMEOUT_TAP), timeout_touch, state);
            break;
        case SMTD_STAGE_SEQUENCE:
            state->timeout = defer_exec(get_smtd_timeout_or_default(state->macro_keycode, SMTD_TIMEOUT_SEQUENCE), timeout_sequence, state);
            break;
        case SMTD_STAGE_HOLD:
            break;
        case SMTD_STAGE_FOLLOWING_TOUCH:
            state->timeout = defer_exec(get_smtd_timeout_or_default(state->macro_keycode, SMTD_TIMEOUT_FOLLOWING_TAP), timeout_join, state);
            break;
        case SMTD_STAGE_RELEASE:
            state->timeout = defer_exec(get_smtd_timeout_or_default(state->macro_keycode, SMTD_TIMEOUT_RELEASE), timeout_release, state);
            break;
    }

    cancel_deferred_exec(prev_token);
}

bool process_smtd_state(uint16_t keycode, keyrecord_t *record, smtd_state *state) {
    if (state->freeze) {
        return true;
    }

    switch (state->stage) {
        case SMTD_STAGE_NONE:
            if (keycode == state->macro_keycode && record->event.pressed) {
                on_smtd_action(state->macro_keycode, SMTD_ACTION_TOUCH, state->sequence_len);
                smtd_next_stage(state, SMTD_STAGE_TOUCH);
                return false;
            }
            return true;

        case SMTD_STAGE_TOUCH:
            if (keycode == state->macro_keycode && !record->event.pressed) {
                smtd_next_stage(state, SMTD_STAGE_SEQUENCE);
                if (!smtd_feature_enabled_or_default(state->macro_keycode, SMTD_FEATURE_AGGREGATE_TAPS)) {
                    DO_ACTION_TAP(state);
                }
                return false;
            }
            if (record->event.pressed) {
                state->following_key = record->event.key;
                smtd_next_stage(state, SMTD_STAGE_FOLLOWING_TOUCH);
                return false;
            }
            return true;

        case SMTD_STAGE_SEQUENCE:
            if (keycode == state->macro_keycode && record->event.pressed) {
                state->sequence_len++;
                on_smtd_action(state->macro_keycode, SMTD_ACTION_TOUCH, state->sequence_len);
                smtd_next_stage(state, SMTD_STAGE_TOUCH);
                return false;
            }
            if (record->event.pressed) {
                if (smtd_feature_enabled_or_default(state->macro_keycode, SMTD_FEATURE_AGGREGATE_TAPS)) {
                    DO_ACTION_TAP(state);
                }
                state->sequence_len = 0;
                smtd_next_stage(state, SMTD_STAGE_NONE);
                return true;
            }
            return true;

        case SMTD_STAGE_FOLLOWING_TOUCH:
            if (keycode == state->macro_keycode && !record->event.pressed) {
                smtd_next_stage(state, SMTD_STAGE_RELEASE);
                return false;
            }
            if (state->following_key.row == record->event.key.row && state->following_key.col == record->event.key.col && !record->event.pressed) {
                on_smtd_action(state->macro_keycode, SMTD_ACTION_HOLD, state->sequence_len);
                smtd_next_stage(state, SMTD_STAGE_HOLD);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                smtd_press_following_key(state, true);
                return false;
            }
            if (keycode != state->macro_keycode && (state->following_key.row != record->event.key.row || state->following_key.col != record->event.key.col) && record->event.pressed) {
                on_smtd_action(state->macro_keycode, SMTD_ACTION_HOLD, state->sequence_len);
                smtd_next_stage(state, SMTD_STAGE_HOLD);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                smtd_press_following_key(state, false);
                return true;
            }
            return true;

        case SMTD_STAGE_HOLD:
            if (keycode == state->macro_keycode && !record->event.pressed) {
                on_smtd_action(state->macro_keycode, SMTD_ACTION_RELEASE, state->sequence_len);
                state->sequence_len = 0;
                smtd_next_stage(state, SMTD_STAGE_NONE);
                return false;
            }
            return true;

        case SMTD_STAGE_RELEASE:
            if (keycode == state->macro_keycode && record->event.pressed) {
                DO_ACTION_TAP(state);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                smtd_press_following_key(state, false);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                on_smtd_action(state->macro_keycode, SMTD_ACTION_TOUCH, state->sequence_len);
                state->sequence_len = 0;
                smtd_next_stage(state, SMTD_STAGE_TOUCH);
                return false;
            }
            if (state->following_key.row == record->event.key.row && state->following_key.col == record->event.key.col && !record->event.pressed) {
                on_smtd_action(state->macro_keycode, SMTD_ACTION_HOLD, state->sequence_len);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                smtd_press_following_key(state, true);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                on_smtd_action(state->macro_keycode, SMTD_ACTION_RELEASE, state->sequence_len);
                state->sequence_len = 0;
                smtd_next_stage(state, SMTD_STAGE_NONE);
                return true;
            }
            if (keycode != state->macro_keycode && (state->following_key.row != record->event.key.row || state->following_key.col != record->event.key.col) && record->event.pressed) {
                DO_ACTION_TAP(state);
                SMTD_SIMULTANEOUS_PRESSES_DELAY;
                smtd_press_following_key(state, false);
                state->sequence_len = 0;
                smtd_next_stage(state, SMTD_STAGE_NONE);
                return true;
            }
            return true;
    }

    return true;
}

/* ************************************* *
 *      ENTRY POINT IMPLEMENTATION       *
 * ************************************* */

bool process_smtd(uint16_t keycode, keyrecord_t *record) {
    for (uint8_t i = 0; i < smtd_active_states_next_idx; i++) {
        smtd_state *state = smtd_active_states[i];
        if (!process_smtd_state(keycode, record, state)) {
            return false;
        }
    }

    for (uint8_t i = 0; i < smtd_states_size; i++) {
        smtd_state *state = &smtd_states[i];
        if (state->stage == SMTD_STAGE_NONE && !process_smtd_state(keycode, record, state)) {
            return false;
        }
    }

    return true;
}
