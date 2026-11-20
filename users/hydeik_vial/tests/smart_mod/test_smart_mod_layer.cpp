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
#include "caps_word.h"
#include "smart_mod.h"
#include "test_glue.h"
}

using testing::_;
using testing::InSequence;

/*
 * The smart mod layer rules. "Item N" refers to the numbered items of the
 * resolution of issue #26, "How does the smart mod layer work on top of the
 * new one-shot mods?".
 *
 * A scan loop handles the key events at the current time and then advances
 * the time by 1 ms. `press()` / `release()` take effect in the next scan loop.
 *
 * The keymap is that of the hydeik_vial keymap family in small. Keys that
 * share a position are the same physical key: which of them a press reaches
 * depends on the layers that are on. So pressing `one_shot_shift` types `f`
 * when the smart mod layer is off.
 *
 *            0        1        2        3     4     5     6    7
 *   base     SMT_MOD  SPC_MOD  ENT_MOD  f     a     b     c    TO(nav)
 *   nav      .        .        .        Sft*  Ctl*  Left  .    TO(base)
 *   mod      .        .        .        Sft*  Ctl*  .     .    .
 *
 * `Sft*` and `Ctl*` are one-shot mod keys; `.` is transparent. `mod` is the
 * smart mod layer.
 */
class SmartModLayer : public TestFixture {
  public:
    KeymapKey smt_mod{TEST_LAYER_BASE, 0, 0, SMT_MOD};
    KeymapKey spc_mod{TEST_LAYER_BASE, 1, 0, SPC_MOD};
    KeymapKey ent_mod{TEST_LAYER_BASE, 2, 0, ENT_MOD};
    KeymapKey key_f{TEST_LAYER_BASE, 3, 0, KC_F};
    KeymapKey key_a{TEST_LAYER_BASE, 4, 0, KC_A};
    KeymapKey key_b{TEST_LAYER_BASE, 5, 0, KC_B};
    KeymapKey key_c{TEST_LAYER_BASE, 6, 0, KC_C};
    KeymapKey to_nav{TEST_LAYER_BASE, 7, 0, TO(TEST_LAYER_NAV)};

    KeymapKey nav_one_shot_shift{TEST_LAYER_NAV, 3, 0, COS_LSFT};
    KeymapKey nav_one_shot_ctrl{TEST_LAYER_NAV, 4, 0, COS_LCTL};
    KeymapKey nav_left{TEST_LAYER_NAV, 5, 0, KC_LEFT};
    KeymapKey to_base{TEST_LAYER_NAV, 7, 0, TO(TEST_LAYER_BASE)};

    KeymapKey one_shot_shift{TEST_LAYER_MOD, 3, 0, COS_LSFT};
    KeymapKey one_shot_ctrl{TEST_LAYER_MOD, 4, 0, COS_LCTL};

    SmartModLayer() {
        smart_mod_reset();
        for (KeymapKey key : {smt_mod, spc_mod, ent_mod, key_f, key_a, key_b, key_c, to_nav}) {
            add_key(key);
        }
        for (KeymapKey key : {nav_one_shot_shift, nav_one_shot_ctrl, nav_left, to_base}) {
            add_key(key);
        }
        for (KeymapKey key : {one_shot_shift, one_shot_ctrl}) {
            add_key(key);
        }
        for (uint8_t col : {0, 1, 2, 6}) {
            add_key(KeymapKey(TEST_LAYER_NAV, col, 0, KC_TRANSPARENT));
        }
        for (uint8_t col : {0, 1, 2, 5, 6, 7}) {
            add_key(KeymapKey(TEST_LAYER_MOD, col, 0, KC_TRANSPARENT));
        }
    }

    ~SmartModLayer() {
        /* Caps Word outlives the clean-up of the test fixture otherwise. */
        caps_word_off();
    }

    /* Press `key`, and release it `duration_ms` (at least 1) later. */
    void hold_for(KeymapKey& key, unsigned duration_ms) {
        key.press();
        run_one_scan_loop();
        idle_for(duration_ms - 1);
        key.release();
        run_one_scan_loop();
    }

