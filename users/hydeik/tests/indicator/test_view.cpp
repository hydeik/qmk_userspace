/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * The view on its way to the other half of a split keyboard: packing it into
 * the bytes of a transaction and unpacking it again, the decision to send it
 * now, and the decision that it has expired. The rules are those of
 * docs/adr/0005-the-master-half-decides-every-indicator-state.md.
 *
 * None of this needs a key press: the tests fill a view by hand. That a view
 * built from the state of the keyboard holds what the logic answers is the
 * matter of test_indicator.cpp.
 */

#include <cstring>
#include <ostream>

#include "gtest/gtest.h"

extern "C" {
#include "indicator.h"
}

/* Defined in test_indicator.cpp. */
void PrintTo(indicator_state_t state, std::ostream *os);

namespace {

constexpr indicator_state_t EVERY_STATE[] = {
    INDICATOR_STATE_NONE, INDICATOR_STATE_PENDING_ONE_SHOT_MOD, INDICATOR_STATE_LATCHED_ONE_SHOT_MOD, INDICATOR_STATE_TRIGGER_KEY_WORKING, INDICATOR_STATE_TARGET_LAYER_ON, INDICATOR_STATE_TARGET_LAYER_PENDING, INDICATOR_STATE_TARGET_LAYER_LOCKED,
};

constexpr uint8_t LAST_ROW = MATRIX_ROWS - 1;
constexpr uint8_t LAST_COL = MATRIX_COLS - 1;

/* The view with no indicator state and the bit off. */
indicator_view_t empty_view() {
    indicator_view_t view;
    memset(&view, 0, sizeof(view));
    return view;
}

/*
 * Pack `view` into bytes that held something else before, and unpack them
 * into a view that held something else before.
 */
indicator_view_t packed_and_unpacked(const indicator_view_t &view) {
    uint8_t packed[INDICATOR_VIEW_PACKED_SIZE];
    memset(packed, 0xFF, sizeof(packed));
    indicator_view_pack(&view, packed);

    indicator_view_t unpacked;
    memset(&unpacked, 0xFF, sizeof(unpacked));
    indicator_view_unpack(packed, &unpacked);
    return unpacked;
}

/* Expect that `actual` holds what `expected` holds, at every position. */
void expect_same_view(const indicator_view_t &actual, const indicator_view_t &expected) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            keypos_t position;
            position.row = row;
            position.col = col;
            EXPECT_EQ(indicator_view_state_at(&actual, position), indicator_view_state_at(&expected, position)) << "at row " << +row << ", column " << +col;
        }
    }
    EXPECT_EQ(actual.any_shown, expected.any_shown);
}

} /* namespace */

/****************************************************************************
 * Packing and unpacking
 ****************************************************************************/

/*
 * Every indicator state comes back at the first key position, with the bit
 * on and with the bit off, and no other position gets a state from it.
 */
TEST(IndicatorViewPacking, EveryStateAtTheFirstPositionComesBack) {
    for (const bool any_shown : {false, true}) {
        for (const indicator_state_t state : EVERY_STATE) {
            indicator_view_t view = empty_view();
            view.states[0][0]     = state;
            view.any_shown        = any_shown;

            SCOPED_TRACE(testing::Message() << "state " << testing::PrintToString(state) << ", bit " << any_shown);
            expect_same_view(packed_and_unpacked(view), view);
        }
    }
}

/* The same at the last key position of the matrix. */
TEST(IndicatorViewPacking, EveryStateAtTheLastPositionComesBack) {
    for (const bool any_shown : {false, true}) {
        for (const indicator_state_t state : EVERY_STATE) {
            indicator_view_t view           = empty_view();
            view.states[LAST_ROW][LAST_COL] = state;
            view.any_shown                  = any_shown;

            SCOPED_TRACE(testing::Message() << "state " << testing::PrintToString(state) << ", bit " << any_shown);
            expect_same_view(packed_and_unpacked(view), view);
        }
    }
}

/*
 * No position takes the state of a neighbour: a view whose neighbouring
 * positions all differ comes back as it was.
 */
TEST(IndicatorViewPacking, ViewWithADifferentStateAtEveryNeighbourComesBack) {
    indicator_view_t view = empty_view();
    uint8_t          next = 1;
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            view.states[row][col] = EVERY_STATE[next];
            next                  = (next + 1) % 7;
        }
    }
    view.any_shown = true;

    expect_same_view(packed_and_unpacked(view), view);
}

/*
 * The packed bytes depend on the view alone, not on what the bytes held
 * before, so two views are the same exactly when their packed bytes are.
 */
