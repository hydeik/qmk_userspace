/* Tap & Hold configurations */
#pragma once

#define TAPPING_TERM 175
#define PERMISSIVE_HOLD
#define QUICK_TAP_TERM_PER_KEY

#ifdef TAPPING_TOGGLE
#    define TAPPING_TOGGLE 1
#endif /* TAPPING_TOGGLE */

#ifdef ACHORDION_ENABLE
#    define ACHORDION_STREAK
#endif /* ACHORDION_ENABLE */
