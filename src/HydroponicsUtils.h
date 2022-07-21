/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Utilities
*/

#ifndef HydroponicsUtils_H
#define HydroponicsUtils_H

struct HydroponicsBitResolution;
template<class T> struct HydroponicsDLinkObject;
template<typename ParameterType, int Slots> class SignalFireTask;
template<class ObjectType, typename ParameterType> class MethodSlotCallTask;
class ActuatorTimedEnableTask;

#include "Hydroponics.h"
#include "HydroponicsObject.h"


// Simple class for describing an analog bit resolution.
// This class is mainly used to calculate analog pin range boundaries. If override flag
// is not set then architecture check is made that may truncate passed bit resolution.
struct HydroponicsBitResolution {
    byte bitRes;                                // Bit resolution (# of bits)
    int maxVal;                                 // Maximum value (2 ^ (# of bits))

    // Convenience constructor
    HydroponicsBitResolution(byte bitRes,
                             bool override = false);

    // Transforms value from raw integer (or initial) value into normalized raw (or transformed) value.
    inline float transform(int intValue) const { return constrain((float)maxVal / intValue, 0.0f, 1.0f); }

    // Inverse transforms value from normalized raw (or transformed) value back into raw integer (or initial) value.
    inline int inverseTransform(float rawValue) const { return constrain((int)((float)maxVal * rawValue), 0, maxVal); }
};


// Delay/Dynamic Loaded/Linked Object Reference
// Simple class for delay loading objects that get references to others during system
// load. T should be a derived class type of HydroponicsObject, with getId() method.
template<class T>
struct HydroponicsDLinkObject {
    HydroponicsIdentity id;
    shared_ptr<T> obj;

    HydroponicsDLinkObject()
        : id(), obj(nullptr) { ; }
    template<class U>
    HydroponicsDLinkObject(const HydroponicsDLinkObject<U> &rhs)
        : id(rhs.id), obj(static_pointer_cast<T>(rhs.obj)) { ; }
    HydroponicsDLinkObject(HydroponicsIdentity idIn)
        : id(idIn), obj(nullptr) { ; }
    template<class U>
    HydroponicsDLinkObject(shared_ptr<U> objIn)
        : id(objIn->getId()), obj(static_pointer_cast<T>(objIn)) { ; }
    HydroponicsDLinkObject(const char *keyStrIn)
        : HydroponicsDLinkObject(HydroponicsIdentity(keyStrIn)) { ; }
    ~HydroponicsDLinkObject() { ; }

    inline bool isId() const { return !obj; }
    inline bool isObj() const { return (bool)obj; }
    inline bool needsResolved() const { return (!obj && (bool)id); }
    inline bool resolveIfNeeded() { return (!obj ? (bool)getObj() : false); }
    inline operator bool() const { return isObj(); }

    HydroponicsIdentity getId() const { return (bool)obj ? obj->getId() : id;  }
    Hydroponics_KeyType getKey() const { return (bool)obj ? obj->getKey() : id.key; }

    shared_ptr<T> getObj();
    inline T* operator->() { return getObj().get(); }

    template<class U>
    HydroponicsDLinkObject<T> &operator=(const HydroponicsDLinkObject<U> &rhs) { id = rhs.id; obj = static_pointer_cast<T>(rhs.obj); }
    HydroponicsDLinkObject<T> &operator=(const HydroponicsIdentity rhs) { id = rhs; obj = nullptr; }
    template<class U>
    HydroponicsDLinkObject<T> &operator=(shared_ptr<U> rhs) { obj = static_pointer_cast<T>(rhs); id = obj->getId(); }

    template<class U>
    bool operator==(const HydroponicsDLinkObject<U> &rhs) const { return id.key == rhs->getKey(); }
    bool operator==(const HydroponicsIdentity rhs) const { return id.key == rhs.key; }
    template<class U>
    bool operator==(shared_ptr<U> rhs) const { return id.key == rhs->getKey(); }
    bool operator==(HydroponicsObject *rhs) const { return id.key == rhs->getKey(); }

