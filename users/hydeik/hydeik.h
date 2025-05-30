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
    _NAV,
};

/****************************************************************************
 * Custom keycodes
 ****************************************************************************/
enum custom_keycode_t {
    UPDIR = QK_USER,
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
    NAV_TAB,
    SYM_SPC,
    NUM_ENT,
    SFT_BSPC,
    /* for home-row mods (BASE layer) */
    HM_A,
    HM_S,
    HM_D,
    HM_F,
    HM_J,
    HM_K,
    HM_L,
    HM_SCLN,
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
    HM_4,
    HM_5,
    HM_6,
    HM_QUOT,
    HM_UNDS,
    HM_EQL,
    HM_COLN2,
    SMTD_KEYCODES_END,
#endif  /* SMTD_ENABLE */
};

#ifndef SMTD_ENABLE

/* mod-tap keys */
#define NAV_TAB   LT(_NAV, KC_TAB)
#define SYM_SPC   LT(_SYM, KC_SPC)
#define NUM_ENT   LT(_NUM, KC_ENT)
#define SFT_BSPC  LSFT_T(KC_BSPC)

/* for home-row mods (BASE layer) */
#define HM_A      LCTL_T(KC_A)
#define HM_S      LALT_T(KC_S)
#define HM_D      LGUI_T(KC_D)
#define HM_F      LSFT_T(KC_F)
#define HM_J      RSFT_T(KC_J)
#define HM_K      RGUI_T(KC_K)
#define HM_L      RALT_T(KC_L)
#define HM_SCLN   RCTL_T(KC_SCLN)

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
#define HM_4      LALT_T(KC_4)
#define HM_5      LGUI_T(KC_5)
#define HM_6      LSFT_T(KC_6)
#define HM_QUOT   RSFT_T(KC_QUOT)
#define HM_UNDS   RGUI_T(KC_KP_2)
#define HM_EQL    RALT_T(KC_EQL)
#define HM_COLN2  RCTL_T(KC_KP_2)

#endif  /* not def: SMTD_ENABLE */


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
 * | A/LCTL  | S/LALT  | D/GUI   | F/LSFT  | G       |   | H       | J/RSFT  | K/RGUI  | L/RALT  | ;/RCTL  |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | Z       | X       | C       | V       | B       |   | N       | M       | ,       | .       | /       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | TAB/NAV | SPC/SYM |   | ENT/NUM |BSPC/LSFT|
 *                               +---------+---------+   +---------+---------+
 */

#define _________________QWERTY_L1_________________      KC_Q,    KC_W,    KC_E,    KC_R,    KC_T
#define _________________QWERTY_L2_________________      HM_A,    HM_S,    HM_D,    HM_F,    KC_G
#define _________________QWERTY_L3_________________      KC_Z,    KC_X,    KC_C,    KC_V,    KC_B

#define _________________QWERTY_R1_________________      KC_Y,    KC_U,    KC_I,    KC_O,    KC_P
#define _________________QWERTY_R2_________________      KC_H,    HM_J,    HM_K,    HM_L,    HM_SCLN
#define _________________QWERTY_R3_________________      KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH

#define ___QWERTY_L4___                                  NAV_TAB, SYM_SPC
#define ___QWERTY_R4___                                  NUM_ENT, SFT_BSPC

/*
 * Symbol layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | \       | %       | $       |         |   |         | ^       | {       | }       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | * /LCTL | (/LALT  | )/LGUI  | :/LSFT  | CWord   |   |         | "/RSFT  | [/RGUI  | ]/RALT  | ;/RCTL  |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | `       | @       | &       | LLCK    |   |         | #       | <       | >       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | _SYM_   |   | -FUN-   |         |
 *                               +---------+---------+   +---------+---------+
 */

#define _________________SYMBOL_L1_________________       XXXXXXX, KC_BSLS, KC_PERC, KC_DLR,  XXXXXXX
#define _________________SYMBOL_L2_________________       HM_ASTR, HM_LPRN, HM_RPRN, HM_COLN1, CW_TOGG
#define _________________SYMBOL_L3_________________       XXXXXXX, KC_GRV,  KC_AT,   KC_AMPR, QK_LLCK

#define _________________SYMBOL_R1_________________       XXXXXXX, KC_CIRC, KC_LCBR, KC_RCBR, XXXXXXX
#define _________________SYMBOL_R2_________________       XXXXXXX, HM_DQUO, HM_LBRC, HM_RBRC, HM_SCLN
#define _________________SYMBOL_R3_________________       XXXXXXX, KC_HASH, KC_LT,   KC_GT,   XXXXXXX

