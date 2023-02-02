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


// Activation Handle
// Since actuators are shared objects, those wishing to enable any actuator must receive
// a valid handle. Actuators may customize how they handle multiple activation handles.
// Handles set an intensity value that may be ranged [0,1] or [-1,1] depending on the
// capabilities of the attached actuator. Handles do not guarantee activation unless their
// forced flag is set (also see Actuator activation signal), but can be set up to ensure
// actuators are enabled for a specified duration, which is able to be async updated.
// Finely timed activations should instead consider utilizing ActuatorTimedEnableTask.
struct HydroActivationHandle {
    SharedPtr<HydroActuator> actuator;  // Actuator owner
    Slot<millis_t> *update;             // Update function slot (owned)
    float intensity;                    // Intensity (normalized [0.0,1.0])
    Hydro_DirectionMode direction;      // Direction setting
    millis_t start;                     // Enablement start timestamp, in milliseconds, else 0
    millis_t duration;                  // Duration time requested/remaining, in milliseconds, else 0 for unlimited/no expiry
    bool forced;                        // If activation should force enable and ignore cursory canEnable checks

    // Default constructor for empty handles
    inline HydroActivationHandle() : actuator(nullptr), update(nullptr), intensity(0.0f), direction(Hydro_DirectionMode_Undefined), start(0), duration(0), forced(false) { ; }
    // Handle constructor that specifies a normalized enablement, ranged: normalized [0.0,1.0] for specified direction
    HydroActivationHandle(HydroActuator *actuator, Hydro_DirectionMode direction, float intensity = 1.0f, millis_t duration = 0, bool force = false);
    // Handle constructor that specifies a binary enablement, ranged: (=0=,!0!) for disable/enable or (<=-1,=0=,>=1) for reverse/stop/forward
    inline HydroActivationHandle(HydroActuator *actuator, int enabled, millis_t duration = 0, bool force = false)
        : HydroActivationHandle(actuator, (enabled > 0 ? Hydro_DirectionMode_Forward : enabled < 0 ? Hydro_DirectionMode_Reverse : Hydro_DirectionMode_Stop), (enabled ? 1.0f : 0.0f), duration, force) { ; }
    // Handle constructor that specifies a variable intensity enablement, ranged: [=0.0=,<=1.0] for disable/enable or [-1.0=>,=0.0=,<=1.0] for reverse/stop/forward
    inline HydroActivationHandle(HydroActuator *actuator, float intensity, millis_t duration = 0, bool force = false)
        : HydroActivationHandle(actuator, (intensity > FLT_EPSILON ? Hydro_DirectionMode_Forward : intensity < -FLT_EPSILON ? Hydro_DirectionMode_Reverse : Hydro_DirectionMode_Stop), fabsf(intensity), duration, force)  { ; }

    HydroActivationHandle(const HydroActivationHandle &handle);
    ~HydroActivationHandle();
    HydroActivationHandle &operator=(const HydroActivationHandle &handle);
    inline HydroActivationHandle &operator=(HydroActuator *actuator) { return operator=(actuator->enableActuator()); }

    // Sets an update slot to run during execution of handle that can further refine duration value.
    // Useful for rate-based or variable activations. Slot receives current system time millis() as parameter.
    void setUpdate(Slot<millis_t> &slot);

    void unset();                       // For disconnecting from an actuator

    // Driving intensity value
    inline float getDriveIntensity() const { return direction == Hydro_DirectionMode_Forward ? intensity :
                                                    direction == Hydro_DirectionMode_Reverse ? -intensity : 0.0f; }
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
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroActuator(Hydro_ActuatorType actuatorType,
                  Hydro_PositionIndex actuatorIndex,
                  int classType = Unknown);
    HydroActuator(const HydroActuatorData *dataIn);

    virtual void update() override;

    virtual bool getCanEnable() override;

    inline HydroActivationHandle enableActuator(Hydro_DirectionMode direction, float intensity = 1.0f, millis_t duration = 0, bool force = false) { return HydroActivationHandle(this, direction, intensity, duration, force); }
    inline HydroActivationHandle enableActuator(float intensity = 1.0f, millis_t duration = 0, bool force = false) { return HydroActivationHandle(this, intensity, duration, force); }
    inline HydroActivationHandle enableActuator(float intensity, millis_t duration = 0) { return HydroActivationHandle(this, intensity, duration); }
    inline HydroActivationHandle enableActuator(float intensity) { return HydroActivationHandle(this, intensity); }
    inline HydroActivationHandle enableActuator(millis_t duration, bool force = false) { return HydroActivationHandle(this, 1.0f, duration, force); }
    inline HydroActivationHandle enableActuator(bool force, millis_t duration = 0) { return HydroActivationHandle(this, 1.0f, duration, force); }

    inline void setEnableMode(Hydro_EnableMode enableMode) { _enableMode = enableMode; _needsUpdate = true; }
    inline Hydro_EnableMode getEnableMode() { return _enableMode; }

    virtual void setContinuousPowerUsage(HydroSingleMeasurement contPowerUsage) override;
    virtual const HydroSingleMeasurement &getContinuousPowerUsage() override;

    virtual HydroAttachment &getParentRail(bool resolve = true) override;
    virtual HydroAttachment &getParentReservoir(bool resolve = true) override;

    inline Hydro_ActuatorType getActuatorType() const { return _id.objTypeAs.actuatorType; }
    inline Hydro_PositionIndex getActuatorIndex() const { return _id.posIndex; }

    Signal<HydroActuator *> &getActivationSignal();

protected:
    bool _enabled;                                          // Enabled state flag
    bool _needsUpdate;                                      // Dirty flag for handle updates
    Hydro_EnableMode _enableMode;                           // Handle activation mode
    Vector<HydroActivationHandle *> _handles;               // Activation handles array
    HydroSingleMeasurement _contPowerUsage;                 // Continuous power draw
    HydroAttachment _rail;                                  // Power rail attachment
    HydroAttachment _reservoir;                             // Reservoir attachment
    Signal<HydroActuator *> _activateSignal;                // Activation update signal

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
                       Hydro_PositionIndex actuatorIndex,
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
                           Hydro_PositionIndex actuatorIndex,
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
                          Hydro_PositionIndex actuatorIndex,
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


// Throttled Pump Actuator
// This actuator acts as a throttleable water pump and attaches to both an input and output
// reservoir. Pumps using this class have variable flow control but also can be paired with
// a flow sensor for more precise pumping calculations.
//class HydroThrottledPumpActuator : public HydroVariableActuator, public HydroPumpObjectInterface, public HydroFlowSensorAttachmentInterface {
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
