#include <cassert>
#include <cstdint>

#include "HydroCoreLogic.h"

static void testBinaryDebounceRollover()
{
    bool pendingState = true;
    bool hasPendingState = true;
    uint32_t pendingStart = UINT32_MAX - 50;
    bool accepted = false;

    accepted = hydroUpdateStableBinaryState(accepted, true, 48, 100,
                                             pendingState, hasPendingState, pendingStart);
    assert(!accepted && hasPendingState);

    accepted = hydroUpdateStableBinaryState(accepted, true, 49, 100,
                                             pendingState, hasPendingState, pendingStart);
    assert(accepted && !hasPendingState);
}

static void testBalancerRangeSymmetry()
{
    assert(hydroBalancingStateForValue(5.79f, 6.0f, 0.4f) ==
           hydroBalancingStateForValue(5.79f, 6.0f, -0.4f));
    assert(hydroBalancingStateForValue(6.00f, 6.0f, 0.4f) ==
           hydroBalancingStateForValue(6.00f, 6.0f, -0.4f));
    assert(hydroBalancingStateForValue(6.21f, 6.0f, 0.4f) ==
           hydroBalancingStateForValue(6.21f, 6.0f, -0.4f));
}

static void testDisabledFeedingCadence()
{
    assert(hydroFeedingIntervalSeconds(0, 0, 0) == 0);
    assert(!hydroFeedingDue(1000, 0, 0));
    assert(!hydroTimedFeedingNeeded(1000, 0, 60, 0));
}

static void testBaseOnlyBinaryRecord()
{
    auto baseOnly = hydroBinaryDataReadPlan(20, 100, 20);
    assert(baseOnly.copyBytes == 0);
    assert(baseOnly.skipBytes == 0);
}

int main()
{
    testBinaryDebounceRollover();
    testBalancerRangeSymmetry();
    testDisabledFeedingCadence();
    testBaseOnlyBinaryRecord();
    return 0;
}
