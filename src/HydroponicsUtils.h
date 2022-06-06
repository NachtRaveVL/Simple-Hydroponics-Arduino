/*  Arduino Controller for Simple Hydroponics.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Utilities
*/

#ifndef HydroponicsUtils_H
#define HydroponicsUtils_H

#include "Hydroponics.h"

extern time_t rtcNow();

extern bool checkInputPinIsAnalog(int pin);
extern bool checkOutputPinIsAnalog(int pin);
extern bool checkPinIsDigital(int pin);
extern bool checkPinIsPWM(int pin);

#endif // /ifndef HydroponicsUtils_H
