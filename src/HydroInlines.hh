/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Common Inlines
*/
#ifndef HydroInlines_HPP
#define HydroInlines_HPP

#include "Hydruino.h"

// Returns if pin is valid
inline bool isValidPin(pintype_t pin) { return pin != (pintype_t)-1; }
// Returns if channel is valid
inline bool isValidChannel(uint8_t channel) { return channel != (uint8_t)-1; }
// Returns if measurement row is valid
inline bool isValidRow(uint8_t row) { return row != (uint8_t)-1; }
// Returns if taskId is valid
inline bool isValidTask(unsigned int taskId) { return taskId != 0xffffU; } // purposeful, not using library define incase not included
// Returns if time millis is valid
inline bool isValidTime(millis_t time) { return time != millis_none; }
// Returns if position index is valid
inline bool isValidIndex(hposi_t index) { return index != hposi_none; }
// Returns if id key is valid
inline bool isValidKey(hkey_t key) { return key != hkey_none; }
// Returns if id type is valid
inline bool isValidType(hid_t type) { return type != hid_none; }
// Returns if frame is valid
inline bool isValidFrame(hframe_t frame) { return frame != hframe_none; }

// Returns if two single-precision floating point values are equal with respect to defined error epsilon.
inline bool isFPEqual(float lhs, float rhs) { return fabsf(rhs - lhs) <= FLT_EPSILON; }
// Returns if two double-precision floating point values are equal with respect to defined error epsilon.
inline bool isFPEqual(double lhs, double rhs) { return fabs(rhs - lhs) <= DBL_EPSILON; }

// Returns the first unit in parameter list that isn't undefined, allowing defaulting chains to be nicely defined.
inline Hydro_UnitsType definedUnitsElse(Hydro_UnitsType units1, Hydro_UnitsType units2) {
    return units1 != Hydro_UnitsType_Undefined ? units1 : units2;
}

// Returns the first unit in parameter list that isn't undefined, allowing defaulting chains to be nicely defined.
inline Hydro_UnitsType definedUnitsElse(Hydro_UnitsType units1, Hydro_UnitsType units2, Hydro_UnitsType units3) {
    return units1 != Hydro_UnitsType_Undefined ? units1 : (units2 != Hydro_UnitsType_Undefined ? units2 : units3);
}

// Rounds floating point value to the number of decimal places.
inline float roundToDecimalPlaces(float value, int decimalPlaces) {
    if (decimalPlaces >= 0) {
        float shiftScaler = powf(10.0f, decimalPlaces);
        return roundf(value * shiftScaler) / shiftScaler;
    }
    return value;
}

#endif // /ifndef HydroInlines_HPP