    /* Tap `key`: press it, and release it 1 ms later. */
    void tap(KeymapKey& key) {
        hold_for(key, 1);
    }

    /*
     * Right after `tap()`, let the time pass so that the next key event comes
     * `ms` (at least 2) after the press of that tap.
     */
    void idle_until_after_press_of_tap(unsigned ms) {
        idle_for(ms - 2);
    }

    /*
     * Press the smart mod layer key `key` and keep it down until QMK has
     * settled that it is held. Nothing is sent to the host.
     */
    void hold(TestDriver& driver, KeymapKey& key) {
        EXPECT_NO_REPORT(driver);
        key.press();
        idle_for(TAPPING_TERM + 1);
        VERIFY_AND_CLEAR(driver);
    }

    /* Release `key` and expect the empty report: no modifier remains. */
    void expect_release_leaving_no_modifier(TestDriver& driver, KeymapKey& key) {
        EXPECT_EMPTY_REPORT(driver);
        key.release();
        run_one_scan_loop();
        VERIFY_AND_CLEAR(driver);
    }

    /* Release `key` and expect that nothing is sent to the host. */
    void expect_silent_release(TestDriver& driver, KeymapKey& key) {
        EXPECT_NO_REPORT(driver);
        key.release();
        run_one_scan_loop();
        VERIFY_AND_CLEAR(driver);
    }

    /* Tap `key`, which sends `code`, and expect that no modifier applies. */
    void expect_unmodified_tap(TestDriver& driver, KeymapKey& key, uint8_t code) {
        EXPECT_REPORT(driver, (code));
        EXPECT_EMPTY_REPORT(driver);
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }

    /*
     * Tap `key`, which sends `code`, and expect that the registered modifier
     * `mod` applies to it and stays registered afterwards.
     */
    void expect_tap_with_registered(TestDriver& driver, KeymapKey& key, uint8_t code, uint8_t mod) {
        EXPECT_REPORT(driver, (mod, code));
        EXPECT_REPORT(driver, (mod));
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

    /*
     * With a smart mod layer key held and the smart mod layer on, tap the
     * one-shot mod key `key` of the modifier `mod`, which latches it: the
     * modifier is registered at the press and stays so after the release.
     */
    void latch(TestDriver& driver, KeymapKey& key, uint8_t mod) {
        EXPECT_REPORT(driver, (mod));
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }
};

/****************************************************************************
 * Entering and latching
 ****************************************************************************/

/* The three smart mod layer keys of the keymap. */
struct SmartModLayerKeyCase {
    const char* name;
    uint8_t     column;
};

/* Lets googletest name a case in its output. */
void PrintTo(const SmartModLayerKeyCase& c, std::ostream* os) {
    *os << c.name;
}

class EachSmartModLayerKey : public ::testing::WithParamInterface<SmartModLayerKeyCase>, public SmartModLayer {
  public:
    KeymapKey& smart_mod_layer_key() {
        KeymapKey* keys[] = {&smt_mod, &spc_mod, &ent_mod};
        return *keys[GetParam().column];
    }
};

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    SmartModLayer,
    EachSmartModLayerKey,
    ::testing::Values(
        SmartModLayerKeyCase{"SmtMod", 0},
        SmartModLayerKeyCase{"SpcMod", 1},
        SmartModLayerKeyCase{"EntMod", 2}
    ),
    [](const ::testing::TestParamInfo<SmartModLayerKeyCase>& info) { return std::string(info.param.name); }
);
// clang-format on

/*
 * Item 1: holding a smart mod layer key turns the smart mod layer on, so the
 * home-row key reaches the one-shot mod key that lies over it.
 */
TEST_P(EachSmartModLayerKey, HeldTurnsTheSmartModLayerOn) {
    TestDriver driver;
    InSequence s;

    hold(driver, smart_mod_layer_key());

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    one_shot_shift.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    one_shot_shift.release();
    run_one_scan_loop();
    expect_release_leaving_no_modifier(driver, smart_mod_layer_key());
}

/*
 * Item 2: a one-shot mod pressed and released on the smart mod layer with no
 * other key in between becomes a latched one-shot mod.
 * Item 6: releasing the smart mod layer key releases it.
 */
