/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Utilities
*/

#ifndef HydroponicsUtils_H
#define HydroponicsUtils_H

struct HydroponicsBitResolution;
#ifndef HYDRUINO_DISABLE_MULTITASKING
template<typename ParameterType, int Slots> class SignalFireTask;
template<class ObjectType, typename ParameterType> class MethodSlotCallTask;
class ActuatorTimedEnableTask;
#endif

#include "Hydroponics.h"
#include "HydroponicsObject.h"
#ifndef HYDRUINO_DISABLE_MULTITASKING
#include "BasicInterruptAbstraction.h"
#endif

// Simple class for describing an analog bit resolution.
// This class is mainly used to calculate analog pin range boundary values.
struct HydroponicsBitResolution {
    uint8_t bitRes;                             // Bit resolution (# of bits)
    int maxVal;                                 // Maximum value (2 ^ (# of bits))

    HydroponicsBitResolution(uint8_t bitRes);      // Constructor

    // Transforms value from raw integer (or initial) value into normalized raw (or transformed) value.
    inline float transform(int intValue) const { return constrain(intValue / (float)maxVal, 0.0f, 1.0f); }

    // Inverse transforms value from normalized raw (or transformed) value back into raw integer (or initial) value.
    inline int inverseTransform(float rawValue) const { return constrain((int)((float)maxVal * rawValue), 0, maxVal); }
};


// Scheduling

#ifndef HYDRUINO_DISABLE_MULTITASKING

// Standard interrupt abstraction
extern BasicArduinoInterruptAbstraction interruptImpl;


// This will schedule an actuator to enable on the next TaskManagerIO runloop using the given intensity and enable time millis.
// Actuator is captured. Returns taskId or TASKMGR_INVALIDID on error.
taskid_t scheduleActuatorTimedEnableOnce(SharedPtr<HydroponicsActuator> actuator, float enableIntensity, time_t enableTimeMillis);

// This will schedule an actuator to enable on the next TaskManagerIO runloop using the given enable time millis.
// Actuator is captured. Returns taskId or TASKMGR_INVALIDID on error.
taskid_t scheduleActuatorTimedEnableOnce(SharedPtr<HydroponicsActuator> actuator, time_t enableTimeMillis);

// This will schedule a signal's fire method on the next TaskManagerIO runloop using the given call/fire parameter.
// Object is captured, if not nullptr. Returns taskId or TASKMGR_INVALIDID on error.
template<typename ParameterType, int Slots> taskid_t scheduleSignalFireOnce(SharedPtr<HydroponicsObjInterface> object, Signal<ParameterType,Slots> &signal, ParameterType fireParam);

// This will schedule a signal's fire method on the next TaskManagerIO runloop using the given call/fire parameter, w/o capturing object.
// Returns taskId or TASKMGR_INVALIDID on error.
template<typename ParameterType, int Slots> taskid_t scheduleSignalFireOnce(Signal<ParameterType,Slots> &signal, ParameterType fireParam);

// This will schedule an object's method to be called on the next TaskManagerIO runloop using the given method slot and call parameter.
// Object is captured. Returns taskId or TASKMGR_INVALIDID on error.
template<class ObjectType, typename ParameterType> taskid_t scheduleObjectMethodCallOnce(SharedPtr<ObjectType> object, void (ObjectType::*method)(ParameterType), ParameterType callParam);

// This will schedule an object's method to be called on the next TaskManagerIO runloop using the given method slot and call parameter, w/o capturing object.
// Object is captured. Returns taskId or TASKMGR_INVALIDID on error.
template<class ObjectType, typename ParameterType> taskid_t scheduleObjectMethodCallOnce(ObjectType *object, void (ObjectType::*method)(ParameterType), ParameterType callParam);

// This will schedule an object's method to be called on the next TaskManagerIO runloop using the taskId that was created.
// Object is captured. Returns taskId or TASKMGR_INVALIDID on error.
template<class ObjectType> taskid_t scheduleObjectMethodCallWithTaskIdOnce(SharedPtr<ObjectType> object, void (ObjectType::*method)(taskid_t));

