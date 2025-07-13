#pragma once

/* Optimization to save memory */
#define NO_ACTION_MACRO
#define NO_ACTION_FUNCTION
#define NO_MUSIC_MODE
#undef LOCKING_SUPPORT_ENABLE
#undef LOCKING_RESYNC_ENABLE

/****************************************************************************
 * Tap & Hold
 ****************************************************************************/
/*
 * Unfortunately, some applications drop or misorder fast key events.
 * This is a partial fix to slow down the rate at which macros are sent.
 */
#define TAP_CODE_DELAY 5
/* Configure Tap & Hold */
#define TAPPING_TERM 250
#define TAPPING_TOGGLE 1

#define TAPPING_TERM_PER_KEY
#define FLOW_TAP_TERM 125
#define CHORDAL_HOLD
#define PERMISSIVE_HOLD
#define HOLD_ON_OTHER_KEY_PRESS_PER_KEY
#define QUICK_TAP_TERM_PER_KEY

/* Configure One Shot Key */
#define ONESHOT_TIMEOUT 2000

/* Configure Leader key */
#define LEADER_TIMEOUT 500
#define LEADER_PER_KEY_TIMING

/* Configure Combo */
#define COMBO_TERM 30
#define EXTRA_SHORT_COMBOS
#define COMBO_STRICT_TIMER
#define COMBO_MUST_TAP_PER_COMBO
#define COMBO_SHOULD_TRIGGER

/* Configure Defferred execution */
#define MAX_DEFERRED_EXECUTORS 10

/****************************************************************************
 * Caps Word
 ****************************************************************************/
#ifdef CAPS_WORD_ENABLE
/*
 * Use Left Ctrl + Right Ctrl to activate the Command feature.
 * Holding Shift while Caps Word is active inverts the shift state.
 */
#define IS_COMMAND() (get_mods() == MOD_MASK_CTRL)
#define CAPS_WORD_INVERT_ON_SHIFT
/* When idle, turn off Caps Word after 5 seconds. */
#define CAPS_WORD_IDLE_TIMEOUT 5000
#endif  /* CAPS_WORD_ENABLE */

/****************************************************************************
 * Layer Lock
 ****************************************************************************/
#ifdef LAYER_LOCK_ENABLE
/* When idle, turn off Layer Lock after 60 seconds. */
#define LAYER_LOCK_IDLE_TIMEOUT 60000
#endif  /* LAYER_LOCK_ENABLE */
