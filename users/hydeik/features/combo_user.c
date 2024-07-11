#pragma once

#include QMK_KEYBOARD_H

#include "../definitions/layers.h"
#include "../definitions/keycodes.h"

enum combo_events {
    CM_M_COMM,
    CM_COMM_DOT,
    CM_UI,
    CM_IO,
};

const uint16_t PROGMEM combo_mins[] = {KC_M, KC_COMM, COMBO_END};
const uint16_t PROGMEM combo_unds[] = {KC_COMM, KC_DOT, COMBO_END};
const uint16_t PROGMEM combo_quot[] = {KC_U, KC_I, COMBO_END};
const uint16_t PROGMEM combo_dqt[] = {KC_I, KC_O, COMBO_END};

combo_t key_combos[] = {
    [CM_M_COMM] = COMBO(combo_mins, KC_MINS),
    [CM_COMM_DOT] = COMBO(combo_unds, KC_UNDS),
    [CM_UI] = COMBO(combo_quot, KC_QUOT),
    [CM_IO] = COMBO(combo_dqt, KC_DQT),
};
