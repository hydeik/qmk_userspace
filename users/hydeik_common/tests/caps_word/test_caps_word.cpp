/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * The Caps Word rule of the common core (caps_word_press_user in common.c):
 * which keys continue Caps Word, and which of them are shifted.
 *
 * This file must not branch on the keymap family or the firmware source.
 */

#include "keyboard_report_util.hpp"
#include "keycode.h"
#include "test_common.hpp"
#include "test_fixture.hpp"
#include "test_keymap_key.hpp"

using testing::AnyNumber;
using testing::AnyOf;
using testing::InSequence;

class CapsWord : public TestFixture {
  public:
    KeymapKey key_a{0, 0, 0, KC_A};
    KeymapKey key_minus{0, 1, 0, KC_MINS};
    KeymapKey key_dot{0, 2, 0, KC_DOT};

    void SetUp() override {
        caps_word_off();
        set_keymap({key_a, key_minus, key_dot});
    }

    /* Reports with no key, or with Shift alone, may come at any time. */
    void allow_reports_without_keys(TestDriver& driver) {
        EXPECT_CALL(driver, send_keyboard_mock(AnyOf(KeyboardReport(), KeyboardReport(KC_LSFT)))).Times(AnyNumber());
    }
};

/*
 * A minus continues Caps Word and is typed as it is, where the firmware
 * source's own rule would shift it into an underscore.
 */
TEST_F(CapsWord, MinusContinuesCapsWordWithoutShift) {
    TestDriver driver;

    allow_reports_without_keys(driver);
    {
        InSequence s;
        EXPECT_REPORT(driver, (KC_LSFT, KC_A));
        EXPECT_REPORT(driver, (KC_MINS));
        EXPECT_REPORT(driver, (KC_LSFT, KC_A));
    }

    caps_word_on();
    tap_keys(key_a, key_minus, key_a);

    EXPECT_TRUE(is_caps_word_on());
    VERIFY_AND_CLEAR(driver);
}

/* A key that the rule does not list ends Caps Word. */
TEST_F(CapsWord, DotEndsCapsWord) {
    TestDriver driver;

    allow_reports_without_keys(driver);
    {
        InSequence s;
        EXPECT_REPORT(driver, (KC_LSFT, KC_A));
        EXPECT_REPORT(driver, (KC_DOT));
        EXPECT_REPORT(driver, (KC_A));
    }

    caps_word_on();
    tap_keys(key_a, key_dot, key_a);

    EXPECT_FALSE(is_caps_word_on());
    VERIFY_AND_CLEAR(driver);
}
