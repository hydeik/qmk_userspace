/*
Copyright 2019 @foostan
Copyright 2020 Drashna Jaelre <@drashna>
Copyright 2025 Hidekazu Ikeno <@hydeik>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hydeik_vial.h"

#define LAYOUT_crkbd_wrapper(...) LAYOUT_split_3x6_3_ex2(__VA_ARGS__)

#define LAYOUT_hydeik_keymap( \
    L01, L02, L03, L04, L05, R01, R02, R03, R04, R05, \
    L06, L07, L08, L09, L10, R06, R07, R08, R09, R10, \
    L11, L12, L13, L14, L15, R11, R12, R13, R14, R15, \
              L16, L17, L18, R16, R17, R18            \
    ) \
    LAYOUT_crkbd_wrapper( \
    KC_NO, L01, L02, L03, L04, L05, KC_NO, KC_NO, R01, R02, R03, R04, R05, KC_NO, \
    KC_NO, L06, L07, L08, L09, L10, KC_NO, KC_NO, R06, R07, R08, R09, R10, KC_NO, \
    KC_NO, L11, L12, L13, L14, L15,               R11, R12, R13, R14, R15, KC_NO, \
                          L16, L17, L18,     R16, R17, R18                        \
    )

#define LAYOUT_hydeik(...) LAYOUT_hydeik_keymap(__VA_ARGS__)

const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM = LAYOUT_crkbd_wrapper(
    'L', 'L', 'L', 'L', 'L', 'L', 'L',   'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L', 'L',   'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L',             'R', 'R', 'R', 'R', 'R', 'R',
                        'L', 'L', 'L',   'R', 'R', 'R'
);


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_hydeik(
        _________________QWERTY_L1_________________, _________________QWERTY_R1_________________,
        _________________QWERTY_L2_________________, _________________QWERTY_R2_________________,
        _________________QWERTY_L3_________________, _________________QWERTY_R3_________________,
                           _____QWERTY_THUMB_L_____, _____QWERTY_THUMB_R_____
    ),

    [_SYM] = LAYOUT_hydeik(
        _________________SYMBOL_L1_________________, _________________SYMBOL_R1_________________,
        _________________SYMBOL_L2_________________, _________________SYMBOL_R2_________________,
        _________________SYMBOL_L3_________________, _________________SYMBOL_R3_________________,
                           _____SYMBOL_THUMB_L_____, _____SYMBOL_THUMB_R_____
    ),

    [_NUM] = LAYOUT_hydeik(
        _________________NUMBER_L1_________________, _________________NUMBER_R1_________________,
        _________________NUMBER_L2_________________, _________________NUMBER_R2_________________,
        _________________NUMBER_L3_________________, _________________NUMBER_R3_________________,
                           _____NUMBER_THUMB_L_____, _____NUMBER_THUMB_R_____
    ),

    [_FUN] = LAYOUT_hydeik(
        ________________FUNCTION_L1________________, ________________FUNCTION_R1________________,
        ________________FUNCTION_L2________________, ________________FUNCTION_R2________________,
        ________________FUNCTION_L3________________, ________________FUNCTION_R3________________,
                           ____FUNCTION_THUMB_L____, ____FUNCTION_THUMB_R____
    ),

    [_NAV] = LAYOUT_hydeik(
        _______________NAVIGATION_L1_______________, _______________NAVIGATION_R1_______________,
        _______________NAVIGATION_L2_______________, _______________NAVIGATION_R2_______________,
        _______________NAVIGATION_L3_______________, _______________NAVIGATION_R3_______________,
                           ___NAVIGATION_THUMB_L___, ___NAVIGATION_THUMB_R___
    ),

    [_MOD] = LAYOUT_hydeik(
        ______________ONESHOT_MOD_L1_______________, ______________ONESHOT_MOD_R1_______________,
        ______________ONESHOT_MOD_L2_______________, ______________ONESHOT_MOD_R2_______________,
        ______________ONESHOT_MOD_L3_______________, ______________ONESHOT_MOD_R3_______________,
                           __ONESHOT_MOD_THUMB_L___, __ONESHOT_MOD_THUMB_R___
    ),
};

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
  [0] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT), ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_RGHT, KC_LEFT), },
  [1] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT), ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_RGHT, KC_LEFT), },
  [2] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT), ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_RGHT, KC_LEFT), },
  [3] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT), ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_RGHT, KC_LEFT), },
  [4] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT), ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_RGHT, KC_LEFT), },
  [5] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT), ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_RGHT, KC_LEFT), },
};
#endif
