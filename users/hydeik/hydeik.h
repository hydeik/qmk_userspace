#pragma once

#include QMK_KEYBOARD_H

/****************************************************************************
 * Layer numbers
 ****************************************************************************/
enum layer_t {
    _BASE,
    _SYM,
    _NUM,
    _FUN,
    _EXT,
};

/****************************************************************************
 * Custom keycodes
 ****************************************************************************/
enum custom_keycodes {
    UPDIR = QK_USER,
    /* --- Macros invoked through the Magic key. */
    M_DOCSTR,
    M_EQEQ,
    M_INCLUDE,
    M_MKGRVS,
    M_UPDIR,
    M_NOOP,
};

enum keycode_aliases {
    /* mod-tap keys */
    SYM_SPC = LT(_SYM, KC_SPC),
    FUN_TAB = LT(_FUN, KC_TAB),
    SFT_ENT = LSFT_T(KC_ENT),
    NUM_BSPC = LT(_NUM, KC_BSPC),
    /* for home-row mods (BASE layer) */
    HM_S = LALT_T(KC_S),
    HM_D = LCTL_T(KC_D),
    HM_F = LSFT_T(KC_F),
    HM_V = LGUI_T(KC_V),
    HM_J = RSFT_T(KC_J),
    HM_K = RCTL_T(KC_K),
    HM_L = RALT_T(KC_L),
    HM_M = RGUI_T(KC_M),
    /* for home-row mods (SYM layer) */
    HM_LPRN = LALT_T(KC_KP_0),
    HM_RPRN = LCTL_T(KC_KP_0),
    HM_COLN = LSFT_T(KC_KP_0),
    HM_AMPR = LGUI_T(KC_KP_1),
    HM_DQUO = RSFT_T(KC_KP_1),
    HM_LBRC = RCTL_T(KC_LBRC),
    HM_RBRC = RALT_T(KC_RBRC),
    HM_HASH = RGUI_T(KC_KP_1),
    /* for home-row mods (NUM layer) */
    HM_1 = LALT_T(KC_1),
    HM_2 = LCTL_T(KC_2),
    HM_3 = LSFT_T(KC_3),
    HM_9 = LGUI_T(KC_9),
    HM_QUOT = RSFT_T(KC_QUOT),
    HM_UNDS = RCTL_T(KC_KP_2),
    HM_EQL = RALT_T(KC_EQL),
    HM_PLUS = RGUI_T(KC_KP_2),
    /* for home-row mods (FUN layer) */
    HM_F3 = LALT_T(KC_F3),
    HM_F2 = LCTL_T(KC_F2),
    HM_F1 = LSFT_T(KC_F1),
    HM_F7 = LGUI_T(KC_F7),
    HM_DOWN = RSFT_T(KC_DOWN),
    HM_UP = RCTL_T(KC_UP),
    HM_RGHT = RALT_T(KC_RGHT),
    HM_ENT = RGUI_T(KC_ENT),
};

/****************************************************************************
 * Keymap blocks
 ****************************************************************************/

/* clang-format off */

/*
 * QWERTY layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | Q       | W       | E       | R       | T       |   | Y       | U       | I       | O       | P       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | A       | S / ALT | D / CTL | F / SFT | G       |   | H       | J / SFT | K / CTL | L / ALT | ;       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | Z       | X       | C       | V / GUI | B       |   | N       | M / GUI | ,       | .       | /       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | SPC/SYM | TAB/FUN |   | ENT/SFT | BSPC/NUM|
 *                               +---------+---------+   +---------+---------+
 */

#define _________________QWERTY_L1_________________      KC_Q,    KC_W,    KC_E,    KC_R,    KC_T
#define _________________QWERTY_L2_________________      KC_A,    HM_S,    HM_D,    HM_F,    KC_G
#define _________________QWERTY_L3_________________      KC_Z,    KC_X,    KC_C,    HM_V,    KC_B

#define _________________QWERTY_R1_________________      KC_Y,    KC_U,    KC_I,    KC_O,    KC_P
#define _________________QWERTY_R2_________________      KC_H,    HM_J,    HM_K,    HM_L,    KC_SCLN
#define _________________QWERTY_R3_________________      KC_N,    HM_M,    KC_COMM, KC_DOT,  KC_SLSH

#define ___QWERTY_L4___                                  SYM_SPC, FUN_TAB
#define ___QWERTY_R4___                                  SFT_ENT, NUM_BSPC

/*
 * Symbol layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | \       | %       | $       |         |   |         | ^       | {       | }       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | *       | ( / ALT | ) / CTL | : / SFT | DEL     |   | BSPC    | " / SFT | [ / CTL | ] / ALT | ;       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | `       | @       | & / GUI | LLCK    |   |         | # / GUI | <       | >       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | _SYM_   | TAB     |   | ENT     | ESC     |
 *                               +---------+---------+   +---------+---------+
 */

#define _________________SYMBOL_L1_________________       XXXXXXX, KC_BSLS, KC_PERC, KC_DLR,  XXXXXXX
#define _________________SYMBOL_L2_________________       KC_ASTR, HM_LPRN, HM_RPRN, HM_COLN, KC_DEL
#define _________________SYMBOL_L3_________________       XXXXXXX, KC_GRV,  KC_AT,   HM_AMPR, QK_LLCK

