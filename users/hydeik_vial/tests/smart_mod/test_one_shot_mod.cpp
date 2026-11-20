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

using testing::_;
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

    /*
     * Right after `tap()`, let the time pass so that the next key event comes
     * `ms` (at least 1) after the release of that tap.
     */
    void idle_until_after_tap(unsigned ms) {
        idle_for(ms - 1);
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

/*
 * Item 5: another one-shot mod key does not consume a pending one-shot mod.
 * Item 9: pending one-shot mods stack.
 */
TEST_F(OneShotMod, PendingOneShotModsStack) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_ctrl  = KeymapKey(0, 0, 0, COS_LCTL);
    auto       one_shot_shift = KeymapKey(0, 1, 0, COS_LSFT);
    auto       key_a          = KeymapKey(0, 2, 0, KC_A);

    set_keymap({one_shot_ctrl, one_shot_shift, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);

    /* The pending Ctrl is not in the reports of the Shift tap. */
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT, KC_A));
    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    tap(key_a);
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_a, KC_A);
}

/*
 * Item 5: a pending one-shot mod passes a held one-shot mod key, which is an
 * ordinary modifier (item 1), and is consumed by the key typed with it.
 */
TEST_F(OneShotMod, PendingOneShotModPassesAHeldOneShotModKey) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_ctrl  = KeymapKey(0, 0, 0, COS_LCTL);
    auto       one_shot_shift = KeymapKey(0, 1, 0, COS_LSFT);
    auto       key_a          = KeymapKey(0, 2, 0, KC_A);
    auto       key_b          = KeymapKey(0, 3, 0, KC_B);

    set_keymap({one_shot_ctrl, one_shot_shift, key_a, key_b});

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    one_shot_shift.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT, KC_A));
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

/*
 * Item 2, read strictly: "tap" means press and release with no other key in
 * between, and another one-shot mod key is such a key. Of two one-shot mod
 * keys whose presses overlap, only the one pressed and released inside the
 * other is a tap. The resolution of #25 does not spell this case out.
 */
TEST_F(OneShotMod, OneShotModKeyPressedInsideAnotherIsTheOnlyTap) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_ctrl  = KeymapKey(0, 0, 0, COS_LCTL);
    auto       one_shot_shift = KeymapKey(0, 1, 0, COS_LSFT);
    auto       key_a          = KeymapKey(0, 2, 0, KC_A);

    set_keymap({one_shot_ctrl, one_shot_shift, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    one_shot_ctrl.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_REPORT(driver);
    one_shot_ctrl.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    expect_tap_consuming(driver, key_a, KC_A, KC_LEFT_SHIFT);
}

/* Item 5: an ordinary modifier key does not consume a pending one-shot mod. */
TEST_F(OneShotMod, OrdinaryModifierKeyDoesNotConsume) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_ctrl = KeymapKey(0, 0, 0, COS_LCTL);
    auto       key_shift     = KeymapKey(0, 1, 0, KC_LEFT_SHIFT);
    auto       key_a         = KeymapKey(0, 2, 0, KC_A);

    set_keymap({one_shot_ctrl, key_shift, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    key_shift.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT, KC_A));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_A));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    tap(key_a);
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_REPORT(driver);
    key_shift.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

/*
 * Item 5, read as: a held mod-tap key is an ordinary modifier key, so it does
 * not consume a pending one-shot mod. The resolution of #25 names no mod-tap
 * key, and the hydeik_vial keymap family places none.
 */
TEST_F(OneShotMod, HeldModTapKeyDoesNotConsume) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_ctrl = KeymapKey(0, 0, 0, COS_LCTL);
    auto       mod_tap_shift = KeymapKey(0, 1, 0, LSFT_T(KC_Z));
    auto       key_a         = KeymapKey(0, 2, 0, KC_A);

    set_keymap({one_shot_ctrl, mod_tap_shift, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    mod_tap_shift.press();
    idle_for(TAPPING_TERM + 1);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT, KC_A));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_A));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    tap(key_a);
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_REPORT(driver);
    mod_tap_shift.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

/*
 * Item 5: a held layer-tap key only switches layers and does not consume a
 * pending one-shot mod; the key typed on its layer does.
 */
