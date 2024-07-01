#include "hydeik.h"

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
    [_QWERTY] = LAYOUT_base_wrapper(
        _________________QWERTY_L1_________________, _________________QWERTY_R1_________________,
        _________________QWERTY_L2_________________, _________________QWERTY_R2_________________,
        _________________QWERTY_L3_________________, _________________QWERTY_R3_________________,
                                  ____QWERTY_L4____, ____QWERTY_R4____
    ),
    [_LOWER] = LAYOUT_base_wrapper(
        _________________LOWER_L1__________________, _________________LOWER_R1__________________,
        _________________LOWER_L2__________________, _________________LOWER_R2__________________,
        _________________LOWER_L3__________________, _________________LOWER_R3__________________,
                                  ____LOWER_L4_____, ____LOWER_R4_____
    ),
    [_RAISE] = LAYOUT_base_wrapper(
        _________________RAISE_L1__________________, _________________RAISE_L1__________________,
        _________________RAISE_L2__________________, _________________RAISE_L2__________________,
        _________________RAISE_L3__________________, _________________RAISE_L3__________________,
                                  ____RAISE_L4_____, ____RAISE_R4_____
    ),
    [_ADJUST] = LAYOUT_base_wrapper(
        _________________ADJUST_L1_________________, _________________ADJUST_R1_________________,
        _________________ADJUST_L2_________________, _________________ADJUST_R2_________________,
        _________________ADJUST_L3_________________, _________________ADJUST_R3_________________,
                                  ____ADJUST_L4____, ____ADJUST_R4____
    ),
};

/* clang-format on */