#define _________________SYMBOL_R1_________________       XXXXXXX, KC_CIRC, KC_LCBR, KC_RCBR, XXXXXXX
#define _________________SYMBOL_R2_________________       KC_BSPC, HM_DQUO, HM_LBRC, HM_RBRC, KC_SCLN
#define _________________SYMBOL_R3_________________       XXXXXXX, HM_HASH, KC_LT,   KC_GT,   XXXXXXX

#define ___SYMBOL_L4___                                   _______, KC_TAB
#define ___SYMBOL_R4___                                   KC_ENT,  KC_ESC

/*
 * Number layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | 4       | 5       | 6       |         |   |         | |       | -       | /       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | .       | 1 / ALT | 2 / CTL | 3 / SFT | DEL     |   | BSPC    | ' / SFT | _ / CTL | = / ALT | *       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | ~       | 7       | 8       | 9 / GUI |         |   | LLCK    | + / GUI | ?       | !       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | 0       | SPC     |   | ENT     | _NUM_   |
 *                               +---------+---------+   +---------+---------+
 */

#define _________________NUMBER_L1_________________       XXXXXXX, KC_4,    KC_5,    KC_6,    XXXXXXX
#define _________________NUMBER_L2_________________       KC_DOT,  HM_1,    HM_2,    HM_3,    KC_DEL
#define _________________NUMBER_L3_________________       KC_TILD, KC_7,    KC_8,    HM_9,    XXXXXXX

#define _________________NUMBER_R1_________________       XXXXXXX, KC_PIPE, KC_MINS, KC_SLSH, XXXXXXX
#define _________________NUMBER_R2_________________       KC_BSPC, HM_QUOT, HM_UNDS, HM_EQL,  KC_ASTR
#define _________________NUMBER_R3_________________       QK_LLCK, HM_PLUS, KC_QUES, KC_EXLM, XXXXXXX

#define ___NUMBER_L4___                                   KC_0,    KC_SPC
#define ___NUMBER_R4___                                   KC_ENT,  _______

/*
 * Function layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | F6      | F5      | F4      | F10     |   | HOME    | PGDN    | PGUP    | END     |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | GUI     | F3/ALT  | F2/CTL  | F1/SFT  | F11     |   | LEFT    | DOWN/SFT| UP/CTL  | RGHT/ALT| BSPC    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | -EXT-   | F9      | F8      | F7/GUI  | F12     |   |         | ENT/GUI | TAB     | REP     | DEL     |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | _FUN_   |   | ENT     | ESC     |
 *                               +---------+---------+   +---------+---------+
 */

#define ________________FUNCTION_L1________________       XXXXXXX, KC_F6,   KC_F5,   KC_F4,   KC_F10
#define ________________FUNCTION_L2________________       KC_LGUI, HM_F3,   HM_F2,   HM_F1,   KC_F11
#define ________________FUNCTION_L3________________       MO(_EXT),KC_F9,   KC_F8,   HM_F7,   KC_F12

#define ________________FUNCTION_R1________________       KC_HOME, KC_PGDN, KC_PGUP, KC_END,  XXXXXXX
#define ________________FUNCTION_R2________________       KC_LEFT, HM_DOWN, HM_UP,   HM_RGHT, KC_BSPC
#define ________________FUNCTION_R3________________       QK_LLCK, HM_ENT,  KC_TAB,  QK_REP,  KC_DEL

#define __FUNCTION_L4__                                   _______, _______
#define __FUNCTION_R4__                                   KC_ENT,  KC_ESC

/*
 * Extras layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | BOOT    |         |         |         |         |   |         | MUTE    | VOLD    | VOLU    | BOOT    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   |         |         | BRID    | BRIU    |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | _EXT_   |         |         |         |         |   |         | MPRV    | MPLY    | MNXT    |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | _EXT_   |   | _EXT_   |         |
 *                               +---------+---------+   +---------+---------+
 */

#define _________________EXTRAS_L1_________________      QK_BOOT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
#define _________________EXTRAS_L2_________________      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
#define _________________EXTRAS_L3_________________      _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX

#define _________________EXTRAS_R1_________________      XXXXXXX, KC_MUTE, KC_VOLD, KC_VOLU, QK_BOOT
#define _________________EXTRAS_R2_________________      XXXXXXX, XXXXXXX, KC_BRID, KC_BRIU, XXXXXXX
#define _________________EXTRAS_R3_________________      XXXXXXX, KC_MPRV, KC_MPLY, KC_MNXT, XXXXXXX

#define ___EXTRAS_L4___                                  _______, _______
#define ___EXTRAS_R4___                                  _______, _______

/* clang-format on */

/****************************************************************************
 * Prototypes for keymap specific customized functions.
 ****************************************************************************/
bool process_record_keymap(uint16_t keycode, keyrecord_t *record);
// void matrix_init_keymap(void);
void matrix_scan_keymap(void);
void keyboard_post_init_keymap(void);
layer_state_t layer_state_set_keymap(layer_state_t state);

