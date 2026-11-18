/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Common core: the SYM, NUM and FUN layers, which are the same in every
 * keymap family apart from the modifier scheme.
 *
 * Each grid is the 3x10 finger keys of one layer. A keymap family builds its
 * layer from the grid plus its own four thumb keys, for example:
 *
 *     #define HOME_ROW(name) KC_##name
 *     #define SYM_LAYER SYM_GRID(HOME_ROW, XXXXXXX), _______, KC_SPC, KC_ESC, _______
 *
 * Grid parameters:
 *
 *   H(NAME)  Hook macro for a home-row key that may carry a modifier. The
 *            keymap family turns the bare NAME into its own keycode.
 *   M(NAME)  Hook macro for a held modifier, used the same way.
 *   BR       The bottom-right corner key.
 *
 * A hook must paste NAME onto a prefix with `##` directly in its own body.
 * NAME is then never macro-expanded on its own, which is what keeps names
 * such as RSFT (a function-like macro in QMK) usable here. Do not pass NAME
 * on to another macro before pasting, and do not write an already prefixed
 * keycode such as KC_ASTR in a hook: it would be expanded to LSFT(KC_8)
 * before the hook sees it.
 *
 * This file must not branch on the keymap family or the firmware source.
 */

#pragma once

/* clang-format off */

/*
 * Symbol layer (finger keys)
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | \       | %       | $       |         |   |         | ^       | {       | }       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | H(*)    | H(()    | H())    | H(:)    | DEL     |   | BSPC    | H(")    | H([)    | H(])    | H(;)    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | `       | @       | &       | LLCK    |   |         | #       | <       | >       | BR      |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 */

#define SYM_GRID(H, BR) \
    XXXXXXX,  KC_BSLS,  KC_PERC,  KC_DLR,   XXXXXXX,  XXXXXXX,  KC_CIRC,  KC_LCBR,  KC_RCBR,  XXXXXXX, \
    H(ASTR),  H(LPRN),  H(RPRN),  H(COLN),  KC_DEL,   KC_BSPC,  H(DQUO),  H(LBRC),  H(RBRC),  H(SCLN), \
    XXXXXXX,  KC_GRV,   KC_AT,    KC_AMPR,  QK_LLCK,  XXXXXXX,  KC_HASH,  KC_LT,    KC_GT,    BR

/*
 * Number layer (finger keys)
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | 4       | 5       | 6       |         |   |         | |       | -       | /       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | H(0)    | H(1)    | H(2)    | H(3)    | DEL     |   | BSPC    | H(')    | H(_)    | H(=)    | H(*)    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | ~       | 7       | 8       | 9       | .       |   | LLCK    | +       | ?       | !       | BR      |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 */

#define NUM_GRID(H, BR) \
    XXXXXXX,  KC_4,     KC_5,     KC_6,     XXXXXXX,  XXXXXXX,  KC_PIPE,  KC_MINS,  KC_SLSH,  XXXXXXX, \
    H(0),     H(1),     H(2),     H(3),     KC_DEL,   KC_BSPC,  H(QUOT),  H(UNDS),  H(EQL),   H(ASTR), \
    KC_TILD,  KC_7,     KC_8,     KC_9,     KC_DOT,   QK_LLCK,  KC_PLUS,  KC_QUES,  KC_EXLM,  BR

/*
 * Function layer (finger keys)
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | BOOT    | F9      | F8      | F7      | F10     |   | MUTE    | VOLD    | VOLU    | BRID    | BRIU    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | F3      | F2      | F1      | F11     |   |         | M(SFT)  | M(GUI)  | M(ALT)  | M(CTL)  |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | F6      | F5      | F4      | F12     |   | LLCK    | MPRV    | MPLY    | MNXT    | BR      |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 */

#define FUN_GRID(M, BR) \
    QK_BOOT,  KC_F9,    KC_F8,    KC_F7,    KC_F10,   KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_BRID,  KC_BRIU, \
    XXXXXXX,  KC_F3,    KC_F2,    KC_F1,    KC_F11,   XXXXXXX,  M(RSFT),  M(RGUI),  M(RALT),  M(RCTL), \
    XXXXXXX,  KC_F6,    KC_F5,    KC_F4,    KC_F12,   QK_LLCK,  KC_MPRV,  KC_MPLY,  KC_MNXT,  BR

/* clang-format on */
