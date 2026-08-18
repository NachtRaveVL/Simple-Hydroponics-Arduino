/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Core Logic Helpers
*/

#ifndef HydroCoreLogic_H
#define HydroCoreLogic_H

#include <stdint.h>
#include <stddef.h>
#include <math.h>

inline uint32_t hydroElapsedTime(uint32_t now, uint32_t start)
{
    return (uint32_t)(now - start);
}

inline bool hydroHasElapsed(uint32_t now, uint32_t start, uint32_t duration)
{
    return hydroElapsedTime(now, start) >= duration;
}

inline int hydroCropPhaseForWeek(int growWeek, const uint8_t *phaseDurationWeeks, int phaseCount)
{
    if (phaseCount <= 0) { return 0; }

    int phaseEndWeek = 0;

    for (int phaseIndex = 0; phaseIndex < phaseCount; ++phaseIndex) {
        phaseEndWeek += phaseDurationWeeks[phaseIndex];
        if (growWeek < phaseEndWeek) { return phaseIndex; }
    }

    return phaseCount;
}

inline bool hydroUpdateStableBinaryState(bool acceptedState, bool sampledState, uint32_t nowMillis,
                                         uint16_t stableTimeMillis, bool &pendingState,
                                         bool &hasPendingState, uint32_t &pendingStateStart)
{
    if (sampledState == acceptedState) {
        hasPendingState = false;
    } else if (!stableTimeMillis) {
        hasPendingState = false;
        return sampledState;
    } else if (!hasPendingState || pendingState != sampledState) {
        pendingState = sampledState;
        pendingStateStart = nowMillis;
        hasPendingState = true;
    } else if (hydroHasElapsed(nowMillis, pendingStateStart, stableTimeMillis)) {
        hasPendingState = false;
        return sampledState;
    }

    return acceptedState;
}

inline uint32_t hydroFeedingIntervalSeconds(uint8_t feedingsPerDay, uint8_t feedingsPerWeek, uint16_t feedIntervalMins)
{
    if (feedingsPerDay) {
        return (24UL * 60UL * 60UL) / feedingsPerDay;
    }
    if (feedingsPerWeek) {
        return (7UL * 24UL * 60UL * 60UL) / feedingsPerWeek;
    }
    return (uint32_t)feedIntervalMins * 60UL;
}

inline bool hydroFeedingDue(uint32_t now, uint32_t lastFeedingTime, uint32_t intervalSeconds)
{
    return intervalSeconds && (!lastFeedingTime || hydroHasElapsed(now, lastFeedingTime, intervalSeconds));
}

inline bool hydroTimedFeedingNeeded(uint32_t now, uint32_t lastFeedingTime, uint32_t feedDurationSeconds, uint32_t intervalSeconds)
{
    if (!intervalSeconds) { return false; }
    if (!lastFeedingTime) { return true; }

    const uint32_t elapsed = hydroElapsedTime(now, lastFeedingTime);
    return elapsed < feedDurationSeconds || elapsed >= intervalSeconds;
}

inline int hydroBalancingStateForValue(float value, float targetSetpoint, float targetRange)
{
    const float halfTargetRange = fabsf(targetRange) * 0.5f;
    if (value < targetSetpoint - halfTargetRange) { return 0; }
    if (value > targetSetpoint + halfTargetRange) { return 2; }
    return 1;
}

inline int hydroBalancingCorrectionForState(int balancingState)
{
    if (balancingState == 0) { return 1; }
    if (balancingState == 2) { return -1; }
    return 0;
}

inline float hydroEstimateDosingMillis(float targetSetpoint, float dosingValue, float lastDosingValue,
                                       float lastDosingMillis, float baseDosingMillis,
                                       float minFraction, float maxFraction)
{
    float dosing = baseDosingMillis;
    if (lastDosingMillis > 0.0f) {
        const float responsePerMs = fabsf(dosingValue - lastDosingValue) / lastDosingMillis;
        if (responsePerMs > 0.000001f) {
            dosing = fabsf(targetSetpoint - dosingValue) / responsePerMs;
        }
    }

    const float minDosing = baseDosingMillis * minFraction;
    const float maxDosing = baseDosingMillis * maxFraction;
    if (dosing < minDosing) { dosing = minDosing; }
    if (dosing > maxDosing) { dosing = maxDosing; }
    return dosing;
}

struct HydroBinaryDataReadPlan
{
    size_t copyBytes;
    size_t skipBytes;
};

inline HydroBinaryDataReadPlan hydroBinaryDataReadPlan(size_t serializedSize, size_t currentSize, size_t baseSize)
{
    if (serializedSize < baseSize || currentSize < baseSize) { return {0, 0}; }

    const size_t serializedRemaining = serializedSize - baseSize;
    const size_t currentRemaining = currentSize - baseSize;
    const size_t copyBytes = serializedRemaining < currentRemaining ? serializedRemaining : currentRemaining;
    return {copyBytes, serializedRemaining - copyBytes};
}

#endif // /ifndef HydroCoreLogic_H