TEST_F(OneShotMod, HeldLayerTapKeyDoesNotConsume) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       layer_tap      = KeymapKey(0, 1, 0, LT(1, KC_TAB));
    auto       key_a          = KeymapKey(0, 2, 0, KC_A);
    auto       key_1          = KeymapKey(1, 2, 0, KC_1);

    set_keymap({one_shot_shift, layer_tap, key_a, key_1});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    layer_tap.press();
    idle_for(TAPPING_TERM + 1);
    VERIFY_AND_CLEAR(driver);

    expect_tap_consuming(driver, key_1, KC_1, KC_LEFT_SHIFT);

    EXPECT_NO_REPORT(driver);
    layer_tap.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

/* Item 5: a tapped layer-tap key consumes a pending one-shot mod. */
TEST_F(OneShotMod, TappedLayerTapKeyConsumes) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       layer_tap      = KeymapKey(0, 1, 0, LT(1, KC_TAB));
    auto       key_a          = KeymapKey(0, 2, 0, KC_A);

    set_keymap({one_shot_shift, layer_tap, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    expect_tap_consuming(driver, layer_tap, KC_TAB, KC_LEFT_SHIFT);

    expect_unmodified_tap(driver, key_a, KC_A);
}

/* Item 5: `TO(...)` only switches layers and does not consume. */
TEST_F(OneShotMod, ToLayerKeyDoesNotConsume) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       to_layer_1     = KeymapKey(0, 1, 0, TO(1));
    auto       key_1          = KeymapKey(1, 2, 0, KC_1);

    set_keymap({one_shot_shift, to_layer_1, key_1});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    tap(to_layer_1);
    VERIFY_AND_CLEAR(driver);

    expect_tap_consuming(driver, key_1, KC_1, KC_LEFT_SHIFT);
}

/* Item 5: `QK_LLCK` only switches layers and does not consume. */
TEST_F(OneShotMod, LayerLockKeyDoesNotConsume) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       layer_tap      = KeymapKey(0, 1, 0, LT(1, KC_TAB));
    auto       layer_lock     = KeymapKey(1, 2, 0, QK_LLCK);
    auto       key_1          = KeymapKey(1, 3, 0, KC_1);

    set_keymap({one_shot_shift, layer_tap, layer_lock, key_1});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    layer_tap.press();
    idle_for(TAPPING_TERM + 1);
    tap(layer_lock);
    layer_tap.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    /* Layer 1 is locked, and the pending Shift is still there. */
    expect_tap_consuming(driver, key_1, KC_1, KC_LEFT_SHIFT);
}

/*
 * Item 5: everything else consumes, including a custom keycode that sends no
 * key itself. The pending one-shot mod is registered for it and dropped.
 */
TEST_F(OneShotMod, CustomKeycodeConsumes) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       custom_key     = KeymapKey(0, 1, 0, COS_MOD);
    auto       key_a          = KeymapKey(0, 2, 0, KC_A);

    set_keymap({one_shot_shift, custom_key, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(custom_key);
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_a, KC_A);
}

/*
 * Items 4 and 5: a tap dance key consumes a pending one-shot mod, at its
 * press. QMK's tap dance remembers the modifiers of that moment and applies
 * them to the key it sends once the tap dance has finished, so the one-shot
 * mod reaches that key although it was dropped long before.
 */
TEST_F(OneShotMod, TapDanceKeyConsumes) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       tap_dance      = KeymapKey(0, 1, 0, TD(TEST_TAP_DANCE_X_Y));
    auto       key_a          = KeymapKey(0, 2, 0, KC_A);

    set_keymap({one_shot_shift, tap_dance, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(tap_dance);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_X));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    idle_for(TAPPING_TERM);
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_a, KC_A);
}

/*
 * Item 5: the output of a combo consumes a pending one-shot mod. The keys of
 * the combo send nothing themselves; `j` and `k` pressed together are `q`.
 */
TEST_F(OneShotMod, ComboOutputConsumes) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       key_j          = KeymapKey(0, 1, 0, KC_J);
    auto       key_k          = KeymapKey(0, 2, 0, KC_K);
    auto       key_a          = KeymapKey(0, 3, 0, KC_A);

    set_keymap({one_shot_shift, key_j, key_k, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_Q));
    EXPECT_REPORT(driver, (KC_Q));
    EXPECT_EMPTY_REPORT(driver);
    tap_combo({key_j, key_k});
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_a, KC_A);
}

/*
 * Items 4 and 5: a key that QMK handles without calling the after-report
 * hook, such as the Caps Word toggle, consumes a pending one-shot mod like
 * any other key. The one-shot mod is dropped in the same scan loop, so the
 * host does not go on seeing it.
 */
