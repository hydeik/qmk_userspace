#pragma once

#include QMK_KEYBOARD_H

/****************************************************************************
 * Layer numbers
 ****************************************************************************/
enum layer_t {
    _BASE,
    _EXT,
    _SYM,
    _NUM,
    _FUN,
};

/****************************************************************************
 * Custom keycodes
 ****************************************************************************/
enum custom_keycode_t {
    LLOCK = QK_USER,
    UPDIR,
    /* --- Macros invoked through the Magic key. */
    M_DOCSTR,
    M_EQEQ,
    M_INCLUDE,
    M_MKGRVS,
    M_UPDIR,
    M_NOOP,
#ifdef SMTD_ENABLE
    /* --- Macro keycode handled by SMTD */
    SMTD_KEYCODES_BEGIN,
    /* mod-tap keys */
    EXT_TAB,
    SYM_SPC,
    NUM_ENT,
    FUN_BSPC,
    /* for home-row mods (BASE layer) */
    HM_A,
    HM_S,
    HM_D,
    HM_F,
    HM_J,
    HM_K,
    HM_L,
    HM_SCLN,
    /* for home-row mods (EXT layer) */
    HM_LEFT,
    HM_DOWN,
    HM_RGHT,
    /* for home-row mods (SYM layer) */
    HM_ASTR,
    HM_LPRN,
    HM_RPRN,
    HM_COLN1,
    HM_DQUO,
    HM_LBRC,
    HM_RBRC,
    /* for home-row mods (NUM layer) */
    HM_0,
    HM_1,
    HM_2,
    HM_3,
    HM_QUOT,
    HM_UNDS,
    HM_EQL,
    HM_COLN2,
    SMTD_KEYCODES_END,
#endif  /* SMTD_ENABLE */
};

#ifndef SMTD_ENABLE

/* mod-tap keys */
#define EXT_TAB  LT(_EXT, KC_TAB)
#define SYM_SPC  LT(_SYM, KC_SPC)
#define NUM_ENT  LT(_NUM, KC_ENT)
#define FUN_BSPC LT(_FUN, KC_BSPC)

/* for home-row mods (BASE layer) */
#define HM_A     LCTL_T(KC_A)
#define HM_S     LALT_T(KC_S)
#define HM_D     LGUI_T(KC_D)
#define HM_F     LSFT_T(KC_F)
#define HM_J     RSFT_T(KC_J)
#define HM_K     RGUI_T(KC_K)
#define HM_L     RALT_T(KC_L)
#define HM_SCLN  RCTL_T(KC_SCLN)

/* for home-row mods (EXT layer) */
#define HM_LEFT  RSFT_T(KC_LEFT)
#define HM_DOWN  RGUI_T(KC_DOWN)
#define HM_RGHT  RALT_T(KC_RGHT)

/* for home-row mods (SYM layer) */
#define HM_ASTR   LCTL_T(KC_KP_0)
#define HM_LPRN   LALT_T(KC_KP_0)
#define HM_RPRN   LGUI_T(KC_KP_0)
#define HM_COLN1  LSFT_T(KC_KP_0)
#define HM_DQUO   RSFT_T(KC_KP_1)
#define HM_LBRC   RGUI_T(KC_LBRC)
#define HM_RBRC   RALT_T(KC_RBRC)

/* for home-row mods (NUM layer) */
#define HM_0      LCTL_T(KC_0)
#define HM_1      LALT_T(KC_1)
#define HM_2      LGUI_T(KC_2)
#define HM_3      LSFT_T(KC_3)
#define HM_QUOT   RSFT_T(KC_QUOT)
#define HM_UNDS   RGUI_T(KC_KP_2)
#define HM_EQL    RALT_T(KC_EQL)
#define HM_COLN2  RCTL_T(KC_KP_2)

#endif  /* not def: SMTD_ENABLE */


/****************************************************************************
 * Keymap blocks
 ****************************************************************************/

/* clang-format off */

#define _________________QWERTY_L1_________________       KC_Q,    KC_W,    KC_E,    KC_R,    KC_T
#define _________________QWERTY_L2_________________       HM_A,    HM_S,    HM_D,    HM_F,    KC_G
#define _________________QWERTY_L3_________________       KC_Z,    KC_X,    KC_C,    KC_V,    KC_B

