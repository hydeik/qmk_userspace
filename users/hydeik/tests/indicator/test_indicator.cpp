/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * The logic of the indicators: the indicator state of every key position,
 * and whether an indicator is shown. The rules are those of the resolutions
 * of the issues #81, #82 and #84 (the tickets of issue #77).
 *
 * The tests drive the real smart_mod and target_layer with key presses and
 * ask the logic for the state at every position of the test matrix. What the
 * keys send is the matter of the tests of the two state machines, so every
 * keyboard report is allowed here.
 */

#include <initializer_list>
#include <ostream>

#include "keyboard_report_util.hpp"
#include "keycode.h"
#include "test_common.hpp"
#include "test_fixture.hpp"
#include "test_keymap_key.hpp"

extern "C" {
#include "indicator.h"
#include "layer_lock.h"
#include "smart_mod.h"
#include "target_layer.h"
#include "test_glue.h"
}

using testing::_;
using testing::AnyNumber;

/* Print an indicator state by its name when an expectation fails. */
void PrintTo(indicator_state_t state, std::ostream *os) {
    switch (state) {
        case INDICATOR_STATE_NONE:
            *os << "none";
            break;
        case INDICATOR_STATE_PENDING_ONE_SHOT_MOD:
            *os << "pending one-shot mod";
            break;
        case INDICATOR_STATE_LATCHED_ONE_SHOT_MOD:
            *os << "latched one-shot mod";
            break;
        case INDICATOR_STATE_TRIGGER_KEY_WORKING:
            *os << "trigger key that works";
            break;
        case INDICATOR_STATE_TARGET_LAYER_ON:
            *os << "target layer that is on";
            break;
        case INDICATOR_STATE_TARGET_LAYER_PENDING:
            *os << "pending target layer";
            break;
        case INDICATOR_STATE_TARGET_LAYER_LOCKED:
            *os << "locked target layer";
            break;
    }
}

namespace {

/* A key position of the test matrix. */
struct Position {
    uint8_t row;
    uint8_t col;
};

/* One key of the keymap of a test: the keycode at a position on a layer. */
struct Cell {
    uint8_t  layer;
    Position position;
    uint16_t keycode;
};

/* The indicator state that a test expects at a position. */
struct Shown {
    Position          position;
    indicator_state_t state;
};

/* The positions of the keymap below. */
constexpr Position TAB_SYM_KEY{0, 0};
constexpr Position ENT_NUM_KEY{0, 1};
constexpr Position SMT_MOD_KEY{0, 2};
constexpr Position SPC_MOD_KEY{0, 3};
constexpr Position KEY_A{1, 0};
constexpr Position KEY_F{1, 1};
constexpr Position KEY_J{1, 2};
constexpr Position ALT_KEY{1, 3};
constexpr Position KEY_Z{2, 0};
constexpr Position KEY_X{2, 1};
constexpr Position KEY_C{2, 2};
constexpr Position KEY_V{2, 3};
constexpr Position KEY_B{2, 4};
constexpr Position UNUSED_KEY{3, 0};

constexpr indicator_state_t PENDING_MOD   = INDICATOR_STATE_PENDING_ONE_SHOT_MOD;
constexpr indicator_state_t LATCHED_MOD   = INDICATOR_STATE_LATCHED_ONE_SHOT_MOD;
constexpr indicator_state_t TRIGGER_WORKS = INDICATOR_STATE_TRIGGER_KEY_WORKING;
constexpr indicator_state_t LAYER_ON      = INDICATOR_STATE_TARGET_LAYER_ON;
constexpr indicator_state_t LAYER_PENDING = INDICATOR_STATE_TARGET_LAYER_PENDING;
constexpr indicator_state_t LAYER_LOCKED  = INDICATOR_STATE_TARGET_LAYER_LOCKED;

} /* namespace */

