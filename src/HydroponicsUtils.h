/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Utilities
*/

#ifndef HydroponicsUtils_H
#define HydroponicsUtils_H

struct HydroponicsBitResolution;
template<class T> struct HydroponicsDLinkObject;
template<typename ParameterType, int Slots> class SignalFireTask;

#include "Hydroponics.h"
#include "HydroponicsObject.h"

// Simple class for describing an analog bit resolution.
// This class is mainly used to calculate analog pin range boundaries. If override flag
// is not set then architecture check is made that may truncate passed bit resolution.
struct HydroponicsBitResolution {
    HydroponicsBitResolution(byte bitRes, bool override = false);

    // Transforms value from raw integer (or initial) value into normalized raw (or transformed) value.
    inline float transform(int intValue) const { return constrain((float)maxVal / intValue, 0.0f, 1.0f); }

    // Inverse transforms value from normalized raw (or transformed) value back into raw integer (or initial) value.
    inline int inverseTransform(float rawValue) const { return constrain((int)((float)maxVal * rawValue), 0, maxVal); }

    byte bitRes;                                // Bit resolution (# of bits)
    int maxVal;                                 // Maximum value (2 ^ (# of bits))
};


// Helpers & Misc

// Returns the active hydroponics instance.
extern Hydroponics *getHydroponicsInstance();

// Computes a hash for a string using a fast and efficient hashing function.
extern Hydroponics_KeyType stringHash(const String &str);

// Returns a string from char array with an exact max length.
extern String stringFromChars(const char *chars, size_t length);

// This will query the active RTC sync device for the current time.
extern time_t rtcNow();

// This will schedule a signal's fire method on the next TaskManagerIO runloop using the given parameter.
template<typename ParameterType, int Slots>
taskid_t scheduleSignalFireOnce(Signal<ParameterType,Slots> &signal, ParameterType fireParam);

#ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT

// This logs a standard message to Serial output (if connected) and then forwards it to the active Hydroponics instance's logging mechanism (if any are enabled).
extern void logMessage(String message, bool flushAfter = false);

// This softly asserts on a failed condition, sending message out to Serial output (if debugging enabled) and/or disk-based logging (if enabled), and then finally continuing program execution.
// See HYDRUINO_SOFT_ASSERT() macro for usage.
extern void softAssert(bool cond, String msg, const char *file, const char *func, int line);

// This hard asserts on a failed condition, sending message out to Serial output (if debugging enabled) and/or disk-based logging (if enabled), then yielding (to allow comms), and then finally aborting program execution.
// See HYDRUINO_HARD_ASSERT() macro for usage.
extern void hardAssert(bool cond, String msg, const char *file, const char *func, int line);

#endif // /ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT

// Units & Conversion

// Tries to convert value from one unit to another (if supported), returning conversion status.
extern bool tryConvertStdUnits(float valueIn, Hydroponics_UnitsType unitsIn, float *valueOut, Hydroponics_UnitsType unitsOut);
// Attempts to convert value in-place from one unit to another, and if successful then assigns those values back overtop of source, with optional decimal place rounding.
extern void convertStdUnits(float *valueInOut, Hydroponics_UnitsType *unitsInOut, Hydroponics_UnitsType unitsOut, int roundToDecPlaces = -1);

extern Hydroponics_UnitsType defaultTemperatureUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);      // Returns default temperature units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultDistanceUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);         // Returns default distance units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultWeightUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);           // Returns default weight units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultLiquidVolumeUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);     // Returns default liquid volume units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultLiquidFlowUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);       // Returns default liquid flow units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultConcentrationUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);    // Returns default concentration units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern int defaultDecimalPlacesRounding(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);                   // Returns default decimal places rounded to based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).

// Pins & Checks

extern bool checkPinIsAnalogInput(byte pin);    // Checks to see if the pin is an analog input pin.
extern bool checkPinIsAnalogOutput(byte pin);   // Checks to see if the pin is an analog output pin.
extern bool checkPinIsDigital(byte pin);        // Checks to see if the pin is a standard digital (non-analog) pin.
extern bool checkPinIsPWMOutput(byte pin);      // Checks to see if the pin can produce a PWM output signal.
extern bool checkPinCanInterrupt(byte pin);     // Checks to see if the pin can be set up with an ISR to handle level changes.

// Enum & String Conversions

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