// This will schedule an object's method to be called on the next TaskManagerIO runloop using the taskId that was created, w/o capturing object.
// Returns taskId or TASKMGR_INVALIDID on error.
template<class ObjectType> taskid_t scheduleObjectMethodCallWithTaskIdOnce(ObjectType *object, void (ObjectType::*method)(taskid_t));


// Signal Fire Task
// This class holds onto the passed signal and parameter to pass it along to the signal's
// fire method upon task execution.
template<typename ParameterType, int Slots>
class SignalFireTask : public Executable {
public:
    taskid_t taskId;
    SignalFireTask(SharedPtr<HydroponicsObjInterface> object,
                   Signal<ParameterType,Slots> &signal,
                   ParameterType &param)
        : taskId(TASKMGR_INVALIDID), _object(object), _signal(&signal), _param(param) { ; }

    virtual void exec() override { _signal->fire(_param); }
private:
    SharedPtr<HydroponicsObjInterface> _object;
    Signal<ParameterType, Slots> *_signal;
    ParameterType _param;
};


// Method Slot Task
// This class holds onto a MethodSlot to call once executed.
template<class ObjectType, typename ParameterType>
class MethodSlotCallTask : public Executable {
public:
    typedef void (ObjectType::*FunctPtr)(ParameterType);
    taskid_t taskId;

    MethodSlotCallTask(SharedPtr<ObjectType> object, FunctPtr method, ParameterType callParam) : taskId(TASKMGR_INVALIDID), _object(object), _methodSlot(object.get(), method), _callParam(callParam) { ; }
    MethodSlotCallTask(ObjectType *object, FunctPtr method, ParameterType callParam) : taskId(TASKMGR_INVALIDID), _object(nullptr), _methodSlot(object, method), _callParam(callParam) { ; }

    virtual void exec() override { _methodSlot(_callParam); }
private:
    SharedPtr<ObjectType> _object;
    MethodSlot<ObjectType,ParameterType> _methodSlot;
    ParameterType _callParam;

    friend taskid_t scheduleObjectMethodCallWithTaskIdOnce<ObjectType>(SharedPtr<ObjectType> object, void (ObjectType::*method)(taskid_t));
    friend taskid_t scheduleObjectMethodCallWithTaskIdOnce<ObjectType>(ObjectType *object, void (ObjectType::*method)(taskid_t));
};


// Actuator Precise Timed Enable Task
// This actuator will enable an actuator for a period of time finely, and then deactivate it.
class ActuatorTimedEnableTask : public Executable {
public:
    taskid_t taskId;
    ActuatorTimedEnableTask(SharedPtr<HydroponicsActuator> actuator,
                            float enableIntensity,
                            time_t enableTimeMillis);

    virtual void exec() override;
private:
    SharedPtr<HydroponicsActuator> _actuator;
    float _enableIntensity;
    time_t _enableTimeMillis;
};


#endif // /ifndef HYDRUINO_DISABLE_MULTITASKING

// Assertions

#ifdef HYDRUINO_USE_DEBUG_ASSERTIONS

// This softly asserts on a failed condition, sending message out to Serial output (if debugging enabled) and/or disk-based logging (if enabled), and then finally continuing program execution.
// See HYDRUINO_SOFT_ASSERT() macro for usage.
extern void softAssert(bool cond, String msg, const char *file, const char *func, int line);

// This hard asserts on a failed condition, sending message out to Serial output (if debugging enabled) and/or disk-based logging (if enabled), then yielding (to allow comms), and then finally aborting program execution.
// See HYDRUINO_HARD_ASSERT() macro for usage.
extern void hardAssert(bool cond, String msg, const char *file, const char *func, int line);

#endif // /ifdef HYDRUINO_USE_DEBUG_ASSERTIONS

// Helpers & Misc

// Returns the active hydroponics instance. Not guaranteed to be non-null.
inline Hydroponics *getHydroponicsInstance();
// Returns the active scheduler instance. Not guaranteed to be non-null.
inline HydroponicsScheduler *getSchedulerInstance();
// Returns the active logger instance. Not guaranteed to be non-null.
inline HydroponicsLogger *getLoggerInstance();
// Returns the active publisher instance. Not guaranteed to be non-null.
inline HydroponicsPublisher *getPublisherInstance();

