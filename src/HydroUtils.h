/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Utilities
*/

#ifndef HydroUtils_H
#define HydroUtils_H

template<typename RTCType> class HydroRTCWrapper;
#ifdef HYDRO_USE_MULTITASKING
template<typename ParameterType, int Slots> class SignalFireTask;
template<class ObjectType, typename ParameterType> class MethodSlotCallTask;
class ActuatorTimedEnableTask;
#endif

#include "Hydruino.h"
#include "HydroObject.h"
#ifdef HYDRO_USE_MULTITASKING
#include "BasicInterruptAbstraction.h"
#endif

// Simple wrapper class for dealing with RTC modules.
// This class is mainly used to abstract which RTC module is used.
template<typename RTCType>
class HydroRTCWrapper : public HydroRTCInterface {
public:
    virtual bool begin(TwoWire *wireInstance) override { return _rtc.begin(wireInstance); }
    virtual void adjust(const DateTime &dt) override { _rtc.adjust(dt); }
    virtual bool lostPower(void) override { return _rtc.lostPower(); }
    virtual DateTime now() override { return _rtc.now(); }
protected:
    RTCType _rtc;
};

// Specialization for older DS1307 that doesn't have lost power tracking.
template<>
class HydroRTCWrapper<RTC_DS1307> : public HydroRTCInterface {
public:
    virtual bool begin(TwoWire *wireInstance) override;
    virtual void adjust(const DateTime &dt) override;
    virtual bool lostPower(void) override;
    virtual DateTime now() override;
protected:
    RTC_DS1307 _rtc;
};

// Scheduling

#ifdef HYDRO_USE_MULTITASKING

// Standard interrupt abstraction
extern BasicArduinoInterruptAbstraction interruptImpl;


// This will schedule an actuator to enable on the next TaskManagerIO runloop using the given intensity and enable time millis.
// Actuator is captured. Returns taskId or TASKMGR_INVALIDID on error.
taskid_t scheduleActuatorTimedEnableOnce(SharedPtr<HydroActuator> actuator, float intensity, time_t enableTime);

// This will schedule an actuator to enable on the next TaskManagerIO runloop using the given enable time millis.
// Actuator is captured. Returns taskId or TASKMGR_INVALIDID on error.
taskid_t scheduleActuatorTimedEnableOnce(SharedPtr<HydroActuator> actuator, time_t enableTime);

// This will schedule a signal's fire method on the next TaskManagerIO runloop using the given call/fire parameter.
// Object is captured, if not nullptr. Returns taskId or TASKMGR_INVALIDID on error.
template<typename ParameterType, int Slots> taskid_t scheduleSignalFireOnce(SharedPtr<HydroObjInterface> object, Signal<ParameterType,Slots> &signal, ParameterType fireParam);

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
    SignalFireTask(SharedPtr<HydroObjInterface> object,
                   Signal<ParameterType,Slots> &signal,
                   ParameterType &param)
        : taskId(TASKMGR_INVALIDID), _object(object), _signal(&signal), _param(param) { ; }

    virtual void exec() override { _signal->fire(_param); }
private:
    SharedPtr<HydroObjInterface> _object;
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
    ActuatorTimedEnableTask(SharedPtr<HydroActuator> actuator,
                            float intensity,
                            millis_t duration);

    virtual void exec() override;
private:
    SharedPtr<HydroActuator> _actuator;
    float _intensity;
    millis_t _duration;
};


#endif // /ifdef HYDRO_USE_MULTITASKING

// Assertions

#ifdef HYDRO_USE_DEBUG_ASSERTIONS

// This softly asserts on a failed condition, sending message out to Serial output (if debugging enabled) and/or disk-based logging (if enabled), and then finally continuing program execution.
// See HYDRO_SOFT_ASSERT() macro for usage.
extern void softAssert(bool cond, String msg, const char *file, const char *func, int line);

// This hard asserts on a failed condition, sending message out to Serial output (if debugging enabled) and/or disk-based logging (if enabled), then yielding (to allow comms), and then finally aborting program execution.
// See HYDRO_HARD_ASSERT() macro for usage.
extern void hardAssert(bool cond, String msg, const char *file, const char *func, int line);

#endif // /ifdef HYDRO_USE_DEBUG_ASSERTIONS

// Helpers & Misc

