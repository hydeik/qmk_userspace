#pragma once

#include QMK_KEYBOARD_H

#include "users/hydeik_common/layers.h"

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
    OSM_SFT = OSM(MOD_LSFT),
    NUM_ENT = LT(_NUM, KC_BSPC),
    /*
     * Trigger keys of the target layers MOU and FUN. `MO()` of a target layer
     * is no momentary layer key: `target_layer` of the common core intercepts
     * it. It switches its target layer on only while the holding key of that
     * layer is held (NAV_SPC for MOU, NUM_ENT for FUN), and does nothing
     * alone. See the table of entries in hydeik.c.
     */
    TRG_MOU = MO(_MOU),
    TRG_FUN = MO(_FUN),
    /* for home-row mods (BASE layer) */
    HM_A = LCTL_T(KC_A),
    HM_S = LALT_T(KC_S),
    HM_D = LGUI_T(KC_D),
    HM_F = LSFT_T(KC_F),
    HM_J = RSFT_T(KC_J),
    HM_K = RGUI_T(KC_K),
    HM_L = RALT_T(KC_L),
    HM_SCLN = RCTL_T(KC_SCLN),
    /* for home-row mods (SYM layer) */
    HM_ASTR = LCTL_T(KC_KP_0),
    HM_LPRN = LALT_T(KC_KP_0),
    HM_RPRN = LGUI_T(KC_KP_0),
    HM_COLN = LSFT_T(KC_KP_0),
    HM_DQUO = RSFT_T(KC_KP_0),
    HM_LBRC = RGUI_T(KC_LBRC),
    HM_RBRC = RALT_T(KC_RBRC),
    /* for home-row mods (NUM layer) */
    HM_0 = LCTL_T(KC_0),
    HM_1 = LALT_T(KC_1),
    HM_2 = LGUI_T(KC_2),
    HM_3 = LSFT_T(KC_3),
    HM_QUOT = RSFT_T(KC_QUOT),
    HM_UNDS = RGUI_T(KC_KP_1),
    HM_EQL = RALT_T(KC_EQL),
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

/*
 * Hooks for the common core's layer grids: home-row keys are the HM_* mod-taps
 * defined above, and held modifiers are the plain modifier keys.
 */
#define HOME_ROW_MOD(name) HM_##name
#define HELD_MOD(name) KC_##name

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
 *                               | TAB/SYM | SPC/NAV |   | SFT*    | ENT/NUM |
 *                               +---------+---------+   +---------+---------+
 */

#define BASE_LAYER \
    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    \
    HM_A,    HM_S,    HM_D,    HM_F,    KC_G,    KC_H,    HM_J,    HM_K,    HM_L,    HM_SCLN, \
    KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, \
                               SYM_TAB, NAV_SPC, OSM_SFT, NUM_ENT

/*
 * symbol layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | \       | %       | $       |         |   |         | ^       | {       | }       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | * / CTL | ( / ALT | ) / GUI | : / SFT | DEL     |   | BSPC    | " / SFT | [ / GUI | ] / ALT | ; / CTL |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | `       | @       | &       | LLCK    |   |         | #       | <       | >       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | _SYM_   | SPC     |   | ESC     |         |
 *                               +---------+---------+   +---------+---------+
 */

#define SYM_LAYER \
    SYM_GRID(HOME_ROW_MOD, XXXXXXX), \
                               _______, KC_SPC,  KC_ESC,  _______

/*
 * Number layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | 4       | 5       | 6       |         |   |         | |       | -       | /       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | 0 / CTL | 1 / ALT | 2 / GUI | 3 / SFT | DEL     |   | BSPC    | ' / SFT | _ / GUI | = / ALT | * / CTL |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | ~       | 7       | 8       | 9       | .       |   | LLCK    | +       | ?       | !       | >FUN<   |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | SPC     |   |         | _NUM_   |
 *                               +---------+---------+   +---------+---------+
 *
 * >FUN< is the trigger key of FUN: pressed while NUM_ENT is held, it switches
 * FUN on until NUM_ENT is released. After a quick release of NUM_ENT, FUN
 * stays on for the next typed key only.
 */

#define NUM_LAYER \
    NUM_GRID(HOME_ROW_MOD, TRG_FUN), \
                               _______, KC_SPC,  _______, _______

/*
 * Navigation layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   | HOME    | PGDN    | PGUP    | END     | INS     |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | CTL*    | ALT*    | GUI*    | SFT*    | >MOU<   |   | LEFT    | DOWN    | UP      | RGHT    | BSPC    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | UNDO    | CUT     | COPY    | PASTE   | LLCK    |   |         | ENT     | REP     | AREP    | DEL     |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | _NAV_   |   |         |         |
 *                               +---------+---------+   +---------+---------+
 *
 * >MOU< is the trigger key of MOU: pressed while NAV_SPC is held, it switches
 * MOU on until NAV_SPC is released. After a quick release of NAV_SPC, MOU
 * stays locked.
 */

#define NAV_LAYER \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,  KC_HOME, KC_PGDN, KC_PGUP, KC_END,  KC_INS,  \
    OS_LCTL, OS_LALT, OS_LGUI, OS_LSFT, TRG_MOU,  KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_BSPC, \
    M_UNDO,  M_CUT,   M_COPY,  M_PSTE,  QK_LLCK,  XXXXXXX, KC_ENT,  QK_REP,  QK_AREP, KC_DEL,  \
                               _______, _______,  _______, _______

/*
 * Mouse layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   | MS_WHLL | MS_WHLD | MS_WHLU | MS_WHLR |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | CTL     | ALT     | GUI     | SFT     | >MOU<   |   | MS_LEFT | MS_DOWN | MS_UP   | MS_RGHT | BTN3    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   | LLCK    |         |         |         |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         |         |   | BTN1    | BTN2    |
 *                               +---------+---------+   +---------+---------+
 *
 * The trigger key of MOU lies here as the key itself, not as a transparent
 * key: NAV is off while MOU is locked, and the trigger key at the same
 * position must still leave MOU. The left thumb keys fall through, to
 * NAV_SPC while it is held and to BASE while MOU is locked.
 */

#define MOUSE_LAYER \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, MS_WHLL, MS_WHLD, MS_WHLU, MS_WHLR, XXXXXXX, \
    KC_LCTL, KC_LALT, KC_LGUI, KC_LSFT, TRG_MOU, MS_LEFT, MS_DOWN, MS_UP,   MS_RGHT, MS_BTN3, \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, QK_LLCK, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, \
                               _______, _______, MS_BTN1, MS_BTN2

/*
 * Function layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | BOOT    | F9      | F8      | F7      | F10     |   | MUTE    | VOLD    | VOLU    | BRID    | BRIU    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | F3      | F2      | F1      | F11     |   |         | SFT     | GUI     | ALT     | CTL     |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | F6      | F5      | F4      | F12     |   | LLCK    | MPRV    | MPLY    | MNXT    | >FUN<   |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         |         |   |         |         |
 *                               +---------+---------+   +---------+---------+
 *
 * The trigger key of FUN lies here as the key itself, not as a transparent
 * key: NUM is off while FUN is pending or locked. The four thumb keys fall
 * through.
 */

#define FUN_LAYER \
    FUN_GRID(HELD_MOD, TRG_FUN), \
                               _______, _______, _______, _______

/* clang-format on */