// Publishes latest data from sensor to Publisher output.
extern void publishData(HydroponicsSensor *sensor);

// Returns current time, with proper time zone offset based on active hydroponics instance.
inline DateTime getCurrentTime();
// Returns the UTC seconds time that today started, accounting for time zone offset based on active hydroponics instance.
inline time_t getCurrentDayStartTime();

// Returns a proper filename for a storage monitoring file (log, data, etc) that uses YYMMDD as filename.
extern String getYYMMDDFilename(String prefix, String ext);
// Returns a proper filename for a storage library data file that uses ## as filename.
extern String getNNFilename(String prefix, unsigned int value, String ext);

// Creates intermediate folders given a filename. Currently only supports a single folder depth.
extern void createDirectoryFor(SDClass *sd, String filename);

// Computes a hash for a string using a fast and efficient (read as: good enough for our use) hashing algorithm.
extern Hydroponics_KeyType stringHash(String string);

// Returns properly formatted address "0xADDR" (size depending on void* size)
extern String addressToString(uintptr_t addr);

// Returns a string from char array with an exact max length.
// Null array or invalid length will abort function before encoding occurs, returning "null".
extern String charsToString(const char *charsIn, size_t length);
// Returns a string formatted to deal with variable time spans.
extern String timeSpanToString(const TimeSpan &span);
// Returns a string formatted to value and unit for dealing with measurements as value/units pair.
extern String measurementToString(float value, Hydroponics_UnitsType units, unsigned int additionalDecPlaces = 0);
// Returns a string formatted to value and unit for dealing with measurements.
inline String measurementToString(const HydroponicsSingleMeasurement &measurement, unsigned int additionalDecPlaces = 0) { return measurementToString(measurement.value, measurement.units, additionalDecPlaces); }

// Encodes a T-typed array to a comma-separated string.
// Null array or invalid length will abort function before encoding occurs, returning "null".
template<typename T> String commaStringFromArray(const T *arrayIn, size_t length);
// Decodes a comma-separated string back to a T-typed array.
// Last value read is repeated on through to last element, with no commas being treated as single value being applied to all elements.
// Empty string, string value of "null", null array, or invalid length will abort function before decoding.
template<typename T> void commaStringToArray(String stringIn, T *arrayOut, size_t length);
// Decodes a comma-separated JSON variant, if not null, object, or array, back to a T-typed array.
// Acts as a redirect for JSON variants so that they receive the additional checks before being converted to string.
template<typename T> void commaStringToArray(JsonVariantConst &variantIn, T *arrayOut, size_t length);

// Encodes a byte array to hexadecimal string.
extern String hexStringFromBytes(const uint8_t *bytesIn, size_t length);
// Decodes a hexadecimal string back to a byte array.
extern void hexStringToBytes(String stringIn, uint8_t *bytesOut, size_t length);
// Decodes a hexadecimal JSON variant, if not null, object, or array, back to a byte array.
extern void hexStringToBytes(JsonVariantConst &variantIn, uint8_t *bytesOut, size_t length);

// Returns # of occurrences of character in string.
int occurrencesInString(String string, char singleChar);
// Returns # of occurrences of substring in string.
int occurrencesInString(String string, String subString);
// Returns # of occurrences of character in string, ignoring case.
int occurrencesInStringIgnoreCase(String string, char singleChar);
// Returns # of occurrences of substring in string, ignoring case.
int occurrencesInStringIgnoreCase(String string, String subString);

// Returns whenever all elements of an array are equal to the specified value, or not.
template<typename T> bool arrayElementsEqual(const T *arrayIn, size_t length, T value);

// Similar to the standard map function, but does it on any type.
template<typename T> inline T mapValue(T value, T inMin, T inMax, T outMin, T outMax) { return ((value - inMin) * ((outMax - outMin) / (inMax - inMin))) + outMin; }

// Returns the amount of space between the stack and heap (ie free space left), else -1 if undeterminable.
extern int freeMemory();