// Returns the active controller instance. Not guaranteed to be non-null.
inline Hydruino *getController();
// Returns the active scheduler instance. Not guaranteed to be non-null.
inline HydroScheduler *getScheduler();
// Returns the active logger instance. Not guaranteed to be non-null.
inline HydroLogger *getLogger();
// Returns the active publisher instance. Not guaranteed to be non-null.
inline HydroPublisher *getPublisher();
#ifdef HYDRO_USE_GUI
// Returns the active UI instance. Not guaranteed to be non-null.
inline HydruinoUIInterface *getUI();
#endif

// Publishes latest data from sensor to Publisher output.
extern void publishData(HydroSensor *sensor);

// Converts from local DateTime (offset by system TZ) back into unix/UTC time_t.
inline time_t unixTime(DateTime localTime);
// Converts from unix/UTC time_t into local DateTime (offset by system TZ).
inline DateTime localTime(time_t unixTime);
// Returns unix/UTC time_t of when today started.
inline time_t unixDayStart(time_t unixTime = unixNow());
// Returns local DateTime (offset by system TZ) of when today started.
inline DateTime localDayStart(time_t unixTime = unixNow());
// Sets global RTC current time to passed unix/UTC time_t.
// Returns update success after calling appropriate system notifiers.
inline bool setUnixTime(time_t unixTime);
// Sets global RTC current time to passed local DateTime (offset by system TZ).
// Returns update success after calling appropriate system notifiers.
inline bool setLocalTime(DateTime localTime);

// Returns a proper filename for a storage monitoring file (log, data, etc) that uses YYMMDD as filename.
extern String getYYMMDDFilename(String prefix, String ext);
// Returns a proper filename for a storage library data file that uses ## as filename.
extern String getNNFilename(String prefix, unsigned int value, String ext);

// Creates intermediate folders given a filename. Currently only supports a single folder depth.
extern void createDirectoryFor(SDClass *sd, String filename);

// Computes a hash for a string using a fast and efficient (read as: good enough for our use) hashing algorithm.
extern hkey_t stringHash(String string);

// Returns properly formatted address "0xADDR" (size depending on void* size)
extern String addressToString(uintptr_t addr);

// Returns a string from char array with an exact max length.
// Null array or invalid length will abort function before encoding occurs, returning "null".
extern String charsToString(const char *charsIn, size_t length);
// Returns a string formatted to deal with variable time spans.
extern String timeSpanToString(const TimeSpan &span);
// Returns a string formatted to value and unit for dealing with measurements as value/units pair.
extern String measurementToString(float value, Hydro_UnitsType units, unsigned int additionalDecPlaces = 0);
// Returns a string formatted to value and unit for dealing with measurements.
inline String measurementToString(const HydroSingleMeasurement &measurement, unsigned int additionalDecPlaces = 0) { return measurementToString(measurement.value, measurement.units, additionalDecPlaces); }

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
extern int occurrencesInString(String string, char singleChar);
// Returns # of occurrences of substring in string.
extern int occurrencesInString(String string, String subString);
// Returns # of occurrences of character in string, ignoring case.
extern int occurrencesInStringIgnoreCase(String string, char singleChar);
// Returns # of occurrences of substring in string, ignoring case.
extern int occurrencesInStringIgnoreCase(String string, String subString);

// Returns whenever all elements of an array are equal to the specified value, or not.
template<typename T> bool arrayElementsEqual(const T *arrayIn, size_t length, T value);

// Similar to the standard map function, but does it on any type.
template<typename T> inline T mapValue(T value, T inMin, T inMax, T outMin, T outMax) { return ((value - inMin) * ((outMax - outMin) / (inMax - inMin))) + outMin; }

// Returns the amount of space between the stack and heap (ie free space left), else -1 if undeterminable.
extern unsigned int freeMemory();

// This delays a finely timed amount, with spin loop nearer to end. Used in finely timed dispensers.
extern void delayFine(millis_t time);

// This will query the active RTC sync device for the current time.
extern time_t rtcNow();

// This will return the current time as unix/UTC time_t (secs since 1970). Uses rtc time if available, otherwise 2000-Jan-1 + time since turned on.
// Default storage format. Do not use for data display/night-day-calcs - use localNow() or localTime() for local DateTime (offset by system TZ).
inline time_t unixNow() { return rtcNow() ?: (now() + SECONDS_FROM_1970_TO_2000); } // rtcNow returns 0 if not set
// This will return the current time as local DateTime (offset by system TZ). Uses rtc time if available, otherwise 2000-Jan-1 + time since turned on.
// Meant to be used as a temporary or for runtime only. Do not use for data storage - use unixNow() or unixTime() for unix/UTC time_t (secs since 1970).
inline DateTime localNow() { return localTime(unixNow()); }

