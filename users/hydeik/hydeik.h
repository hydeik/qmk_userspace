#pragma once

#include QMK_KEYBOARD_H

/****************************************************************************
 * Layer numbers
 ****************************************************************************/
enum layer_t {
    _BASE,
    _SYM,
    _NUM,
    _NAV,
    _MOU,
    _FUN,
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
    SYM_TAB = LT(_SYM, KC_TAB),
    NAV_SPC = LT(_NAV, KC_SPC),
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
    // /* for home-row mods (SYM layer) */
    // HM_ASTR = LCTL_T(KC_KP_0),
    // HM_LPRN = LALT_T(KC_KP_0),
    // HM_RPRN = LGUI_T(KC_KP_0),
    // HM_COLN = LSFT_T(KC_KP_0),
    // HM_DQUO = RSFT_T(KC_KP_1),
    // HM_LBRC = RGUI_T(KC_LBRC),
    // HM_RBRC = RALT_T(KC_RBRC),
    // /* for home-row mods (NUM layer) */
    // HM_DOT = LCTL_T(KC_DOT),
    // HM_1 = LALT_T(KC_1),
    // HM_2 = LGUI_T(KC_2),
    // HM_3 = LSFT_T(KC_3),
    // HM_QUOT = RSFT_T(KC_QUOT),
    // HM_UNDS = RGUI_T(KC_KP_2),
    // HM_EQL = RALT_T(KC_EQL),
    /* Clipboard */
#if defined (HYDEIK_CLIPBOARD_FUN)
    M_REDO = KC_AGAIN,
    M_UNDO = KC_UNDO,
    M_CUT = KC_CUT,
    M_COPY = KC_COPY,
    M_PSTE = KC_PSTE,
#elif defined (HYDEIK_CLIPBOARD_MAC)
    M_REDO = SCMD(KC_Z),
    M_UNDO = LCMD(KC_Z),
    M_CUT = LCMD(KC_X),
    M_COPY = LCMD(KC_C),
    M_PSTE = LCMD(KC_V),
#elif defined (HYDEIK_CLIPBOARD_WIN)
    M_REDO = C(KC_Y),
    M_UNDO = C(KC_Z),
    M_CUT = C(KC_X),
    M_COPY = C(KC_C),
    M_PSTE = C(KC_V),
#else
    M_REDO = KC_AGAIN,
    M_UNDO = KC_UNDO,
    M_CUT = S(KC_DEL),
    M_COPY = C(KC_INS),
    M_PSTE = S(KC_INS),
#endif
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
 *                               | TAB/SYM | SPC/NAV |   | ENT/SFT | BSPC/NUM|
 *                               +---------+---------+   +---------+---------+
 */

#define BASE_LAYER \
    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    \
    KC_A,    HM_S,    HM_D,    HM_F,    KC_G,    KC_H,    HM_J,    HM_K,    HM_L,    KC_SCLN, \
    KC_Z,    KC_X,    KC_C,    HM_V,    KC_B,    KC_N,    HM_M,    KC_COMM, KC_DOT,  KC_SLSH, \
                               SYM_TAB, NAV_SPC, SFT_ENT, NUM_BSPC

/*
 * symbol layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | \       | %       | $       |         |   |         | ^       | {       | }       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | *       | (       | )       | :       | DEL     |   | BSPC    | "       | [       | ]       | ;       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | `       | @       | &       | LLCK    |   |         | #       | <       | >       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | _SYM_   | SPC     |   | ENT     | ESC     |
 *                               +---------+---------+   +---------+---------+
 */

#define SYM_LAYER \
    XXXXXXX, KC_BSLS, KC_PERC, KC_DLR,  XXXXXXX, XXXXXXX, KC_CIRC, KC_LCBR, KC_RCBR, XXXXXXX, \
    KC_ASTR, KC_LPRN, KC_RPRN, KC_COLN, KC_DEL,  KC_BSPC, KC_DQUO, KC_LBRC, KC_RBRC, KC_SCLN, \
    XXXXXXX, KC_GRV,  KC_AT,   KC_AMPR, QK_LLCK, XXXXXXX, KC_HASH, KC_LT,   KC_GT,   XXXXXXX, \
                               _______, KC_SPC,  KC_ENT,  KC_ESC


/*
 * Number layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | 9       | 8       | 7       |         |   |         | |       | -       | /       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | .       | 3       | 2       | 1       | DEL     |   | BSPC    | '       | _       | =       | *       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | ~       | 6       | 5       | 4       |         |   | LLCK    | +       | ?       | !       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | 0       | SPC     |   | ENT     | _NUM_   |
 *                               +---------+---------+   +---------+---------+
 */

#define NUM_LAYER \
    XXXXXXX, KC_9,    KC_8,    KC_7,    XXXXXXX, XXXXXXX, KC_PIPE, KC_MINS, KC_SLSH, XXXXXXX, \
    KC_DOT,  KC_3,    KC_2,    KC_1,    KC_DEL,  KC_BSPC, KC_QUOT, KC_UNDS, KC_EQL,  KC_ASTR, \
    KC_TILD, KC_6,    KC_5,    KC_4,    XXXXXXX, QK_LLCK, KC_PLUS, KC_QUES, KC_EXLM, XXXXXXX, \
                               KC_0,    KC_SPC,  KC_ENT,  _______

/*
 * Navigation layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   | REDO    | PASTE   | COPY    | CUT     | UNDO    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | ALT*    | CTL*    | SFT*    | -MOU-   |   | LEFT    | DOWN    | UP      | RGHT    | CWORD   |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         | GUI*    | LLCK    |   | HOME    | PGDN    | PGUP    | END     | INS     |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | _NAV_   |   | -FUN-   | ESC     |
 *                               +---------+---------+   +---------+---------+
 */

#define NAV_LAYER \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,  M_REDO,  M_PSTE,  M_COPY,  M_CUT,   M_UNDO,  \
    XXXXXXX, OS_LALT, OS_LCTL, OS_LSFT, MO(_MOU), KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, CW_TOGG, \
    XXXXXXX, XXXXXXX, XXXXXXX, OS_LGUI, QK_LLCK,  KC_HOME, KC_PGDN, KC_PGUP, KC_END,  KC_INS,  \
                               _______, _______,  MO(_FUN), KC_ESC

/*
 * Mouse layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   | REDO    | PASTE   | COPY    | CUT     | UNDO    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | ALT*    | CTL*    | SFT*    | _MOU_   |   | MS_LEFT | MS_DOWN | MS_UP   | MS_RGHT | BTN3    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         | GUI*    | LLCK    |   | MS_WHLL | MS_WHLD | MS_WHLU | MS_WHLR |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | _NAV_   |   | BTN1    | BTN2    |
 *                               +---------+---------+   +---------+---------+
 */

#define MOUSE_LAYER \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, M_REDO,  M_PSTE,  M_COPY,  M_CUT,   M_UNDO,  \
    XXXXXXX, OS_LALT, OS_LCTL, OS_LSFT, _______, MS_LEFT, MS_DOWN, MS_UP,   MS_RGHT, MS_BTN3, \
    XXXXXXX, XXXXXXX, XXXXXXX, OS_LGUI, QK_LLCK, MS_WHLL, MS_WHLD, MS_WHLU, MS_WHLR, XXXXXXX, \
                               _______, _______, MS_BTN1, MS_BTN2

/*
 * Function layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | BOOT    | F9      | F8      | F7      | F10     |   | BRIU    | MPRV    | MPLY    | MNXT    | VOLU     |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | F3      | F2      | F1      | F11     |   | BRID    | SFT*    | CTL*    | ALT*    | VOLD    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | F6      | F5      | F4      | F12     |   | LLCK    | GUI*    |         |         | MUTE    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | _FUN_   |   | _FUN_   |         |
 *                               +---------+---------+   +---------+---------+
 */

#define FUN_LAYER \
    QK_BOOT, KC_F9,   KC_F8,   KC_F7,   KC_F10,  KC_BRIU, KC_MPRV, KC_MPLY, KC_MNXT, KC_VOLU, \
    XXXXXXX, KC_F3,   KC_F2,   KC_F1,   KC_F11,  KC_BRID, OS_RSFT, OS_RCTL, OS_RALT, KC_VOLD, \
    XXXXXXX, KC_F6,   KC_F5,   KC_F4,   KC_F12,  QK_LLCK, OS_RGUI, XXXXXXX, XXXXXXX, KC_MUTE, \
                               _______, _______, _______, _______

/* clang-format on */

/****************************************************************************
 * Prototypes for keymap specific customized functions.
 ****************************************************************************/
bool process_record_keymap(uint16_t keycode, keyrecord_t *record);
// void matrix_init_keymap(void);
void matrix_scan_keymap(void);
void keyboard_post_init_keymap(void);
layer_state_t layer_state_set_keymap(layer_state_t state);

