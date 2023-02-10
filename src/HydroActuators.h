/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Actuators
*/

#ifndef HydroActuators_H
#define HydroActuators_H

struct HydroActivationHandle;

class HydroActuator;
class HydroRelayActuator;
class HydroRelayPumpActuator;
class HydroVariableActuator;
//class HydroVariablePumpActuator;

struct HydroActuatorData;
struct HydroPumpActuatorData;

#include "Hydruino.h"

// Creates actuator object from passed actuator data (return ownership transfer - user code *must* delete returned object)
extern HydroActuator *newActuatorObjectFromData(const HydroActuatorData *dataIn);


// Activation Flags
enum Hydro_ActivationFlags : unsigned char {
    Hydro_ActivationFlags_Forced        = 0x01,             // Force enable / ignore cursory canEnable checks
    Hydro_ActivationFlags_None          = 0x00              // Placeholder
};

// Activation Handle
// Since actuators are shared objects, those wishing to enable any actuator must receive
// a valid handle. Actuators may customize how they handle multiple activation handles.
// Handles represent a driving intensity value ranged [0,1] or [-1,1] depending on the
// capabilities of the attached actuator. Handles do not guarantee activation unless their
// forced flag is set (also see Actuator activation signal), but can be set up to ensure
// actuators are enabled for a specified duration, which is able to be async updated.
struct HydroActivationHandle {
    SharedPtr<HydroActuator> actuator;                      // Actuator owner, set only when activation requested (use operator= to set)
    struct Activation {
        Hydro_DirectionMode direction;                      // Normalized driving direction
        float intensity;                                    // Normalized driving intensity ([0.0,1.0])
        millis_t duration;                                  // Duration time remaining, in milliseconds, else -1 for non-diminishing/unlimited or 0 for finished
        Hydro_ActivationFlags flags;                        // Activation flags

        inline Activation(Hydro_DirectionMode directionIn, float intensityIn, millis_t durationIn, Hydro_ActivationFlags flagsIn) : direction(directionIn), intensity(constrain(intensityIn, 0.0f, 1.0f)), duration(durationIn), flags(flagsIn) { ; }
        inline Activation() : Activation(Hydro_DirectionMode_Undefined, 0.0f, 0, Hydro_ActivationFlags_None) { ; }

        inline bool isValid() const { return direction != Hydro_DirectionMode_Undefined; }
        inline bool isDone() const { return duration == 0; }
        inline bool isUntimed() const { return duration == -1; }
        inline bool isForced() const { return flags & Hydro_ActivationFlags_Forced; }
    } activation;                                           // Activation data
    millis_t checkTime;                                     // Last check timestamp, in milliseconds, else 0 for not started
    millis_t elapsed;                                       // Elapsed time accumulator, in milliseconds, else 0

    // Handle constructor that specifies a normalized enablement, ranged: [0.0,1.0] for specified direction
    HydroActivationHandle(SharedPtr<HydroActuator> actuator, Hydro_DirectionMode direction, float intensity = 1.0f, millis_t duration = -1, bool force = false);

    // Default constructor for empty handles
    inline HydroActivationHandle() : HydroActivationHandle(nullptr, Hydro_DirectionMode_Undefined, 0.0f, 0, false) { ; }
    HydroActivationHandle(const HydroActivationHandle &handle);
    ~HydroActivationHandle();
    HydroActivationHandle &operator=(SharedPtr<HydroActuator> actuator);
    inline HydroActivationHandle &operator=(const Activation &activationIn) { activation = activationIn; return *this; }
    inline HydroActivationHandle &operator=(const HydroActivationHandle &handle) { activation = handle.activation; return operator=(handle.actuator); }

    // Disconnects activation from an actuator (removes handle reference from actuator)
    void unset();

    // Elapses activation by delta, updating relevant activation values
    void elapseBy(millis_t delta);
    inline void elapseTo(millis_t time = nzMillis()) { elapseBy(time - checkTime); }

    inline bool isActive() const { return actuator && checkTime > 0; }
    inline bool isValid() const { return activation.isValid(); }
    inline bool isDone() const { return activation.isDone(); }
    inline bool isUntimed() const { return activation.isUntimed(); }
    inline bool isForced() const { return activation.isForced(); }

