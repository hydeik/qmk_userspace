/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "keyboard_report_util.hpp"
#include "keycode.h"
#include "test_common.hpp"
#include "test_fixture.hpp"
#include "test_keymap_key.hpp"

extern "C" {
#include "test_glue.h"
}

using testing::InSequence;

class OneShotMod : public TestFixture {};

TEST_F(OneShotMod, IsAnOrdinaryModifierWhileHeld) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       key_a          = KeymapKey(0, 1, 0, KC_A);

    set_keymap({one_shot_shift, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    one_shot_shift.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_A));
    key_a.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    key_a.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_REPORT(driver);
    one_shot_shift.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}
