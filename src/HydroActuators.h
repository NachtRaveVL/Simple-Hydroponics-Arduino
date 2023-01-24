/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Actuators
*/

#ifndef HydroActuators_H
#define HydroActuators_H

class HydroActuator;
class HydroRelayActuator;
class HydroPumpRelayActuator;
class HydroPWMActuator;

struct HydroActuatorData;
struct HydroPumpRelayActuatorData;

#include "Hydruino.h"

// Creates actuator object from passed actuator data (return ownership transfer - user code *must* delete returned object)
extern HydroActuator *newActuatorObjectFromData(const HydroActuatorData *dataIn);


// Actuator Base
// This is the base class for all actuators, which defines how the actuator is identified,
// where it lives, and what it's attached to.
class HydroActuator : public HydroObject, public HydroActuatorObjectInterface, public HydroRailAttachmentInterface, public HydroReservoirAttachmentInterface {
public:
    const enum : signed char { Relay, RelayPump, VariablePWM, Unknown = -1 } classType; // Actuator class type (custom RTTI)
    inline bool isRelayClass() const { return classType == Relay; }
    inline bool isRelayPumpClass() const { return classType == RelayPump; }
    inline bool isVariablePWMClass() const { return classType == VariablePWM; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroActuator(Hydro_ActuatorType actuatorType,
                  Hydro_PositionIndex actuatorIndex,
                  int classType = Unknown);
    HydroActuator(const HydroActuatorData *dataIn);

    virtual void update() override;

    virtual bool enableActuator(float intensity = 1.0f, bool force = false) = 0;
    virtual bool getCanEnable() override;
    virtual bool isEnabled(float tolerance = 0.0f) const = 0;

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
    HydroSingleMeasurement _contPowerUsage;                 // Continuous power draw
    HydroAttachment _rail;                                  // Power rail attachment
    HydroAttachment _reservoir;                             // Reservoir attachment
    Signal<HydroActuator *> _activateSignal;                // Activation update signal

    virtual HydroData *allocateData() const override;
    virtual void saveToData(HydroData *dataOut) override;
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

    virtual bool enableActuator(float intensity = 1.0f, bool force = false) override;
    virtual void disableActuator() override;
    virtual bool isEnabled(float tolerance = 0.0f) const override;

    inline const HydroDigitalPin &getOutputPin() const { return _outputPin; }

protected:
    HydroDigitalPin _outputPin;                             // Digital output pin

    virtual void saveToData(HydroData *dataOut) override;
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

    virtual bool enableActuator(float intensity = 1.0f, bool force = false) override;
    virtual void disableActuator() override;

    virtual bool canPump(float volume, Hydro_UnitsType volumeUnits = Hydro_UnitsType_Undefined) override;
    virtual bool pump(float volume, Hydro_UnitsType volumeUnits = Hydro_UnitsType_Undefined) override;
    virtual bool canPump(time_t timeMillis) override;
    virtual bool pump(time_t timeMillis) override;

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
    time_t _pumpTimeBegMillis;                              // Time millis pump was activated at
    time_t _pumpTimeAccMillis;                              // Time millis pump has been accumulated up to

    virtual void saveToData(HydroData *dataOut) override;

    void checkPumpingReservoirs();
    void pollPumpingSensors();
    void handlePumpTime(time_t timeMillis);
};


// PWM-based Variable Actuator
// This actuator acts as a variable range dial, typically paired with a device that supports
// PWM throttling of some kind, such as a powered exhaust fan, or variable level LEDs.
class HydroPWMActuator : public HydroActuator {
public:
    HydroPWMActuator(Hydro_ActuatorType actuatorType,
                     Hydro_PositionIndex actuatorIndex,
                     HydroAnalogPin outputPin,
                     int classType = VariablePWM);
    HydroPWMActuator(const HydroActuatorData *dataIn);
    virtual ~HydroPWMActuator();

    virtual bool enableActuator(float intensity = 1.0f, bool force = false) override;
    virtual void disableActuator() override;
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
};


// Actuator Serialization Data
struct HydroActuatorData : public HydroObjectData
{
    HydroPinData outputPin;
    HydroMeasurementData contPowerUsage;
    char railName[HYDRO_NAME_MAXSIZE];
    char reservoirName[HYDRO_NAME_MAXSIZE];

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
