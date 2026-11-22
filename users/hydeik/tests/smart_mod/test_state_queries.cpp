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
#include "matrix.h"
#include "smart_mod.h"
#include "test_glue.h"
}

using testing::_;
using testing::AnyNumber;

/*
 * What smart_mod answers about its state: the pending and the latched
 * one-shot mods, by the resolution of issue #84, "What do smart_mod and
 * target_layer answer about their state, and how is the indicator module
 * cut?". What the host is sent is the matter of the other tests of this
 * directory, so every report is allowed here.
 *
 * A scan loop handles the key events at the current time and then advances
 * the time by 1 ms. `press()` / `release()` take effect in the next scan loop.
 *
 * The keymap. Keys that share a position are the same physical key:
 *
 *            0        1        2     3     4     5     6
 *   base     SMT_MOD  SPC_MOD  Sft*  RSft* LCtl  a     b
 *   mod      .        .        .     .     .     Ctl*  .
 *
 * `Sft*`, `RSft*` and `Ctl*` are one-shot mod keys, of Left Shift, Right
 * Shift and Left Ctrl; `LCtl` is the ordinary modifier key; `.` is
 * transparent. `mod` is the smart mod layer.
 */
class SmartModStateQueries : public TestFixture {
   public:
    KeymapKey smt_mod{TEST_LAYER_BASE, 0, 0, SMT_MOD};
    KeymapKey spc_mod{TEST_LAYER_BASE, 1, 0, SPC_MOD};
    KeymapKey one_shot_shift{TEST_LAYER_BASE, 2, 0, COS_LSFT};
    KeymapKey one_shot_right_shift{TEST_LAYER_BASE, 3, 0, COS_RSFT};
    KeymapKey left_ctrl{TEST_LAYER_BASE, 4, 0, KC_LEFT_CTRL};
    KeymapKey key_a{TEST_LAYER_BASE, 5, 0, KC_A};
    KeymapKey key_b{TEST_LAYER_BASE, 6, 0, KC_B};

    /* The one-shot mod key of Left Ctrl on the smart mod layer, over `key_a`. */
    KeymapKey one_shot_ctrl{TEST_LAYER_MOD, 5, 0, COS_LCTL};

    TestDriver driver;

    SmartModStateQueries() {
        smart_mod_reset();
        for (KeymapKey key : {smt_mod, spc_mod, one_shot_shift, one_shot_right_shift, left_ctrl, key_a, key_b, one_shot_ctrl}) {
            add_key(key);
        }
        for (uint8_t col : {0, 1, 2, 3, 4, 6}) {
            add_key(KeymapKey(TEST_LAYER_MOD, col, 0, KC_TRANSPARENT));
        }
        EXPECT_CALL(driver, send_keyboard_mock(_)).Times(AnyNumber());
    }

    ~SmartModStateQueries() {
        /*
         * Release what a test left down while its reports are still allowed:
         * the clean-up of the test fixture expects no report of its own.
         */
        for (KeymapKey* key : {&smt_mod, &spc_mod, &one_shot_shift, &one_shot_right_shift, &left_ctrl, &key_a, &key_b}) {
            if (matrix_is_on(key->position.row, key->position.col)) {
                key->release();
            }
        }
        run_one_scan_loop();
    }

    /* Press `key`, and release it 1 ms later: well within the One Shot Term. */
    void tap(KeymapKey& key) {
        key.press();
        run_one_scan_loop();
        key.release();
        run_one_scan_loop();
    }

    /* Press `key` and keep it down. */
    void press(KeymapKey& key) {
        key.press();
        run_one_scan_loop();
    }

    /* Release `key`. */
    void let_go(KeymapKey& key) {
        key.release();
        run_one_scan_loop();
    }

    /*
     * Press the smart mod layer key `key` and keep it down until QMK has
     * settled that it is held.
     */
    void hold(KeymapKey& key) {
        key.press();
        idle_for(TAPPING_TERM + 1);
    }
};

/****************************************************************************
 * Pending one-shot mods
 ****************************************************************************/

/* A tap of a one-shot mod key leaves its modifier pending. */
TEST_F(SmartModStateQueries, TapOfAOneShotModKeyIsAnsweredAsPending) {
    EXPECT_EQ(smart_mod_pending_mods(), 0);

    tap(one_shot_shift);

    EXPECT_EQ(smart_mod_pending_mods(), MOD_BIT_LSHIFT);
}

