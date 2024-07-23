#pragma once

#include QMK_KEYBOARD_H
#include "layers.h"

enum custom_keycode_t {
    LLOCK = QK_USER,

    TAB_EXT,
    SPC_NUM,
    ENT_SYM,
    ESC_FUN,

    /* home-row mods */
    HM_A,
    HM_S,
    HM_D,
    HM_F,
    HM_J,
    HM_K,
    HM_L,
    HM_SCLN,

    HM_DOWN,
    HM_UP,
    HM_RGHT,
    HM_BSPC,

    HM_DQT,
    HM_LT,
    HM_LPRN,
    HM_LCBR,
    HM_UNDS,
    HM_EQL,
    HM_PLUS,
    HM_COLN,

    HM_1,
    HM_2,
    HM_3,
    HM_4,
    HM_7,
    HM_8,
    HM_9,
    HM_0,
};

/* mod-tap keys */
#define EXT_SPC  LT(_EXT, KC_SPC)
#define SYM_ENT  LT(_SYM, KC_ENT)
#define FNU_BSPC LT(_FUN, KC_BSPC)

/* mod-tap keys (home row) */
#define HM_A     LCTL_T(KC_A)
#define HM_S     LALT_T(KC_S)
#define HM_D     LGUI_T(KC_D)
#define HM_F     LSFT_T(KC_F)
#define HM_J     RSFT_T(KC_J)
#define HM_K     RGUI_T(KC_K)
#define HM_L     RALT_T(KC_L)
#define HM_SC    RCTL_T(KC_SCLN)

#define HM_LEFT  LALT_T(KC_LEFT)
#define HM_DOWN  LGUI_T(KC_DOWN)
#define HM_RGHT  LSFT_T(KC_RGHT)
#define HM_4     RSFT_T(KC_4)
#define HM_5     RGUI_T(KC_5)
#define HM_6     RALT_T(KC_6)
#define HM_0     RCTL_T(KC_0)

/* KC_KP_0 and KC_KP_1 are used as a placeholder for the tapping keycode */
#define HM_LCBR  LCTL_T(KC_KP_0)
#define HM_LPRN  LALT_T(KC_KP_0)
#define HM_RPRN  LGUI_T(KC_KP_0)
#define HM_RCBR  LSFT_T(KC_KP_0)
#define HM_PLUS  RSFT_T(KC_KP_1)
#define HM_MINS  RGUI_T(KC_MINS)
#define HM_SLSH  RALT_T(KC_SLSH)
#define HM_DQUO  RCTL_T(KC_KP_1)
