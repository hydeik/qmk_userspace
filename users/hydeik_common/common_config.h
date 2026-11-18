/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Common core: config.h settings that do not depend on the keymap family.
 *
 * Each keymap family includes this file at the top of its config.h. This
 * file must not branch on the keymap family or the firmware source.
 */

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
#define TAPPING_TOGGLE 1

/* Configure One Shot Key */
#define ONESHOT_TIMEOUT 2000

/****************************************************************************
 * Caps Word
 ****************************************************************************/
#ifdef CAPS_WORD_ENABLE
/*
 * Use Left Ctrl + Right Ctrl to activate the Command feature.
 * Holding Shift while Caps Word is active inverts the shift state.
 */
#define IS_COMMAND() (get_mods() == MOD_MASK_CTRL)
#ifndef CAPS_WORD_INVERT_ON_SHIFT
#define CAPS_WORD_INVERT_ON_SHIFT
#endif /* CAPS_WORD_INVERT_ON_SHIFT */
#define DOUBLE_TAP_SHIFT_TURNS_ON_CAPS_WORD
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