/*
 * The keymap of the tests, on the 4x10 test matrix, by row and column. An
 * empty cell is a transparent key, `-` a key that does nothing, and a
 * position that is not listed does nothing on BASE and is transparent on
 * every other layer. `Ctl*`, `Sft*`, `RSft*` and `Alt*` are the one-shot mod
 * keys of Left Ctrl, Left Shift, Right Shift and Left Alt.
 *
 * | Position      | BASE    | SYM     | NUM     | NAV  | MOU | FUN     | MOD   | OFF     |
 * | ------------- | ------- | ------- | ------- | ---- | --- | ------- | ----- | ------- |
 * | `TAB_SYM_KEY` | TAB_SYM |         |         |      |     |         |       | Ctl*    |
 * | `ENT_NUM_KEY` | ENT_NUM |         |         |      |     |         |       |         |
 * | `SMT_MOD_KEY` | SMT_MOD |         |         |      |     |         |       |         |
 * | `SPC_MOD_KEY` | SPC_MOD |         |         |      |     |         |       |         |
 * | `KEY_A`       | A       |         |         | Ctl* |     |         | Ctl*  | Alt*    |
 * | `KEY_F`       | F       |         |         |      |     |         | Sft*  |         |
 * | `KEY_J`       | J       |         |         |      |     |         | RSft* |         |
 * | `ALT_KEY`     | Alt*    |         |         |      |     |         |       |         |
 * | `KEY_Z`       | Z       | MO(MOU) |         |      |     |         |       | Alt*    |
 * | `KEY_X`       | X       | MO(NAV) |         |      |     |         |       | Ctl*    |
 * | `KEY_C`       | C       |         | MO(FUN) |      |     | MO(FUN) |       |         |
 * | `KEY_V`       | V       |         |         |      |     |         |       | MO(NAV) |
 * | `KEY_B`       | B       | MO(MOU) |         | Left |     |         |       |         |
 * | `UNUSED_KEY`  | -       |         |         |      |     |         |       | TAB_SYM |
 *
 * No key switches the layer OFF on: what lies there lies on a layer that is
 * off, whatever the test does. By the table of entries of test_glue.h,
 * `TAB_SYM` is the holding key of NAV and MOU, which a quick release locks,
 * and `ENT_NUM` that of FUN, which a quick release leaves pending, and of
 * NAV.
 *
 * A scan loop handles the key events at the current time and then advances
 * the time by 1 ms. `press()` / `release()` take effect in the next scan loop.
 */
class Indicator : public TestFixture {
   public:
    TestDriver driver;