// This delays a finely timed amount, with spin loop nearer to end. Used in finely timed dispensers.
extern void delayFine(time_t timeMillis);

// This will query the active RTC sync device for the current time.
extern time_t rtcNow();

// This will return the time in unixtime (secs since 1970).
inline time_t unixNow() { return rtcNow() ?: now() + SECONDS_FROM_1970_TO_2000; } // rtcNow returns 0 if not set

// This will handle interrupts for task manager.
extern void handleInterrupt(pintype_t pin);

// This is used to force debug statements through to serial monitor.
inline void flushYield() {
    #if defined(HYDRUINO_ENABLE_DEBUG_OUTPUT) && HYDRUINO_SYS_DEBUGOUT_FLUSH_YIELD
        Serial.flush(); yield();
    #else
        return;
    #endif
}

// Units & Conversion

// Tries to convert value from one unit to another (if supported), returning conversion success flag.
// Convert param used in certain unit conversions as external additional value (e.g. voltage for power/current conversion).
// This is the main conversion function that all others wrap around.
extern bool tryConvertUnits(float valueIn, Hydroponics_UnitsType unitsIn, float *valueOut, Hydroponics_UnitsType unitsOut, float convertParam = FLT_UNDEF);

// Attempts to convert value in-place from one unit to another, and if successful then assigns value back overtop of itself.
// Convert param used in certain unit conversions. Returns conversion success flag.
extern bool convertUnits(float *valueInOut, Hydroponics_UnitsType *unitsInOut, Hydroponics_UnitsType outUnits, float convertParam = FLT_UNDEF);
// Attempts to convert value from one unit to another, and if successful then assigns value, and optionally units, to output.
// Convert param used in certain unit conversions. Returns conversion success flag.
extern bool convertUnits(float valueIn, float *valueOut, Hydroponics_UnitsType unitsIn, Hydroponics_UnitsType outUnits, Hydroponics_UnitsType *unitsOut = nullptr, float convertParam = FLT_UNDEF);
// Attempts to convert measurement in-place from one unit to another, and if successful then assigns value and units back overtop of itself.
// Convert param used in certain unit conversions. Returns conversion success flag.
inline bool convertUnits(HydroponicsSingleMeasurement *measureInOut, Hydroponics_UnitsType outUnits, float convertParam = FLT_UNDEF) { return convertUnits(&measureInOut->value, &measureInOut->units, outUnits, convertParam); }
// Attemps to convert measurement from one unit to another, and if successful then assigns value and units to output measurement.
// Convert param used in certain unit conversions. Returns conversion success flag.
inline bool convertUnits(const HydroponicsSingleMeasurement *measureIn, HydroponicsSingleMeasurement *measureOut, Hydroponics_UnitsType outUnits, float convertParam = FLT_UNDEF) { return convertUnits(measureIn->value, &measureOut->value, measureIn->units, outUnits, &measureOut->units, convertParam); }

// Returns the base units from a rate unit (e.g. L/min -> L).
extern Hydroponics_UnitsType baseUnitsFromRate(Hydroponics_UnitsType units);
// Returns the base units from a dilution unit (e.g. mL/L -> L).
Hydroponics_UnitsType baseUnitsFromDilution(Hydroponics_UnitsType units);

// Returns default temperature units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultTemperatureUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);
// Returns default distance units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultDistanceUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);
// Returns default weight units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultWeightUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);
// Returns default liquid volume units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultLiquidVolumeUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);
// Returns default liquid flow units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultLiquidFlowUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);
// Returns default liquid dilution units to use based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern Hydroponics_UnitsType defaultLiquidDilutionUnits(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);
// Returns default decimal places rounded to based on measureMode (if undefined then uses active Hydroponics instance's measurement mode, else default mode).
extern int defaultDecimalPlaces(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);

