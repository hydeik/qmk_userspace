/*
 * PROTOTYPE - throwaway. Not built by any keymap.
 *
 * Variant B: complete shared layers written in family-neutral key names.
 *
 * The shared file owns the whole layer, thumbs included, as a flat list of
 * names. Nothing is passed in and nothing is pasted together: each keymap
 * family supplies a table saying what every neutral name (HR_*, MD_*, and the
 * per-layer slots) means for it, and must define the table before the shared
 * file is included.
 */

/* ======================= would live in users/hydeik ======================= */
#ifdef FAMILY_HYDEIK

/* Home-row keys: the family's mod-taps. */
#define HR_ASTR HM_ASTR
#define HR_LPRN HM_LPRN
#define HR_RPRN HM_RPRN
#define HR_COLN HM_COLN
#define HR_DQUO HM_DQUO
#define HR_LBRC HM_LBRC
#define HR_RBRC HM_RBRC
#define HR_SCLN HM_SCLN
#define HR_0    HM_0
#define HR_1    HM_1
#define HR_2    HM_2
#define HR_3    HM_3
#define HR_QUOT HM_QUOT
#define HR_UNDS HM_UNDS
#define HR_EQL  HM_EQL
/* Held modifiers. */
#define MD_RSFT KC_RSFT
#define MD_RGUI KC_RGUI
#define MD_RALT KC_RALT
#define MD_RCTL KC_RCTL
/* Slots. */
#define SYM_BR     XXXXXXX
#define SYM_THUMBS _______, KC_SPC,  KC_ESC,  _______
#define NUM_BR     XXXXXXX
#define NUM_THUMBS _______, KC_SPC,  _______, _______
#define FUN_BL     QK_LLCK
#define FUN_BR     XXXXXXX
#define FUN_THUMBS _______, _______, _______, _______

#endif

/* ===================== would live in users/hydeik_vial ==================== */
#ifdef FAMILY_VIAL

#define HR_ASTR KC_ASTR
#define HR_LPRN KC_LPRN
#define HR_RPRN KC_RPRN
#define HR_COLN KC_COLN
#define HR_DQUO KC_DQUO
#define HR_LBRC KC_LBRC
#define HR_RBRC KC_RBRC
#define HR_SCLN KC_SCLN
#define HR_0    KC_0
#define HR_1    KC_1
#define HR_2    KC_2
#define HR_3    KC_3
#define HR_QUOT KC_QUOT
#define HR_UNDS KC_UNDS
#define HR_EQL  KC_EQL
#define MD_RSFT COS_RSFT
#define MD_RGUI COS_RGUI
#define MD_RALT COS_RALT
#define MD_RCTL COS_RCTL
#define SYM_BR     TO(_NAV)
#define SYM_THUMBS _______,   _______, _______, KC_ESC
#define NUM_BR     TO(_NAV)
#define NUM_THUMBS KC_ESC,    _______, _______, _______
#define FUN_BL     KC_MSTP
#define FUN_BR     _______
#define FUN_THUMBS TO(_BASE), _______, _______, _______

#endif

/* ======================= would live in the shared file ==================== */

#define SYM_LAYER \
    XXXXXXX,  KC_BSLS,  KC_PERC,  KC_DLR,   XXXXXXX,  XXXXXXX,  KC_CIRC,  KC_LCBR,  KC_RCBR,  XXXXXXX, \
    HR_ASTR,  HR_LPRN,  HR_RPRN,  HR_COLN,  KC_DEL,   KC_BSPC,  HR_DQUO,  HR_LBRC,  HR_RBRC,  HR_SCLN, \
    XXXXXXX,  KC_GRV,   KC_AT,    KC_AMPR,  QK_LLCK,  XXXXXXX,  KC_HASH,  KC_LT,    KC_GT,    SYM_BR,  \
                                  SYM_THUMBS

#define NUM_LAYER \
    XXXXXXX,  KC_4,     KC_5,     KC_6,     XXXXXXX,  XXXXXXX,  KC_PIPE,  KC_MINS,  KC_SLSH,  XXXXXXX, \
    HR_0,     HR_1,     HR_2,     HR_3,     KC_DEL,   KC_BSPC,  HR_QUOT,  HR_UNDS,  HR_EQL,   HR_ASTR, \
    KC_TILD,  KC_7,     KC_8,     KC_9,     KC_DOT,   QK_LLCK,  KC_PLUS,  KC_QUES,  KC_EXLM,  NUM_BR,  \
                                  NUM_THUMBS

#define FUN_LAYER \
    QK_BOOT,  KC_F9,    KC_F8,    KC_F7,    KC_F10,   KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_BRID,  KC_BRIU, \
    XXXXXXX,  KC_F3,    KC_F2,    KC_F1,    KC_F11,   XXXXXXX,  MD_RSFT,  MD_RGUI,  MD_RALT,  MD_RCTL, \
    XXXXXXX,  KC_F6,    KC_F5,    KC_F4,    KC_F12,   FUN_BL,   KC_MPRV,  KC_MPLY,  KC_MNXT,  FUN_BR,  \
                                  FUN_THUMBS