#define ___SYMBOL_L4___                                   _______, _______
#define ___SYMBOL_R4___                                   _______, _______

/*
 * Number layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | 7       | 8       | 9       |         |   |         | |       | -       | /       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | 0/LCTL  | 4/LALT  | 5/LGUI  | 6/LSFT  |         |   |         | '/RSFT  | _/RGUI  | =/RALT  | :/RCTL  |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | ~       | 1       | 2       | 3       |         |   | LLCK    | +       | ?       | !       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | .       | -FUN-   |   | _NUM_   |         |
 *                               +---------+---------+   +---------+---------+
 */


#define _________________NUMBER_L1_________________       XXXXXXX, KC_7,    KC_8,    KC_9,    XXXXXXX
#define _________________NUMBER_L2_________________       HM_0,    HM_4,    HM_5,    HM_6,    KC_DOT
#define _________________NUMBER_L3_________________       KC_TILD, KC_1,    KC_2,    KC_3,    XXXXXXX

#define _________________NUMBER_R1_________________       XXXXXXX, KC_PIPE, KC_MINS, KC_SLSH, XXXXXXX
#define _________________NUMBER_R2_________________       XXXXXXX, HM_QUOT, HM_UNDS, HM_EQL,  HM_COLN2
#define _________________NUMBER_R3_________________       QK_LLCK, KC_PLUS, KC_QUES, KC_EXLM, XXXXXXX

#define ___NUMBER_L4___                                   KC_DOT,  _______
#define ___NUMBER_R4___                                   _______, _______

/*
 * Function layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | LPAD    | F6      | F5      | F4      | F10     |   | MUTE    | VOLD    | VOLU    | BRID    | BRIU    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | PSCR    | F3      | F2      | F1      | F11     |   |         | RSFT    | RGUI    | RALT    | RCTL    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | MCTL    | F9      | F8      | F7      | F12     |   | LLCK    | MSTP    | MPRV    | MPLY    | MNXT    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | _FUN_   |   | _FUN_   |         |
 *                               +---------+---------+   +---------+---------+
 */

#define ________________FUNCTION_L1________________       KC_LPAD, KC_F6,   KC_F5,   KC_F4,   KC_F10
#define ________________FUNCTION_L2________________       KC_PSCR, KC_F3,   KC_F2,   KC_F1,   KC_F11
#define ________________FUNCTION_L3________________       KC_MCTL, KC_F9,   KC_F8,   KC_F7,   KC_F12

#define ________________FUNCTION_R1________________       KC_MUTE, KC_VOLD, KC_VOLU, KC_BRID, KC_BRIU
#define ________________FUNCTION_R2________________       XXXXXXX, OSM(MOD_RSFT), OSM(MOD_RGUI), OSM(MOD_RALT), OSM(MOD_RCTL)
#define ________________FUNCTION_R3________________       QK_LLCK, KC_MSTP, KC_MPRV, KC_MPLY, KC_MNXT

#define __FUNCTION_L4__                                   _______, _______
#define __FUNCTION_R4__                                   _______, _______

/*
 * Navigation layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   |         | HOME    | UP      | END     | PGUP    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | LCTL    | LALT    | LGUI    | LSFT    |         |   |         | LEFT    | DOWN    | RGHT    | PGDN    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         | LLCK    |   |         | TAB     | REP     | AREP    |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         |         |   |         |         |
 *                               +---------+---------+   +---------+---------+
 */

#define _______________NAVIGATION_L1_______________      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
#define _______________NAVIGATION_L2_______________      OSM(MOD_LCTL), OSM(MOD_LALT), OSM(MOD_LGUI), OSM(MOD_LSFT), CW_TOGG
#define _______________NAVIGATION_L3_______________      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, QK_LLCK

#define _______________NAVIGATION_R1_______________      XXXXXXX, KC_HOME, KC_UP,   KC_END,  KC_PGUP
#define _______________NAVIGATION_R2_______________      XXXXXXX, KC_LEFT, KC_DOWN, KC_RGHT, KC_PGDN
#define _______________NAVIGATION_R3_______________      XXXXXXX, KC_TAB,  QK_REP,  QK_AREP, XXXXXXX

#define _NAVIGATION_L4_                                  _______, KC_SPC
#define _NAVIGATION_R4_                                  KC_ESC,  KC_DEL

/* clang-format on */

/****************************************************************************
 * Prototypes for keymap specific customized functions.
 ****************************************************************************/
bool process_record_keymap(uint16_t keycode, keyrecord_t *record);
// void matrix_init_keymap(void);
void matrix_scan_keymap(void);
void keyboard_post_init_keymap(void);
// layer_state_t layer_state_set_keymap(layer_state_t state);