    template<class U>
    bool operator!=(const HydroponicsDLinkObject<U> &rhs) const { return id.key != rhs->getKey(); }
    bool operator!=(const HydroponicsIdentity rhs) const { return id.key != rhs.key; }
    template<class U>
    bool operator!=(shared_ptr<U> rhs) const { return id.key != rhs->getKey(); }
    bool operator!=(HydroponicsObject *rhs) const { return id.key != rhs->getKey(); }
};


// Scheduling

#ifndef HYDRUINO_DISABLE_MULTITASKING

#include "BasicInterruptAbstraction.h"

// Standard interrupt abstraction
extern BasicArduinoInterruptAbstraction interruptImpl;

// This will schedule an actuator to enable on the next TaskManagerIO runloop using the given intensity and enable time millis.
// Actuator is captured. Returns taskId or TASKMGR_INVALIDID on error.
taskid_t scheduleActuatorTimedEnableOnce(shared_ptr<HydroponicsActuator> actuator, float enableIntensity, time_t enableTimeMillis);

// This will schedule an actuator to enable on the next TaskManagerIO runloop using the given enable time millis.
// Actuator is captured. Returns taskId or TASKMGR_INVALIDID on error.
taskid_t scheduleActuatorTimedEnableOnce(shared_ptr<HydroponicsActuator> actuator, time_t enableTimeMillis);

// This will schedule a signal's fire method on the next TaskManagerIO runloop using the given call/fire parameter.
// Object is captured, if not nullptr. Returns taskId or TASKMGR_INVALIDID on error.
template<typename ParameterType, int Slots> taskid_t scheduleSignalFireOnce(shared_ptr<HydroponicsObject> object, Signal<ParameterType,Slots> &signal, ParameterType fireParam);

// This will schedule a signal's fire method on the next TaskManagerIO runloop using the given call/fire parameter, w/o capturing object.
// Returns taskId or TASKMGR_INVALIDID on error.
template<typename ParameterType, int Slots> taskid_t scheduleSignalFireOnce(Signal<ParameterType,Slots> &signal, ParameterType fireParam);

// This will schedule an object's method to be called on the next TaskManagerIO runloop using the given method slot and call parameter.
// Object is captured. Returns taskId or TASKMGR_INVALIDID on error. */
template<class ObjectType, typename ParameterType> taskid_t scheduleObjectMethodCallOnce(shared_ptr<ObjectType> object, void (ObjectType::*method)(ParameterType), ParameterType callParam);

// This will schedule an object's method to be called on the next TaskManagerIO runloop using the given method slot and call parameter, w/o capturing object.
// Returns taskId or TASKMGR_INVALIDID on error.
template<class ObjectType, typename ParameterType> taskid_t scheduleObjectMethodCallOnce(ObjectType *object, void (ObjectType::*method)(ParameterType), ParameterType callParam);

// This will schedule an object's method to be called on the next TaskManagerIO runloop using the taskId that was created, w/o capturing object.
// Object is captured. Returns taskId or TASKMGR_INVALIDID on error.
template<class ObjectType> taskid_t scheduleObjectMethodCallWithTaskIdOnce(shared_ptr<ObjectType> object, void (ObjectType::*method)(taskid_t));

// This will schedule an object's method to be called on the next TaskManagerIO runloop using the taskId that was created, w/o capturing object.
// Returns taskId or TASKMGR_INVALIDID on error.
template<class ObjectType> taskid_t scheduleObjectMethodCallWithTaskIdOnce(ObjectType *object, void (ObjectType::*method)(taskid_t));

// Given a valid task id, tries making the task repeating. Returns true if valid task and task is repeating, false otherwise.
bool tryEnableRepeatingTask(taskid_t taskId, time_t intervalMillis = 0);
// Given a valid task id, tries making the task non-repeating. Returns true if valid task and task is non-repeating, false otherwise.
bool tryDisableRepeatingTask(taskid_t taskId, time_t intervalMillis = 0);


// Signal Fire Task
// This class holds onto the passed signal and parameter to pass it along to the signal's
// fire method upon task execution.
template<typename ParameterType, int Slots>
class SignalFireTask : public Executable {
public:
    taskid_t taskId;
    SignalFireTask(shared_ptr<HydroponicsObject> object,
                   Signal<ParameterType,Slots> &signal,
                   ParameterType &param)
        : taskId(TASKMGR_INVALIDID), _object(object), _signal(&signal), _param(param) { ; }
    virtual ~SignalFireTask() { ; }

