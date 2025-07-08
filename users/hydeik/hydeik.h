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
#ifdef SMTD_ENABLE
enum custom_keycode_t {
    /* --- Macro keycode handled by SMTD */
    SMTD_KEYCODES_BEGIN = QK_USER,
    /* mod-tap keys */
    CKC_TAB,
    CKC_SPC,
    CKC_ENT,
    CKC_ESC,
    /* for home-row mods (BASE layer) */
    CKC_A,
    CKC_S,
    CKC_D,
    CKC_F,
    CKC_J,
    CKC_K,
    CKC_L,
    CKC_SCLN,
    /* for home-row mods (SYM layer) */
    CKC_ASTR,
    CKC_LPRN,
    CKC_RPRN,
    CKC_COLN,
    CKC_DQUO,
    CKC_LBRC,
    CKC_RBRC,
    /* for home-row mods (NUM layer) */
    CKC_DOT,
    CKC_1,
    CKC_2,
    CKC_3,
    CKC_QUOT,
    CKC_UNDS,
    CKC_EQL,
    SMTD_KEYCODES_END,
};

#else  /* NOT SMTD_ENABLE */

/* mod-tap keys */
#define CKC_TAB   LT(_SYM, KC_TAB)
#define CKC_SPC   LT(_NAV, KC_SPC)
#define CKC_ENT   LSFT_T(KC_ENT)
#define CKC_ESC   LT(_NUM, KC_ESC)

/* for home-row mods (BASE layer) */
#define CKC_A      LCTL_T(KC_A)
#define CKC_S      LALT_T(KC_S)
#define CKC_D      LGUI_T(KC_D)
#define CKC_F      LSFT_T(KC_F)
#define CKC_J      RSFT_T(KC_J)
#define CKC_K      RGUI_T(KC_K)
#define CKC_L      RALT_T(KC_L)
#define CKC_SCLN   RCTL_T(KC_SCLN)

/* for home-row mods (SYM layer) */
#define CKC_ASTR   LCTL_T(KC_KP_0)
#define CKC_LPRN   LALT_T(KC_KP_0)
#define CKC_RPRN   LGUI_T(KC_KP_0)
#define CKC_COLN   LSFT_T(KC_KP_0)
#define CKC_DQUO   RSFT_T(KC_KP_1)
#define CKC_LBRC   RGUI_T(KC_LBRC)
#define CKC_RBRC   RALT_T(KC_RBRC)

/* for home-row mods (NUM layer) */
#define CKC_DOT    LCTL_T(KC_DOT)
#define CKC_1      LALT_T(KC_1)
#define CKC_2      LGUI_T(KC_2)
#define CKC_3      LSFT_T(KC_3)
#define CKC_QUOT   RSFT_T(KC_QUOT)
#define CKC_UNDS   RGUI_T(KC_KP_2)
#define CKC_EQL    RALT_T(KC_EQL)

#endif  /* not def: SMTD_ENABLE */

#define OS_LCTL    OSM(MOD_LCTL)
#define OS_LALT    OSM(MOD_LALT)
#define OS_LGUI    OSM(MOD_LGUI)
#define OS_LSFT    OSM(MOD_LSFT)
#define OS_RCTL    OSM(MOD_RCTL)
#define OS_RALT    OSM(MOD_RALT)
#define OS_RGUI    OSM(MOD_RGUI)
#define OS_RSFT    OSM(MOD_RSFT)

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
 * | A / CTL | S / ALT | D / GUI | F / SFT | G       |   | H       | J / SFT | K / GUI | L / ALT | ; / CTL |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | Z       | X       | C       | V       | B       |   | N       | M       | ,       | .       | /       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | TAB/SYM | SPC/NAV |   | ENT/SFT | ESC/NUM |
 *                               +---------+---------+   +---------+---------+
 */

#define _________________QWERTY_L1_________________      KC_Q,     KC_W,     KC_E,     KC_R,     KC_T
#define _________________QWERTY_L2_________________      CKC_A,    CKC_S,    CKC_D,    CKC_F,    KC_G
#define _________________QWERTY_L3_________________      KC_Z,     KC_X,     KC_C,     KC_V,     KC_B

#define _________________QWERTY_R1_________________      KC_Y,     KC_U,     KC_I,     KC_O,     KC_P
#define _________________QWERTY_R2_________________      KC_H,     CKC_J,    CKC_K,    CKC_L,    CKC_SCLN
#define _________________QWERTY_R3_________________      KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH

#define ___QWERTY_L4___                                  CKC_TAB,  CKC_SPC
#define ___QWERTY_R4___                                  CKC_ENT,  CKC_ESC

/*
 * Symbol layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | \       | %       | $       |         |   |         | ^       | {       | }       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | * / CTL | ( / ALT | ) / GUI | : / SFT | DEL     |   | BSPC    | " / SFT | [ / GUI | ] / ALT | ; / CTL |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | -FUN-   | `       | @       | &       | LLCK    |   |         | #       | <       | >       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | _SYM_   | SPC     |   | ENT     | DEL     |
 *                               +---------+---------+   +---------+---------+
 */

#define _________________SYMBOL_L1_________________       XXXXXXX,  KC_BSLS,  KC_PERC,  KC_DLR,   XXXXXXX
#define _________________SYMBOL_L2_________________       CKC_ASTR, CKC_LPRN, CKC_RPRN, CKC_COLN, KC_DEL
#define _________________SYMBOL_L3_________________       MO(_FUN), KC_GRV,   KC_AT,    KC_AMPR,  QK_LLCK