// Rounds value according to default decimal places rounding, as typically used for data export, with optional additional decimal places.
inline float roundForExport(float value, unsigned int additionalDecPlaces = 0) { return roundToDecimalPlaces(value, defaultDecimalPlaces() + additionalDecPlaces); }
// Rounds value according to default decimal places rounding, as typically used for data export, to string with optional additional decimal places.
inline String roundToString(float value, unsigned int additionalDecPlaces = 0) { return String(roundToDecimalPlaces(value, defaultDecimalPlaces() + additionalDecPlaces), defaultDecimalPlaces() + additionalDecPlaces); }

// Linkages & Filtering

// Returns linkages list filtered down to just actuators.
template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE> Vector<HydroponicsObject *, N> linksFilterActuators(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links);
// Returns linkages list filtered down to just crops.
template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE> Vector<HydroponicsObject *, N> linksFilterCrops(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links);

// Returns linkages list filtered down to just actuators of a certain type that operate on a specific reservoir.
template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE> Vector<HydroponicsObject *, N> linksFilterActuatorsByReservoirAndType(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links, HydroponicsReservoir *srcReservoir, Hydroponics_ActuatorType actuatorType);
// Returns linkages list filtered down to just pump actuators that pump from a specific reservoir to a certain reservoir type.
template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE> Vector<HydroponicsObject *, N> linksFilterPumpActuatorsByInputReservoirAndOutputReservoirType(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links, HydroponicsReservoir *srcReservoir, Hydroponics_ReservoirType destReservoirType);
// Returns linkages list filtered down to just pump actuators that pump to a specific reservoir from a certain reservoir type.
template<size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE> Vector<HydroponicsObject *, N> linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links, HydroponicsReservoir *destReservoir, Hydroponics_ReservoirType srcReservoirType);

// Returns the # of crops found in the linkages list.
int linksCountCrops(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links);
// Returns the # of actuators of a certain type that operate on a specific reservoir.
int linksCountActuatorsByReservoirAndType(Pair<uint8_t, Pair<HydroponicsObject *, int8_t> *> links, HydroponicsReservoir *srcReservoir, Hydroponics_ActuatorType actuatorType);

// Recombines filtered object list back into SharedPtr actuator list.
template<size_t N> void linksResolveActuatorsByType(Vector<HydroponicsObject *, N> &actuatorsIn, Vector<SharedPtr<HydroponicsActuator>, N> &actuatorsOut, Hydroponics_ActuatorType actuatorType);
// Recombines filtered object list back into SharedPtr actuator list paired with rate value.
template<size_t N> void linksResolveActuatorsPairRateByType(Vector<HydroponicsObject *, N> &actuatorsIn, float rateValue, Vector<Pair<SharedPtr<HydroponicsActuator>, float>, N> &actuatorsOut, Hydroponics_ActuatorType actuatorType);

// Pins & Checks

// Checks to see if the pin is an analog input pin.
extern bool checkPinIsAnalogInput(pintype_t pin);
// Checks to see if the pin is an analog output pin.
extern bool checkPinIsAnalogOutput(pintype_t pin);
// Checks to see if the pin is a standard digital (non-analog) pin.
inline bool checkPinIsDigital(pintype_t pin) { return !checkPinIsAnalogInput(pin) && !checkPinIsAnalogOutput(pin); }
// Checks to see if the pin can produce a digital PWM output signal.
inline bool checkPinIsPWMOutput(pintype_t pin);
// Checks to see if the pin can be set up with an ISR to handle digital level changes.
inline bool checkPinCanInterrupt(pintype_t pin) { return isValidPin(digitalPinToInterrupt(pin)); }

// Enums & Conversions

// Converts from system mode enum to string, with optional exclude for special types (instead returning "").
extern String systemModeToString(Hydroponics_SystemMode systemMode, bool excludeSpecial = false);
// Converts back to system mode enum from string.
extern Hydroponics_SystemMode systemModeFromString(String systemModeStr);

// Converts from measurement mode enum to string, with optional exclude for special types (instead returning "").
extern String measurementModeToString(Hydroponics_MeasurementMode measurementMode, bool excludeSpecial = false);
// Converts back to measurement mode enum from string.
extern Hydroponics_MeasurementMode measurementModeFromString(String measurementModeStr);

