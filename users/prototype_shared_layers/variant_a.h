/*
 * PROTOTYPE - throwaway. Not built by any keymap.
 *
 * Variant A: one grid per layer, parameterised by the keymap family.
 *
 * The shared file owns the whole 3x10 grid. Keys that carry a home-row
 * modifier are written as H(NAME); the keymap family passes in the macro that
 * turns NAME into its own keycode. Keys that differ outright between keymap
 * families are extra parameters ("slots"). Thumb keys stay with the family.
 */

/* ======================= would live in the shared file ==================== */

#define SYM_GRID(H, BR) \
    XXXXXXX,  KC_BSLS,  KC_PERC,  KC_DLR,   XXXXXXX,  XXXXXXX,  KC_CIRC,  KC_LCBR,  KC_RCBR,  XXXXXXX, \
    H(ASTR),  H(LPRN),  H(RPRN),  H(COLN),  KC_DEL,   KC_BSPC,  H(DQUO),  H(LBRC),  H(RBRC),  H(SCLN), \
    XXXXXXX,  KC_GRV,   KC_AT,    KC_AMPR,  QK_LLCK,  XXXXXXX,  KC_HASH,  KC_LT,    KC_GT,    BR

#define NUM_GRID(H, BR) \
    XXXXXXX,  KC_4,     KC_5,     KC_6,     XXXXXXX,  XXXXXXX,  KC_PIPE,  KC_MINS,  KC_SLSH,  XXXXXXX, \
    H(0),     H(1),     H(2),     H(3),     KC_DEL,   KC_BSPC,  H(QUOT),  H(UNDS),  H(EQL),   H(ASTR), \
    KC_TILD,  KC_7,     KC_8,     KC_9,     KC_DOT,   QK_LLCK,  KC_PLUS,  KC_QUES,  KC_EXLM,  BR

/* M(NAME) is a held modifier; BL is the first key of the right bottom row. */
#define FUN_GRID(M, BL, BR) \
    QK_BOOT,  KC_F9,    KC_F8,    KC_F7,    KC_F10,   KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_BRID,  KC_BRIU, \
    XXXXXXX,  KC_F3,    KC_F2,    KC_F1,    KC_F11,   XXXXXXX,  M(RSFT),  M(RGUI),  M(RALT),  M(RCTL), \
    XXXXXXX,  KC_F6,    KC_F5,    KC_F4,    KC_F12,   BL,       KC_MPRV,  KC_MPLY,  KC_MNXT,  BR

/* ======================= would live in users/hydeik ======================= */
#ifdef FAMILY_HYDEIK

#define HRM(name) HM_##name /* HM_ASTR etc. stay defined as mod-taps here */
#define MOD(name) KC_##name

#define SYM_LAYER SYM_GRID(HRM, XXXXXXX),          _______, KC_SPC,  KC_ESC,  _______
#define NUM_LAYER NUM_GRID(HRM, XXXXXXX),          _______, KC_SPC,  _______, _______
#define FUN_LAYER FUN_GRID(MOD, QK_LLCK, XXXXXXX), _______, _______, _______, _______

#endif

/* ===================== would live in users/hydeik_vial ==================== */
#ifdef FAMILY_VIAL

#define PLAIN(name) KC_##name
#define COS(name) COS_##name

#define SYM_LAYER SYM_GRID(PLAIN, TO(_NAV)),       _______,   _______, _______, KC_ESC
#define NUM_LAYER NUM_GRID(PLAIN, TO(_NAV)),       KC_ESC,    _______, _______, _______
#define FUN_LAYER FUN_GRID(COS, KC_MSTP, _______), TO(_BASE), _______, _______, _______

#endif