// This will return a non-zero millis time value, so that 0 time values can be reserved for other use.
inline millis_t nzMillis() { return millis() ?: 1; }

// This will handle interrupts for task manager.
extern void handleInterrupt(pintype_t pin);

// Function pointer to pinMode for binary actuators and sensors. Allows an intermediary, such as a port extender or multiplexer, to be used. By default uses pinMode.
extern void (*hy_bin_pinMode)(pintype_t,uint8_t);
// Function pointer to digitalWrite for binary actuators. Allows an intermediary, such as a port extender or multiplexer, to be used. By default uses digitalWrite.
extern void (*hy_bin_digitalWrite)(pintype_t,uint8_t);
// Function pointer to digitalRead for binary sensors. Allows an intermediary, such as a port extender or multiplexer, to be used. By default uses digitalRead.
extern uint8_t (*hy_bin_digitalRead)(pintype_t);

// This is used to force debug statements through to serial monitor.
inline void flushYield() {
    #if defined(HYDRO_ENABLE_DEBUG_OUTPUT) && HYDRO_SYS_DEBUGOUT_FLUSH_YIELD
        Serial.flush(); yield();
    #else
        return;
    #endif
}

// Units & Conversion

// Tries to convert value from one unit to another (if supported), returning conversion success flag.
// Convert param used in certain unit conversions as external additional value (e.g. voltage for power/current conversion).
// This is the main conversion function that all others wrap around.
extern bool tryConvertUnits(float valueIn, Hydro_UnitsType unitsIn, float *valueOut, Hydro_UnitsType unitsOut, float convertParam = FLT_UNDEF);

// Attempts to convert value in-place from one unit to another, and if successful then assigns value back overtop of itself.
// Convert param used in certain unit conversions. Returns conversion success flag.
inline bool convertUnits(float *valueInOut, Hydro_UnitsType *unitsInOut, Hydro_UnitsType outUnits, float convertParam = FLT_UNDEF);
// Attempts to convert value from one unit to another, and if successful then assigns value, and optionally units, to output.
// Convert param used in certain unit conversions. Returns conversion success flag.
inline bool convertUnits(float valueIn, float *valueOut, Hydro_UnitsType unitsIn, Hydro_UnitsType outUnits, Hydro_UnitsType *unitsOut = nullptr, float convertParam = FLT_UNDEF);
// Attempts to convert measurement in-place from one unit to another, and if successful then assigns value and units back overtop of itself.
// Convert param used in certain unit conversions. Returns conversion success flag.
inline bool convertUnits(HydroSingleMeasurement *measureInOut, Hydro_UnitsType outUnits, float convertParam = FLT_UNDEF);
// Attemps to convert measurement from one unit to another, and if successful then assigns value and units to output measurement.
// Convert param used in certain unit conversions. Returns conversion success flag.
inline bool convertUnits(const HydroSingleMeasurement *measureIn, HydroSingleMeasurement *measureOut, Hydro_UnitsType outUnits, float convertParam = FLT_UNDEF);

// Returns the base units from a rate unit (e.g. L/min -> L). Also will convert dilution to volume.
extern Hydro_UnitsType baseUnits(Hydro_UnitsType units);
// Returns the rate units from a base unit (e.g. mm -> mm/min).
extern Hydro_UnitsType rateUnits(Hydro_UnitsType units);
// Returns the base units from a dilution unit (e.g. mL/L -> L).
extern Hydro_UnitsType volumeUnits(Hydro_UnitsType units);
// Returns the dilution units from a base unit (e.g. L -> mL/L).
extern Hydro_UnitsType dilutionUnits(Hydro_UnitsType units);

// Returns default units based on category and measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
extern Hydro_UnitsType defaultUnits(Hydro_UnitsCategory unitsCategory, Hydro_MeasurementMode measureMode = Hydro_MeasurementMode_Undefined);

