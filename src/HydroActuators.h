/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Actuators
*/

#ifndef HydroActuators_H
#define HydroActuators_H

struct HydroActivationHandle;

class HydroActuator;
class HydroRelayActuator;
class HydroPumpRelayActuator;
class HydroVariableActuator;

struct HydroActuatorData;
struct HydroPumpRelayActuatorData;

#include "Hydruino.h"

// Creates actuator object from passed actuator data (return ownership transfer - user code *must* delete returned object)
extern HydroActuator *newActuatorObjectFromData(const HydroActuatorData *dataIn);


// Activation Handle
// Since actuators are shared objects, those wishing to enable any actuator must receive
// a valid handle. Actuators may customize how they handle multiple activation handles.
// Handles set an intensity value that may be ranged [0,1] or [-1,1] depending on the
// capabilities of the attached actuator. Handles do not guarantee activation unless their
// forced flag is set (also see Actuator activation signal), but can be set up to ensure
// actuators are enabled for a specified duration, which is able to be updated asyncly.
// Finely timed activations should instead consider utilizing ActuatorTimedEnableTask.
struct HydroActivationHandle {
    HydroActuator *actuator;            // Actuator owner (strong)
    float intensity;                    // Intensity (normalized [0.0,1.0])
    Hydro_DirectionMode direction;      // Direction setting
    millis_t startMillis;               // Enablement start timestamp, in milliseconds, else 0
    millis_t durationMillis;            // Duration time requested/remaining, in milliseconds, else 0 for unlimited/no expiry
    bool forced;                        // If activation should force enable and ignore cursory canEnable checks

    // Default constructor for empty handles
    inline HydroActivationHandle() : actuator(nullptr), intensity(0.0f), direction(Hydro_DirectionMode_Undefined), startMillis(0), durationMillis(0), forced(false) { ; }
    // Handle constructor that specifies a normalized enablement, ranged: normalized [0.0,1.0] for specified direction
    HydroActivationHandle(HydroActuator *actuator, Hydro_DirectionMode direction, float intensity = 1.0f, millis_t forMillis = 0, bool force = false);
    // Handle constructor that specifies a binary enablement, ranged: (=0=,!0!) for disable/enable or (<=-1,=0=,>=1) for reverse/stop/forward
    inline HydroActivationHandle(HydroActuator *actuator, int isEnabled, millis_t forMillis = 0, bool force = false)
        : HydroActivationHandle(actuator, (isEnabled > 0 ? Hydro_DirectionMode_Forward : isEnabled < 0 ? Hydro_DirectionMode_Reverse : Hydro_DirectionMode_Stop), (isEnabled ? 1.0f : 0.0f), forMillis, force) { ; }
    // Handle constructor that specifies a variable intensity enablement, ranged: [=0.0=,<=1.0] for disable/enable or [-1.0=>,=0.0=,<=1.0] for reverse/stop/forward
    inline HydroActivationHandle(HydroActuator *actuator, float atIntensity, millis_t forMillis = 0, bool force = false)
        : HydroActivationHandle(actuator, (atIntensity > FLT_EPSILON ? Hydro_DirectionMode_Forward : atIntensity < -FLT_EPSILON ? Hydro_DirectionMode_Reverse : Hydro_DirectionMode_Stop), fabsf(atIntensity), forMillis, force)  { ; }

    HydroActivationHandle(const HydroActivationHandle &handle);
    ~HydroActivationHandle();
    HydroActivationHandle &operator=(const HydroActivationHandle &handle);

    // Driving intensity value ([-1,1] non-normalized, [0,1] normalized)
    inline float getDriveIntensity(bool normalized = false) const { return direction == Hydro_DirectionMode_Forward ? intensity :
                                                                           direction == Hydro_DirectionMode_Reverse ? (normalized ? intensity : -intensity) : 0.0f; }
};


// Actuator Base
// This is the base class for all actuators, which defines how the actuator is identified,
// where it lives, and what it's attached to.
class HydroActuator : public HydroObject, public HydroActuatorObjectInterface, public HydroRailAttachmentInterface, public HydroReservoirAttachmentInterface {
public:
    const enum : signed char { Relay, RelayPump, Variable, Unknown = -1 } classType; // Actuator class type (custom RTTI)
    inline bool isRelayClass() const { return classType == Relay; }
    inline bool isRelayPumpClass() const { return classType == RelayPump; }
    inline bool isVariableClass() const { return classType == Variable; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroActuator(Hydro_ActuatorType actuatorType,
                  Hydro_PositionIndex actuatorIndex,
                  int classType = Unknown);
    HydroActuator(const HydroActuatorData *dataIn);

    virtual void update() override;

    inline HydroActivationHandle enableActuator(Hydro_DirectionMode direction, float normIntensity = 1.0f, millis_t forMillis = 0, bool force = false) { return HydroActivationHandle(this, direction, normIntensity, forMillis, force); }
    inline HydroActivationHandle enableActuator(float driveIntensity = 1.0f, millis_t forMillis = 0, bool force = false) { return HydroActivationHandle(this, driveIntensity, forMillis, force); }
    inline HydroActivationHandle enableActuator(float driveIntensity, millis_t forMillis = 0) { return HydroActivationHandle(this, driveIntensity, forMillis); }
    inline HydroActivationHandle enableActuator(float driveIntensity) { return HydroActivationHandle(this, driveIntensity); }
    inline HydroActivationHandle enableActuator(millis_t forMillis, bool force = false) { return HydroActivationHandle(this, 1.0f, forMillis, force); }
    inline HydroActivationHandle enableActuator(bool force, millis_t forMillis = 0) { return HydroActivationHandle(this, 1.0f, forMillis, force); }

    virtual bool getCanEnable() override;
    virtual bool isEnabled(float tolerance = 0.0f) const = 0;

    inline void setEnableMode(Hydro_EnableMode enableMode) { _enableMode = enableMode; _needsUpdate = true; }
    inline Hydro_EnableMode getEnableMode() { return _enableMode; }

    virtual void setContinuousPowerUsage(float contPowerUsage, Hydro_UnitsType contPowerUsageUnits = Hydro_UnitsType_Undefined) override;
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

    friend struct HydroActivationHandle;
};


// Relay-based Binary Actuator
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
    virtual bool isEnabled(float tolerance = 0.0f) const override;

