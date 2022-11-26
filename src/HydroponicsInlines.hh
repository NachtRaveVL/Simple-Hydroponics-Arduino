/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Common Inlines
*/
#ifndef HydroponicsInlines_HPP
#define HydroponicsInlines_HPP

#include "Hydroponics.h"

// Checks if pin is valid or not.
static inline bool isValidPin(pintype_t pin) { return pin != (pintype_t)-1; }
// Checks if measurement row is valid or not.
static inline bool isValidRow(uint8_t row) { return row != (uint8_t)-1; }
// Checks if taskId is valid or not.
static inline bool isValidTask(unsigned int taskId) { return taskId != 0xffffU; }

// Checks if two single-precision floating point values are equal with respect to defined error epsilon.
static inline bool isFPEqual(float lhs, float rhs) { return fabsf(rhs - lhs) <= FLT_EPSILON; }
// Checks if two double-precision floating point values are equal with respect to defined error epsilon.
static inline bool isFPEqual(double lhs, double rhs) { return fabs(rhs - lhs) <= DBL_EPSILON; }

// Returns the first unit in parameter list that isn't undefined, allowing defaulting chains to be nicely defined.
static inline Hydroponics_UnitsType definedUnitsElse(Hydroponics_UnitsType units1, Hydroponics_UnitsType units2) {
    return units1 != Hydroponics_UnitsType_Undefined ? units1 : units2;
}

// Returns the first unit in parameter list that isn't undefined, allowing defaulting chains to be nicely defined.
static inline Hydroponics_UnitsType definedUnitsElse(Hydroponics_UnitsType units1, Hydroponics_UnitsType units2, Hydroponics_UnitsType units3) {
    return units1 != Hydroponics_UnitsType_Undefined ? units1 : (units2 != Hydroponics_UnitsType_Undefined ? units2 : units3);
}

// Rounds floating point value to the number of decimal places.
static inline float roundToDecimalPlaces(float value, int decimalPlaces) {
    if (decimalPlaces >= 0) {
        float shiftScaler = powf(10.0f, decimalPlaces);
        return roundf(value * shiftScaler) / shiftScaler;
    }
    return value;
}

#endif // /ifndef HydroponicsInlines_HPP