    void exec() override { _signal->fire(_param); }
private:
    shared_ptr<HydroponicsObject> _object;
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

    MethodSlotCallTask(shared_ptr<ObjectType> object, FunctPtr method, ParameterType callParam) : taskId(TASKMGR_INVALIDID), _object(object), _methodSlot(object.get(), method), _callParam(callParam) { ; }
    MethodSlotCallTask(ObjectType *object, FunctPtr method, ParameterType callParam) : taskId(TASKMGR_INVALIDID), _object(nullptr), _methodSlot(object, method), _callParam(callParam) { ; }
    virtual ~MethodSlotCallTask() { ; }

    void exec() override { _methodSlot(_callParam); }
private:
    shared_ptr<ObjectType> _object;
    MethodSlot<ObjectType,ParameterType> _methodSlot;
    ParameterType _callParam;

    friend taskid_t scheduleObjectMethodCallWithTaskIdOnce<ObjectType>(shared_ptr<ObjectType> object, void (ObjectType::*method)(taskid_t));
    friend taskid_t scheduleObjectMethodCallWithTaskIdOnce<ObjectType>(ObjectType *object, void (ObjectType::*method)(taskid_t));
};


// Actuator Precise Timed Enable Task
class ActuatorTimedEnableTask : public Executable {
    public:
    taskid_t taskId;
    ActuatorTimedEnableTask(shared_ptr<HydroponicsActuator> actuator,
                            float enableIntensity,
                            time_t enableTimeMillis);
    virtual ~ActuatorTimedEnableTask();