#define _________________QWERTY_R1_________________       KC_Y,    KC_U,    KC_I,    KC_O,    KC_P
#define _________________QWERTY_R2_________________       KC_H,    HM_J,    HM_K,    HM_L,    HM_SCLN
#define _________________QWERTY_R3_________________       KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH

#define _______________NAVIGATION_L1_______________       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
#define _______________NAVIGATION_L2_______________       KC_LCTL, KC_LALT, KC_LGUI, KC_LSFT, CW_TOGG
#define _______________NAVIGATION_L3_______________       KC_ESC,  XXXXXXX, LSFT(KC_TAB), KC_TAB, LLOCK

#define _______________NAVIGATION_R1_______________       KC_PGUP, KC_PGDN, KC_UP,   KC_END,  XXXXXXX
#define _______________NAVIGATION_R2_______________       KC_PGDN, HM_LEFT, HM_DOWN, HM_RGHT, KC_RCTL
#define _______________NAVIGATION_R3_______________       CW_TOGG, QK_REP,  QK_AREP, XXXXXXX, XXXXXXX

#define _________________SYMBOL_L1_________________       XXXXXXX, KC_BSLS, KC_PERC, KC_DLR,  XXXXXXX
#define _________________SYMBOL_L2_________________       HM_ASTR, HM_LPRN, HM_RPRN, HM_COLN1, XXXXXXX
#define _________________SYMBOL_L3_________________       XXXXXXX, KC_GRV,  KC_AT,   KC_AMPR, LLOCK

#define _________________SYMBOL_R1_________________       XXXXXXX, KC_CIRC, KC_LCBR, KC_RCBR, XXXXXXX
#define _________________SYMBOL_R2_________________       XXXXXXX, HM_DQUO, HM_LBRC, HM_RBRC, HM_SCLN
#define _________________SYMBOL_R3_________________       XXXXXXX, KC_HASH, KC_LT,   KC_GT,   XXXXXXX

#define _________________NUMBER_L1_________________       XXXXXXX, KC_4,    KC_5,    KC_6,    XXXXXXX
#define _________________NUMBER_L2_________________       HM_0,    HM_1,    HM_2,    HM_3,    XXXXXXX
#define _________________NUMBER_L3_________________       KC_TILD, KC_7,    KC_8,    KC_9,    KC_DOT

#define _________________NUMBER_R1_________________       XXXXXXX, KC_PIPE, KC_MINS, KC_SLSH, XXXXXXX
#define _________________NUMBER_R2_________________       XXXXXXX, HM_QUOT, HM_UNDS, HM_EQL,  HM_COLN2
#define _________________NUMBER_R3_________________       LLOCK,   KC_PLUS, KC_QUES, KC_EXLM, XXXXXXX

#define ________________FUNCTION_L1________________       KC_LPAD, KC_F4,   KC_F5,   KC_F6,   KC_F10
#define ________________FUNCTION_L2________________       KC_PSCR, KC_F1,   KC_F2,   KC_F3,   KC_F11
#define ________________FUNCTION_L3________________       KC_MCTL, KC_F7,   KC_F8,   KC_F9,   KC_F12

#define ________________FUNCTION_R1________________       KC_MUTE, KC_VOLD, KC_VOLU, KC_BRID, KC_BRIU
#define ________________FUNCTION_R2________________       XXXXXXX, OSM(MOD_RSFT), OSM(MOD_RGUI), OSM(MOD_RALT), OSM(MOD_RCTL)
#define ________________FUNCTION_R3________________       LLOCK,   KC_MSTP, KC_MPRV, KC_MPLY, KC_MNXT

/* clang-format on */

/****************************************************************************
 * Prototypes for keymap specific customized functions.
 ****************************************************************************/
bool process_record_keymap(uint16_t keycode, keyrecord_t *record);
// void matrix_init_keymap(void);
void matrix_scan_keymap(void);
void keyboard_post_init_keymap(void);
// layer_state_t layer_state_set_keymap(layer_state_t state);

