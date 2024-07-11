#pragma once

#include QMK_KEYBOARD_H

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

