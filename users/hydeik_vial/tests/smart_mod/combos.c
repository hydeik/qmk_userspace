/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * The combos of the test keymap. In the firmware build of the hydeik_vial
 * keymap family they come from Vial instead. This file is included into the
 * keymap of the test build (see test_keymap_tables.c).
 *
 * The keys of a combo are held back for the combo term, so a combo must be
 * made of keycodes that no other test uses.
 */

#include "quantum.h"
#include "test_glue.h"

static const uint16_t PROGMEM test_combo_j_k[] = {KC_J, KC_K, COMBO_END};

combo_t key_combos[] = {
    [TEST_COMBO_J_K_Q] = COMBO(test_combo_j_k, KC_Q),
};