    inline millis_t getTimeLeft() const { return activation.duration; }
    inline millis_t getTimeActive(millis_t time = nzMillis()) const { return isActive() ? (time - checkTime) + elapsed : elapsed; }

    // De-normalized driving intensity value [-1.0,1.0]
    inline float getDriveIntensity() const { return activation.direction == Hydro_DirectionMode_Forward ? activation.intensity :
                                                    activation.direction == Hydro_DirectionMode_Reverse ? -activation.intensity : 0.0f; }
};


// Actuator Base
// This is the base class for all actuators, which defines how the actuator is identified,
// where it lives, and what it's attached to.
class HydroActuator : public HydroObject, public HydroActuatorObjectInterface, public HydroRailAttachmentInterface, public HydroReservoirAttachmentInterface {
public:
    const enum : signed char { Relay, RelayPump, Variable, VariablePump, Unknown = -1 } classType; // Actuator class type (custom RTTI)
    inline bool isRelayClass() const { return classType == Relay; }
    inline bool isRelayPumpClass() const { return classType == RelayPump; }
    inline bool isVariableClass() const { return classType == Variable; }
    inline bool isVariablePumpClass() const { return classType == VariablePump; }
    inline bool isAnyBinaryClass() const { isRelayClass() || isRelayPumpClass(); }
    inline bool isAnyVariableClass() const { isVariableClass() || isVariablePumpClass(); }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroActuator(Hydro_ActuatorType actuatorType,
                  hposi_t actuatorIndex,
                  int classType = Unknown);
    HydroActuator(const HydroActuatorData *dataIn);

    virtual void update() override;

    virtual bool getCanEnable() override;

    // Activating actuators is done through activation handles, which must stay memory
    // resident in order for the actuator to pick up and process it. Enablement mode
    // affects how handles are processed - in parallel, or in serial - and what the
    // applied output is. See HydroActuatorAttachment for an abstraction of this process.
    inline HydroActivationHandle enableActuator(Hydro_DirectionMode direction, float intensity = 1.0f, millis_t duration = -1, bool force = false) { return HydroActivationHandle(::getSharedPtr<HydroActuator>(this), direction, intensity, duration, force); }
    inline HydroActivationHandle enableActuator(float value, millis_t duration = -1, bool force = false) { return enableActuator(Hydro_DirectionMode_Forward, calibrationInvTransform(value), duration, force); }
    inline HydroActivationHandle enableActuator(millis_t duration, bool force = false) { return enableActuator(Hydro_DirectionMode_Forward, 1.0f, duration, force); }
    inline HydroActivationHandle enableActuator(bool force, millis_t duration = -1) { return enableActuator(Hydro_DirectionMode_Forward, 1.0f, duration, force); }

    inline void setEnableMode(Hydro_EnableMode enableMode) { _enableMode = enableMode; setNeedsUpdate(); }
    inline Hydro_EnableMode getEnableMode() { return _enableMode; }

    inline bool isSerialMode() { return getActuatorIsSerialFromMode(getEnableMode()); }
    inline bool isPumpType() { return getActuatorIsPumpFromType(getActuatorType()); }
    inline bool isDirectionalType() { return false; }

    virtual void setContinuousPowerUsage(HydroSingleMeasurement contPowerUsage) override;
    virtual const HydroSingleMeasurement &getContinuousPowerUsage() override;

    virtual HydroAttachment &getParentRail(bool resolve = true) override;
    virtual HydroAttachment &getParentReservoir(bool resolve = true) override;

    void setUserCalibrationData(HydroCalibrationData *userCalibrationData);
    inline const HydroCalibrationData *getUserCalibrationData() const { return _calibrationData; }

