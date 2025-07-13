/*
Copyright 2019 @foostan
Copyright 2020 Drashna Jaelre <@drashna>

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

#include "hydeik.h"

#define LAYOUT_crkbd_wrapper(...) LAYOUT_split_3x6_3_ex2(__VA_ARGS__)

#define LAYOUT_hydeik_keymap( \
    L01, L02, L03, L04, L05, R01, R02, R03, R04, R05, \
    L06, L07, L08, L09, L10, R06, R07, R08, R09, R10, \
    L11, L12, L13, L14, L15, R11, R12, R13, R14, R15, \
                   L16, L17, R16, R17                 \
    ) \
    LAYOUT_crkbd_wrapper( \
    KC_NO, L01, L02, L03, L04, L05, KC_NO, KC_NO, R01, R02, R03, R04, R05, KC_NO, \
    KC_NO, L06, L07, L08, L09, L10, KC_NO, KC_NO, R06, R07, R08, R09, R10, KC_NO, \
    KC_NO, L11, L12, L13, L14, L15,               R11, R12, R13, R14, R15, KC_NO, \
                          KC_NO, L16, L17, R16, R17, KC_NO                        \
    )

#define LAYOUT_hydeik(...) LAYOUT_hydeik_keymap(__VA_ARGS__)

const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM = LAYOUT_crkbd_wrapper(
    'L', 'L', 'L', 'L', 'L', 'L', 'L',   'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L', 'L',   'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L',             'R', 'R', 'R', 'R', 'R', 'R',
                        '*', '*', '*',   '*', '*', '*'
);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_hydeik(
        _________________QWERTY_L1_________________, _________________QWERTY_R1_________________,
        _________________QWERTY_L2_________________, _________________QWERTY_R2_________________,
        _________________QWERTY_L3_________________, _________________QWERTY_R3_________________,
                                    ___QWERTY_L4___, ___QWERTY_R4___
    ),

    [_SYM] = LAYOUT_hydeik(
        _________________SYMBOL_L1_________________, _________________SYMBOL_R1_________________,
        _________________SYMBOL_L2_________________, _________________SYMBOL_R2_________________,
        _________________SYMBOL_L3_________________, _________________SYMBOL_R3_________________,
                                    ___SYMBOL_L4___, ___SYMBOL_R4___
    ),

    [_NUM] = LAYOUT_hydeik(
        _________________NUMBER_L1_________________, _________________NUMBER_R1_________________,
        _________________NUMBER_L2_________________, _________________NUMBER_R2_________________,
        _________________NUMBER_L3_________________, _________________NUMBER_R3_________________,
                                    ___NUMBER_L4___, ___NUMBER_R4___
    ),

    [_FUN] = LAYOUT_hydeik(
        ________________FUNCTION_L1________________, ________________FUNCTION_R1________________,
        ________________FUNCTION_L2________________, ________________FUNCTION_R2________________,
        ________________FUNCTION_L3________________, ________________FUNCTION_R3________________,
                                    __FUNCTION_L4__, __FUNCTION_R4__
    ),

    [_EXT] = LAYOUT_hydeik(
        _________________EXTRAS_L1_________________, _________________EXTRAS_R1_________________,
        _________________EXTRAS_L2_________________, _________________EXTRAS_R2_________________,
        _________________EXTRAS_L3_________________, _________________EXTRAS_R3_________________,
                                    ___EXTRAS_L4___, ___EXTRAS_R4___
    ),
};

/* clang-format on */

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
  [0] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT), ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_RGHT, KC_LEFT), },
  [1] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT), ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_RGHT, KC_LEFT), },
  [2] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT), ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_RGHT, KC_LEFT), },
  [3] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT), ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_RGHT, KC_LEFT), },
  [4] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT), ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(KC_RGHT, KC_LEFT), },
};
#endif