    void exec() override;
private:
    shared_ptr<HydroponicsActuator> _actuator;
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
extern Hydroponics *getHydroponicsInstance();
// Returns the active scheduler instance. Not guaranteed to be non-null.
extern HydroponicsScheduler *getSchedulerInstance();
// Returns the active logger instance. Not guaranteed to be non-null.
extern HydroponicsLogger *getLoggerInstance();
// Returns the active publisher instance. Not guaranteed to be non-null.
extern HydroponicsPublisher *getPublisherInstance();

// This logs a standard message to Logger output, with optional flush afterwards.
extern void logMessage(String message, bool flushAfter = false);
// This logs a standard message to Logger output, with optional flush afterwards.
extern void logWarning(String warning, bool flushAfter = false);
// This logs a standard message to Logger output, with optional flush afterwards.
extern void logError(String error, bool flushAfter = false);

// Publishes latest data from sensor to Publisher output.
extern void publishData(HydroponicsSensor *sensor);

// Returns current time, with proper time zone offset based on active hydroponics instance.
extern DateTime getCurrentTime();
// Returns the UTC seconds time that today started, accounting for time zone offset based on active hydroponics instance.
extern time_t getCurrentDayStartTime();
// Returns a proper filename for a storage monitoring file (log, data, etc) that uses YYMMDD as filename.
extern String getYYMMDDFilename(String prefix, String ext);

// Computes a hash for a string using a fast and efficient (read as: good enough for our use) hashing algorithm.
extern Hydroponics_KeyType stringHash(String string);

// Returns a string from char array with an exact max length.
// Null array or invalid length will abort function before encoding occurs, returning "null".
extern String stringFromChars(const char *charsIn, size_t length);

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
template<typename T> bool arrayElementsEqual(const T *arrayIn, size_t length, T value);

// Similar to the standard map function, but does it on any type.
template<typename T> T mapValue(T value, T inMin, T inMax, T outMin, T outMax);

// Returns the amount of space between the stack and heap (ie free space left), else -1 if undeterminable.
extern int freeMemory();

// This delays a finely timed amount, with spin loop nearer to end. Used in finely timed dispensers.
extern void delayFine(time_t timeMillis);

// This will query the active RTC sync device for the current time.
extern time_t rtcNow();

// This will handle interrupts for task manager.
extern void handleInterrupt(byte pin);

// Units & Conversion

// Tries to convert value from one unit to another (if supported), returning conversion success boolean.
// Convert param used in certain unit conversions as external additional value (e.g. voltage for power/current conversion).
// This is the main conversion function that all others wrap around.
extern bool tryConvertUnits(float valueIn, Hydroponics_UnitsType unitsIn, float *valueOut, Hydroponics_UnitsType unitsOut, float convertParam = FLT_UNDEF);

// Attempts to convert value in-place from one unit to another, and if successful then assigns value back overtop of itself.
// Convert param used in certain unit conversions. Returns conversion success boolean.
extern bool convertUnits(float *valueInOut, Hydroponics_UnitsType *unitsInOut, Hydroponics_UnitsType outUnits, float convertParam = FLT_UNDEF);
// Attempts to convert value from one unit to another, and if successful then assigns value, and optionally units, to output.
// Convert param used in certain unit conversions. Returns conversion success boolean.
extern bool convertUnits(float valueIn, float *valueOut, Hydroponics_UnitsType unitsIn, Hydroponics_UnitsType outUnits, Hydroponics_UnitsType *unitsOut = nullptr, float convertParam = FLT_UNDEF);
// Attempts to convert measurement in-place from one unit to another, and if successful then assigns value and units back overtop of itself.
// Convert param used in certain unit conversions. Returns conversion success boolean.
inline bool convertUnits(HydroponicsSingleMeasurement *measureInOut, Hydroponics_UnitsType outUnits, float convertParam = FLT_UNDEF) { return convertUnits(&measureInOut->value, &measureInOut->units, outUnits, convertParam); }
// Attemps to convert measurement from one unit to another, and if successful then assigns value and units to output measurement.
// Convert param used in certain unit conversions. Returns conversion success boolean.
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
extern int defaultDecimalPlacesRounding(Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Undefined);

// Rounds value according to default decimal places rounding, as typically used for data export, with optional additional decimal places.
inline float roundForExport(float value, unsigned int additionalDecPlaces = 0) { return roundToDecimalPlaces(value, defaultDecimalPlacesRounding() + additionalDecPlaces); }

// Linkages & Filtering

// Returns linkages list filtered down to actuators.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterActuators(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links);
// Returns linkages list filtered down to actuators by type.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterActuatorsByType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_ActuatorType actuatorType);
// Returns linkages list filtered down to sensors.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterSensors(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links);
// Returns linkages list filtered down to sensors by type.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterSensorsByType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_SensorType sensorType);
// Returns linkages list filtered down to crops.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterCrops(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links);
// Returns linkages list filtered down to crops by type.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterCropsByType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_CropType cropType);
// Returns linkages list filtered down to reservoirs.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterReservoirs(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links);
// Returns linkages list filtered down to reservoirs by type.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterReservoirsByType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_ReservoirType reservoirType);
// Returns linkages list filtered down to rails.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterRails(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links);
// Returns linkages list filtered down to rails by type.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterRailsByType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_RailType railType);
// Returns linkages list filtered down to pump actuators by source reservoir.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterPumpActuatorsByInputReservoir(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, HydroponicsReservoir *inputReservoir);
// Returns linkages list filtered down to pump actuators by source reservoir type.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterPumpActuatorsByInputReservoirType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_ReservoirType reservoirType);
// Returns linkages list filtered down to pump actuators by destination reservoir.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterPumpActuatorsByOutputReservoir(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, HydroponicsReservoir *outputReservoir);
// Returns linkages list filtered down to pump actuators by destination reservoir type.
template<size_t N = HYDRUINO_OBJ_LINKS_MAXSIZE> typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type linksFilterPumpActuatorsByOutputReservoirType(const typename Map<Hydroponics_KeyType, HydroponicsObject *, N>::type &links, Hydroponics_ReservoirType reservoirType);

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

// Sets random seed to an appropriate backing, in order: RTC's time, last analog pin's read (x4 to form 32-bit seed), or micros from system start.
extern void setupRandomSeed();

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

// Explicit Specializations

template<> String commaStringFromArray<float>(const float *arrayIn, size_t length);
template<> String commaStringFromArray<double>(const double *arrayIn, size_t length);
template<> void commaStringToArray<float>(String stringIn, float *arrayOut, size_t length);
template<> void commaStringToArray<double>(String stringIn, double *arrayOut, size_t length);
template<> bool arrayElementsEqual<float>(const float *arrayIn, size_t length, float value);
template<> bool arrayElementsEqual<double>(const double *arrayIn, size_t length, double value);

#endif // /ifndef HydroponicsUtils_H