// Returns default concentrate units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Hydro_UnitsType defaultConcentrateUnits(Hydro_MeasurementMode measureMode = Hydro_MeasurementMode_Undefined) { return defaultUnits(Hydro_UnitsCategory_Concentration, measureMode); }
// Returns default distance units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Hydro_UnitsType defaultDistanceUnits(Hydro_MeasurementMode measureMode = Hydro_MeasurementMode_Undefined) { return defaultUnits(Hydro_UnitsCategory_Distance, measureMode); }
// Returns default liquid flow rate units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Hydro_UnitsType defaultFlowRateUnits(Hydro_MeasurementMode measureMode = Hydro_MeasurementMode_Undefined) { return defaultUnits(Hydro_UnitsCategory_LiqFlowRate, measureMode); }
// Returns default liquid dilution units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Hydro_UnitsType defaultDilutionUnits(Hydro_MeasurementMode measureMode = Hydro_MeasurementMode_Undefined) { return defaultUnits(Hydro_UnitsCategory_LiqDilution, measureMode); }
// Returns default power units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Hydro_UnitsType defaultPowerUnits(Hydro_MeasurementMode measureMode = Hydro_MeasurementMode_Undefined) { return defaultUnits(Hydro_UnitsCategory_Power, measureMode); }
// Returns default temperature units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Hydro_UnitsType defaultTemperatureUnits(Hydro_MeasurementMode measureMode = Hydro_MeasurementMode_Undefined) { return defaultUnits(Hydro_UnitsCategory_Temperature, measureMode); }
// Returns default liquid volume units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Hydro_UnitsType defaultVolumeUnits(Hydro_MeasurementMode measureMode = Hydro_MeasurementMode_Undefined) { return defaultUnits(Hydro_UnitsCategory_LiqVolume, measureMode); }
// Returns default weight units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Hydro_UnitsType defaultWeightUnits(Hydro_MeasurementMode measureMode = Hydro_MeasurementMode_Undefined) { return defaultUnits(Hydro_UnitsCategory_Weight, measureMode); }
// Returns default decimal places rounded to based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline int defaultDecimalPlaces(Hydro_MeasurementMode measureMode = Hydro_MeasurementMode_Undefined) { return (int)defaultUnits(Hydro_UnitsCategory_Count, measureMode); }

// Rounds value according to default decimal places rounding, as typically used for data export, with optional additional decimal places.
inline float roundForExport(float value, unsigned int additionalDecPlaces = 0) { return roundToDecimalPlaces(value, defaultDecimalPlaces() + additionalDecPlaces); }
// Rounds value according to default decimal places rounding, as typically used for data export, to string with optional additional decimal places.
inline String roundToString(float value, unsigned int additionalDecPlaces = 0) { return String(roundToDecimalPlaces(value, defaultDecimalPlaces() + additionalDecPlaces), defaultDecimalPlaces() + additionalDecPlaces); }

// Linkages & Filtering

// Returns linkages list filtered down to just actuators.
template<size_t N = HYDRO_DEFAULT_MAXSIZE> Vector<HydroObject *, N> linksFilterActuators(Pair<uint8_t, Pair<HydroObject *, int8_t> *> links);
// Returns linkages list filtered down to just crops.
template<size_t N = HYDRO_DEFAULT_MAXSIZE> Vector<HydroObject *, N> linksFilterCrops(Pair<uint8_t, Pair<HydroObject *, int8_t> *> links);

// Returns linkages list filtered down to just actuators of a certain type that operate on a specific reservoir.
template<size_t N = HYDRO_DEFAULT_MAXSIZE> Vector<HydroObject *, N> linksFilterActuatorsByReservoirAndType(Pair<uint8_t, Pair<HydroObject *, int8_t> *> links, HydroReservoir *srcReservoir, Hydro_ActuatorType actuatorType);
// Returns linkages list filtered down to just pump actuators that pump from a specific reservoir to a certain reservoir type.
template<size_t N = HYDRO_DEFAULT_MAXSIZE> Vector<HydroObject *, N> linksFilterPumpActuatorsBySourceReservoirAndOutputReservoirType(Pair<uint8_t, Pair<HydroObject *, int8_t> *> links, HydroReservoir *srcReservoir, Hydro_ReservoirType destReservoirType);
// Returns linkages list filtered down to just pump actuators that pump to a specific reservoir from a certain reservoir type.
template<size_t N = HYDRO_DEFAULT_MAXSIZE> Vector<HydroObject *, N> linksFilterPumpActuatorsByOutputReservoirAndSourceReservoirType(Pair<uint8_t, Pair<HydroObject *, int8_t> *> links, HydroReservoir *destReservoir, Hydro_ReservoirType srcReservoirType);

