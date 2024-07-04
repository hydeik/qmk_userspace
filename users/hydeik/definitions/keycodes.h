#pragma once

#include QMK_KEYBOARD_H

enum custom_keycode_t {
    CKC_TAB = QK_USER,
    CKC_SPC,
    CKC_ENT,
    CKC_ESC,
  
    /* home-row mods */
    CKC_A,
    CKC_S,
    CKC_D,
    CKC_F,
    CKC_J,
    CKC_K,
    CKC_L,
    CKC_SCLN,

    CKC_DOWN,
    CKC_UP,
    CKC_RGHT, 
    CKC_BSPC,

    CKC_DQT,
    CKC_LT,
    CKC_LPRN,
    CKC_LCBR,
    CKC_UNDS,
    CKC_PLUS,
    CKC_COLN,
    /* CKC_SCLN, */
};

