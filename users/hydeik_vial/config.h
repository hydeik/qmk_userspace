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
#define TAPPING_TERM 200
#define TAPPING_TOGGLE 1

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

/* Disable RGB effects */
#undef RGBLIGHT_ANIMATIONS
#undef RGBLIGHT_LAYERS
#undef RGBLIGHT_EFFECT_BREATHING
#undef RGBLIGHT_EFFECT_RAINBOW_MOOD
#undef RGBLIGHT_EFFECT_RAINBOW_SWIRL
#undef RGBLIGHT_EFFECT_SNAKE
#undef RGBLIGHT_EFFECT_KNIGHT
#undef RGBLIGHT_EFFECT_CHRISTMAS
#undef RGBLIGHT_EFFECT_STATIC_GRADIENT
#undef RGBLIGHT_EFFECT_RGB_TEST
#undef RGBLIGHT_EFFECT_ALTERNATING
#undef RGBLIGHT_EFFECT_TWINKLE

#undef ENABLE_RGB_MATRIX_ALPHAS_MODS
#undef ENABLE_RGB_MATRIX_GRADIENT_UP_DOWN
#undef ENABLE_RGB_MATRIX_GRADIENT_LEFT_RIGHT
#undef ENABLE_RGB_MATRIX_BREATHING
#undef ENABLE_RGB_MATRIX_BAND_SAT
#undef ENABLE_RGB_MATRIX_BAND_VAL
#undef ENABLE_RGB_MATRIX_BAND_PINWHEEL_SAT
#undef ENABLE_RGB_MATRIX_BAND_PINWHEEL_VAL
#undef ENABLE_RGB_MATRIX_BAND_SPIRAL_SAT
#undef ENABLE_RGB_MATRIX_BAND_SPIRAL_VAL
#undef ENABLE_RGB_MATRIX_CYCLE_ALL
#undef ENABLE_RGB_MATRIX_CYCLE_LEFT_RIGHT
#undef ENABLE_RGB_MATRIX_CYCLE_UP_DOWN
#undef ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON
#undef ENABLE_RGB_MATRIX_CYCLE_OUT_IN
#undef ENABLE_RGB_MATRIX_CYCLE_OUT_IN_DUAL
#undef ENABLE_RGB_MATRIX_CYCLE_PINWHEEL
#undef ENABLE_RGB_MATRIX_CYCLE_SPIRAL
#undef ENABLE_RGB_MATRIX_DUAL_BEACON
#undef ENABLE_RGB_MATRIX_RAINBOW_BEACON
#undef ENABLE_RGB_MATRIX_RAINBOW_PINWHEELS
#undef ENABLE_RGB_MATRIX_RAINDROPS
#undef ENABLE_RGB_MATRIX_JELLYBEAN_RAINDROPS
#undef ENABLE_RGB_MATRIX_HUE_BREATHING
#undef ENABLE_RGB_MATRIX_HUE_PENDULUM
#undef ENABLE_RGB_MATRIX_HUE_WAVE
#undef ENABLE_RGB_MATRIX_PIXEL_FRACTAL
#undef ENABLE_RGB_MATRIX_PIXEL_RAIN

#undef ENABLE_RGB_MATRIX_TYPING_HEATMAP
#undef ENABLE_RGB_MATRIX_DIGITAL_RAIN

#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_CROSS
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTICROSS
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_NEXUS
#undef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS
#undef ENABLE_RGB_MATRIX_SPLASH
#undef ENABLE_RGB_MATRIX_MULTISPLASH
#undef ENABLE_RGB_MATRIX_SOLID_SPLASH
#undef ENABLE_RGB_MATRIX_SOLID_MULTISPLASH

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
