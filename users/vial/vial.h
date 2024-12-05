#pragma once

#include QMK_KEYBOARD_H

/****************************************************************************
 * Layer numbers
 ****************************************************************************/
enum layer_t {
    _BASE,
    _EXT,
    _SYM,
    _FUN,
    _MOD,
};

/****************************************************************************
 * Custom keycodes
 ****************************************************************************/
enum custom_keycode_t {
    LLOCK = QK_KB_0,
    /* Custom oneshot mod layer. */
    OSL_MOD,
    /* Custom oneshot mods implementation. */
    OSM_SFT,
    OSM_CTL,
    OSM_ALT,
    OSM_GUI,

    UPDIR,
    /* --- Macros invoked through the Magic key. */
    M_DOCSTR,
    M_EQEQ,
    M_INCLUDE,
    M_MKGRVS,
    M_UPDIR,
    M_NOOP,
};

/****************************************************************************
 * Keymap blocks
 ****************************************************************************/

/* clang-format off */

#define _________________QWERTY_L1_________________       KC_Q,    KC_W,    KC_E,    KC_R,    KC_T
#define _________________QWERTY_L2_________________       KC_A,    KC_S,    KC_D,    KC_F,    KC_G
#define _________________QWERTY_L3_________________       KC_Z,    KC_X,    KC_C,    KC_V,    KC_B

#define _________________QWERTY_R1_________________       KC_Y,    KC_U,    KC_I,    KC_O,    KC_P
#define _________________QWERTY_R2_________________       KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN
#define _________________QWERTY_R3_________________       KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH

#define _______________NAVIGATION_L1_______________       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
#define _______________NAVIGATION_L2_______________       OSM_CTL, OSM_ALT, OSM_GUI, OSM_SFT, XXXXXXX
#define _______________NAVIGATION_L3_______________       KC_ESC,  XXXXXXX, QK_AREP, QK_REP,  LLOCK

#define _______________NAVIGATION_R1_______________       KC_HOME, KC_PGDN, KC_PGUP, KC_END,  CW_TOGG
#define _______________NAVIGATION_R2_______________       KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_BSPC
#define _______________NAVIGATION_R3_______________       XXXXXXX, LSFT(KC_TAB), KC_TAB, XXXXXXX, KC_DEL

#define _________________SYMBOL_L1_________________       KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC
#define _________________SYMBOL_L2_________________       KC_GRV,  KC_LT,   KC_LPRN, KC_LCBR, KC_LBRC
#define _________________SYMBOL_L3_________________       KC_TILD, KC_GT,   KC_RPRN, KC_RCBR, KC_RBRC

#define _________________SYMBOL_R1_________________       KC_CIRC, KC_AMPR, KC_ASTR, KC_BSLS, KC_QUES
#define _________________SYMBOL_R2_________________       KC_PIPE, KC_QUOT, KC_MINS, KC_EQL,  KC_COLN
#define _________________SYMBOL_R3_________________       LLOCK,   KC_DQUO, KC_UNDS, KC_PLUS, KC_SLSH

#define ________________FUNCTION_L1________________       KC_1,    KC_2,    KC_3,    KC_4,    KC_5
#define ________________FUNCTION_L2________________       OSM_CTL, OSM_ALT, OSM_GUI, OSM_SFT, KC_F11
#define ________________FUNCTION_L3________________       KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5

#define ________________FUNCTION_R1________________       KC_6,    KC_7,    KC_8,    KC_9,    KC_0
#define ________________FUNCTION_R2________________       KC_F12,  OSM_SFT, OSM_GUI, OSM_ALT, OSM_CTL
#define ________________FUNCTION_R3________________       KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10

/* clang-format on */

/****************************************************************************
 * Prototypes for keymap specific customized functions.
 ****************************************************************************/
bool process_record_keymap(uint16_t keycode, keyrecord_t *record);
// void matrix_init_keymap(void);
void matrix_scan_keymap(void);
void keyboard_post_init_keymap(void);
// layer_state_t layer_state_set_keymap(layer_state_t state);

