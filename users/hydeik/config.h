/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Settings of the hydeik keymap family, for every keymap of the family.
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
#define TAPPING_TERM 200
/*
 * A holding key or a smart mod layer key is a hold as soon as another key is
 * pressed. This is the tap-hold setting of a keymap without Vial. A keymap
 * with Vial ignores it in favour of its own per-key setting, which is a Vial
 * setting; features/firmware_default.c sets its default to the same value.
 */
#define HOLD_ON_OTHER_KEY_PRESS

/* Configure One Shot Key */
#define ONESHOT_TIMEOUT 2000

/* Configure Combo */
#define COMBO_TERM 30
#define EXTRA_SHORT_COMBOS
#define COMBO_STRICT_TIMER
#define COMBO_MUST_TAP_PER_COMBO
#define COMBO_SHOULD_TRIGGER

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

/****************************************************************************
 * RGB Matrix and the indicators
 *
 * These settings are the same for every keymap of the family. They take
 * effect only in a keymap that switches RGB Matrix on in its rules.mk.
 ****************************************************************************/
/*
 * The keyboard starts with a black background: RGB Matrix on, so that the
 * indicators are drawn, with a solid colour at brightness 0. A default is
 * written only when the EEPROM is initialised.
 */
#define RGB_MATRIX_DEFAULT_ON true
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_SOLID_COLOR
#define RGB_MATRIX_DEFAULT_VAL 0

/*
 * Four effects that the keyboard enables are taken out of the build, because
 * an indicator cannot be drawn right over them. The first three redraw only
 * one LED now and then, so the colour of an indicator would stay behind
 * after the indicator has gone. The last hides LEDs from the hook that draws
 * the indicators.
 */
#undef ENABLE_RGB_MATRIX_RAINDROPS
#undef ENABLE_RGB_MATRIX_JELLYBEAN_RAINDROPS
#undef ENABLE_RGB_MATRIX_PIXEL_RAIN
#undef ENABLE_RGB_MATRIX_DIGITAL_RAIN

/*
 * `SPLIT_TRANSACTION_IDS_USER`, the split transaction that carries the view
 * of the indicators to the other half, is not set here but in rules.mk, for
 * a keymap with RGB Matrix only: set here, it would add the code of the
 * custom split transactions to a split keyboard without LEDs as well.
 */
