/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#pragma once

#include "users/hydeik_common/layers.h"

/****************************************************************************
 * Layer numbers
 ****************************************************************************/
enum layer_t {
    _BASE,
    _SYM,
    _NUM,
    _NAV,
    _FUN,
    _MOD,
};

/****************************************************************************
 * Custom keycodes
 ****************************************************************************/
enum custom_keycode_t {
    /*
     * Reserved slot with no behaviour. It stays so that the one-shot mod keys
     * keep the numbers QK_KB_0 + 1 .. QK_KB_0 + 8, which keymaps stored by
     * Vial hold. Do not remove or reuse it.
     */
    COS_MOD = QK_KB_0,
    /*
     * One-shot mod keys: an ordinary modifier while held, and a pending
     * one-shot mod for the next key press when tapped. On the smart mod layer
     * they latch for as long as a smart mod layer key is held. See
     * features/smart_mod.h.
     */
    COS_LCTL,
    COS_LSFT,
    COS_LALT,
    COS_LGUI,
    COS_RCTL,
    COS_RSFT,
    COS_RALT,
    COS_RGUI,
    /*
     * Keep "customKeycodes" in the vial.json of every keymap of this keymap
     * family, and the copy of this enum in tests/smart_mod/test_glue.h, in
     * step with this enum: the same entries in the same order.
     */
};

/*
 * Smart mod layer keys: held, they turn the smart mod layer on. Tapped,
 * `SMT_MOD` makes Shift a pending one-shot mod, or cancels all pending
 * one-shot mods, and a double tap turns Caps Word on; its tap keycode KC_KP_0
 * is never sent. `SPC_MOD` and `ENT_MOD` are Space and Enter when tapped.
 * No layer here places `ENT_MOD`; it is kept for keymaps edited in Vial.
 */
#define SMT_MOD  LT(_MOD, KC_KP_0)
#define SPC_MOD  LT(_MOD, KC_SPC)
#define ENT_MOD  LT(_MOD, KC_ENT)

/*
 * Holding keys: held, they keep their holding layer on, SYM or NUM, and make
 * the trigger key of their target layer work. Tapped, they are Tab and Enter.
 */
#define TAB_SYM  LT(_SYM, KC_TAB)
#define ENT_NUM  LT(_NUM, KC_ENT)

/*
 * Trigger keys: `MO()` of a target layer is no momentary layer key. Pressed
 * while its holding key is held, `TAB_SYM` for NAV and `ENT_NUM` for FUN, it
 * switches to its target layer; pressed again, or on a pending or locked
 * target layer, it switches that layer off. Alone it does nothing. A quick
 * release of the holding key locks NAV and leaves FUN pending. See
 * users/hydeik_common/features/target_layer.h and the table of entries in
 * hydeik_vial.c.
 *
 * A target layer holds its own trigger key at the position of the trigger
 * key of its holding layer, so that the same key enters and leaves it. The
 * diagrams below show the trigger keys as `>NAV<` and `>FUN<`.
 */
#define TRG_NAV  MO(_NAV)
#define TRG_FUN  MO(_FUN)

/****************************************************************************
 * Keymap blocks
 ****************************************************************************/

/*
 * Hooks for the common core's layer grids: home-row keys are plain keys, and
 * modifier keys are one-shot mod keys.
 */
#define HOME_ROW_KEY(name) KC_##name
#define ONESHOT_MOD(name) COS_##name

/* clang-format off */

/*
 * QWERTY layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | Q       | W       | E       | R       | T       |   | Y       | U       | I       | O       | P       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | A       | S       | D       | F       | G       |   | H       | J       | K       | L       | ;       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | Z       | X       | C       | V       | B       |   | N       | M       | ,       | .       | /       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | -SYM-   | SMTMOD  |   | SPC/MOD | -NUM-   |
 *                               +---------+---------+   +---------+---------+
 */

#define BASE_LAYER \
    KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,    \
    KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN, \
    KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH, \
                                  TAB_SYM,  SMT_MOD,  SPC_MOD,  ENT_NUM

/*
 * symbol layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | \       | %       | $       |         |   |         | ^       | {       | }       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | *       | (       | )       | :       | DEL     |   | BSPC    | "       | [       | ]       | ;       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | `       | @       | &       | LLCK    |   |         | #       | <       | >       | >NAV<   |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | _SYM_   |         |   | MOD     | ESC     |
 *                               +---------+---------+   +---------+---------+
 */
#define SYM_LAYER \
    SYM_GRID(HOME_ROW_KEY, TRG_NAV), \
                                  _______,  _______,   _______,  KC_ESC

/*
 * Number layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | 4       | 5       | 6       |         |   |         | |       | -       | /       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | 0       | 1       | 2       | 3       | DEL     |   | BSPC    | '       | _       | =       | *       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | ~       | 7       | 8       | 9       | .       |   | LLCK    | +       | ?       | !       | >FUN<   |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | ESC     |         |   |         | _NUM_   |
 *                               +---------+---------+   +---------+---------+
 */

#define NUM_LAYER \
    NUM_GRID(HOME_ROW_KEY, TRG_FUN), \
                                  KC_ESC,   _______,   _______,  _______

/*
 * Navigation layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | ESC     |         |         |         |         |   |         | HOME    | UP      | END     | PGUP    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | CTL*    | ALT*    | GUI*    | SFT*    | DEL     |   | BSPC    | LEFT    | DOWN    | RGHT    | PGDN    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   | LLCK    |         |         |         | >NAV<   |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         |         |   |         |         |
 *                               +---------+---------+   +---------+---------+
 */

#define NAV_LAYER \
    KC_ESC,   XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,   XXXXXXX,  KC_HOME,  KC_UP,    KC_END,   KC_PGUP,  \
    COS_LCTL, COS_LALT, COS_LGUI, COS_LSFT, KC_DEL,    KC_BSPC,  KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_PGDN,  \
    XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,   QK_LLCK,  XXXXXXX,  XXXXXXX,  XXXXXXX,  TRG_NAV,  \
                                  _______,  _______,   _______,  _______

/*
 * Function layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | BOOT    | F9      | F8      | F7      | F10     |   | MUTE    | VOLD    | VOLU    | BRID    | BRIU    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | F3      | F2      | F1      | F11     |   |         | SFT*    | GUI*    | ALT*    | CTL*    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | F6      | F5      | F4      | F12     |   | LLCK    | MPRV    | MPLY    | MNXT    | >FUN<   |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         |         |   |         |         |
 *                               +---------+---------+   +---------+---------+
 */

#define FUN_LAYER \
    FUN_GRID(ONESHOT_MOD, TRG_FUN), \
                                  _______,  _______,   _______,  _______

/*
 * Smart mod layer: one-shot mod keys over the home row. It is on while a
 * smart mod layer key is held and the one-shot mods are being chosen, and
 * switches itself off once the chosen one-shot mod keys are all released.
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   |         |         |         |         |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | CTL*    | ALT*    | GUI*    | SFT*    |         |   |         | SFT*    | GUI*    | ALT*    | CTL*    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   |         |         |         |         |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         |         |   |         |         |
 *                               +---------+---------+   +---------+---------+
 */

#define MOD_LAYER \
    _______,  _______,  _______,  _______,  _______,   _______,  _______,  _______,  _______,  _______,  \
    COS_LCTL, COS_LALT, COS_LGUI, COS_LSFT, _______,   _______,  COS_RSFT, COS_RGUI, COS_RALT, COS_RCTL, \
    _______,  _______,  _______,  _______,  _______,   _______,  _______,  _______,  _______,  _______,  \
                                  _______,  _______,   _______,  _______

/* clang-format on */