    // Transformation methods that convert from normalized driving intensity/driver value to calibration units
    inline float calibrationTransform(float value) const { return _calibrationData ? _calibrationData->transform(value) : value; }
    inline void calibrationTransform(float *valueInOut, Hydro_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->transform(valueInOut, unitsOut); } }
    inline HydroSingleMeasurement calibrationTransform(HydroSingleMeasurement measurement) { return _calibrationData ? HydroSingleMeasurement(_calibrationData->transform(measurement.value), _calibrationData->calibUnits, measurement.timestamp, measurement.frame) : measurement; }
    inline void calibrationTransform(HydroSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->transform(&measurementInOut->value, &measurementInOut->units); } }

    // Transformation methods that convert from calibration units to normalized driving intensity/driver value
    inline float calibrationInvTransform(float value) const { return _calibrationData ? _calibrationData->inverseTransform(value) : value; }
    inline void calibrationInvTransform(float *valueInOut, Hydro_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->inverseTransform(valueInOut, unitsOut); } }
    inline HydroSingleMeasurement calibrationInvTransform(HydroSingleMeasurement measurement) { return _calibrationData ? HydroSingleMeasurement(_calibrationData->inverseTransform(measurement.value), _calibrationData->calibUnits, measurement.timestamp, measurement.frame) : measurement; }
    inline void calibrationInvTransform(HydroSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->inverseTransform(&measurementInOut->value, &measurementInOut->units); } }

    inline Hydro_ActuatorType getActuatorType() const { return _id.objTypeAs.actuatorType; }
    inline hposi_t getActuatorIndex() const { return _id.posIndex; }

    inline void setNeedsUpdate() { _needsUpdate = true; }
    inline bool needsUpdate() { return _needsUpdate; }

    Signal<HydroActuator *, HYDRO_ACTUATOR_SIGNAL_SLOTS> &getActivationSignal();

protected:
    bool _enabled;                                          // Enabled state flag
    bool _needsUpdate;                                      // Stale flag for handle updates
    Hydro_EnableMode _enableMode;                           // Handle activation mode
    Vector<HydroActivationHandle *> _handles;               // Activation handles array
    HydroSingleMeasurement _contPowerUsage;                 // Continuous power draw
    HydroAttachment _rail;                                  // Power rail attachment
    HydroAttachment _reservoir;                             // Reservoir attachment
    const HydroCalibrationData *_calibrationData;           // Calibration data
    Signal<HydroActuator *, HYDRO_ACTUATOR_SIGNAL_SLOTS> _activateSignal; // Activation update signal

    virtual HydroData *allocateData() const override;
    virtual void saveToData(HydroData *dataOut) override;

    virtual void handleActivation() override;

    friend struct HydroActivationHandle;
};


// Binary Relay Actuator
// This actuator acts as a standard on/off switch, typically paired with a variety of
// different equipment from pumps to grow lights and heaters.
class HydroRelayActuator : public HydroActuator {
public:
    HydroRelayActuator(Hydro_ActuatorType actuatorType,
                       hposi_t actuatorIndex,
                       HydroDigitalPin outputPin,
                       int classType = Relay);
    HydroRelayActuator(const HydroActuatorData *dataIn);
    virtual ~HydroRelayActuator();

    virtual bool getCanEnable() override;
    virtual float getDriveIntensity() override;
    virtual bool isEnabled(float tolerance = 0.0f) const override;

    inline const HydroDigitalPin &getOutputPin() const { return _outputPin; }

protected:
    HydroDigitalPin _outputPin;                             // Digital output pin

    virtual void saveToData(HydroData *dataOut) override;

    virtual void _enableActuator(float intensity = 1.0) override;
    virtual void _disableActuator() override;
};


// Relay Pump Actuator
// This actuator acts as a water pump and attaches to both an input and output reservoir.
// Pumps using this class are either on/off and do not contain any variable flow control,
// but can be paired with a flow sensor for more precise pumping calculations.
class HydroRelayPumpActuator : public HydroRelayActuator, public HydroPumpObjectInterface, public HydroFlowSensorAttachmentInterface {
public:
    HydroRelayPumpActuator(Hydro_ActuatorType actuatorType,
                           hposi_t actuatorIndex,
                           HydroDigitalPin outputPin,
                           int classType = RelayPump);
    HydroRelayPumpActuator(const HydroPumpActuatorData *dataIn);

