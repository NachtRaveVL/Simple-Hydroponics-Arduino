/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Utilities
*/

#ifndef HydroponicsUtils_H
#define HydroponicsUtils_H

#include "Hydroponics.h"

extern time_t rtcNow();

extern bool tryConvertValue(float valueIn, Hydroponics_UnitsType unitsIn, float *valueOut, Hydroponics_UnitsType unitsOut);
extern void convertAndAssign(float *valueInOut, Hydroponics_UnitsType *unitsInOut, Hydroponics_UnitsType unitsOut, int roundToDecPlaces = -1);

extern bool checkInputPinIsAnalog(int pin);
extern bool checkOutputPinIsAnalog(int pin);
extern bool checkPinIsDigital(int pin);
extern bool checkPinIsPWM(int pin);

#endif // /ifndef HydroponicsUtils_H