// Returns the # of crops that are currently sowable as found in the linkages list.
extern int linksCountSowableCrops(Pair<uint8_t, Pair<HydroObject *, int8_t> *> links);
// Returns the # of actuators of a certain type that operate on a specific reservoir.
extern int linksCountActuatorsByReservoirAndType(Pair<uint8_t, Pair<HydroObject *, int8_t> *> links, HydroReservoir *srcReservoir, Hydro_ActuatorType actuatorType);

// Recombines filtered object list back into SharedPtr actuator list.
template<size_t N = HYDRO_DEFAULT_MAXSIZE> void linksResolveActuatorsToAttachmentsByType(Vector<HydroObject *, N> &actuatorsIn, HydroObjInterface *parent, Vector<HydroActuatorAttachment, N> &activationsOut, Hydro_ActuatorType actuatorType);
// Recombines filtered object list back into SharedPtr actuator list paired with rate value.
template<size_t N = HYDRO_DEFAULT_MAXSIZE> void linksResolveActuatorsToAttachmentsByRateAndType(Vector<HydroObject *, N> &actuatorsIn, HydroObjInterface *parent, float rateMultiplier, Vector<HydroActuatorAttachment, N> &activationsOut, Hydro_ActuatorType actuatorType);

// Pins & Checks

// Checks to see if the pin is an analog input pin.
extern bool checkPinIsAnalogInput(pintype_t pin);
// Checks to see if the pin is an analog output pin.
extern bool checkPinIsAnalogOutput(pintype_t pin);
// Checks to see if the pin is a standard digital (non-analog) pin.
inline bool checkPinIsDigital(pintype_t pin);
// Checks to see if the pin can produce a digital PWM output signal.
inline bool checkPinIsPWMOutput(pintype_t pin);
// Checks to see if the pin can be set up with an ISR to handle digital level changes.
inline bool checkPinCanInterrupt(pintype_t pin);

// Enums & Conversions

// Converts from system mode enum to string, with optional exclude for special types (instead returning "").
extern String systemModeToString(Hydro_SystemMode systemMode, bool excludeSpecial = false);
// Converts back to system mode enum from string.
extern Hydro_SystemMode systemModeFromString(String systemModeStr);

// Converts from measurement mode enum to string, with optional exclude for special types (instead returning "").
extern String measurementModeToString(Hydro_MeasurementMode measurementMode, bool excludeSpecial = false);
// Converts back to measurement mode enum from string.
extern Hydro_MeasurementMode measurementModeFromString(String measurementModeStr);

// Converts from display output mode enum to string, with optional exclude for special types (instead returning "").
extern String displayOutputModeToString(Hydro_DisplayOutputMode displayOutMode, bool excludeSpecial = false);
// Converts back to display output mode enum from string.
extern Hydro_DisplayOutputMode displayOutputModeFromString(String displayOutModeStr);

// Converts from control input mode enum to string, with optional exclude for special types (instead returning "").
extern String controlInputModeToString(Hydro_ControlInputMode controlInMode, bool excludeSpecial = false);
// Converts back to control input mode enum from string.
extern Hydro_ControlInputMode controlInputModeFromString(String controlInModeStr);

// Returns true for actuators that "live" in water (thus must do empty checks) as derived from actuator type enumeration.
inline bool getActuatorInWaterFromType(Hydro_ActuatorType actuatorType) { return actuatorType == Hydro_ActuatorType_WaterAerator || actuatorType == Hydro_ActuatorType_WaterPump || actuatorType == Hydro_ActuatorType_WaterHeater; }
// Returns true for actuators that pump liquid (thus must do empty/filled checks) as derived from actuator type enumeration.
inline bool getActuatorIsPumpFromType(Hydro_ActuatorType actuatorType) { return actuatorType == Hydro_ActuatorType_PeristalticPump || actuatorType == Hydro_ActuatorType_WaterPump; }
// Returns true for actuators that operate activation handles serially (as opposed to in-parallel) as derived from enabled mode enumeration.
inline bool getActuatorIsSerialFromMode(Hydro_EnableMode enableMode) { return enableMode >= Hydro_EnableMode_Serial; }

