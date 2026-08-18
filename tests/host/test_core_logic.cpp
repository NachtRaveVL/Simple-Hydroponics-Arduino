#include <cassert>
#include <cstdint>
#include <cmath>

#include "HydroCoreLogic.h"

static bool nearlyEqual(float lhs, float rhs, float eps = 0.001f)
{
    return std::fabs(lhs - rhs) <= eps;
}

static void testElapsedTime()
{
    assert(hydroElapsedTime(150, 100) == 50);
    assert(!hydroHasElapsed(149, 100, 50));
    assert(hydroHasElapsed(150, 100, 50));

    const uint32_t start = UINT32_MAX - 24;
    assert(hydroElapsedTime(25, start) == 50);
    assert(!hydroHasElapsed(24, start, 50));
    assert(hydroHasElapsed(25, start, 50));
}

static void testCropPhases()
{
    const uint8_t phases[3] = {2, 4, 8};

    assert(hydroCropPhaseForWeek(0, phases, 0) == 0);
    assert(hydroCropPhaseForWeek(0, phases, -1) == 0);
    assert(hydroCropPhaseForWeek(0, phases, 3) == 0);
    assert(hydroCropPhaseForWeek(1, phases, 3) == 0);
    assert(hydroCropPhaseForWeek(2, phases, 3) == 1);
    assert(hydroCropPhaseForWeek(5, phases, 3) == 1);
    assert(hydroCropPhaseForWeek(6, phases, 3) == 2);
    assert(hydroCropPhaseForWeek(13, phases, 3) == 2);
    assert(hydroCropPhaseForWeek(14, phases, 3) == 3);
}

static void testFeedingCadence()
{
    assert(hydroFeedingIntervalSeconds(3, 0, 0) == 8UL * 60UL * 60UL);
    assert(hydroFeedingIntervalSeconds(0, 2, 0) == 84UL * 60UL * 60UL);
    assert(hydroFeedingIntervalSeconds(0, 0, 90) == 90UL * 60UL);

    assert(hydroFeedingDue(1000, 0, 600));
    assert(!hydroFeedingDue(1599, 1000, 600));
    assert(hydroFeedingDue(1600, 1000, 600));
    assert(!hydroFeedingDue(1000, 500, 0));

    // Once a feeding begins, the crop remains hungry for the configured run duration.
    assert(hydroTimedFeedingNeeded(1000, 0, 15 * 60, 60 * 60));
    assert(hydroTimedFeedingNeeded(1000 + 14 * 60, 1000, 15 * 60, 60 * 60));
    assert(!hydroTimedFeedingNeeded(1000 + 15 * 60, 1000, 15 * 60, 60 * 60));
    assert(!hydroTimedFeedingNeeded(1000 + 59 * 60, 1000, 15 * 60, 60 * 60));
    assert(hydroTimedFeedingNeeded(1000 + 60 * 60, 1000, 15 * 60, 60 * 60));
    assert(!hydroTimedFeedingNeeded(1000, 500, 15 * 60, 0));

    // Elapsed-time checks remain safe when the 32-bit clock wraps.
    assert(hydroFeedingDue(100, UINT32_MAX - 100, 150));
    assert(hydroTimedFeedingNeeded(100, UINT32_MAX - 100, 250, 300));
}

static void testBinaryDebounce()
{
    bool pendingState = false;
    bool hasPendingState = false;
    uint32_t pendingStart = 0;
    bool accepted = false;

    accepted = hydroUpdateStableBinaryState(accepted, true, 0, 100, pendingState, hasPendingState, pendingStart);
    assert(!accepted && hasPendingState && pendingState && pendingStart == 0);

    accepted = hydroUpdateStableBinaryState(accepted, true, 50, 100, pendingState, hasPendingState, pendingStart);
    assert(!accepted && hasPendingState);

    // A bounce back to the accepted state cancels the pending change.
    accepted = hydroUpdateStableBinaryState(accepted, false, 60, 100, pendingState, hasPendingState, pendingStart);
    assert(!accepted && !hasPendingState);

    accepted = hydroUpdateStableBinaryState(accepted, true, 100, 100, pendingState, hasPendingState, pendingStart);
    assert(!accepted && hasPendingState);
    accepted = hydroUpdateStableBinaryState(accepted, true, 199, 100, pendingState, hasPendingState, pendingStart);
    assert(!accepted);
    accepted = hydroUpdateStableBinaryState(accepted, true, 200, 100, pendingState, hasPendingState, pendingStart);
    assert(accepted && !hasPendingState);

    accepted = hydroUpdateStableBinaryState(accepted, false, 201, 0, pendingState, hasPendingState, pendingStart);
    assert(!accepted && !hasPendingState);
}

