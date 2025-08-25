#pragma once

#include QMK_KEYBOARD_H

/****************************************************************************
 * Layer numbers
 ****************************************************************************/
enum layer_t {
    _BASE,
    _SYM,
    _NUM,
    _NAV,
    _MOU,
    _FUN,
};

/****************************************************************************
 * Custom keycodes
 ****************************************************************************/
enum custom_keycodes {
    UPDIR = QK_USER,
    /* --- Macros invoked through the Magic key. */
    M_DOCSTR,
    M_EQEQ,
    M_INCLUDE,
    M_MKGRVS,
    M_UPDIR,
    M_NOOP,
};

enum keycode_aliases {
    /* mod-tap keys */
    SYM_TAB = LT(_SYM, KC_TAB),
    NAV_SPC = LT(_NAV, KC_SPC),
    SFT_ENT = LSFT_T(KC_ENT),
    NUM_BSPC = LT(_NUM, KC_BSPC),
    /* for home-row mods (BASE layer) */
    HM_A = LCTL_T(KC_A),
    HM_S = LALT_T(KC_S),
    HM_D = LGUI_T(KC_D),
    HM_F = LSFT_T(KC_F),
    HM_J = RSFT_T(KC_J),
    HM_K = RGUI_T(KC_K),
    HM_L = RALT_T(KC_L),
    HM_SCLN = RCTL_T(KC_SCLN),
    /* for home-row mods (SYM layer) */
    HM_ASTR = LCTL_T(KC_KP_0),
    HM_LPRN = LALT_T(KC_KP_0),
    HM_RPRN = LGUI_T(KC_KP_0),
    HM_COLN = LSFT_T(KC_KP_0),
    HM_DQUO = RSFT_T(KC_KP_1),
    HM_LBRC = RGUI_T(KC_LBRC),
    HM_RBRC = RALT_T(KC_RBRC),
    /* for home-row mods (NUM layer) */
    HM_DOT = LCTL_T(KC_DOT),
    HM_1 = LALT_T(KC_1),
    HM_2 = LGUI_T(KC_2),
    HM_3 = LSFT_T(KC_3),
    HM_QUOT = RSFT_T(KC_QUOT),
    HM_UNDS = RGUI_T(KC_KP_2),
    HM_EQL = RALT_T(KC_EQL),
    /* Clipboard */
#if defined (HYDEIK_CLIPBOARD_FUN)
    M_REDO = KC_AGAIN,
    M_UNDO = KC_UNDO,
    M_CUT = KC_CUT,
    M_COPY = KC_COPY,
    M_PSTE = KC_PSTE,
#elif defined (HYDEIK_CLIPBOARD_MAC)
    M_REDO = SCMD(KC_Z),
    M_UNDO = LCMD(KC_Z),
    M_CUT = LCMD(KC_X),
    M_COPY = LCMD(KC_C),
    M_PSTE = LCMD(KC_V),
#elif defined (HYDEIK_CLIPBOARD_WIN)
    M_REDO = C(KC_Y),
    M_UNDO = C(KC_Z),
    M_CUT = C(KC_X),
    M_COPY = C(KC_C),
    M_PSTE = C(KC_V),
#else
    M_REDO = KC_AGAIN,
    M_UNDO = KC_UNDO,
    M_CUT = S(KC_DEL),
    M_COPY = C(KC_INS),
    M_PSTE = S(KC_INS),
#endif
};

/****************************************************************************
 * Keymap blocks
 ****************************************************************************/

/* clang-format off */

/*
 * QWERTY layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | Q       | W       | E       | R       | T       |   | Y       | U       | I       | O       | P       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | A / CTL | S / ALT | D / GUI | F / SFT | G       |   | H       | J / SFT | K / GUI | L / ALT | ; / CTL |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | Z       | X       | C       | V       | B       |   | N       | M       | ,       | .       | /       |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | TAB/SYM | SPC/NAV |   | ENT/SFT | BSPC/NUM|
 *                               +---------+---------+   +---------+---------+
 */

#define BASE_LAYER \
    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    \
    HM_A,    HM_S,    HM_D,    HM_F,    KC_G,    KC_H,    HM_J,    HM_K,    HM_L,    HM_SCLN, \
    KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, \
                               SYM_TAB, NAV_SPC, NUM_BSPC, SFT_ENT

/*
 * symbol layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | \       | %       | $       |         |   |         | ^       | {       | }       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | * / CTL | ( / ALT | ) / GUI | : / SFT | DEL     |   | BSPC    | " / SFT | [ / GUI | ] / ALT | ; / CTL |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | `       | @       | &       | LLCK    |   |         | #       | <       | >       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | _SYM_   | SPC     |   | ENT     | ESC     |
 *                               +---------+---------+   +---------+---------+
 */

