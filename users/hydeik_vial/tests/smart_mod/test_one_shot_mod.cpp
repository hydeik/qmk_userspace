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
#include "smart_mod.h"
#include "test_glue.h"
}

using testing::InSequence;

/*
 * The single one-shot mod rules. "Item N" refers to the numbered items of the
 * resolution of issue #25, "How should a single one-shot mod behave in
 * hydeik_vial?".
 *
 * A scan loop handles the key events at the current time and then advances
 * the time by 1 ms. `press()` / `release()` take effect in the next scan loop.
 */
class OneShotMod : public TestFixture {
  public:
    OneShotMod() {
        smart_mod_reset();
    }

    /* Press `key`, and release it `duration_ms` (at least 1) later. */
    void hold_for(KeymapKey& key, unsigned duration_ms) {
        key.press();
        run_one_scan_loop();
        idle_for(duration_ms - 1);
        key.release();
        run_one_scan_loop();
    }

    /* Tap `key` well within the One Shot Term. */
    void tap(KeymapKey& key) {
        hold_for(key, 1);
    }

    /* Tap `key`, which sends `code`, and expect that no modifier applies. */
    void expect_unmodified_tap(TestDriver& driver, KeymapKey& key, uint8_t code) {
        EXPECT_REPORT(driver, (code));
        EXPECT_EMPTY_REPORT(driver);
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }

    /*
     * Tap `key`, which sends `code`, and expect that it consumes the pending
     * one-shot mod `mod`: the modifier is registered at the key press and
     * dropped once the report of the key has been sent.
     */
    void expect_tap_consuming(TestDriver& driver, KeymapKey& key, uint8_t code, uint8_t mod) {
        EXPECT_REPORT(driver, (mod));
        EXPECT_REPORT(driver, (mod, code));
        EXPECT_REPORT(driver, (code));
        EXPECT_EMPTY_REPORT(driver);
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }
};

/* Item 1: a one-shot mod key is an ordinary modifier while it is held. */
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

/*
 * Item 3: a tapped one-shot mod is unregistered on release and becomes a
 * pending one-shot mod.
 * Item 4: the next key press consumes it, and it applies to that key only.
 */
TEST_F(OneShotMod, TapAppliesToTheNextKeyPressOnly) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       key_a          = KeymapKey(0, 1, 0, KC_A);

    set_keymap({one_shot_shift, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_A));
    EXPECT_REPORT(driver, (KC_A));
    key_a.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_REPORT(driver);
    key_a.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_A));
    key_a.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_REPORT(driver);
    key_a.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

/* Item 2: a tap released just before the One Shot Term ends becomes pending. */
TEST_F(OneShotMod, TapReleasedJustWithinTheOneShotTermBecomesPending) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_ctrl = KeymapKey(0, 0, 0, COS_LCTL);
    auto       key_a         = KeymapKey(0, 1, 0, KC_A);

    set_keymap({one_shot_ctrl, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    hold_for(one_shot_ctrl, SMART_MOD_ONE_SHOT_TERM - 1);
    VERIFY_AND_CLEAR(driver);

    expect_tap_consuming(driver, key_a, KC_A, KC_LEFT_CTRL);
}

/* Item 2: released when the One Shot Term has ended, nothing remains. */
TEST_F(OneShotMod, ReleasedAfterTheOneShotTermLeavesNothing) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_ctrl = KeymapKey(0, 0, 0, COS_LCTL);
    auto       key_a         = KeymapKey(0, 1, 0, KC_A);

    set_keymap({one_shot_ctrl, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    hold_for(one_shot_ctrl, SMART_MOD_ONE_SHOT_TERM);
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_a, KC_A);
}

/*
 * Item 2: released after another key was pressed while it was held, nothing
 * remains, however short the hold was.
 */
TEST_F(OneShotMod, ReleasedAfterAnotherKeyWasPressedLeavesNothing) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       key_a          = KeymapKey(0, 1, 0, KC_A);
    auto       key_b          = KeymapKey(0, 2, 0, KC_B);

    set_keymap({one_shot_shift, key_a, key_b});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    one_shot_shift.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_A));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    tap(key_a);
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_REPORT(driver);
    one_shot_shift.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_b, KC_B);
}