/* Left and right are told apart, and several pending one-shot mods add up. */
TEST_F(SmartModStateQueries, PendingOneShotModsTellLeftFromRight) {
    tap(one_shot_right_shift);
    EXPECT_EQ(smart_mod_pending_mods(), MOD_BIT_RSHIFT);

    tap(one_shot_shift);
    EXPECT_EQ(smart_mod_pending_mods(), MOD_BIT_LSHIFT | MOD_BIT_RSHIFT);
}

/*
 * A tap of `SMT_MOD` leaves Shift pending with no one-shot mod key pressed,
 * and a second tap, once the tapping term of the first has ended, cancels it.
 */
TEST_F(SmartModStateQueries, TapOfSmtModIsAnsweredAsAPendingShift) {
    tap(smt_mod);
    EXPECT_EQ(smart_mod_pending_mods(), MOD_BIT_LSHIFT);
    EXPECT_EQ(smart_mod_latched_mods(), 0);

    idle_for(TAPPING_TERM);
    tap(smt_mod);
    EXPECT_EQ(smart_mod_pending_mods(), 0);
}

/* Pressing the same one-shot mod key again cancels its pending one-shot mod. */
TEST_F(SmartModStateQueries, CancelledPendingOneShotModIsNotAnswered) {
    tap(one_shot_shift);
    tap(one_shot_right_shift);

    press(one_shot_shift);
    EXPECT_EQ(smart_mod_pending_mods(), MOD_BIT_RSHIFT);

    let_go(one_shot_shift);
    EXPECT_EQ(smart_mod_pending_mods(), MOD_BIT_RSHIFT);
}

/*
 * Not answered: a pending one-shot mod that a key press has consumed. It
 * leaves the answer at that press: it is gone already when the report of the
 * key, with the modifier, is sent.
 */
TEST_F(SmartModStateQueries, ConsumedOneShotModLeavesTheAnswerAtTheKeyPress) {
    tap(one_shot_shift);
    ASSERT_EQ(smart_mod_pending_mods(), MOD_BIT_LSHIFT);

    EXPECT_REPORT(driver, (KC_LEFT_SHIFT, KC_A)).WillOnce([](report_keyboard_t&) {
        EXPECT_EQ(smart_mod_pending_mods(), 0);
        EXPECT_EQ(smart_mod_latched_mods(), 0);
    });
    press(key_a);
    EXPECT_EQ(smart_mod_pending_mods(), 0);

    let_go(key_a);
    EXPECT_EQ(smart_mod_pending_mods(), 0);
}

/* A pending one-shot mod is answered until its timeout ends, and not after. */
TEST_F(SmartModStateQueries, PendingOneShotModLeavesTheAnswerAfterItsTimeout) {
    tap(one_shot_shift);

    idle_for(SMART_MOD_ONE_SHOT_TIMEOUT - 1);
    EXPECT_EQ(smart_mod_pending_mods(), MOD_BIT_LSHIFT);

    run_one_scan_loop();
    EXPECT_EQ(smart_mod_pending_mods(), 0);
}

/*
 * Not answered: a modifier that is merely held, by a one-shot mod key or by
 * an ordinary modifier key.
 */
TEST_F(SmartModStateQueries, ModifierThatIsMerelyHeldIsNotAnswered) {
    press(one_shot_shift);
    EXPECT_EQ(smart_mod_pending_mods(), 0);
    EXPECT_EQ(smart_mod_latched_mods(), 0);

    idle_for(SMART_MOD_ONE_SHOT_TERM);
    EXPECT_EQ(smart_mod_pending_mods(), 0);
    EXPECT_EQ(smart_mod_latched_mods(), 0);

    let_go(one_shot_shift);
    EXPECT_EQ(smart_mod_pending_mods(), 0);
    EXPECT_EQ(smart_mod_latched_mods(), 0);

    press(left_ctrl);
    EXPECT_EQ(smart_mod_pending_mods(), 0);
    EXPECT_EQ(smart_mod_latched_mods(), 0);
}

/****************************************************************************
 * Latched one-shot mods
 ****************************************************************************/

/*
 * A one-shot mod chosen on the smart mod layer is latched, not pending. A
 * typed key does not consume it.
 */