    virtual void update() override;

    virtual bool getCanEnable() override;

    virtual bool canPump(float volume, Hydro_UnitsType volumeUnits = Hydro_UnitsType_Undefined) override;
    virtual HydroActivationHandle pump(float volume, Hydro_UnitsType volumeUnits = Hydro_UnitsType_Undefined) override;
    virtual bool canPump(millis_t time) override;
    virtual HydroActivationHandle pump(millis_t time) override;

    virtual void setFlowRateUnits(Hydro_UnitsType flowRateUnits) override;
    virtual Hydro_UnitsType getFlowRateUnits() const override;

    virtual HydroAttachment &getParentReservoir(bool resolve = true) override;
    virtual HydroAttachment &getDestinationReservoir(bool resolve = true) override;

    virtual void setContinuousFlowRate(HydroSingleMeasurement contFlowRate) override;
    virtual const HydroSingleMeasurement &getContinuousFlowRate() override;

    virtual HydroSensorAttachment &getFlowRate(bool poll = false) override;

protected:
    Hydro_UnitsType _flowRateUnits;                         // Flow rate units preferred
    HydroSingleMeasurement _contFlowRate;                   // Continuous flow rate
    HydroSensorAttachment _flowRate;                        // Flow rate sensor attachment
    HydroAttachment _destReservoir;                         // Destination output reservoir
    float _pumpVolumeAccum;                                 // Accumulator for total volume of fluid pumped
    millis_t _pumpTimeStart;                                // Time millis pump was activated at
    millis_t _pumpTimeAccum;                                // Time millis pump has been accumulated up to

    virtual void saveToData(HydroData *dataOut) override;

    virtual void handleActivation() override;

    virtual void pollPumpingSensors() override;
    virtual void handlePumpTime(millis_t time) override;
};


// Variable Actuator
// This actuator acts as a simple variable ranged dial, typically paired with a variety of
// different equipment that allows analog throttle or position control.
class HydroVariableActuator : public HydroActuator {
public:
    HydroVariableActuator(Hydro_ActuatorType actuatorType,
                          hposi_t actuatorIndex,
                          HydroAnalogPin outputPin,
                          int classType = Variable);
    HydroVariableActuator(const HydroActuatorData *dataIn);
    virtual ~HydroVariableActuator();

    virtual bool getCanEnable() override;
    virtual float getDriveIntensity() override;
    virtual bool isEnabled(float tolerance = 0.0f) const override;

    inline int getDriveIntensity_raw() const { return _outputPin.bitRes.inverseTransform(_intensity); }

    inline const HydroAnalogPin &getOutputPin() const { return _outputPin; }

protected:
    HydroAnalogPin _outputPin;                              // Analog output pin
    float _intensity;                                       // Current set intensity

    virtual void saveToData(HydroData *dataOut) override;

    virtual void _enableActuator(float intensity = 1.0) override;
    virtual void _disableActuator() override;
    virtual void handleActivation() override;
};


// Variable/Throttled Pump Actuator
// This actuator acts as a throttleable water pump and attaches to both an input and output
// reservoir. Pumps using this class have variable flow control but also can be paired with
// a flow sensor for more precise pumping calculations.
//class HydroVariablePumpActuator : public HydroVariableActuator, public HydroPumpObjectInterface, public HydroFlowSensorAttachmentInterface {
// TODO
//};


// Actuator Serialization Data
struct HydroActuatorData : public HydroObjectData
{
    HydroPinData outputPin;
    Hydro_EnableMode enableMode;
    HydroMeasurementData contPowerUsage;
    char railName[HYDRO_NAME_MAXSIZE];
    char reservoirName[HYDRO_NAME_MAXSIZE];

    HydroActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Pump Actuator Serialization Data
struct HydroPumpActuatorData : public HydroActuatorData
{
    Hydro_UnitsType flowRateUnits;
    HydroMeasurementData contFlowRate;
    char destReservoir[HYDRO_NAME_MAXSIZE];
    char flowRateSensor[HYDRO_NAME_MAXSIZE];

    HydroPumpActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroActuators_H
