#include "arsenik.h"

/* clang-format off */
#define LAYOUT_wrapper(...) LAYOUT(__VA_ARGS__)
#define LAYOUT_base( \
    L01, L02, L03, L04, L05, R01, R02, R03, R04, R05, \
    L06, L07, L08, L09, L10, R06, R07, R08, R09, R10, \
    L11, L12, L13, L14, L15, R11, R12, R13, R14, R15, \
                   L16, L17, R16, R17                 \
    ) \
    LAYOUT_wrapper( \
    KC_NO, L01, L02, L03, L04, L05, R01, R02, R03, R04, R05, KC_NO, \
    KC_NO, L06, L07, L08, L09, L10, R06, R07, R08, R09, R10, KC_NO, \
    KC_NO, L11, L12, L13, L14, L15, R11, R12, R13, R14, R15, KC_NO, \
            KC_NO, L16, L17, KC_NO, KC_NO, R16, R17, KC_NO          \
    )

#define LAYOUT_base_wrapper(...) LAYOUT_base(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/*
 * Layer 0: Alpha keys
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | Q       | W       | E       | R       | T       |   | Y       | U       | I       | O       | P       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | A/LCTL  | S/LALT  | D/LGUI  | F/LSFT  | G       |   | H       | J/RSFT  | K/RGUI  | L/RALT  | ;/RCTL  |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | Z       | X       | C       | V       | B       |   | N       | M       | ,       | .       | /       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | OS_SFT  | SPC/EXT |   | ENT/SYM | BSPC/FUN|
 *                               +---------+---------+   +---------+---------+
 */
    [_BASE] = LAYOUT_base_wrapper(
        _________________QWERTY_L1_________________, _________________QWERTY_R1_________________,
        _________________QWERTY_L2_________________, _________________QWERTY_R2_________________,
        _________________QWERTY_L3_________________, _________________QWERTY_R3_________________,
                             OSM(MOD_LSFT), EXT_SPC, SYM_ENT, FUN_BSPC
    ),
/*
 * Layer 1: Navigation + Numbers
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | HOME    | UP      | END     | PGUP    |   | /       | 7       | 8       | 9       | :       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | LCTL    |LEFT/LALT|DOWN/LGUI|RGHT/LSFT| PGDN    |   | -       | 4/RSFT  | 5/RGUI  | 6/RALT  | 0/RCTL  |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         | S+TAB   | TAB     | LLOCK   |   | ,       | 1       | 2       | 3       | .       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | *EXT*   |   | ESC     | DEL     |
 *                               +---------+---------+   +---------+---------+
 */
    [_EXT] = LAYOUT_base_wrapper(
        _______________NAVIGATION_L1_______________, _______________NAVIGATION_R1_______________,
        _______________NAVIGATION_L2_______________, _______________NAVIGATION_R2_______________,
        _______________NAVIGATION_L3_______________, _______________NAVIGATION_R3_______________,
                                   _______, _______,  KC_ESC,  KC_DEL
    ),
/*
 * Layer 2: Symbols
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | @       | <       | >       | $       | %       |   | ^       | &       | *       | '       | `       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | {/LCTL  | (/LALT  | )/LGUI  | }/LSFT  | =       |   | \       | +/RSFT  | -/RGUI  | //RALT  | "/RCTL  |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | ~       | [       | ]       | _       | #       |   | |       | !       | ;       | :       | ?       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | BSPC    | SPC     |   | *SYM*   |         |
 *                               +---------+---------+   +---------+---------+
 */
    [_SYM] = LAYOUT_base_wrapper(
        _________________SYMBOL_L1_________________, _________________SYMBOL_R1_________________,
        _________________SYMBOL_L2_________________, _________________SYMBOL_R2_________________,
        _________________SYMBOL_L3_________________, _________________SYMBOL_R3_________________,
                                   KC_BSPC,  KC_SPC, _______, _______
    ),
/*
 * Layer 3: Function and media keys
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | F1      | F2      | F3      | F4      | F5      |   | MUTE    | VOLD    | VOLU    | BRID    | BRIU    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | F6      | F7      | F8      | F9      | F10     |   | CWORD   | RSFT    | RGUI    | RALT    | RCTL    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | F11     | F12     | MCTL    | LPAD    | PrScr   |   | LLOCK   | MSTP    | MPRV    | MPLY    | MNXT    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | AREP    | REP     |   |         | *FNC*   |
 *                               +---------+---------+   +---------+---------+
 */
    [_FUN] = LAYOUT_base_wrapper(
        ________________FUNCTION_L1________________, ________________FUNCTION_R1________________,
        ________________FUNCTION_L2________________, ________________FUNCTION_R2________________,
        ________________FUNCTION_L3________________, ________________FUNCTION_R3________________,
                                   QK_AREP,  QK_REP, _______, _______
    ),
/*
 * (Template)
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   |         |         |         |         |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   |         |         |         |         |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   |         |         |         |         |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         |         |   |         |         |
 *                               +---------+---------+   +---------+---------+
 */
};

/* clang-format on */
