/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Utilities
*/

#ifndef HydroponicsUtils_H
#define HydroponicsUtils_H

struct HydroponicsBitResolution;

#include "Hydroponics.h"

// Simple class for describing an analog bit resolution.
// This class is mainly used to calculate analog pin range boundaries.
struct HydroponicsBitResolution {
    HydroponicsBitResolution(byte bitRes);

    // Transforms value from raw integer (or initial) value into normalized raw (or transformed) value.
    inline float transform(int intValue) const { return constrain((float)maxVal / intValue, 0.0f, 1.0f); }

    // Inverse transforms value from normalized raw (or transformed) value back into raw integer (or initial) value.
    inline int inverseTransform(float rawValue) const { return constrain((int)((float)maxVal * rawValue), 0, maxVal); }

    byte bitRes;                                    // Bit resolution
    int maxVal;                                     // Maximum value
};


// This will query the active RTC sync device for the current time.
extern time_t rtcNow();

// Tries to convert value from one unit to another (if supported), returning conversion status.
extern bool tryConvertStdUnits(float valueIn, Hydroponics_UnitsType unitsIn, float *valueOut, Hydroponics_UnitsType unitsOut);
// Attempts to convert value in-place from one unit to another, and if successful then assigns those values back overtop of source, with optional decimal place rounding.
extern void convertStdUnits(float *valueInOut, Hydroponics_UnitsType *unitsInOut, Hydroponics_UnitsType unitsOut, int roundNumDecPlaces = -1);

extern bool checkPinIsAnalogInput(byte pin);    // Checks to see if the pin is an analog input pin.
extern bool checkPinIsAnalogOutput(byte pin);   // Checks to see if the pin is an analog output pin.
extern bool checkPinIsDigital(byte pin);        // Checks to see if the pin is a standard digital (non-analog) pin.
extern bool checkPinCanPWMOutput(byte pin);     // Checks to see if the pin can produce a PWM output signal.
extern bool checkPinCanInterrupt(byte pin);     // Checks to see if the pin can be set up with an ISR to handle level changes.

// Converts from actuator type enum to string, with optional exclude for special types (instead returning "").
extern String actuatorTypeToString(Hydroponics_ActuatorType actuatorType, bool excludeSpecial = false);
// Converts back to actuator type enum from string.
extern Hydroponics_ActuatorType actuatorTypeFromString(String actuatorTypeStr);

// Converts from sensor type enum to string, with optional exclude for special types (instead returning "").
extern String sensorTypeToString(Hydroponics_SensorType sensorType, bool excludeSpecial = false);
// Converts back to sensor type enum from string.
extern Hydroponics_SensorType sensorTypeFromString(String sensorTypeStr);

// Converts from crop type enum to string, with optional exclude for special types (instead returning "").
extern String cropTypeToString(Hydroponics_CropType cropType, bool excludeSpecial = false);
// Converts back to crop type enum from string.
extern Hydroponics_CropType cropTypeFromString(String cropTypeStr);

// Converts from fluid reservoir enum to string, with optional exclude for special types (instead returning "").
extern String reservoirTypeToString(Hydroponics_ReservoirType reservoirType, bool excludeSpecial = false);
// Converts back to fluid reservoir enum from string
extern Hydroponics_ReservoirType reservoirTypeFromString(String reservoirTypeStr);

// Converts from power rail enum to string, with optional exclude for special types (instead returning "")
extern String railTypeToString(Hydroponics_RailType railType, bool excludeSpecial = false);
// Converts back to power rail enum from string
extern Hydroponics_RailType railTypeFromString(String railTypeStr);

// Converts from units type enum to symbol string, with optional exclude for special types (instead returning "")
extern String unitsTypeToSymbol(Hydroponics_UnitsType unitsType, bool excludeSpecial = false);
// Converts back to units type enum from symbol
extern Hydroponics_UnitsType unitsTypeFromSymbol(String unitsSymbolStr);

// Converts from position index to string, with optional exclude for special types (instead returning "");
extern String positionIndexToString(Hydroponics_PositionIndex positionIndex, bool excludeSpecial = false);
// Converts back to position index from string
extern Hydroponics_PositionIndex positionIndexFromString(String positionIndexStr);

#endif // /ifndef HydroponicsUtils_H