TEST(IndicatorViewPacking, PackedBytesDependOnTheViewAlone) {
    indicator_view_t view           = empty_view();
    view.states[0][0]               = INDICATOR_STATE_TARGET_LAYER_LOCKED;
    view.states[LAST_ROW][LAST_COL] = INDICATOR_STATE_PENDING_ONE_SHOT_MOD;
    view.any_shown                  = true;

    uint8_t over_zeros[INDICATOR_VIEW_PACKED_SIZE];
    uint8_t over_ones[INDICATOR_VIEW_PACKED_SIZE];
    memset(over_zeros, 0x00, sizeof(over_zeros));
    memset(over_ones, 0xFF, sizeof(over_ones));
    indicator_view_pack(&view, over_zeros);
    indicator_view_pack(&view, over_ones);

    EXPECT_EQ(memcmp(over_zeros, over_ones, INDICATOR_VIEW_PACKED_SIZE), 0);

    indicator_view_t other = view;
    other.states[0][0]     = INDICATOR_STATE_TARGET_LAYER_ON;
    uint8_t packed_other[INDICATOR_VIEW_PACKED_SIZE];
    indicator_view_pack(&other, packed_other);

    EXPECT_NE(memcmp(over_zeros, packed_other, INDICATOR_VIEW_PACKED_SIZE), 0);
}

/*
 * Bytes that are all zero, as a buffer is before anything arrived, are the
 * empty view, and the empty view packs to them.
 */
TEST(IndicatorViewPacking, BytesThatAreAllZeroAreTheEmptyView) {
    uint8_t zeros[INDICATOR_VIEW_PACKED_SIZE];
    memset(zeros, 0x00, sizeof(zeros));

    indicator_view_t unpacked;
    memset(&unpacked, 0xFF, sizeof(unpacked));
    indicator_view_unpack(zeros, &unpacked);
    expect_same_view(unpacked, empty_view());

    const indicator_view_t empty = empty_view();
    uint8_t                packed[INDICATOR_VIEW_PACKED_SIZE];
    memset(packed, 0xFF, sizeof(packed));
    indicator_view_pack(&empty, packed);
    EXPECT_EQ(memcmp(packed, zeros, INDICATOR_VIEW_PACKED_SIZE), 0);
}

/****************************************************************************
 * "Send now"
 *
 * The literal times are those of the ADR: 500 ms between two sends of a view
 * that did not change, 2 seconds until a view that is not renewed expires.
 ****************************************************************************/

/* The defaults of the two times are those of the ADR. */
TEST(IndicatorViewTimes, DefaultsAreThoseOfTheAdr) {
    EXPECT_EQ(INDICATOR_VIEW_SEND_INTERVAL, 500);
    EXPECT_EQ(INDICATOR_VIEW_TIMEOUT, 2000);
}

/* A view that changed is sent at once, however short ago the last send was. */
TEST(IndicatorViewSendNow, ViewThatChangedIsSentAtOnce) {
    EXPECT_TRUE(indicator_view_should_send(true, false, 0));
    EXPECT_TRUE(indicator_view_should_send(true, false, 499));
}

/* After a send that failed, the view is sent again at once, changed or not. */
TEST(IndicatorViewSendNow, ViewIsSentAgainAtOnceAfterASendThatFailed) {
    EXPECT_TRUE(indicator_view_should_send(false, true, 0));
    EXPECT_TRUE(indicator_view_should_send(false, true, 499));
}

/*
 * A view that did not change is sent again once 500 ms have passed since the
 * last send, and not before.
 */
TEST(IndicatorViewSendNow, ViewThatDidNotChangeIsSentAgainAfter500Ms) {
    EXPECT_FALSE(indicator_view_should_send(false, false, 499));
    EXPECT_TRUE(indicator_view_should_send(false, false, 500));
    EXPECT_TRUE(indicator_view_should_send(false, false, 501));
}

/* With none of the three reasons nothing is sent. */
TEST(IndicatorViewSendNow, NothingIsSentWithoutAReason) {
    EXPECT_FALSE(indicator_view_should_send(false, false, 0));
    EXPECT_FALSE(indicator_view_should_send(false, false, 1));
}

/****************************************************************************
 * "The view has expired"
 ****************************************************************************/

/* Before the first view arrived there is no view, however short the wait. */
TEST(IndicatorViewExpiry, ViewHasExpiredBeforeTheFirstViewArrived) {
    EXPECT_TRUE(indicator_view_has_expired(false, 0));
    EXPECT_TRUE(indicator_view_has_expired(false, 1999));
}

/*
 * A view that arrived stays until 2 seconds have passed since it arrived, and
 * has expired from then on.
 */
TEST(IndicatorViewExpiry, ViewThatArrivedExpiresAfter2Seconds) {
    EXPECT_FALSE(indicator_view_has_expired(true, 0));
    EXPECT_FALSE(indicator_view_has_expired(true, 1999));
    EXPECT_TRUE(indicator_view_has_expired(true, 2000));
    EXPECT_TRUE(indicator_view_has_expired(true, 2001));
}