#define _________________SYMBOL_R1_________________       XXXXXXX,  KC_CIRC,  KC_LCBR,  KC_RCBR,  XXXXXXX
#define _________________SYMBOL_R2_________________       KC_BSPC,  CKC_DQUO, CKC_LBRC, CKC_RBRC, CKC_SCLN
#define _________________SYMBOL_R3_________________       XXXXXXX,  KC_HASH,  KC_LT,    KC_GT,    XXXXXXX

#define ___SYMBOL_L4___                                   _______,  KC_SPC
#define ___SYMBOL_R4___                                   KC_ENT,   KC_ESC

/*
 * Number layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | 4       | 5       | 6       |         |   |         | |       | -       | /       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | . / CTL | 1 / ALT | 2 / GUI | 3 / SFT | DEL     |   | BSPC    | ' / SFT | _ / GUI | = / ALT | * / CTL |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | ~       | 7       | 8       | 9       |         |   | LLCK    | +       | ?       | !       | -FUN-   |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | 0       | SPC     |   | ENT     | _NUM_   |
 *                               +---------+---------+   +---------+---------+
 */


#define _________________NUMBER_L1_________________       XXXXXXX,  KC_4,     KC_5,     KC_6,    XXXXXXX
#define _________________NUMBER_L2_________________       CKC_DOT,  CKC_1,    CKC_2,    CKC_3,   KC_DEL
#define _________________NUMBER_L3_________________       KC_TILD,  KC_7,     KC_8,     KC_9,    XXXXXXX

#define _________________NUMBER_R1_________________       XXXXXXX,  KC_PIPE,  KC_MINS,  KC_SLSH,  XXXXXXX
#define _________________NUMBER_R2_________________       KC_BSPC,  CKC_QUOT, CKC_UNDS, CKC_EQL,  CKC_ASTR
#define _________________NUMBER_R3_________________       QK_LLCK,  KC_PLUS,  KC_QUES,  KC_EXLM,  MO(_FUN)

#define ___NUMBER_L4___                                   KC_0,     KC_SPC
#define ___NUMBER_R4___                                   KC_ENT,   _______

/*
 * Function layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | F6      | F5      | F4      | F10     |   | MUTE    | VOLD    | VOLU    | BRID    | BRIU    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | LPAD    | F3      | F2      | F1      | F11     |   |         | RSFT*   | RGUI*   | RALT*   | RCTL*   |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | _FUN_   | F9      | F8      | F7      | F12     |   | LLCK    | MPRV    | MPLY    | MNXT    | _FUN_   |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | BOOT    | _FUN_   |   | _FUN_   | BOOT    |
 *                               +---------+---------+   +---------+---------+
 */

#define ________________FUNCTION_L1________________       KC_LPAD,  KC_F6,    KC_F5,    KC_F4,    KC_F10
#define ________________FUNCTION_L2________________       KC_MCTL,  KC_F3,    KC_F2,    KC_F1,    KC_F11
#define ________________FUNCTION_L3________________       _______,  KC_F9,    KC_F8,    KC_F7,    KC_F12

#define ________________FUNCTION_R1________________       KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_BRID,  KC_BRIU
#define ________________FUNCTION_R2________________       XXXXXXX,  OS_RSFT,  OS_RGUI,  OS_RALT,  OS_RCTL
#define ________________FUNCTION_R3________________       QK_LLCK,  KC_MPRV,  KC_MPLY,  KC_MNXT,  _______

#define __FUNCTION_L4__                                   QK_BOOT,  _______
#define __FUNCTION_R4__                                   _______,  QK_BOOT

/*
 * Navigation layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   | HOME    | PGDN    | PGUP    | END     |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | LCTL*   | LALT*   | LGUI*   | LSFT*   | CWord   |   | LEFT    | DOWN    | UP      | RGHT    | BSPC    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | UNDO    | CUT     | COPY    | PASTE   | LLCK    |   |         | ENT     | TAB     | REP     | DEL     |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | _NAV_   |   | ENT     | ESC     |
 *                               +---------+---------+   +---------+---------+
 */

#define _______________NAVIGATION_L1_______________      XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX
#define _______________NAVIGATION_L2_______________      OS_LCTL,  OS_LALT,  OS_LGUI,  OS_LSFT,  CW_TOGG
#define _______________NAVIGATION_L3_______________      KC_UNDO,  KC_CUT,   KC_COPY,  KC_PASTE, QK_LLCK

#define _______________NAVIGATION_R1_______________      KC_HOME,  KC_PGDN,  KC_PGUP,  KC_END,   XXXXXXX
#define _______________NAVIGATION_R2_______________      KC_LEFT,  KC_DOWN,  KC_UP,    KC_RGHT,  KC_BSPC
#define _______________NAVIGATION_R3_______________      XXXXXXX,  KC_ENT,   KC_TAB,   QK_REP,   KC_DEL

#define _NAVIGATION_L4_                                  KC_TAB,   _______
#define _NAVIGATION_R4_                                  KC_ENT,   KC_ESC

/* clang-format on */

/****************************************************************************
 * Prototypes for keymap specific customized functions.
 ****************************************************************************/
bool process_record_keymap(uint16_t keycode, keyrecord_t *record);
// void matrix_init_keymap(void);
void matrix_scan_keymap(void);
void keyboard_post_init_keymap(void);
// layer_state_t layer_state_set_keymap(layer_state_t state);