    inline const HydroDigitalPin &getOutputPin() const { return _outputPin; }

protected:
    HydroDigitalPin _outputPin;                             // Digital output pin

    virtual void saveToData(HydroData *dataOut) override;

    virtual bool _enableActuator(float intensity = 1.0) override;
    virtual void _disableActuator() override;
};


// Pump-based Relay Actuator
// This actuator acts as a water pump, and as such can attach to reservoirs
class HydroPumpRelayActuator : public HydroRelayActuator, public HydroPumpObjectInterface, public HydroFlowSensorAttachmentInterface {
public:
    HydroPumpRelayActuator(Hydro_ActuatorType actuatorType,
                           Hydro_PositionIndex actuatorIndex,
                           HydroDigitalPin outputPin,
                           int classType = RelayPump);
    HydroPumpRelayActuator(const HydroPumpRelayActuatorData *dataIn);

    virtual void update() override;

    virtual bool getCanEnable() override;

    virtual bool canPump(float volume, Hydro_UnitsType volumeUnits = Hydro_UnitsType_Undefined) override;
    virtual bool pump(float volume, Hydro_UnitsType volumeUnits = Hydro_UnitsType_Undefined) override;
    virtual bool canPump(millis_t timeMillis) override;
    virtual bool pump(millis_t timeMillis) override;

    virtual void setFlowRateUnits(Hydro_UnitsType flowRateUnits) override;
    virtual Hydro_UnitsType getFlowRateUnits() const override;

    virtual HydroAttachment &getParentReservoir(bool resolve = true) override;
    virtual HydroAttachment &getDestinationReservoir(bool resolve = true) override;

    virtual void setContinuousFlowRate(float contFlowRate, Hydro_UnitsType contFlowRateUnits = Hydro_UnitsType_Undefined) override;
    virtual void setContinuousFlowRate(HydroSingleMeasurement contFlowRate) override;
    virtual const HydroSingleMeasurement &getContinuousFlowRate() override;

    virtual HydroSensorAttachment &getFlowRate(bool poll) override;

protected:
    Hydro_UnitsType _flowRateUnits;                         // Flow rate units preferred
    HydroSingleMeasurement _contFlowRate;                   // Continuous flow rate
    HydroSensorAttachment _flowRate;                        // Flow rate sensor attachment
    HydroAttachment _destReservoir;                         // Destination output reservoir
    float _pumpVolumeAcc;                                   // Accumulator for total volume of fluid pumped
    millis_t _pumpTimeBegMillis;                            // Time millis pump was activated at
    millis_t _pumpTimeAccMillis;                            // Time millis pump has been accumulated up to

    virtual void saveToData(HydroData *dataOut) override;

    virtual bool _enableActuator(float intensity = 1.0) override;
    virtual void _disableActuator() override;

    void pollPumpingSensors();
    void handlePumpTime(millis_t timeMillis);
};


// Variable Actuator
// This actuator acts as a variable range dial, typically paired with a device that supports
// throttling of some kind, such as a powered exhaust fan, or variable level LEDs.
class HydroVariableActuator : public HydroActuator {
public:
    HydroVariableActuator(Hydro_ActuatorType actuatorType,
                          Hydro_PositionIndex actuatorIndex,
                          HydroAnalogPin outputPin,
                          int classType = Variable);
    HydroVariableActuator(const HydroActuatorData *dataIn);
    virtual ~HydroVariableActuator();

    virtual bool isEnabled(float tolerance = 0.0f) const override;

    inline float getPWMAmount() const { return _pwmAmount; }
    int getPWMAmount(int) const;
    void setPWMAmount(float amount);
    void setPWMAmount(int amount);

    inline const HydroAnalogPin &getOutputPin() const { return _outputPin; }

protected:
    HydroAnalogPin _outputPin;                              // Analog output pin
    float _pwmAmount;                                       // Current set PWM amount

    virtual void saveToData(HydroData *dataOut) override;

    virtual bool _enableActuator(float intensity = 1.0) override;
    virtual void _disableActuator() override;
};


// Actuator Serialization Data
struct HydroActuatorData : public HydroObjectData
{
    HydroPinData outputPin;
    HydroMeasurementData contPowerUsage;
    char railName[HYDRO_NAME_MAXSIZE];
    char reservoirName[HYDRO_NAME_MAXSIZE];
    Hydro_EnableMode enableMode;

    HydroActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Pump Relay Actuator Serialization Data
struct HydroPumpRelayActuatorData : public HydroActuatorData
{
    Hydro_UnitsType flowRateUnits;
    HydroMeasurementData contFlowRate;
    char destReservoir[HYDRO_NAME_MAXSIZE];
    char flowRateSensor[HYDRO_NAME_MAXSIZE];

    HydroPumpRelayActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroActuators_H
