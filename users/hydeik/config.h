#pragma once

#include "users/hydeik_common/common_config.h"

/****************************************************************************
 * Tap & Hold
 ****************************************************************************/
#define TAPPING_TERM 250

#define TAPPING_TERM_PER_KEY
#define FLOW_TAP_TERM 125
#define CHORDAL_HOLD
#define PERMISSIVE_HOLD
#define HOLD_ON_OTHER_KEY_PRESS_PER_KEY
#define QUICK_TAP_TERM_PER_KEY

/****************************************************************************
 * Caps Word
 ****************************************************************************/
#ifdef CAPS_WORD_ENABLE
#define BOTH_SHIFTS_TURNS_ON_CAPS_WORD
#endif  /* CAPS_WORD_ENABLE */

#define HYDEIK_CLIPBOARD_MAC