TEST_F(OneShotMod, KeyHandledWithoutTheAfterReportHookConsumes) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_ctrl    = KeymapKey(0, 0, 0, COS_LCTL);
    auto       caps_word_toggle = KeymapKey(0, 1, 0, CW_TOGG);
    auto       key_1            = KeymapKey(0, 2, 0, KC_1);

    set_keymap({one_shot_ctrl, caps_word_toggle, key_1});

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    caps_word_toggle.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    caps_word_toggle.release();
    run_one_scan_loop();
    /* Caps Word off again. */
    tap(caps_word_toggle);
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_1, KC_1);
}

/*
 * Item 6: pressing the same one-shot mod key again cancels its pending
 * one-shot mod, and that second press never becomes a one-shot, although it
 * is released well within the One Shot Term.
 */
TEST_F(OneShotMod, PressingTheSameKeyAgainCancels) {
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
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_a, KC_A);
}

/* Item 6: the second press cancels that pending one-shot mod only. */
TEST_F(OneShotMod, PressingTheSameKeyAgainKeepsTheOtherPendingOneShotMods) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_ctrl  = KeymapKey(0, 0, 0, COS_LCTL);
    auto       one_shot_shift = KeymapKey(0, 1, 0, COS_LSFT);
    auto       key_a          = KeymapKey(0, 2, 0, KC_A);

    set_keymap({one_shot_ctrl, one_shot_shift, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    expect_tap_consuming(driver, key_a, KC_A, KC_LEFT_CTRL);
}

/*
 * Item 6: held, the second press is an ordinary modifier, and it leaves
 * nothing behind on release.
 */
TEST_F(OneShotMod, SecondPressHeldIsAnOrdinaryModifier) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       key_a          = KeymapKey(0, 1, 0, KC_A);
    auto       key_b          = KeymapKey(0, 2, 0, KC_B);

    set_keymap({one_shot_shift, key_a, key_b});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    one_shot_shift.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_A));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    tap(key_a);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_B));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    tap(key_b);
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_REPORT(driver);
    one_shot_shift.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_a, KC_A);
}

/*
 * Named scenario "a tapped modifier is gone after the timeout".
 * Item 7: the timeout clears a pending one-shot mod, and the host sees
 * nothing of it. The letter comes exactly when the timeout ends.
 */
TEST_F(OneShotMod, TappedModifierIsGoneAfterTheTimeout) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       key_a          = KeymapKey(0, 1, 0, KC_A);

    set_keymap({one_shot_shift, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    idle_until_after_tap(SMART_MOD_ONE_SHOT_TIMEOUT);
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_a, KC_A);
}

/* Item 7: just before the timeout ends, the one-shot mod is still pending. */
TEST_F(OneShotMod, TappedModifierIsStillPendingJustBeforeTheTimeout) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       key_a          = KeymapKey(0, 1, 0, KC_A);

    set_keymap({one_shot_shift, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    idle_until_after_tap(SMART_MOD_ONE_SHOT_TIMEOUT - 1);
    VERIFY_AND_CLEAR(driver);

    expect_tap_consuming(driver, key_a, KC_A, KC_LEFT_SHIFT);
}

/*
 * Item 7: the timeout is counted from the last one-shot mod tap, for all
 * pending one-shot mods. Just before it ends, the earlier one is still
 * pending too, although its own tap is longer ago than the timeout.
 */
TEST_F(OneShotMod, TimeoutIsCountedFromTheLastOneShotModTap) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_ctrl  = KeymapKey(0, 0, 0, COS_LCTL);
    auto       one_shot_shift = KeymapKey(0, 1, 0, COS_LSFT);
    auto       key_a          = KeymapKey(0, 2, 0, KC_A);

    set_keymap({one_shot_ctrl, one_shot_shift, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    idle_until_after_tap(SMART_MOD_ONE_SHOT_TIMEOUT / 2);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    idle_until_after_tap(SMART_MOD_ONE_SHOT_TIMEOUT - 1);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT, KC_A));
    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    tap(key_a);
    VERIFY_AND_CLEAR(driver);
}

