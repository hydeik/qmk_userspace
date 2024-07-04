#pragma once

#include QMK_KEYBOARD_H


/* clang-format off */

/* Blocks for the major keyboard layouts. */
#define _________________QWERTY_L1_________________       KC_Q,    KC_W,    KC_E,    KC_R,    KC_T
#define _________________QWERTY_L2_________________       KC_A,    KC_S,    KC_D,    KC_F,    KC_G
#define _________________QWERTY_L3_________________       KC_Z,    KC_X,    KC_C,    KC_V,    KC_B

#define _________________QWERTY_R1_________________       KC_Y,    KC_U,    KC_I,    KC_O,    KC_P
#define _________________QWERTY_R2_________________       KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN
#define _________________QWERTY_R3_________________       KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH


#define _________________COLEMAK_L1________________       KC_Q,    KC_W,    KC_F,    KC_P,    KC_G
#define _________________COLEMAK_L2________________       KC_A,    KC_R,    KC_S,    KC_T,    KC_D
#define _________________COLEMAK_L3________________       KC_Z,    KC_X,    KC_C,    KC_V,    KC_B

#define _________________COLEMAK_R1________________       KC_J,    KC_L,    KC_U,    KC_Y,    KC_SCLN
#define _________________COLEMAK_R2________________       KC_H,    KC_N,    KC_E,    KC_I,    KC_O
#define _________________COLEMAK_R3________________       KC_K,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH


#define ______________COLEMAK_MOD_DH_L1____________       KC_Q,    KC_W,    KC_F,    KC_P,    KC_B
#define ______________COLEMAK_MOD_DH_L2____________       KC_A,    KC_R,    KC_S,    KC_T,    KC_G
#define ______________COLEMAK_MOD_DH_L3____________       KC_Z,    KC_X,    KC_C,    KC_D,    KC_V

#define ______________COLEMAK_MOD_DH_R1____________       KC_J,    KC_L,    KC_U,    KC_Y,    KC_SCLN
#define ______________COLEMAK_MOD_DH_R2____________       KC_M,    KC_N,    KC_E,    KC_I,    KC_O
#define ______________COLEMAK_MOD_DH_R3____________       KC_K,    KC_H,    KC_COMM, KC_DOT,  KC_SLASH


#define _________________DVORAK_L1_________________       KC_QUOT, KC_COMM, KC_DOT, KC_P,     KC_Y
#define _________________DVORAK_L2_________________       KC_A,    KC_O,    KC_E,   KC_U,     KC_I
#define _________________DVORAK_L3_________________       KC_SCLN, KC_Q,    KC_J,   KC_K,     KC_X

#define _________________DVORAK_R1_________________       KC_F,    KC_G,    KC_C,    KC_R,    KC_L
#define _________________DVORAK_R2_________________       KC_D,    KC_H,    KC_T,    KC_N,    KC_S
#define _________________DVORAK_R3_________________       KC_B,    KC_M,    KC_W,    KC_V,    KC_Z


#define ________________DVORAK_AU_L1_______________       KC_QUOT, KC_COMM, KC_DOT, KC_P,     KC_Y
#define ________________DVORAK_AU_L2_______________       KC_O,    KC_A,    KC_E,   KC_I,     KC_U
#define ________________DVORAK_AU_L3_______________       KC_SCLN, KC_Q,    KC_J,   KC_K,     KC_X

#define ________________DVORAK_AU_R1_______________       KC_F,    KC_G,    KC_C,    KC_R,    KC_L
#define ________________DVORAK_AU_R2_______________       KC_D,    KC_H,    KC_T,    KC_N,    KC_S
#define ________________DVORAK_AU_R3_______________       KC_B,    KC_M,    KC_W,    KC_V,    KC_Z


#define _________________WORKMAN_L1________________       KC_Q,    KC_D,    KC_R,   KC_W,     KC_B
#define _________________WORKMAN_L2________________       KC_A,    KC_S,    KC_H,   KC_T,     KC_G
#define _________________WORKMAN_L3________________       KC_Z,    KC_X,    KC_M,   KC_C,     KC_V

