/* Throwaway probe for issue #37: tables QMK reads from the keymap. */

#include "quantum.h"

#ifdef CHORDAL_HOLD
/*
 * As in users/hydeik_common/keyboards/: the holding keys (columns 0 and 9
 * of row 0) are thumb keys, which belong to neither hand.
 */
const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM = {
    {'*', 'L', 'L', 'L', 'L', 'R', 'R', 'R', 'R', '*'},
    {'L', 'L', 'L', 'L', 'L', 'R', 'R', 'R', 'R', 'R'},
    {'L', 'L', 'L', 'L', 'L', 'R', 'R', 'R', 'R', 'R'},
    {'L', 'L', 'L', 'L', 'L', 'R', 'R', 'R', 'R', 'R'},
};
#endif