TEST_F(SmartModLayer, OneShotModPressedAndReleasedThereLatches) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    one_shot_shift.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    /* The modifier stays registered: the host is told nothing. */
    expect_silent_release(driver, one_shot_shift);

    expect_tap_with_registered(driver, key_b, KC_B, KC_LEFT_SHIFT);

    expect_release_leaving_no_modifier(driver, spc_mod);

    expect_unmodified_tap(driver, key_b, KC_B);
}

/*
 * Item 2: it latches whatever the length of the press; the One Shot Term and
 * the release window do not matter here.
 */
TEST_F(SmartModLayer, OneShotModLatchesWhateverTheLengthOfThePress) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    hold_for(one_shot_shift, SMART_MOD_ONE_SHOT_TERM + SMART_MOD_LAYER_RELEASE_WINDOW);
    VERIFY_AND_CLEAR(driver);

    expect_tap_with_registered(driver, key_b, KC_B, KC_LEFT_SHIFT);

    expect_release_leaving_no_modifier(driver, spc_mod);
}

/* Item 2: several one-shot mods are chosen by overlapping their presses. */
TEST_F(SmartModLayer, SeveralOneShotModsAreLatchedByOverlappingTheirPresses) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    one_shot_ctrl.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    one_shot_shift.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    expect_silent_release(driver, one_shot_ctrl);
    expect_silent_release(driver, one_shot_shift);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT, KC_B));
    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    tap(key_b);
    VERIFY_AND_CLEAR(driver);

    /* Item 6: every latched one-shot mod is released. */
    expect_release_leaving_no_modifier(driver, spc_mod);

    expect_unmodified_tap(driver, key_b, KC_B);
}

/*
 * Item 3: when all one-shot mod keys are released the smart mod layer
 * switches itself off, although the smart mod layer key is still held: the
 * home-row keys beneath are letters again, and no modifier can be added.
 */
TEST_F(SmartModLayer, SwitchesItselfOffOnceAllOneShotModKeysAreReleased) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);
    latch(driver, one_shot_shift, KC_LEFT_SHIFT);

    /* The keys of the two one-shot mod keys, pressed again. */
    expect_tap_with_registered(driver, key_f, KC_F, KC_LEFT_SHIFT);
    expect_tap_with_registered(driver, key_a, KC_A, KC_LEFT_SHIFT);

    expect_release_leaving_no_modifier(driver, spc_mod);
}

/*
 * Item 3: the smart mod layer stays on as long as one of the one-shot mod
 * keys is held, so another one-shot mod can still be added.
 */
TEST_F(SmartModLayer, StaysOnWhileAOneShotModKeyIsHeld) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    one_shot_shift.press();
    idle_for(SMART_MOD_ONE_SHOT_TIMEOUT);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_LEFT_CTRL));
    tap(one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);

    expect_silent_release(driver, one_shot_shift);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_LEFT_CTRL, KC_A));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_LEFT_CTRL));
    tap(key_a);
    VERIFY_AND_CLEAR(driver);

    expect_release_leaving_no_modifier(driver, spc_mod);
}

/*
 * Item 4: a latched one-shot mod is really registered. It applies to every
 * key, and it never times out.
 */
TEST_F(SmartModLayer, LatchedOneShotModAppliesToEveryKeyAndNeverTimesOut) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);
    latch(driver, one_shot_shift, KC_LEFT_SHIFT);

    /* Still registered: the host is told nothing. */
    EXPECT_NO_REPORT(driver);
    idle_for(SMART_MOD_ONE_SHOT_TIMEOUT + 1);
    VERIFY_AND_CLEAR(driver);

    expect_tap_with_registered(driver, key_b, KC_B, KC_LEFT_SHIFT);

    EXPECT_NO_REPORT(driver);
    idle_for(SMART_MOD_ONE_SHOT_TIMEOUT + 1);
    VERIFY_AND_CLEAR(driver);

    expect_tap_with_registered(driver, key_c, KC_C, KC_LEFT_SHIFT);

    expect_release_leaving_no_modifier(driver, spc_mod);
}