#define _________________WORKMAN_R1________________       KC_J,    KC_F,    KC_U,    KC_P,    KC_SCLN
#define _________________WORKMAN_R2________________       KC_Y,    KC_N,    KC_E,    KC_O,    KC_I
#define _________________WORKMAN_R3________________       KC_K,    KC_L,    KC_COMM, KC_DOT,  KC_SLSH


#define _________________EUCALYN_L1________________       KC_Q,    KC_W,    KC_COMM, KC_DOT,  KC_SCLN
#define _________________EUCALYN_L2________________       KC_A,    KC_O,    KC_E,    KC_I,    KC_U
#define _________________EUCALYN_L3________________       KC_Z,    KC_X,    KC_C,    KC_V,    KC_F

#define _________________EUCALYN_R1________________       KC_M,    KC_R,    KC_D,    KC_Y,    KC_P
#define _________________EUCALYN_R2________________       KC_G,    KC_T,    KC_K,    KC_S,    KC_N
#define _________________EUCALYN_R3________________       KC_B,    KC_H,    KC_J,    KC_L,    KC_SLSH


/* Blocks for numbers/symbols/navigation/functions */
#define ________________NUMBER_LEFT________________       KC_1,    KC_2,    KC_3,    KC_4,    KC_5
#define ________________NUMBER_RIGHT_______________       KC_6,    KC_7,    KC_8,    KC_9,    KC_0

#define ___________________BLANK___________________       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX


#define ________________FUNCTION_L1________________       KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5
#define ________________FUNCTION_L2________________       KC_LCTL, KC_LALT, KC_LGUI, KC_LSFT, KC_F11
#define ________________FUNCTION_L3________________       KC_MSTP, KC_MPRV, KC_MPLY, KC_MNXT, KC_NO

#define ________________FUNCTION_R1________________       KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10
#define ________________FUNCTION_R2________________       KC_F12,  KC_RSFT, KC_RGUI, KC_RALT, KC_RCTL
#define ________________FUNCTION_R3________________       KC_MUTE, KC_VOLD, KC_VOLU, KC_BRID, KC_BRIU


#define ________________SYMBOL1_L1_________________      ________________NUMBER_LEFT________________ 
#define ________________SYMBOL1_L2_________________
#define ________________SYMBOL1_L3_________________

#define ________________SYMBOL1_R1_________________      ________________NUMBER_RIGHT_______________
#define ________________SYMBOL1_R2_________________
#define ________________SYMBOL1_R3_________________

#define ________________SYMBOL2_L1_________________      
#define ________________SYMBOL2_L2_________________
#define ________________SYMBOL2_L3_________________

#define ________________SYMBOL2_R1_________________      ___________________BLANK___________________
#define ________________SYMBOL2_R2_________________      ___________________BLANK___________________
#define ________________SYMBOL2_R3_________________      ___________________BLANK___________________

/* QWERTY */
/*+--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | Q      | W      | E      | R      | T      |   | Y      | U      | I      | O      | P      |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | A/LCTL | S/LALT | D/LGUI | F/LSFT | G      |   | H      | J/RSFT | K/RGUI | L/RALT | ;/RCTL |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | Z      | X      | C      | V      | B      |   | N      | M      | ,      | .      | /      |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
                             | LOWER  | SPC    |   | ENT    | RAISE  |
                             +--------+--------+   +--------+--------+*/

#define _________________QWERTY_L1_________________ KC_Q   , KC_W   , KC_E   , KC_R   , KC_T
#define _________________QWERTY_L2_________________ KC_A   , KC_S   , KC_D   , KC_F   , KC_G
#define _________________QWERTY_L3_________________ KC_Z   , KC_X   , KC_C   , KC_V   , KC_B
#define                           ____QWERTY_L4____                            MO(_EXT), KC_SPC