    /* Build the keymap above, with `changes` laid over it. */
    explicit Indicator(std::initializer_list<Cell> changes = {}) {
        layer_lock_all_off();
        layer_clear();
        target_layer_reset();
        smart_mod_reset();

        for (uint8_t layer = 0; layer < TEST_LAYER_COUNT; layer++) {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                    keymap_of_test[layer][row][col] = layer == TEST_LAYER_BASE ? KC_NO : KC_TRNS;
                }
            }
        }
        for (const Cell &cell : std::initializer_list<Cell>{
                 {TEST_LAYER_BASE, TAB_SYM_KEY, TAB_SYM}, {TEST_LAYER_BASE, ENT_NUM_KEY, ENT_NUM}, {TEST_LAYER_BASE, SMT_MOD_KEY, SMT_MOD}, {TEST_LAYER_BASE, SPC_MOD_KEY, SPC_MOD}, {TEST_LAYER_BASE, KEY_A, KC_A},    {TEST_LAYER_BASE, KEY_F, KC_F},    {TEST_LAYER_BASE, KEY_J, KC_J},    {TEST_LAYER_BASE, ALT_KEY, COS_LALT},        {TEST_LAYER_BASE, KEY_Z, KC_Z},        {TEST_LAYER_BASE, KEY_X, KC_X}, {TEST_LAYER_BASE, KEY_C, KC_C}, {TEST_LAYER_BASE, KEY_V, KC_V}, {TEST_LAYER_BASE, KEY_B, KC_B}, {TEST_LAYER_SYM, KEY_Z, MO(TEST_LAYER_MOU)}, {TEST_LAYER_SYM, KEY_X, MO(TEST_LAYER_NAV)}, {TEST_LAYER_SYM, KEY_B, MO(TEST_LAYER_MOU)}, {TEST_LAYER_NUM, KEY_C, MO(TEST_LAYER_FUN)}, {TEST_LAYER_NAV, KEY_A, COS_LCTL}, {TEST_LAYER_NAV, KEY_B, KC_LEFT}, {TEST_LAYER_FUN, KEY_C, MO(TEST_LAYER_FUN)},
                 {TEST_LAYER_MOD, KEY_A, COS_LCTL},       {TEST_LAYER_MOD, KEY_F, COS_LSFT},       {TEST_LAYER_MOD, KEY_J, COS_RSFT},       {TEST_LAYER_OFF, TAB_SYM_KEY, COS_LCTL}, {TEST_LAYER_OFF, KEY_A, COS_LALT}, {TEST_LAYER_OFF, KEY_Z, COS_LALT}, {TEST_LAYER_OFF, KEY_X, COS_LCTL}, {TEST_LAYER_OFF, KEY_V, MO(TEST_LAYER_NAV)}, {TEST_LAYER_OFF, UNUSED_KEY, TAB_SYM},
             }) {
            keymap_of_test[cell.layer][cell.position.row][cell.position.col] = cell.keycode;
        }
        for (const Cell &cell : changes) {
            keymap_of_test[cell.layer][cell.position.row][cell.position.col] = cell.keycode;
        }
        for (uint8_t layer = 0; layer < TEST_LAYER_COUNT; layer++) {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                    add_key(KeymapKey{layer, col, row, keymap_of_test[layer][row][col]});
                }
            }
        }
        EXPECT_CALL(driver, send_keyboard_mock(_)).Times(AnyNumber());
    }

    ~Indicator() {
        /*
         * Release what a test left down while its reports are still allowed:
         * the clean-up of the test fixture expects no report of its own.
         */
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                if (matrix_is_on(row, col)) {
                    key_at({row, col}).release();
                }
            }
        }
        run_one_scan_loop();
        layer_lock_all_off();
    }

    /* Press the key at `position` and keep it down. */
    void press(Position position) {
        key_at(position).press();
        run_one_scan_loop();
    }

    /* Release the key at `position`. */
    void let_go(Position position) {
        key_at(position).release();
        run_one_scan_loop();
    }

    /* Press the key at `position`, and release it 1 ms later. */
    void tap(Position position) {
        press(position);
        let_go(position);
    }

    /*
     * Press the holding key or the smart mod layer key at `position` and keep
     * it down until QMK has made it a hold.
     */
    void hold(Position position) {
        key_at(position).press();
        idle_for(TAPPING_TERM + 1);
    }

    /* The quick gesture: hold `holding`, tap `trigger`, release `holding`. */
    void quick_gesture(Position holding, Position trigger) {
        hold(holding);
        tap(trigger);
        let_go(holding);
    }

    /* The indicator state at `position`, for the state as it is now. */
    indicator_state_t state_at(Position position) {
        const indicator_snapshot_t snapshot = indicator_snapshot();
        keypos_t                   keypos;
        keypos.row = position.row;
        keypos.col = position.col;
        return indicator_state_at(&snapshot, keypos);
    }

    /*
     * Expect the indicator states `shown` at their positions and none at
     * every other position of the matrix, and that "an indicator is shown" is
     * true exactly when `shown` lists a position.
     */
    void expect_shown(std::initializer_list<Shown> shown) {
        const indicator_snapshot_t snapshot = indicator_snapshot();

        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                indicator_state_t expected = INDICATOR_STATE_NONE;
                for (const Shown &one : shown) {
                    if (one.position.row == row && one.position.col == col) {
                        expected = one.state;
                    }
                }
                keypos_t position;
                position.row = row;
                position.col = col;
                EXPECT_EQ(indicator_state_at(&snapshot, position), expected) << "at row " << +row << ", column " << +col;
            }
        }
        EXPECT_EQ(indicator_is_any_shown(&snapshot), shown.size() != 0);
    }

   private:
    uint16_t keymap_of_test[TEST_LAYER_COUNT][MATRIX_ROWS][MATRIX_COLS];

    /* The key at `position`, to press or to release it. */
    KeymapKey key_at(Position position) {
        return KeymapKey{TEST_LAYER_BASE, position.col, position.row, keymap_of_test[TEST_LAYER_BASE][position.row][position.col]};
    }
};

/****************************************************************************
 * One-shot mods
 ****************************************************************************/

/* With no state there is no indicator, and none is shown. */
TEST_F(Indicator, NoStateShowsNothing) {
    expect_shown({});
}

/*
 * A pending one-shot mod is shown at every position that holds its one-shot
 * mod key, on a layer that is on (`ALT_KEY`) and on one that is off.
 */
TEST_F(Indicator, PendingOneShotModIsShownAtEveryPositionThatHoldsItsKey) {
    tap(ALT_KEY);

    expect_shown({{ALT_KEY, PENDING_MOD}, {KEY_A, PENDING_MOD}, {KEY_Z, PENDING_MOD}});
}

