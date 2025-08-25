#include QMK_KEYBOARD_H

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

#define LAYOUT_hydeik(...) LAYOUT_base(__VA_ARGS__)

const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM = LAYOUT_wrapper(
    'L', 'L', 'L', 'L', 'L', 'L',   'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L',   'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L',   'R', 'R', 'R', 'R', 'R', 'R',
              '*', '*', '*', '*',   '*', '*', '*', '*'
);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_hydeik(BASE_LAYER),
    [_SYM] = LAYOUT_hydeik(SYM_LAYER),
    [_NUM] = LAYOUT_hydeik(NUM_LAYER),
    [_NAV] = LAYOUT_hydeik(NAV_LAYER),
    [_MOU] = LAYOUT_hydeik(MOUSE_LAYER),
    [_FUN] = LAYOUT_hydeik(FUN_LAYER),
};

/* clang-format on */