#define _________________QWERTY_R1_________________ KC_Y   , KC_U   , KC_I   , KC_O   , KC_P
#define _________________QWERTY_R2_________________ KC_H   , KC_J   , KC_K   , KC_L   , KC_SCLN
#define _________________QWERTY_R3_________________ KC_N   , KC_M   , KC_COMM, KC_DOT , KC_SLSH
#define ____QWERTY_R4____                           KC_ENT , MO(_SYM1)

/* Lower (numbers + navigation) */
/*+--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | 1      | 2      | 3      | 4      | 5      |   | 6      | 7      | 8      | 9      | 0      |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | LCTL   | LALT   | LGUI   | LSFT   |        |   | LEFT   | DOWN   | UP     | RIGHT  | BSPC   |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | ESC    | S+TAB  |        | TAB    |        |   | HOME   | PgDN   | PgUP   | END    | DEL    |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
                             |*LOWER* |        |   |        | ADJUST |
                             +--------+--------+   +--------+--------+*/
#define _________________LOWER_L1__________________ KC_1   , KC_2   , KC_3   , KC_4   , KC_5
#define _________________LOWER_L2__________________ KC_LCTL, KC_LALT, KC_LGUI, KC_LSFT, KC_NO
#define _________________LOWER_L3__________________ KC_ESC , LSFT(KC_TAB), KC_NO  , KC_TAB , KC_NO
#define                           ____LOWER_L4_____                            KC_TRNS, KC_NO

#define _________________LOWER_R1__________________ KC_6   , KC_7   , KC_8   , KC_9   , KC_0
#define _________________LOWER_R2__________________ KC_LEFT, KC_DOWN, KC_UP  , KC_RGHT, KC_BSPC
#define _________________LOWER_R3__________________ KC_HOME, KC_PGDN, KC_PGUP, KC_END , KC_DEL
#define ____LOWER_R4_____                           KC_NO  , KC_TRNS

/* Upper (symbols) */
/*+--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | !      | @      | #      | $      | %      |   | ^      | &      | *      | (      | )      |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  |        | `      | {      | [      | +      |   | -      | |      |        |        |        |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | DEL    | BSPC   | }      | ]      | =      |   | _      | '      | "      |        | \      |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
                             | ADJUST |        |   |        |*RAISE* |
                             +--------+--------+   +--------+--------+*/

/*+--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | !      | @      | #      | $      | %      |   | ^      | &      | *      | `      | ~      |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | :      | <      | {      | (      | [      |   | |      | _      | +      | "      | ;      |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | ?      | >      | }      | )      | ]      |   | \      | -      | =      | '      | /      |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
                             | ADJUST |        |   |        |*RAISE* |
                             +--------+--------+   +--------+--------+*/

/*+--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | !      | @      | #      | $      | %      |   | ^      | &      | *      | `      | ~      |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | "      | <      | {      | (      | [      |   | |      | _      | +      | :      | ;      |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
  | '      | >      | }      | )      | ]      |   | \      | -      | =      | ?      | /      |
  +--------+--------+--------+--------+--------+   +--------+--------+--------+--------+--------+
                             | ADJUST |        |   |        |*RAISE* |
                             +--------+--------+   +--------+--------+*/

#define _________________RAISE_L1__________________ KC_EXLM, KC_AT  , KC_HASH, KC_DLR , KC_PERC
#define _________________RAISE_L2__________________ KC_DQT , KC_LT  , KC_LPRN, KC_LCBR, KC_LBRC
#define _________________RAISE_L3__________________ KC_QUOT, KC_GT  , KC_RPRN, KC_RCBR, KC_RBRC
#define                           ____RAISE_L4_____                            KC_TRNS, KC_NO

#define _________________RAISE_R1__________________ KC_CIRC, KC_AMPR, KC_ASTR, KC_GRV , KC_TILD
#define _________________RAISE_R2__________________ KC_PIPE, KC_UNDS, KC_PLUS, KC_COLN, KC_SCLN
#define _________________RAISE_R3__________________ KC_HOME, KC_BSLS, KC_MINS, KC_QUES, KC_SLSH
#define ____RAISE_R4_____                           KC_NO  , KC_TRNS

/* clang-format on */