/*
 * A tap of `SMT_MOD` makes Shift pending with no one-shot mod key pressed.
 * It is shown at the one-shot mod key of that Shift, which lies on the smart
 * mod layer, a layer that is off, and not at the key of Right Shift.
 */
TEST_F(Indicator, ShiftOfATapOfSmtModIsShownAtTheOneShotModKeyOfThatShift) {
    tap(SMT_MOD_KEY);

    expect_shown({{KEY_F, PENDING_MOD}});
}

/*
 * A latched one-shot mod is shown at every position that holds its one-shot
 * mod key. The smart mod layer has switched itself off by then, so each of
 * them lies on a layer that is off.
 */
TEST_F(Indicator, LatchedOneShotModIsShownAtEveryPositionThatHoldsItsKey) {
    hold(SPC_MOD_KEY);
    expect_shown({});

    tap(KEY_A);

    expect_shown({{KEY_A, LATCHED_MOD}, {TAB_SYM_KEY, LATCHED_MOD}, {KEY_X, LATCHED_MOD}});
}

/* Left and right are different keys: Right Shift is not shown at Left Shift. */
TEST_F(Indicator, OneShotModOfTheRightHandIsNotShownAtTheKeyOfTheLeftHand) {
    hold(SPC_MOD_KEY);
    tap(KEY_J);

    expect_shown({{KEY_J, LATCHED_MOD}});
}

/* The indicator goes out when the one-shot mod ends. */
TEST_F(Indicator, OneShotModThatHasEndedIsNotShown) {
    tap(ALT_KEY);
    tap(KEY_F);
    expect_shown({});

    hold(SPC_MOD_KEY);
    tap(KEY_A);
    idle_for(SMART_MOD_LAYER_RELEASE_WINDOW);
    let_go(SPC_MOD_KEY);
    expect_shown({});
}

/****************************************************************************
 * Trigger keys that work
 ****************************************************************************/

/*
 * While a holding key is held, the trigger keys of its target layers work.
 * Each is shown where a press now gives it: `MO(MOU)` at `KEY_Z` and `KEY_B`
 * and `MO(NAV)` at `KEY_X`, on the holding layer. The holding key itself
 * shows nothing, and neither does `KEY_V`, where `MO(NAV)` lies only on a
 * layer that is off.
 */
TEST_F(Indicator, TriggerKeyThatWorksIsShownWhereAPressGivesItNow) {
    hold(TAB_SYM_KEY);

    expect_shown({{KEY_Z, TRIGGER_WORKS}, {KEY_X, TRIGGER_WORKS}, {KEY_B, TRIGGER_WORKS}});

    let_go(TAB_SYM_KEY);
    expect_shown({});
}

/*
 * Only the trigger keys of the target layers of the held holding key work:
 * `ENT_NUM` is no holding key of MOU, and no trigger key of NAV lies on its
 * holding layer.
 */
TEST_F(Indicator, TriggerKeyOfAnotherHoldingKeyIsNotShown) {
    hold(ENT_NUM_KEY);

    expect_shown({{KEY_C, TRIGGER_WORKS}});
}

/*
 * A trigger key under a transparent key is shown. NAV is locked, so it is on
 * above the holding layer of `ENT_NUM`, and it is transparent at `KEY_C`: a
 * press there gives the `MO(FUN)` beneath.
 */
TEST_F(Indicator, TriggerKeyUnderATransparentKeyIsShown) {
    quick_gesture(TAB_SYM_KEY, KEY_X);
    ASSERT_EQ(target_layer_state_of(TEST_LAYER_NAV), TARGET_LAYER_STATE_LOCKED);

    hold(ENT_NUM_KEY);

    EXPECT_EQ(state_at(KEY_C), TRIGGER_WORKS);
}

/*
 * A trigger key under another key is not shown: a press there does not give
 * it. NAV is on and holds Left at `KEY_B`, over the `MO(MOU)` of the holding
 * layer, while the `MO(MOU)` at `KEY_Z` is still reached.
 */
TEST_F(Indicator, TriggerKeyUnderAnotherKeyIsNotShown) {
    hold(TAB_SYM_KEY);
    tap(KEY_X);
    ASSERT_EQ(target_layer_state_of(TEST_LAYER_NAV), TARGET_LAYER_STATE_ON);

    EXPECT_EQ(state_at(KEY_B), INDICATOR_STATE_NONE);
    EXPECT_EQ(state_at(KEY_Z), TRIGGER_WORKS);
}

