/*
 * PROTOTYPE - throwaway. Not built by any keymap.
 *
 * Variant C: shared five-key blocks, family assembles the layer.
 *
 * The shared file owns only the half-rows that are identical in both keymap
 * families. Each family writes its own home row, corner keys and thumbs, and
 * stitches the layer together. No parameters, no name mapping; the price is
 * that the home-row symbols are written once per family.
 */

/* ======================= would live in the shared file ==================== */

#define SYM_L1   XXXXXXX,  KC_BSLS,  KC_PERC,  KC_DLR,   XXXXXXX
#define SYM_R1   XXXXXXX,  KC_CIRC,  KC_LCBR,  KC_RCBR,  XXXXXXX
#define SYM_L3   XXXXXXX,  KC_GRV,   KC_AT,    KC_AMPR,  QK_LLCK
#define SYM_R3_4 XXXXXXX,  KC_HASH,  KC_LT,    KC_GT /* four keys: the corner is the family's */

#define NUM_L1   XXXXXXX,  KC_4,     KC_5,     KC_6,     XXXXXXX
#define NUM_R1   XXXXXXX,  KC_PIPE,  KC_MINS,  KC_SLSH,  XXXXXXX
#define NUM_L3   KC_TILD,  KC_7,     KC_8,     KC_9,     KC_DOT
#define NUM_R3_4 QK_LLCK,  KC_PLUS,  KC_QUES,  KC_EXLM

#define FUN_L1   QK_BOOT,  KC_F9,    KC_F8,    KC_F7,    KC_F10
#define FUN_R1   KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_BRID,  KC_BRIU
#define FUN_L2   XXXXXXX,  KC_F3,    KC_F2,    KC_F1,    KC_F11
#define FUN_L3   XXXXXXX,  KC_F6,    KC_F5,    KC_F4,    KC_F12
#define FUN_R3_3 KC_MPRV,  KC_MPLY,  KC_MNXT /* three keys: both ends are the family's */

/* ======================= would live in users/hydeik ======================= */
#ifdef FAMILY_HYDEIK

#define SYM_LAYER \
    SYM_L1,                                           SYM_R1,                                            \
    HM_ASTR,  HM_LPRN,  HM_RPRN,  HM_COLN,  KC_DEL,   KC_BSPC,  HM_DQUO,  HM_LBRC,  HM_RBRC,  HM_SCLN, \
    SYM_L3,                                           SYM_R3_4,                               XXXXXXX, \
                                  _______,  KC_SPC,   KC_ESC,   _______

#define NUM_LAYER \
    NUM_L1,                                           NUM_R1,                                            \
    HM_0,     HM_1,     HM_2,     HM_3,     KC_DEL,   KC_BSPC,  HM_QUOT,  HM_UNDS,  HM_EQL,   HM_ASTR, \
    NUM_L3,                                           NUM_R3_4,                               XXXXXXX, \
                                  _______,  KC_SPC,   _______,  _______

#define FUN_LAYER \
    FUN_L1,                                           FUN_R1,                                            \
    FUN_L2,                                           XXXXXXX,  KC_RSFT,  KC_RGUI,  KC_RALT,  KC_RCTL, \
    FUN_L3,                                           QK_LLCK,  FUN_R3_3,                     XXXXXXX, \
                                  _______,  _______,  _______,  _______

#endif

/* ===================== would live in users/hydeik_vial ==================== */
#ifdef FAMILY_VIAL

#define SYM_LAYER \
    SYM_L1,                                           SYM_R1,                                            \
    KC_ASTR,  KC_LPRN,  KC_RPRN,  KC_COLN,  KC_DEL,   KC_BSPC,  KC_DQUO,  KC_LBRC,  KC_RBRC,  KC_SCLN, \
    SYM_L3,                                           SYM_R3_4,                               TO(_NAV), \
                                  _______,  _______,  _______,  KC_ESC

#define NUM_LAYER \
    NUM_L1,                                           NUM_R1,                                            \
    KC_0,     KC_1,     KC_2,     KC_3,     KC_DEL,   KC_BSPC,  KC_QUOT,  KC_UNDS,  KC_EQL,   KC_ASTR, \
    NUM_L3,                                           NUM_R3_4,                               TO(_NAV), \
                                  KC_ESC,   _______,  _______,  _______

#define FUN_LAYER \
    FUN_L1,                                           FUN_R1,                                            \
    FUN_L2,                                           XXXXXXX,  COS_RSFT, COS_RGUI, COS_RALT, COS_RCTL, \
    FUN_L3,                                           KC_MSTP,  FUN_R3_3,                     _______, \
                                  TO(_BASE), _______, _______,  _______

#endif