/* Item 7: the timeout clears all pending one-shot mods at once. */
TEST_F(OneShotMod, TimeoutClearsAllPendingOneShotMods) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_ctrl  = KeymapKey(0, 0, 0, COS_LCTL);
    auto       one_shot_shift = KeymapKey(0, 1, 0, COS_LSFT);
    auto       key_a          = KeymapKey(0, 2, 0, KC_A);

    set_keymap({one_shot_ctrl, one_shot_shift, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    idle_until_after_tap(SMART_MOD_ONE_SHOT_TIMEOUT);
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_a, KC_A);
}

/*
 * Item 7: a key that lets a pending one-shot mod pass (item 5) is not a
 * one-shot mod tap, so it does not restart the timeout.
 */
TEST_F(OneShotMod, KeyThatDoesNotConsumeDoesNotRestartTheTimeout) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       to_layer_1     = KeymapKey(0, 1, 0, TO(1));
    auto       to_layer_1_too = KeymapKey(1, 1, 0, TO(1));
    auto       key_1          = KeymapKey(1, 2, 0, KC_1);

    set_keymap({one_shot_shift, to_layer_1, to_layer_1_too, key_1});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    idle_until_after_tap(SMART_MOD_ONE_SHOT_TIMEOUT / 2);
    tap(to_layer_1);
    /* That tap took 2 ms. The next key event comes when the timeout ends. */
    idle_for(SMART_MOD_ONE_SHOT_TIMEOUT - SMART_MOD_ONE_SHOT_TIMEOUT / 2 - 2);
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_1, KC_1);
}

/*
 * Items 1 and 4: dropping a consumed one-shot mod does not release the same
 * modifier of an ordinary modifier key that is still held.
 */
TEST_F(OneShotMod, ConsumedOneShotModDoesNotReleaseAHeldOrdinaryModifier) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       key_shift      = KeymapKey(0, 1, 0, KC_LEFT_SHIFT);
    auto       key_a          = KeymapKey(0, 2, 0, KC_A);
    auto       key_b          = KeymapKey(0, 3, 0, KC_B);

    set_keymap({one_shot_shift, key_shift, key_a, key_b});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    key_shift.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_A));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    tap(key_a);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_B));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    tap(key_b);
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_REPORT(driver);
    key_shift.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

/*
 * Item 2: a one-shot mod key held for so long that the 16-bit key event time
 * has wrapped around is still not a tap.
 */
TEST_F(OneShotMod, ReleasedAfterTheKeyEventTimeWrappedAroundLeavesNothing) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       key_a          = KeymapKey(0, 1, 0, KC_A);

    set_keymap({one_shot_shift, key_a});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    hold_for(one_shot_shift, UINT16_MAX + 2);
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_a, KC_A);
}

/*
 * Rolled keys. Item 4: Shift tap, `t` down, `h` down gives `Th`: the pending
 * Shift is dropped once the report of `t` has been sent, before `h` is.
 */
TEST_F(OneShotMod, RolledKeysGetTheOneShotModOnTheFirstKeyOnly) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       key_t          = KeymapKey(0, 1, 0, KC_T);
    auto       key_h          = KeymapKey(0, 2, 0, KC_H);

    set_keymap({one_shot_shift, key_t, key_h});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_T));
    EXPECT_REPORT(driver, (KC_T));
    key_t.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_T, KC_H));
    key_h.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_H));
    key_t.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_REPORT(driver);
    key_h.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

/* Rolled keys, item 4: the same when both presses arrive in one scan loop. */
TEST_F(OneShotMod, KeysPressedInTheSameScanLoopGetTheOneShotModOnTheFirstKeyOnly) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_shift = KeymapKey(0, 0, 0, COS_LSFT);
    auto       key_t          = KeymapKey(0, 1, 0, KC_T);
    auto       key_h          = KeymapKey(0, 2, 0, KC_H);

    set_keymap({one_shot_shift, key_t, key_h});

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_shift);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_T));
    EXPECT_REPORT(driver, (KC_T));
    EXPECT_REPORT(driver, (KC_T, KC_H));
    key_t.press();
    key_h.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_H));
    EXPECT_EMPTY_REPORT(driver);
    key_t.release();
    key_h.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

/*
 * Named scenario "no report carries a modifier while a one-shot mod is
 * pending", for each of the four modifiers and both of their keys.
 */
struct PendingOneShotModCase {
    const char* name;
    /* The one-shot mod key that is tapped first, and the modifier it sends. */
    uint16_t one_shot_keycode;
    uint8_t  mod;
    /* The one-shot mod key that is tapped second, and the modifier it sends. */
    uint16_t second_one_shot_keycode;
    uint8_t  second_mod;
};