/****************************************************************************
 * Target layer states
 ****************************************************************************/

/*
 * A target layer that is on is shown where a press now gives its trigger key
 * (`KEY_X`, through the transparent key of NAV) and at every position that
 * holds one of its holding keys: `TAB_SYM`, also where it lies on a layer
 * that is off, and `ENT_NUM`, which is not held.
 */
TEST_F(Indicator, TargetLayerThatIsOnIsShownAtItsTriggerKeyAndItsHoldingKeys) {
    hold(TAB_SYM_KEY);
    tap(KEY_X);

    expect_shown({{KEY_X, LAYER_ON}, {TAB_SYM_KEY, LAYER_ON}, {UNUSED_KEY, LAYER_ON}, {ENT_NUM_KEY, LAYER_ON}, {KEY_Z, TRIGGER_WORKS}});

    /* No quick release: the One Shot Term has ended. */
    idle_for(TARGET_LAYER_ONE_SHOT_TERM);
    let_go(TAB_SYM_KEY);
    expect_shown({});
}

/*
 * A pending target layer is shown after its holding key was released: at its
 * holding key, and at its trigger key on the pending target layer itself.
 */
TEST_F(Indicator, PendingTargetLayerIsShownAfterItsHoldingKeyWasReleased) {
    quick_gesture(ENT_NUM_KEY, KEY_C);

    expect_shown({{ENT_NUM_KEY, LAYER_PENDING}, {KEY_C, LAYER_PENDING}});

    tap(KEY_F);
    expect_shown({});
}

/*
 * A locked target layer is shown at its holding keys after the holding key
 * was released. Its trigger key lies on the holding layer, which is off now,
 * so it is shown nowhere.
 */
TEST_F(Indicator, LockedTargetLayerIsShownAfterItsHoldingKeyWasReleased) {
    quick_gesture(TAB_SYM_KEY, KEY_X);

    expect_shown({{TAB_SYM_KEY, LAYER_LOCKED}, {UNUSED_KEY, LAYER_LOCKED}, {ENT_NUM_KEY, LAYER_LOCKED}});
}

/****************************************************************************
 * Priority on one position
 ****************************************************************************/

/*
 * A target layer state wins over a one-shot mod. NAV is on and Left Ctrl is
 * pending: the one-shot mod key of Left Ctrl also lies, on a layer that is
 * off, at the holding key `TAB_SYM_KEY` and at the trigger key `KEY_X`, and
 * both show "on". `KEY_A` holds nothing but that one-shot mod key.
 */
TEST_F(Indicator, TargetLayerStateWinsOverAOneShotMod) {
    hold(TAB_SYM_KEY);
    tap(KEY_X);
    tap(KEY_A);
    ASSERT_EQ(smart_mod_pending_mods(), MOD_BIT_LCTRL);

    EXPECT_EQ(state_at(TAB_SYM_KEY), LAYER_ON);
    EXPECT_EQ(state_at(KEY_X), LAYER_ON);
    EXPECT_EQ(state_at(KEY_A), PENDING_MOD);
}

/* A locked target layer wins over a one-shot mod too. */
TEST_F(Indicator, LockedTargetLayerWinsOverAOneShotMod) {
    quick_gesture(TAB_SYM_KEY, KEY_X);
    tap(KEY_A);
    ASSERT_EQ(smart_mod_pending_mods(), MOD_BIT_LCTRL);

    EXPECT_EQ(state_at(TAB_SYM_KEY), LAYER_LOCKED);
    EXPECT_EQ(state_at(KEY_A), PENDING_MOD);
}

/*
 * A one-shot mod wins over a trigger key that works. `TAB_SYM` is held and
 * Left Alt is pending: its one-shot mod key also lies, on a layer that is
 * off, at `KEY_Z`, where a press gives the trigger key `MO(MOU)`.
 */
TEST_F(Indicator, OneShotModWinsOverATriggerKeyThatWorks) {
    hold(TAB_SYM_KEY);
    tap(ALT_KEY);
    ASSERT_EQ(smart_mod_pending_mods(), MOD_BIT_LALT);

    EXPECT_EQ(state_at(KEY_Z), PENDING_MOD);
    EXPECT_EQ(state_at(KEY_B), TRIGGER_WORKS);
}

