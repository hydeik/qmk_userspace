#include QMK_KEYBOARD_H

#include "hydeik.h"
#include "users/hydeik_common/keyboards/mothwing.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_HYDEIK_34(BASE_LAYER),
    [_SYM] = LAYOUT_HYDEIK_34(SYM_LAYER),
    [_NUM] = LAYOUT_HYDEIK_34(NUM_LAYER),
    [_NAV] = LAYOUT_HYDEIK_34(NAV_LAYER),
    [_MOU] = LAYOUT_HYDEIK_34(MOUSE_LAYER),
    [_FUN] = LAYOUT_HYDEIK_34(FUN_LAYER),
};