/*
 * Item 5: a one-shot mod held on the smart mod layer while another key is
 * typed is an ordinary modifier that was used. It is released with its key
 * and does not latch.
 */
TEST_F(SmartModLayer, OneShotModHeldWhileAnotherKeyIsTypedDoesNotLatch) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    one_shot_shift.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    expect_tap_with_registered(driver, key_b, KC_B, KC_LEFT_SHIFT);

    expect_release_leaving_no_modifier(driver, one_shot_shift);

    expect_unmodified_tap(driver, key_b, KC_B);
    /* Item 3: the smart mod layer has switched itself off here too. */
    expect_unmodified_tap(driver, key_f, KC_F);

    expect_silent_release(driver, spc_mod);

    expect_unmodified_tap(driver, key_b, KC_B);
}

/*
 * Item 5, with item 2: of two overlapping one-shot mods, the one released
 * before a key is typed latches, and the one held while it is typed does not.
 */
TEST_F(SmartModLayer, OnlyTheOneShotModReleasedBeforeAKeyIsTypedLatches) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    one_shot_ctrl.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    one_shot_shift.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    expect_silent_release(driver, one_shot_ctrl);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT, KC_B));
    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    tap(key_b);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    one_shot_shift.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    expect_tap_with_registered(driver, key_b, KC_B, KC_LEFT_CTRL);

    expect_release_leaving_no_modifier(driver, spc_mod);
}

/****************************************************************************
 * Releasing the smart mod layer key
 ****************************************************************************/

/*
 * Item 6, two smart mod layer keys held at once: the latched one-shot mods
 * are released only when the last of them is released.
 */
TEST_F(SmartModLayer, LatchedOneShotModsAreReleasedWithTheLastSmartModLayerKey) {
    TestDriver driver;
    InSequence s;

    hold(driver, smt_mod);
    latch(driver, one_shot_shift, KC_LEFT_SHIFT);
    hold(driver, spc_mod);

    expect_silent_release(driver, smt_mod);

    expect_tap_with_registered(driver, key_b, KC_B, KC_LEFT_SHIFT);

    expect_release_leaving_no_modifier(driver, spc_mod);

    expect_unmodified_tap(driver, key_b, KC_B);
}

/*
 * Item 6, the same with the keys released in the order they were pressed in
 * reverse.
 */
TEST_F(SmartModLayer, LatchedOneShotModsOutliveTheSecondSmartModLayerKey) {
    TestDriver driver;
    InSequence s;

    hold(driver, smt_mod);
    latch(driver, one_shot_shift, KC_LEFT_SHIFT);
    hold(driver, spc_mod);

    expect_silent_release(driver, spc_mod);

    expect_tap_with_registered(driver, key_b, KC_B, KC_LEFT_SHIFT);

    expect_release_leaving_no_modifier(driver, smt_mod);

    expect_unmodified_tap(driver, key_b, KC_B);
}

/*
 * Items 4, 6 and 7, smart mod layer key + Shift + several key presses: every
 * press is shifted, and nothing is left after the smart mod layer key is
 * released, however quickly: no report carries a modifier afterwards.
 */
TEST_F(SmartModLayer, SeveralKeyPressesAreAllShiftedAndNothingIsLeftAfterwards) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);
    latch(driver, one_shot_shift, KC_LEFT_SHIFT);

    expect_tap_with_registered(driver, key_b, KC_B, KC_LEFT_SHIFT);
    expect_tap_with_registered(driver, key_c, KC_C, KC_LEFT_SHIFT);
    expect_tap_with_registered(driver, key_f, KC_F, KC_LEFT_SHIFT);

    /* Well within the release window. */
    expect_release_leaving_no_modifier(driver, spc_mod);

    expect_unmodified_tap(driver, key_b, KC_B);

    EXPECT_NO_REPORT(driver);
    idle_for(SMART_MOD_ONE_SHOT_TIMEOUT);
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_c, KC_C);
}

/*
 * Item 7: if no key was typed and the smart mod layer key is released just
 * before the release window ends, the latched one-shot mods become pending
 * one-shot mods. The window is measured from the press of the one-shot mod
 * key.
 */