/* A latched one-shot mod wins over a trigger key that works as well. */
TEST_F(Indicator, LatchedOneShotModWinsOverATriggerKeyThatWorks) {
    hold(SPC_MOD_KEY);
    tap(KEY_A);
    hold(TAB_SYM_KEY);
    ASSERT_EQ(smart_mod_latched_mods(), MOD_BIT_LCTRL);

    EXPECT_EQ(state_at(KEY_X), LATCHED_MOD);
    EXPECT_EQ(state_at(KEY_Z), TRIGGER_WORKS);
}

/*
 * The trigger key of the target layer that is on shows "on", not "trigger
 * key that works", although its holding key is still held.
 */
TEST_F(Indicator, TriggerKeyOfTheTargetLayerThatIsOnShowsOn) {
    hold(TAB_SYM_KEY);
    EXPECT_EQ(state_at(KEY_X), TRIGGER_WORKS);

    tap(KEY_X);
    ASSERT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_NAV));

    EXPECT_EQ(state_at(KEY_X), LAYER_ON);
}

/*
 * The target layer that is on wins over a locked one. The example of the
 * resolution of issue #84: NAV is locked, and `TAB_SYM` with `MO(MOU)`
 * switches MOU on. `TAB_SYM` is a holding key of both and shows "on".
 * `ENT_NUM` is a holding key of NAV only, and the trigger key of NAV is
 * reached again while `TAB_SYM` is held: both show "locked".
 */
TEST_F(Indicator, TargetLayerThatIsOnWinsOverALockedOne) {
    quick_gesture(TAB_SYM_KEY, KEY_X);
    hold(TAB_SYM_KEY);
    tap(KEY_Z);
    ASSERT_EQ(target_layer_state_of(TEST_LAYER_NAV), TARGET_LAYER_STATE_LOCKED);
    ASSERT_EQ(target_layer_state_of(TEST_LAYER_MOU), TARGET_LAYER_STATE_ON);

    expect_shown({{TAB_SYM_KEY, LAYER_ON}, {UNUSED_KEY, LAYER_ON}, {KEY_Z, LAYER_ON}, {ENT_NUM_KEY, LAYER_LOCKED}, {KEY_X, LAYER_LOCKED}});
}

/*
 * A pending target layer wins over a locked one. NAV is locked and FUN is
 * pending: `ENT_NUM` is a holding key of both and shows "pending", while
 * `TAB_SYM`, no holding key of FUN, shows "locked".
 */
TEST_F(Indicator, PendingTargetLayerWinsOverALockedOne) {
    quick_gesture(TAB_SYM_KEY, KEY_X);
    quick_gesture(ENT_NUM_KEY, KEY_C);
    ASSERT_EQ(target_layer_state_of(TEST_LAYER_NAV), TARGET_LAYER_STATE_LOCKED);
    ASSERT_EQ(target_layer_state_of(TEST_LAYER_FUN), TARGET_LAYER_STATE_PENDING);

    expect_shown({{ENT_NUM_KEY, LAYER_PENDING}, {KEY_C, LAYER_PENDING}, {TAB_SYM_KEY, LAYER_LOCKED}, {UNUSED_KEY, LAYER_LOCKED}});
}

/*
 * A pending one-shot mod wins over a latched one. Left Alt is pending and
 * Left Ctrl is latched: `KEY_A` holds the one-shot mod keys of both, on
 * different layers, and shows "pending". `TAB_SYM_KEY` holds only that of
 * Left Ctrl and `ALT_KEY` only that of Left Alt.
 */
TEST_F(Indicator, PendingOneShotModWinsOverALatchedOne) {
    tap(ALT_KEY);
    hold(SPC_MOD_KEY);
    tap(KEY_A);
    ASSERT_EQ(smart_mod_pending_mods(), MOD_BIT_LALT);
    ASSERT_EQ(smart_mod_latched_mods(), MOD_BIT_LCTRL);

    expect_shown({{KEY_A, PENDING_MOD}, {ALT_KEY, PENDING_MOD}, {KEY_Z, PENDING_MOD}, {TAB_SYM_KEY, LATCHED_MOD}, {KEY_X, LATCHED_MOD}});
}

/****************************************************************************
 * A key that lies nowhere in the keymap in use
 ****************************************************************************/