#define SYM_LAYER \
    XXXXXXX, KC_BSLS, KC_PERC, KC_DLR,  XXXXXXX, XXXXXXX, KC_CIRC, KC_LCBR, KC_RCBR, XXXXXXX, \
    HM_ASTR, HM_LPRN, HM_RPRN, HM_COLN, KC_DEL,  KC_BSPC, HM_DQUO, HM_LBRC, HM_RBRC, HM_SCLN, \
    XXXXXXX, KC_GRV,  KC_AT,   KC_AMPR, QK_LLCK, XXXXXXX, KC_HASH, KC_LT,   KC_GT,   XXXXXXX, \
                               _______, KC_SPC,  KC_ENT,  KC_ESC


/*
 * Number layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | 4       | 5       | 6       |         |   |         | |       | -       | /       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | . / CTL | 1 / ALT | 2 / GUI | 3 / SFT | DEL     |   | BSPC    | ' / SFT | _ / GUI | = / ALT | * / CTL |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | ~       | 7       | 8       | 9       |         |   | LLCK    | +       | ?       | !       |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               | 0       | SPC     |   | ENT     | _NUM_   |
 *                               +---------+---------+   +---------+---------+
 */

#define NUM_LAYER \
    XXXXXXX, KC_4,    KC_5,    KC_6,    XXXXXXX, XXXXXXX, KC_PIPE, KC_MINS, KC_SLSH, XXXXXXX, \
    HM_DOT,  HM_1,    HM_2,    HM_3,    KC_DEL,  KC_BSPC, HM_QUOT, HM_UNDS, HM_EQL,  HM_ASTR, \
    KC_TILD, KC_7,    KC_8,    KC_9,    XXXXXXX, QK_LLCK, KC_PLUS, KC_QUES, KC_EXLM, XXXXXXX, \
                               KC_0,    KC_SPC,  KC_ENT,  _______

/*
 * Navigation layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   | REDO    | PASTE   | COPY    | CUT     | UNDO    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | CTL     | ALT     | GUI     | SFT     | -MOU-   |   | LEFT    | DOWN    | UP      | RGHT    | CWORD   |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         | LLCK    |   | HOME    | PGDN    | PGUP    | END     | INS     |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | _NAV_   |   | -FUN-   | ESC     |
 *                               +---------+---------+   +---------+---------+
 */

#define NAV_LAYER \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,  M_REDO,  M_PSTE,  M_COPY,  M_CUT,   M_UNDO,  \
    KC_LCTL, KC_LALT, KC_LGUI, KC_LSFT, MO(_MOU), KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, CW_TOGG, \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, QK_LLCK,  KC_HOME, KC_PGDN, KC_PGUP, KC_END,  KC_INS,  \
                               _______, _______,  MO(_FUN), KC_ESC

/*
 * Mouse layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         |         |   | REDO    | PASTE   | COPY    | CUT     | UNDO    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | CTL     | ALT     | GUI     | SFT     | _MOU_   |   | MS_LEFT | MS_DOWN | MS_UP   | MS_RGHT | BTN3    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         |         |         |         | LLCK    |   | MS_WHLL | MS_WHLD | MS_WHLU | MS_WHLR |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | _NAV_   |   | BTN1    | BTN2    |
 *                               +---------+---------+   +---------+---------+
 */

#define MOUSE_LAYER \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, M_REDO,  M_PSTE,  M_COPY,  M_CUT,   M_UNDO,  \
    KC_LCTL, KC_LALT, KC_LGUI, KC_LSFT, _______, MS_LEFT, MS_DOWN, MS_UP,   MS_RGHT, MS_BTN3, \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, QK_LLCK, MS_WHLL, MS_WHLD, MS_WHLU, MS_WHLR, XXXXXXX, \
                               _______, _______, MS_BTN1, MS_BTN2

/*
 * Function layer
 *
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * | BOOT    | F6      | F5      | F4      | F10     |   | MUTE    | VOLD    | VOLU    | BRID    | BRIU    |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | F3      | F2      | F1      | F11     |   |         | SFT     | GUI     | ALT     | CTL     |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 * |         | F9      | F8      | F7      | F12     |   | LLCK    | MPRV    | MPLY    | MNXT    |         |
 * +---------+---------+---------+---------+---------+   +---------+---------+---------+---------+---------+
 *                               |         | _FUN_   |   | _FUN_   |         |
 *                               +---------+---------+   +---------+---------+
 */

#define FUN_LAYER \
    QK_BOOT, KC_F6,   KC_F5,   KC_F4,   KC_F10,  KC_MUTE, KC_VOLD, KC_VOLU, KC_BRID, KC_BRIU, \
    XXXXXXX, KC_F3,   KC_F2,   KC_F1,   KC_F11,  XXXXXXX, KC_RSFT, KC_RGUI, KC_RALT, KC_RCTL, \
    XXXXXXX, KC_F9,   KC_F8,   KC_F7,   KC_F12,  QK_LLCK, KC_MPRV, KC_MPLY, KC_MNXT, XXXXXXX, \
                               _______, _______, _______, _______