static int balancingCorrection(float value, float targetSetpoint, float targetRange)
{
    return hydroBalancingCorrectionForState(hydroBalancingStateForValue(value, targetSetpoint, targetRange));
}

static void testBalancerStates()
{
    assert(hydroBalancingStateForValue(5.79f, 6.0f, 0.4f) == 0);
    assert(hydroBalancingStateForValue(5.80f, 6.0f, 0.4f) == 1);
    assert(hydroBalancingStateForValue(6.00f, 6.0f, 0.4f) == 1);
    assert(hydroBalancingStateForValue(6.20f, 6.0f, 0.4f) == 1);
    assert(hydroBalancingStateForValue(6.21f, 6.0f, 0.4f) == 2);

    // Small noisy movements inside the target range must remain balanced.
    assert(hydroBalancingStateForValue(5.91f, 6.0f, 0.4f) == 1);
    assert(hydroBalancingStateForValue(6.09f, 6.0f, 0.4f) == 1);

    // Representative environment controls must drive in the correct direction,
    // settle inside their target range, and reverse only after crossing the opposite edge.
    assert(balancingCorrection(5.5f, 6.0f, 0.4f) == 1);   // pH: dose upward
    assert(balancingCorrection(6.0f, 6.0f, 0.4f) == 0);
    assert(balancingCorrection(6.5f, 6.0f, 0.4f) == -1);  // pH: dose downward

    assert(balancingCorrection(1.5f, 1.8f, 0.2f) == 1);   // EC: add nutrient
    assert(balancingCorrection(1.8f, 1.8f, 0.2f) == 0);
    assert(balancingCorrection(2.1f, 1.8f, 0.2f) == -1);  // EC: dilute

    assert(balancingCorrection(19.0f, 22.0f, 2.0f) == 1);  // temperature: heat
    assert(balancingCorrection(22.0f, 22.0f, 2.0f) == 0);
    assert(balancingCorrection(25.0f, 22.0f, 2.0f) == -1); // temperature: cool

    assert(balancingCorrection(700.0f, 900.0f, 100.0f) == 1);   // CO2: enrich
    assert(balancingCorrection(900.0f, 900.0f, 100.0f) == 0);
    assert(balancingCorrection(1100.0f, 900.0f, 100.0f) == -1); // CO2: exhaust
}

static void testTimedDosingEstimate()
{
    // Large remaining error is capped to the configured maximum dose increase.
    float dose = hydroEstimateDosingMillis(6.0f, 5.2f, 5.0f, 1000.0f, 1000.0f, 0.5f, 1.5f);
    assert(nearlyEqual(dose, 1500.0f));

    // Small remaining error is capped to the configured minimum dose.
    dose = hydroEstimateDosingMillis(6.0f, 5.9f, 5.7f, 1000.0f, 1000.0f, 0.5f, 1.5f);
    assert(nearlyEqual(dose, 500.0f));

    // No measurable response falls back to the base dose instead of dividing by zero.
    dose = hydroEstimateDosingMillis(6.0f, 5.0f, 5.0f, 1000.0f, 1000.0f, 0.5f, 1.5f);
    assert(nearlyEqual(dose, 1000.0f));

    // Falling readings are handled symmetrically for a TooHigh correction path.
    dose = hydroEstimateDosingMillis(6.0f, 6.3f, 6.5f, 1000.0f, 1000.0f, 0.5f, 1.5f);
    assert(nearlyEqual(dose, 1500.0f));
}

static void testBinaryDataReadPlan()
{
    auto same = hydroBinaryDataReadPlan(100, 100, 20);
    assert(same.copyBytes == 80 && same.skipBytes == 0);

    // Older append-only records copy what exists and leave new fields at constructor defaults.
    auto older = hydroBinaryDataReadPlan(80, 100, 20);
    assert(older.copyBytes == 60 && older.skipBytes == 0);

    // Newer append-only records copy the known prefix and skip unknown trailing fields.
    auto newer = hydroBinaryDataReadPlan(120, 100, 20);
    assert(newer.copyBytes == 80 && newer.skipBytes == 20);

    auto invalidSerialized = hydroBinaryDataReadPlan(10, 100, 20);
    assert(invalidSerialized.copyBytes == 0 && invalidSerialized.skipBytes == 0);

    auto invalidCurrent = hydroBinaryDataReadPlan(100, 10, 20);
    assert(invalidCurrent.copyBytes == 0 && invalidCurrent.skipBytes == 0);
}

int main()
{
    testElapsedTime();
    testBinaryDataReadPlan();
    testCropPhases();
    testFeedingCadence();
    testBinaryDebounce();
    testBalancerStates();
    testTimedDosingEstimate();
    return 0;
}
