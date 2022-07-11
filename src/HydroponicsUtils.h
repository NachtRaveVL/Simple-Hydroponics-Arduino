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
    byte bitRes;                                // Bit resolution (# of bits)
    int maxVal;                                 // Maximum value (2 ^ (# of bits))

    // Convenience constructor
    HydroponicsBitResolution(byte bitRes, bool override = false);

    // Transforms value from raw integer (or initial) value into normalized raw (or transformed) value.
    inline float transform(int intValue) const { return constrain((float)maxVal / intValue, 0.0f, 1.0f); }

    // Inverse transforms value from normalized raw (or transformed) value back into raw integer (or initial) value.
    inline int inverseTransform(float rawValue) const { return constrain((int)((float)maxVal * rawValue), 0, maxVal); }
};


// Helpers & Misc

// Returns the active hydroponics instance. Not guaranteed to be non-null.
extern Hydroponics *getHydroponicsInstance();
// Returns the active scheduler instance. Not guaranteed to be non-null.
extern HydroponicsScheduler *getSchedulerInstance();

// Returns current time, with proper time zone offset based on active hydroponics instance.
DateTime getCurrentTime();
// Returns the UTC unix time that today started, accounting for time zone offset based on active hydroponics instance.
time_t getCurrentDayStartTime();

// Computes a hash for a string using a fast and efficient (read as: good enough for our use) hashing algorithm.
extern Hydroponics_KeyType stringHash(String string);

// Returns a string from char array with an exact max length.
// Null array or invalid length will abort function before encoding occurs, returning "null".
extern String stringFromChars(const char *charsIn, size_t length);

// Encodes a T-typed array to a comma-separated string.
// Null array or invalid length will abort function before encoding occurs, returning "null".
template<typename T>
String commaStringFromArray(const T *arrayIn, size_t length);
// Decodes a comma-separated string back to a T-typed array.
// Last value read is repeated on through to last element, with no commas being treated as single value being applied to all elements.
// Empty string, string value of "null", null array, or invalid length will abort function before decoding.
template<typename T>
void commaStringToArray(String stringIn, T *arrayOut, size_t length);
// Decodes a comma-separated JSON variant, if not null, object, or array, back to a T-typed array.
// Acts as a redirect for JSON variants so that they receive the additional checks before being converted to string.
template<typename T>
void commaStringToArray(JsonVariantConst &variantIn, T *arrayOut, size_t length);

// Encodes a byte array to hexadecimal string.
extern String hexStringFromBytes(const byte *bytesIn, size_t length);
// Decodes a hexadecimal string back to a byte array.
extern void hexStringToBytes(String stringIn, byte *bytesOut, size_t length);
// Decodes a hexadecimal JSON variant, if not null, object, or array, back to a byte array.
extern void hexStringToBytes(JsonVariantConst &variantIn, byte *bytesOut, size_t length);

// Returns # of occurrences of character in string.
int occurrencesInString(String string, char singleChar);
// Returns # of occurrences of substring in string.
int occurrencesInString(String string, String subString);
// Returns # of occurrences of character in string, ignoring case.
int occurrencesInStringIgnoreCase(String string, char singleChar);
// Returns # of occurrences of substring in string, ignoring case.
int occurrencesInStringIgnoreCase(String string, String subString);

// Returns whenever all elements of an array are equal to the specified value, or not.
template<typename T>
bool arrayElementsEqual(const T *arrayIn, size_t length, T value);

// Similar to the standard map function, but does it on any type.
template<typename T>
T mapValue(T value, T inMin, T inMax, T outMin, T outMax);

// Returns the amount of space between the stack and heap (ie free space left), else -1 if undeterminable.
extern int freeMemory();

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
extern bool convertStdUnits(float *valueInOut, Hydroponics_UnitsType *unitsInOut, Hydroponics_UnitsType unitsOut, int roundToDecPlaces = -1);

// Returns default temperature units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultTemperatureUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);
// Returns default distance units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultDistanceUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);
// Returns default weight units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultWeightUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);
// Returns default liquid volume units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultWaterVolumeUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);
// Returns default liquid flow units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultLiquidFlowUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);
// Returns default liquid dilution units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultLiquidDilutionUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);
// Returns default concentration units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultConcentrationUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);
// Returns default decimal places rounded to based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern int defaultDecimalPlacesRounding(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);

// Linkages Filtering

// Returns links list filtered down to actuators.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterActuators(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links);
// Returns links list filtered down to actuators by type.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterActuatorsByType(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links, Hydroponics_ActuatorType actuatorType);
// Returns links list filtered down to sensors.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterSensors(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links);
// Returns links list filtered down to sensors by type.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterSensorsByType(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links, Hydroponics_SensorType sensorType);
// Returns links list filtered down to crops.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterCrops(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links);
// Returns links list filtered down to crops by type.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterCropsByType(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links, Hydroponics_CropType cropType);
// Returns links list filtered down to reservoirs.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterReservoirs(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links);
// Returns links list filtered down to reservoirs by type.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterReservoirsByType(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links, Hydroponics_ReservoirType reservoirType);
// Returns links list filtered down to rails.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterRails(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links);
// Returns links list filtered down to rails by type.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterRailsByType(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links, Hydroponics_RailType railType);
// Returns links list filtered down to pump actuators by source reservoir.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterPumpActuatorsByInputReservoir(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links, HydroponicsReservoir *inputReservoir);
// Returns links list filtered down to pump actuators by source reservoir type.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterPumpActuatorsByInputReservoirType(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links, Hydroponics_ReservoirType reservoirType);
// Returns links list filtered down to pump actuators by destination reservoir.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterPumpActuatorsByOutputReservoir(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links, HydroponicsReservoir *outputReservoir);
// Returns links list filtered down to pump actuators by destination reservoir type.
template<size_t N> arx::map<Hydroponics_KeyType, HydroponicsObject *,N> linksFilterPumpActuatorsByOutputReservoirType(const arx::map<Hydroponics_KeyType, HydroponicsObject *,N> &links, Hydroponics_ReservoirType reservoirType);

// Pins & Checks

// Checks to see if the pin is an analog input pin.
extern bool checkPinIsAnalogInput(byte pin);
// Checks to see if the pin is an analog output pin.
extern bool checkPinIsAnalogOutput(byte pin);
// Checks to see if the pin is a standard digital (non-analog) pin.
extern bool checkPinIsDigital(byte pin);
// Checks to see if the pin can produce a digital PWM output signal.
extern bool checkPinIsPWMOutput(byte pin);
// Checks to see if the pin can be set up with an ISR to handle digital level changes.
extern bool checkPinCanInterrupt(byte pin);

// Enum & String Conversions

// Converts from boolean value to triggered/not-triggered trigger state.
inline Hydroponics_TriggerState triggerStateFromBool(bool value) { return value ? Hydroponics_TriggerState_Triggered : Hydroponics_TriggerState_NotTriggered; }

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

// Explicit Specializations

template<> String commaStringFromArray<float>(const float *arrayIn, size_t length);
template<> String commaStringFromArray<double>(const double *arrayIn, size_t length);
template<> void commaStringToArray<float>(String stringIn, float *arrayOut, size_t length);
template<> void commaStringToArray<double>(String stringIn, double *arrayOut, size_t length);
template<> bool arrayElementsEqual<float>(const float *arrayIn, size_t length, float value);
template<> bool arrayElementsEqual<double>(const double *arrayIn, size_t length, double value);

#endif // /ifndef HydroponicsUtils_H