/****************************************************************************
 * Layout macros
 ****************************************************************************/

#define LAYOUT_hydeik_split_3x5_2( \
        L01, L02, L03, L04, L05, R01, R02, R03, R04, R05, \
        L06, L07, L08, L09, L10, R06, R07, R08, R09, R10, \
        L11, L12, L13, L14, L15, R11, R12, R13, R14, R15, \
                       L16, L17, R16, R17                 \
    ) \
    LAYOUT_split_3x5_2( \
        L01, L02, L03, L04, L05, R01, R02, R03, R04, R05, \
        L06, L07, L08, L09, L10, R06, R07, R08, R09, R10, \
        L11, L12, L13, L14, L15, R11, R12, R13, R14, R15, \
                       L16, L17, R16, R17                 \
    )

#define LAYOUT_hydeik_split_3x5_3( \
        l01, l02, l03, l04, l05, r01, r02, r03, r04, r05, \
        l06, l07, l08, l09, l10, r06, r07, r08, r09, r10, \
        l11, l12, l13, l14, l15, r11, r12, r13, r14, r15, \
                       l16, l17, r16, r17                 \
    ) \
    LAYOUT_split_3x5_3( \
        L01, L02, L03, L04, L05, R01, R02, R03, R04, R05, \
        L06, L07, L08, L09, L10, R06, R07, R08, R09, R10, \
        L11, L12, L13, L14, L15, R11, R12, R13, R14, R15, \
                KC_NO, L16, L17, R16, R17, KC_NO          \
    )

#define LAYOUT_hydeik_split_3x6_3( \
        l01, l02, l03, l04, l05, r01, r02, r03, r04, r05, \
        l06, l07, l08, l09, l10, r06, r07, r08, r09, r10, \
        l11, l12, l13, l14, l15, r11, r12, r13, r14, r15, \
                       l16, l17, r16, r17                 \
    ) \
    LAYOUT_split_3x6_3( \
        KC_NO, l01, l02, l03, l04, l05, r01, r02, r03, r04, r05, KC_NO, \
        KC_NO, l06, l07, l08, l09, l10, r06, r07, r08, r09, r10, KC_NO, \
        KC_NO, l11, l12, l13, l14, l15, r11, r12, r13, r14, r15, KC_NO, \
                       KC_NO, l16, l17, r16, r17, KC_NO                 \
    )


#define LAYOUT_wrapper_split_3x6_4(...) LAYOUT(__VA_ARGS__)
#define LAYOUT_hydeik_split_3x6_4( \
        l01, l02, l03, l04, l05, r01, r02, r03, r04, r05, \
        l06, l07, l08, l09, l10, r06, r07, r08, r09, r10, \
        l11, l12, l13, l14, l15, r11, r12, r13, r14, r15, \
                       l16, l17, r16, r17                 \
    ) \
    LAYOUT_wrapper_split_3x6_4( \
        KC_NO, l01, l02, l03, l04, l05, r01, r02, r03, r04, r05, KC_NO, \
        KC_NO, l06, l07, l08, l09, l10, r06, r07, r08, r09, r10, KC_NO, \
        KC_NO, l11, l12, l13, l14, l15, r11, r12, r13, r14, r15, KC_NO, \
                KC_NO, l16, l17, KC_NO, KC_NO, r16, r17, KC_NO          \
    )

#define LAYOUT_hydeik_split_3x6_3_ex2( \
        l01, l02, l03, l04, l05, r01, r02, r03, r04, r05, \
        l06, l07, l08, l09, l10, r06, r07, r08, r09, r10, \
        l11, l12, l13, l14, l15, r11, r12, r13, r14, r15, \
                       l16, l17, r16, r17                 \
    ) \
    LAYOUT_split_3x6_3_ex2( \
        KC_NO, l01, l02, l03, l04, l05, KC_NO, KC_NO, r01, r02, r03, r04, r05, KC_NO, \
        KC_NO, l06, l07, l08, l09, l10, KC_NO, KC_NO, r06, r07, r08, r09, r10, KC_NO, \
        KC_NO, l11, l12, l13, l14, l15,               r11, r12, r13, r14, r15, KC_NO, \
                       KC_NO, l16, l17,               r16, r17, KC_NO                 \
    )

/* clang-format on */

/****************************************************************************
 * Prototypes for keymap specific customized functions.
 ****************************************************************************/
bool process_record_keymap(uint16_t keycode, keyrecord_t *record);
// void matrix_init_keymap(void);
void matrix_scan_keymap(void);
void keyboard_post_init_keymap(void);
layer_state_t layer_state_set_keymap(layer_state_t state);



