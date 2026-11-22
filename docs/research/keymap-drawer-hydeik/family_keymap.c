/*
 * Throwaway input for `qmk c2json`: the seven layers of the hydeik keymap
 * family as 34 keys each, with no layout adapter and no keyboard.
 *
 * `qmk c2json` runs plain `cpp` on this file (no include paths, no QMK
 * headers) and reads the arguments of every LAYOUT*() call as text. So:
 *   - layout.h is included by a relative path;
 *   - LAYOUT_HYDEIK_34 is deliberately left undefined, so that the 34 keys
 *     stay as its arguments;
 *   - the layer names are defined as numbers, because keymap-drawer only
 *     understands MO(<digits>) and LT(<digits>, ...). This turns the enum of
 *     layout.h into nonsense, which is harmless: the file is only
 *     preprocessed, never compiled.
 */
#define _BASE 0
#define _SYM  1
#define _NUM  2
#define _NAV  3
#define _MOU  4
#define _FUN  5
#define _MOD  6

#include "../../../users/hydeik/layout.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_HYDEIK_34(BASE_LAYER),
    [_SYM]  = LAYOUT_HYDEIK_34(SYM_LAYER),
    [_NUM]  = LAYOUT_HYDEIK_34(NUM_LAYER),
    [_NAV]  = LAYOUT_HYDEIK_34(NAV_LAYER),
    [_MOU]  = LAYOUT_HYDEIK_34(MOU_LAYER),
    [_FUN]  = LAYOUT_HYDEIK_34(FUN_LAYER),
    [_MOD]  = LAYOUT_HYDEIK_34(MOD_LAYER),
};
