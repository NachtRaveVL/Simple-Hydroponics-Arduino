/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Inlines
*/
#ifndef HydroponicsInlines_HPP
#define HydroponicsInlines_HPP

#include "Hydroponics.h"

// Checks if pin is valid or not.
static inline bool isValidPin(byte pin) { return pin != -1; }

// Checks if two floating point values are equal with respect to defined error epsilon.
static inline bool isFPEqual(float lhs, float rhs) { return fabsf(rhs - lhs) <= FLT_EPSILON; }

// Rounds floating point value to the number of decimal places.
static inline float roundToDecPlaces(float valueIn, int numDecPlaces) {
    if (numDecPlaces >= 0) {
        float shiftScaler = powf(10.0f, numDecPlaces);
        return roundf(valueIn * shiftScaler) / shiftScaler;
    }
    return valueIn;
}

#endif // /ifndef HydroponicsInlines_HPP