// Converts from actuator type enum to string, with optional exclude for special types (instead returning "").
extern String actuatorTypeToString(Hydro_ActuatorType actuatorType, bool excludeSpecial = false);
// Converts back to actuator type enum from string.
extern Hydro_ActuatorType actuatorTypeFromString(String actuatorTypeStr);

// Converts from sensor type enum to string, with optional exclude for special types (instead returning "").
extern String sensorTypeToString(Hydro_SensorType sensorType, bool excludeSpecial = false);
// Converts back to sensor type enum from string.
extern Hydro_SensorType sensorTypeFromString(String sensorTypeStr);

// Converts from crop type enum to string, with optional exclude for special types (instead returning "").
extern String cropTypeToString(Hydro_CropType cropType, bool excludeSpecial = false);
// Converts back to crop type enum from string.
extern Hydro_CropType cropTypeFromString(String cropTypeStr);

// Converts from substrate type enum to string, with optional exclude for special types (instead returning "").
extern String substrateTypeToString(Hydro_SubstrateType substrateType, bool excludeSpecial = false);
// Converts back to substrate type enum from string.
extern Hydro_SubstrateType substrateTypeFromString(String substrateTypeStr);

// Converts from fluid reservoir enum to string, with optional exclude for special types (instead returning "").
extern String reservoirTypeToString(Hydro_ReservoirType reservoirType, bool excludeSpecial = false);
// Converts back to fluid reservoir enum from string.
extern Hydro_ReservoirType reservoirTypeFromString(String reservoirTypeStr);

// Returns nominal rail voltage as derived from rail type enumeration.
extern float getRailVoltageFromType(Hydro_RailType railType);

// Converts from power rail enum to string, with optional exclude for special types (instead returning "").
extern String railTypeToString(Hydro_RailType railType, bool excludeSpecial = false);
// Converts back to power rail enum from string.
extern Hydro_RailType railTypeFromString(String railTypeStr);

// Converts from pin mode enum to string, with optional exclude for special types (instead returning "").
extern String pinModeToString(Hydro_PinMode pinMode, bool excludeSpecial = false);
// Converts back to pin mode enum from string.
extern Hydro_PinMode pinModeFromString(String pinModeStr);

// Converts from actuator enable mode enum to string, with optional exclude for special types (instead returning "").
extern String enableModeToString(Hydro_EnableMode enableMode, bool excludeSpecial = false);
// Converts back to actuator enable mode enum from string.
extern Hydro_EnableMode enableModeFromString(String enableModeStr);

// Converts from units category enum to string, with optional exclude for special types (instead returning "").
extern String unitsCategoryToString(Hydro_UnitsCategory unitsCategory, bool excludeSpecial = false);
// Converts back to units category enum from string.
extern Hydro_UnitsCategory unitsCategoryFromString(String unitsCategoryStr);

// Converts from units type enum to symbol string, with optional exclude for special types (instead returning "").
extern String unitsTypeToSymbol(Hydro_UnitsType unitsType, bool excludeSpecial = false);
// Converts back to units type enum from symbol.
extern Hydro_UnitsType unitsTypeFromSymbol(String unitsSymbolStr);

// Converts from position index to string, with optional exclude for special types (instead returning "").
extern String positionIndexToString(hposi_t positionIndex, bool excludeSpecial = false);
// Converts back to position index from string.
extern hposi_t positionIndexFromString(String positionIndexStr);

// Converts from boolean value to triggered/not-triggered trigger state.
inline Hydro_TriggerState triggerStateFromBool(bool value) { return value ? Hydro_TriggerState_Triggered : Hydro_TriggerState_NotTriggered; }
// Converts from triggered/not-triggered trigger state back into boolean value.
inline bool triggerStateToBool(Hydro_TriggerState state) { return state == Hydro_TriggerState_Triggered; }

// Explicit Specializations

template<> String commaStringFromArray<float>(const float *arrayIn, size_t length);
template<> String commaStringFromArray<double>(const double *arrayIn, size_t length);
template<> void commaStringToArray<float>(String stringIn, float *arrayOut, size_t length);
template<> void commaStringToArray<double>(String stringIn, double *arrayOut, size_t length);
template<> void commaStringToArray<String>(String stringIn, String *arrayOut, size_t length);
template<> bool arrayElementsEqual<float>(const float *arrayIn, size_t length, float value);
template<> bool arrayElementsEqual<double>(const double *arrayIn, size_t length, double value);

#endif // /ifndef HydroUtils_H