// Converts from display output mode enum to string, with optional exclude for special types (instead returning "").
extern String displayOutputModeToString(Hydroponics_DisplayOutputMode displayOutMode, bool excludeSpecial = false);
// Converts back to display output mode enum from string.
extern Hydroponics_DisplayOutputMode displayOutputModeFromString(String displayOutModeStr);

// Converts from control input mode enum to string, with optional exclude for special types (instead returning "").
extern String controlInputModeToString(Hydroponics_ControlInputMode controlInMode, bool excludeSpecial = false);
// Converts back to control input mode enum from string.
extern Hydroponics_ControlInputMode controlInputModeFromString(String controlInModeStr);

// Returns true for actuators that "live" in water (thus must do empty checks) as derived from actuator type enumeration.
extern bool getActuatorInWaterFromType(Hydroponics_ActuatorType actuatorType);
// Returns true for actuators that pump liquid (thus must do empty/filled checks) as derived from actuator type enumeration.
extern bool getActuatorIsPumpFromType(Hydroponics_ActuatorType actuatorType);

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

// Converts from substrate type enum to string, with optional exclude for special types (instead returning "").
extern String substrateTypeToString(Hydroponics_SubstrateType substrateType, bool excludeSpecial = false);
// Converts back to substrate type enum from string.
extern Hydroponics_SubstrateType substrateTypeFromString(String substrateTypeStr);

// Converts from fluid reservoir enum to string, with optional exclude for special types (instead returning "").
extern String reservoirTypeToString(Hydroponics_ReservoirType reservoirType, bool excludeSpecial = false);
// Converts back to fluid reservoir enum from string.
extern Hydroponics_ReservoirType reservoirTypeFromString(String reservoirTypeStr);

// Returns nominal rail voltage as derived from rail type enumeration.
extern float getRailVoltageFromType(Hydroponics_RailType railType);

// Converts from power rail enum to string, with optional exclude for special types (instead returning "").
extern String railTypeToString(Hydroponics_RailType railType, bool excludeSpecial = false);
// Converts back to power rail enum from string.
extern Hydroponics_RailType railTypeFromString(String railTypeStr);

// Converts from units category enum to string, with optional exclude for special types (instead returning "").
extern String unitsCategoryToString(Hydroponics_UnitsCategory unitsCategory, bool excludeSpecial = false);
// Converts back to units category enum from string.
extern Hydroponics_UnitsCategory unitsCategoryFromString(String unitsCategoryStr);

// Converts from units type enum to symbol string, with optional exclude for special types (instead returning "").
extern String unitsTypeToSymbol(Hydroponics_UnitsType unitsType, bool excludeSpecial = false);
// Converts back to units type enum from symbol.
extern Hydroponics_UnitsType unitsTypeFromSymbol(String unitsSymbolStr);

// Converts from position index to string, with optional exclude for special types (instead returning "").
extern String positionIndexToString(Hydroponics_PositionIndex positionIndex, bool excludeSpecial = false);
// Converts back to position index from string.
extern Hydroponics_PositionIndex positionIndexFromString(String positionIndexStr);

// Converts from boolean value to triggered/not-triggered trigger state.
inline Hydroponics_TriggerState triggerStateFromBool(bool value) { return value ? Hydroponics_TriggerState_Triggered : Hydroponics_TriggerState_NotTriggered; }
// Converts from triggered/not-triggered trigger state back into boolean value.
inline bool triggerStateToBool(Hydroponics_TriggerState state) { return state == Hydroponics_TriggerState_Triggered; }

// Explicit Specializations

template<> String commaStringFromArray<float>(const float *arrayIn, size_t length);
template<> String commaStringFromArray<double>(const double *arrayIn, size_t length);
template<> void commaStringToArray<float>(String stringIn, float *arrayOut, size_t length);
template<> void commaStringToArray<double>(String stringIn, double *arrayOut, size_t length);
template<> bool arrayElementsEqual<float>(const float *arrayIn, size_t length, float value);
template<> bool arrayElementsEqual<double>(const double *arrayIn, size_t length, double value);

#endif // /ifndef HydroponicsUtils_H
