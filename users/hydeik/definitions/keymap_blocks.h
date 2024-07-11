#pragma once

#include QMK_KEYBOARD_H

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

#define _______________NAVIGATION_R1_______________       KC_HOME, KC_PGDN, KC_PGUP, KC_END,  XXXXXXX
#define _______________NAVIGATION_R2_______________       KC_LEFT, HM_DOWN, HM_UP,   HM_RGHT, HM_BSPC
#define _______________NAVIGATION_R3_______________       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_DEL

#define _________________SYMBOL_L1_________________       KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC
#define _________________SYMBOL_L2_________________       HM_DQT,  HM_LT,   HM_LCBR, HM_LPRN, KC_LBRC
#define _________________SYMBOL_L3_________________       KC_QUOT, KC_GT,   KC_RCBR, KC_RPRN, KC_RBRC

#define _________________SYMBOL_R1_________________       KC_CIRC, KC_AMPR, KC_ASTR, KC_GRV,  KC_TILD
#define _________________SYMBOL_R2_________________       KC_PIPE, HM_UNDS, HM_EQL,  HM_PLUS, HM_COLN
#define _________________SYMBOL_R3_________________       KC_BSLS, KC_MINS, KC_COMM, KC_DOT,  KC_QUES

#define _________________NUMBER_L1_________________       KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC
#define _________________NUMBER_L2_________________       HM_1,    HM_2,    HM_3,    HM_4,    KC_5
#define _________________NUMBER_L3_________________       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, LLOCK

#define _________________NUMBER_R1_________________       KC_CIRC, KC_AMPR, KC_ASTR, KC_PLUS, KC_COLN
#define _________________NUMBER_R2_________________       KC_6,    HM_7,    HM_8,    HM_9,    HM_0
#define _________________NUMBER_R3_________________       KC_UNDS, KC_MINS, KC_COMM, KC_DOT,  KC_SLSH

#define ________________FUNCTION_L1________________       KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5
#define ________________FUNCTION_L2________________       KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10
#define ________________FUNCTION_L3________________       KC_F11,  KC_F12,  XXXXXXX, XXXXXXX, KC_PSCR

#define ________________FUNCTION_R1________________       KC_MUTE, KC_VOLD, KC_VOLU, KC_BRID, KC_BRIU
#define ________________FUNCTION_R2________________       XXXXXXX, KC_RSFT, KC_RGUI, KC_RALT, KC_RCTL
#define ________________FUNCTION_R3________________       LLOCK,   KC_MSTP, KC_MPRV, KC_MPLY, KC_MNXT

/* clang-format on */