/* Lets googletest name a case in its output. */
void PrintTo(const PendingOneShotModCase& c, std::ostream* os) {
    *os << c.name;
}

class PendingOneShotMod : public ::testing::WithParamInterface<PendingOneShotModCase>, public OneShotMod {};

/*
 * Item 3: between the report that releases the tapped key and the key press
 * that consumes it, no report is sent at all.
 */
TEST_P(PendingOneShotMod, NoReportCarriesAModifierWhileAOneShotModIsPending) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_mod = KeymapKey(0, 0, 0, GetParam().one_shot_keycode);
    auto       key_a        = KeymapKey(0, 1, 0, KC_A);

    set_keymap({one_shot_mod, key_a});

    EXPECT_REPORT(driver, (GetParam().mod));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_mod);
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    idle_until_after_tap(SMART_MOD_ONE_SHOT_TIMEOUT - 1);
    VERIFY_AND_CLEAR(driver);

    expect_tap_consuming(driver, key_a, KC_A, GetParam().mod);
}

/*
 * Items 3 and 9: the same while a second one-shot mod is tapped.
 *
 * Read with items 1 and 11: the second one-shot mod key is an ordinary
 * modifier while it is held, so its own tap sends a lone modifier press and
 * release. What must not happen is that the pending one-shot mod shows up in
 * those reports, or in any other before the consuming key press. The ticket
 * words this as "no report with a modifier is sent, including when a second
 * one-shot mod is tapped", which taken literally contradicts item 1.
 */
TEST_P(PendingOneShotMod, NoReportCarriesThePendingModifierWhileASecondOneShotModIsTapped) {
    TestDriver driver;
    InSequence s;
    auto       one_shot_mod        = KeymapKey(0, 0, 0, GetParam().one_shot_keycode);
    auto       second_one_shot_mod = KeymapKey(0, 1, 0, GetParam().second_one_shot_keycode);
    auto       key_a               = KeymapKey(0, 2, 0, KC_A);

    set_keymap({one_shot_mod, second_one_shot_mod, key_a});

    EXPECT_REPORT(driver, (GetParam().mod));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_mod);
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    idle_until_after_tap(SMART_MOD_ONE_SHOT_TIMEOUT / 2);
    VERIFY_AND_CLEAR(driver);

    /* Only the modifier of the key that is held, never the pending one. */
    EXPECT_REPORT(driver, (GetParam().second_mod));
    EXPECT_EMPTY_REPORT(driver);
    tap(second_one_shot_mod);
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    idle_until_after_tap(SMART_MOD_ONE_SHOT_TIMEOUT - 1);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (GetParam().mod, GetParam().second_mod));
    EXPECT_REPORT(driver, (GetParam().mod, GetParam().second_mod, KC_A));
    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    tap(key_a);
    VERIFY_AND_CLEAR(driver);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    EachModifier,
    PendingOneShotMod,
    ::testing::Values(
        PendingOneShotModCase{"LeftCtrl",   COS_LCTL, KC_LEFT_CTRL,   COS_LSFT, KC_LEFT_SHIFT},
        PendingOneShotModCase{"LeftShift",  COS_LSFT, KC_LEFT_SHIFT,  COS_LALT, KC_LEFT_ALT},
        PendingOneShotModCase{"LeftAlt",    COS_LALT, KC_LEFT_ALT,    COS_LGUI, KC_LEFT_GUI},
        PendingOneShotModCase{"LeftGui",    COS_LGUI, KC_LEFT_GUI,    COS_LCTL, KC_LEFT_CTRL},
        PendingOneShotModCase{"RightCtrl",  COS_RCTL, KC_RIGHT_CTRL,  COS_RSFT, KC_RIGHT_SHIFT},
        PendingOneShotModCase{"RightShift", COS_RSFT, KC_RIGHT_SHIFT, COS_RALT, KC_RIGHT_ALT},
        PendingOneShotModCase{"RightAlt",   COS_RALT, KC_RIGHT_ALT,   COS_RGUI, KC_RIGHT_GUI},
        PendingOneShotModCase{"RightGui",   COS_RGUI, KC_RIGHT_GUI,   COS_RCTL, KC_RIGHT_CTRL}
    ),
    [](const ::testing::TestParamInfo<PendingOneShotModCase>& info) { return std::string(info.param.name); }
);
// clang-format on