TEST_F(SmartModStateQueries, OneShotModChosenOnTheSmartModLayerIsAnsweredAsLatched) {
    hold(spc_mod);
    EXPECT_EQ(smart_mod_latched_mods(), 0);

    tap(one_shot_ctrl);
    EXPECT_EQ(smart_mod_latched_mods(), MOD_BIT_LCTRL);
    EXPECT_EQ(smart_mod_pending_mods(), 0);

    tap(key_b);
    EXPECT_EQ(smart_mod_latched_mods(), MOD_BIT_LCTRL);
    EXPECT_EQ(smart_mod_pending_mods(), 0);
}

/* Left and right are told apart, and several latched one-shot mods add up. */
TEST_F(SmartModStateQueries, LatchedOneShotModsTellLeftFromRight) {
    hold(spc_mod);

    tap(one_shot_right_shift);
    EXPECT_EQ(smart_mod_latched_mods(), MOD_BIT_RSHIFT);

    /* The smart mod layer has switched itself off: these latch together. */
    let_go(spc_mod);
    hold(spc_mod);
    press(one_shot_ctrl);
    press(one_shot_shift);
    let_go(one_shot_ctrl);
    let_go(one_shot_shift);
    EXPECT_EQ(smart_mod_latched_mods(), MOD_BIT_LCTRL | MOD_BIT_LSHIFT);
}

/*
 * Not answered: a one-shot mod key that is down on the smart mod layer and
 * has not latched yet. It is latched from its release on.
 */
TEST_F(SmartModStateQueries, OneShotModKeyDownOnTheSmartModLayerIsNotAnsweredBeforeItLatches) {
    hold(spc_mod);

    press(one_shot_ctrl);
    EXPECT_EQ(smart_mod_latched_mods(), 0);
    EXPECT_EQ(smart_mod_pending_mods(), 0);

    let_go(one_shot_ctrl);
    EXPECT_EQ(smart_mod_latched_mods(), MOD_BIT_LCTRL);
    EXPECT_EQ(smart_mod_pending_mods(), 0);
}

/*
 * A one-shot mod key of the smart mod layer that was held while a key was
 * typed was an ordinary modifier: it does not latch.
 */
TEST_F(SmartModStateQueries, OneShotModKeyHeldWhileAKeyIsTypedIsNotAnswered) {
    hold(spc_mod);

    press(one_shot_ctrl);
    tap(key_b);
    let_go(one_shot_ctrl);
    EXPECT_EQ(smart_mod_latched_mods(), 0);
    EXPECT_EQ(smart_mod_pending_mods(), 0);
}

/*
 * The latched one-shot mods become pending when the last smart mod layer key
 * is released within the release window with no key typed.
 */
TEST_F(SmartModStateQueries, LatchedOneShotModsBecomePendingAtAQuickReleaseOfTheSmartModLayerKey) {
    hold(spc_mod);
    tap(one_shot_ctrl);

    let_go(spc_mod);
    EXPECT_EQ(smart_mod_latched_mods(), 0);
    EXPECT_EQ(smart_mod_pending_mods(), MOD_BIT_LCTRL);
}

/*
 * Released when the release window has ended, the last smart mod layer key
 * leaves nothing latched and nothing pending.
 */
TEST_F(SmartModStateQueries, LatchedOneShotModsLeaveTheAnswerWithTheLastSmartModLayerKey) {
    hold(spc_mod);
    tap(one_shot_ctrl);

    idle_for(SMART_MOD_LAYER_RELEASE_WINDOW);
    EXPECT_EQ(smart_mod_latched_mods(), MOD_BIT_LCTRL);

    let_go(spc_mod);
    EXPECT_EQ(smart_mod_latched_mods(), 0);
    EXPECT_EQ(smart_mod_pending_mods(), 0);
}

/* A pending and a latched one-shot mod are answered each by its own query. */
TEST_F(SmartModStateQueries, PendingAndLatchedOneShotModsAreAnsweredApart) {
    tap(one_shot_shift);
    hold(spc_mod);
    tap(one_shot_ctrl);

    EXPECT_EQ(smart_mod_pending_mods(), MOD_BIT_LSHIFT);
    EXPECT_EQ(smart_mod_latched_mods(), MOD_BIT_LCTRL);
}