/* The keymap of the tests without the one-shot mod key of Left Shift. */
class IndicatorWithoutLeftShiftKey : public Indicator {
   public:
    IndicatorWithoutLeftShiftKey() : Indicator({{TEST_LAYER_MOD, KEY_F, KC_TRNS}}) {}
};

/*
 * A state whose key lies on no position shows nothing, at no fallback
 * position, and so no indicator is shown: a tap of `SMT_MOD` makes Shift
 * pending, and no position holds the one-shot mod key of Left Shift.
 */
TEST_F(IndicatorWithoutLeftShiftKey, OneShotModWhoseKeyLiesNowhereShowsNothing) {
    tap(SMT_MOD_KEY);
    ASSERT_EQ(smart_mod_pending_mods(), MOD_BIT_LSHIFT);

    expect_shown({});
}

/* The keymap of the tests with the trigger key of FUN on a layer that is off only. */
class IndicatorWithoutReachableTriggerKey : public Indicator {
   public:
    IndicatorWithoutReachableTriggerKey() : Indicator({{TEST_LAYER_NUM, KEY_C, KC_TRNS}, {TEST_LAYER_FUN, KEY_C, KC_TRNS}, {TEST_LAYER_OFF, KEY_C, MO(TEST_LAYER_FUN)}}) {}
};

/*
 * A trigger key that works, but lies only on a layer that is off, shows
 * nothing, and so no indicator is shown.
 */
TEST_F(IndicatorWithoutReachableTriggerKey, TriggerKeyThatLiesOnlyOnALayerThatIsOffShowsNothing) {
    hold(ENT_NUM_KEY);
    ASSERT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_FUN));

    expect_shown({});
}

/****************************************************************************
 * The view
 ****************************************************************************/

class IndicatorView : public Indicator {
   public:
    /*
     * Expect that the view built now holds the indicator states `shown` at
     * their positions and none at every other position of the matrix, and
     * that its bit "an indicator is shown" is `any_shown`.
     */
    void expect_view(std::initializer_list<Shown> shown, bool any_shown) {
        indicator_view_t view;
        indicator_view_build(&view);

        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                indicator_state_t expected = INDICATOR_STATE_NONE;
                for (const Shown &one : shown) {
                    if (one.position.row == row && one.position.col == col) {
                        expected = one.state;
                    }
                }
                keypos_t position;
                position.row = row;
                position.col = col;
                EXPECT_EQ(indicator_view_state_at(&view, position), expected) << "at row " << +row << ", column " << +col;
            }
        }
        EXPECT_EQ(view.any_shown, any_shown);
    }
};

/* With no state the view is empty: no indicator state, and the bit is off. */
TEST_F(IndicatorView, ViewOfNoStateIsEmpty) {
    expect_view({}, false);
}

/*
 * The view holds, for every position, the indicator state that the logic
 * answers there, and its bit is on: here a target layer that is on, with the
 * trigger key of another target layer that works.
 */
TEST_F(IndicatorView, ViewHoldsTheIndicatorStateOfEveryPosition) {
    hold(TAB_SYM_KEY);
    tap(KEY_X);

    expect_view({{KEY_X, LAYER_ON}, {TAB_SYM_KEY, LAYER_ON}, {UNUSED_KEY, LAYER_ON}, {ENT_NUM_KEY, LAYER_ON}, {KEY_Z, TRIGGER_WORKS}}, true);
}

/* The view follows the state: it is empty again when the state has ended. */
TEST_F(IndicatorView, ViewIsEmptyAgainWhenTheStateHasEnded) {
    tap(ALT_KEY);
    expect_view({{ALT_KEY, PENDING_MOD}, {KEY_A, PENDING_MOD}, {KEY_Z, PENDING_MOD}}, true);

    tap(KEY_F);
    expect_view({}, false);
}

/*
 * A state whose key lies on no position leaves the view empty, with the bit
 * off: the bit agrees with "an indicator is shown", not with "there is
 * state".
 */
TEST_F(IndicatorWithoutLeftShiftKey, ViewOfAStateWhoseKeyLiesNowhereIsEmpty) {
    tap(SMT_MOD_KEY);
    ASSERT_EQ(smart_mod_pending_mods(), MOD_BIT_LSHIFT);

    const indicator_snapshot_t snapshot = indicator_snapshot();
    indicator_view_t           view;
    indicator_view_build(&view);

    EXPECT_FALSE(view.any_shown);
    EXPECT_EQ(view.any_shown, indicator_is_any_shown(&snapshot));
}