TEST_F(SmartModLayer, ReleasedJustWithinTheReleaseWindowLatchedOneShotModsBecomePending) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);
    latch(driver, one_shot_shift, KC_LEFT_SHIFT);

    EXPECT_NO_REPORT(driver);
    idle_until_after_press_of_tap(SMART_MOD_LAYER_RELEASE_WINDOW - 1);
    VERIFY_AND_CLEAR(driver);

    /* Released, not registered: a pending one-shot mod is invisible. */
    expect_release_leaving_no_modifier(driver, spc_mod);

    expect_tap_consuming(driver, key_b, KC_B, KC_LEFT_SHIFT);

    expect_unmodified_tap(driver, key_b, KC_B);
}

/* Item 7: released when the release window has ended, nothing remains. */
TEST_F(SmartModLayer, ReleasedWhenTheReleaseWindowHasEndedNothingRemains) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);
    latch(driver, one_shot_shift, KC_LEFT_SHIFT);

    EXPECT_NO_REPORT(driver);
    idle_until_after_press_of_tap(SMART_MOD_LAYER_RELEASE_WINDOW);
    VERIFY_AND_CLEAR(driver);

    expect_release_leaving_no_modifier(driver, spc_mod);

    expect_unmodified_tap(driver, key_b, KC_B);
}

/*
 * Item 7: with several one-shot mods the release window is measured from the
 * press of the last one-shot mod key. Just before it ends all of them become
 * pending, also the one that was pressed earlier.
 */
TEST_F(SmartModLayer, ReleaseWindowIsMeasuredFromThePressOfTheLastOneShotModKey) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    one_shot_ctrl.press();
    idle_for(SMART_MOD_LAYER_RELEASE_WINDOW / 2);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    tap(one_shot_shift);
    one_shot_ctrl.release();
    idle_until_after_press_of_tap(SMART_MOD_LAYER_RELEASE_WINDOW - 1);
    VERIFY_AND_CLEAR(driver);

    expect_release_leaving_no_modifier(driver, spc_mod);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT, KC_B));
    EXPECT_REPORT(driver, (KC_B));
    EXPECT_EMPTY_REPORT(driver);
    tap(key_b);
    VERIFY_AND_CLEAR(driver);
}

/* Item 7: the same, released when that release window has ended. */
TEST_F(SmartModLayer, ReleaseWindowOfTheLastOneShotModKeyEndsForAllOfThem) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    one_shot_ctrl.press();
    idle_for(SMART_MOD_LAYER_RELEASE_WINDOW / 2);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    tap(one_shot_shift);
    one_shot_ctrl.release();
    idle_until_after_press_of_tap(SMART_MOD_LAYER_RELEASE_WINDOW);
    VERIFY_AND_CLEAR(driver);

    expect_release_leaving_no_modifier(driver, spc_mod);

    expect_unmodified_tap(driver, key_b, KC_B);
}

/*
 * Item 7: if a key was typed, nothing remains, although the smart mod layer
 * key is released well within the release window.
 */
TEST_F(SmartModLayer, ReleasedAfterAKeyWasTypedNothingRemains) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);
    latch(driver, one_shot_shift, KC_LEFT_SHIFT);

    expect_tap_with_registered(driver, key_b, KC_B, KC_LEFT_SHIFT);

    expect_release_leaving_no_modifier(driver, spc_mod);

    expect_unmodified_tap(driver, key_b, KC_B);
}

/*
 * Item 7: a long hold of the smart mod layer key does not bring the release
 * window back when the 16-bit time of the key events wraps around.
 */
TEST_F(SmartModLayer, ReleasedAfterTheKeyEventTimeWrappedAroundNothingRemains) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);
    latch(driver, one_shot_shift, KC_LEFT_SHIFT);

    EXPECT_NO_REPORT(driver);
    idle_until_after_press_of_tap(UINT16_MAX + 1 + SMART_MOD_LAYER_RELEASE_WINDOW / 2);
    VERIFY_AND_CLEAR(driver);

    expect_release_leaving_no_modifier(driver, spc_mod);

    expect_unmodified_tap(driver, key_b, KC_B);
}

/*
 * Item 7, with item 7 of the single one-shot mod rules: the one-shot mods
 * that became pending time out like any other, counted from the release of
 * the smart mod layer key. Just before the timeout ends they are still
 * pending.
 */
TEST_F(SmartModLayer, OneShotModsMadePendingByTheReleaseAreStillPendingJustBeforeTheTimeout) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);
    latch(driver, one_shot_shift, KC_LEFT_SHIFT);
    expect_release_leaving_no_modifier(driver, spc_mod);

    EXPECT_NO_REPORT(driver);
    idle_for(SMART_MOD_ONE_SHOT_TIMEOUT - 2);
    VERIFY_AND_CLEAR(driver);

    expect_tap_consuming(driver, key_b, KC_B, KC_LEFT_SHIFT);
}

/* Item 7: the same, when the timeout has ended. */
TEST_F(SmartModLayer, OneShotModsMadePendingByTheReleaseAreGoneAfterTheTimeout) {
    TestDriver driver;
    InSequence s;

    hold(driver, spc_mod);
    latch(driver, one_shot_shift, KC_LEFT_SHIFT);
    expect_release_leaving_no_modifier(driver, spc_mod);

    EXPECT_NO_REPORT(driver);
    idle_for(SMART_MOD_ONE_SHOT_TIMEOUT - 1);
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_b, KC_B);
}

/****************************************************************************
 * Pending one-shot mods and the smart mod layer key
 ****************************************************************************/

/*
 * Item 8: holding a smart mod layer key does not latch one-shot mods that
 * were already pending. They stay pending, invisible to the host, and are
 * consumed by the next key.
 */
TEST_F(SmartModLayer, HoldingASmartModLayerKeyDoesNotLatchPendingOneShotMods) {
    TestDriver driver;
    InSequence s;

    tap(to_nav);
    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    tap(nav_one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);
    tap(to_base);

    hold(driver, spc_mod);

    expect_tap_consuming(driver, key_b, KC_B, KC_LEFT_CTRL);

    expect_unmodified_tap(driver, key_b, KC_B);

    expect_silent_release(driver, spc_mod);

    expect_unmodified_tap(driver, key_b, KC_B);
}

/*
 * Item 8: a one-shot mod latched while another is pending does not latch the
 * pending one either. The next key gets both, the key after it only the
 * latched one.
 */
TEST_F(SmartModLayer, PendingOneShotModIsConsumedWhileAnotherIsLatched) {
    TestDriver driver;
    InSequence s;

    tap(to_nav);
    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    tap(nav_one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);
    tap(to_base);

    hold(driver, spc_mod);
    latch(driver, one_shot_shift, KC_LEFT_SHIFT);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT));
    EXPECT_REPORT(driver, (KC_LEFT_CTRL, KC_LEFT_SHIFT, KC_B));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_B));
    EXPECT_REPORT(driver, (KC_LEFT_SHIFT));
    tap(key_b);
    VERIFY_AND_CLEAR(driver);

    expect_tap_with_registered(driver, key_b, KC_B, KC_LEFT_SHIFT);

    expect_release_leaving_no_modifier(driver, spc_mod);
}

/*
 * Item 9: a pending one-shot mod is cancelled by holding a smart mod layer
 * key and pressing the same one-shot mod key again. That press does not
 * latch (item 6 of the single one-shot mod rules).
 */
TEST_F(SmartModLayer, PressingTheSameOneShotModOnTheSmartModLayerCancelsThePendingOne) {
    TestDriver driver;
    InSequence s;

    tap(to_nav);
    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    tap(nav_one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);
    tap(to_base);

    hold(driver, spc_mod);

    EXPECT_REPORT(driver, (KC_LEFT_CTRL));
    EXPECT_EMPTY_REPORT(driver);
    tap(one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);

    expect_unmodified_tap(driver, key_b, KC_B);

    expect_silent_release(driver, spc_mod);

    expect_unmodified_tap(driver, key_b, KC_B);
}
