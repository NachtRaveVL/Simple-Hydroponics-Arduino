/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Actuators
*/

#ifndef HydroponicsActuators_H
#define HydroponicsActuators_H

class HydroponicsActuator;
class HydroponicsRelayActuator;
class HydroponicsPumpRelayActuator;
class HydroponicsPWMActuator;

struct HydroponicsActuatorData;
struct HydroponicsRelayActuatorData;
struct HydroponicsPumpRelayActuatorData;
struct HydroponicsPWMActuatorData;

#include "Hydroponics.h"

// Creates actuator object from passed actuator data (return ownership transfer - user code *must* delete returned object)
extern HydroponicsActuator *newActuatorObjectFromData(const HydroponicsActuatorData *dataIn);

// Hydroponics Actuator Base
// This is the base class for all actuators, which defines how the actuator is identified,
// where it lives, and what it's attached to.
class HydroponicsActuator : public HydroponicsObject, public HydroponicsActuatorObjectInterface, public HydroponicsRailAttachmentInterface, public HydroponicsReservoirAttachmentInterface {
public:
    const enum { Relay, RelayPump, PWM, Unknown = -1 } classType; // Actuator class type (custom RTTI)
    inline bool isRelayClass() { return classType == Relay; }
    inline bool isRelayPumpClass() { return classType == RelayPump; }
    inline bool isPWMClass() { return classType == PWM; }
    inline bool isUnknownClass() { return classType <= Unknown; }
    inline bool isAnyPumpClass() { return isRelayPumpClass(); }
    inline bool isAnyRelayClass() { return isRelayClass() || isRelayPumpClass(); }

    HydroponicsActuator(Hydroponics_ActuatorType actuatorType,
                        Hydroponics_PositionIndex actuatorIndex,
                        byte outputPin = -1,
                        int classType = Unknown);
    HydroponicsActuator(const HydroponicsActuatorData *dataIn);
    virtual ~HydroponicsActuator();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual bool enableActuator(bool override = false, float intensity = 1.0f) = 0;
    virtual void disableAt(time_t disableTime) override;
    virtual void disableActuator() = 0;
    virtual bool getCanEnable() override;
    virtual bool getIsEnabled(float tolerance = 0.5) const = 0;

    void setRail(HydroponicsIdentity powerRailId) override;
    void setRail(shared_ptr<HydroponicsRail> powerRail) override;
    shared_ptr<HydroponicsRail> getRail() override;

    void setReservoir(HydroponicsIdentity reservoirId) override;
    void setReservoir(shared_ptr<HydroponicsReservoir> reservoir) override;
    shared_ptr<HydroponicsReservoir> getReservoir() override;

    byte getOutputPin() const;
    Hydroponics_ActuatorType getActuatorType() const;
    Hydroponics_PositionIndex getActuatorIndex() const;

    Signal<HydroponicsActuator *> &getActivationSignal();

protected:
    byte _outputPin;                                        // Output pin
    time_t _disableTime;                                    // Disable after time

    HydroponicsDLinkObject<HydroponicsRail> _rail;          // Power rail linkage
    HydroponicsDLinkObject<HydroponicsReservoir> _reservoir; // Reservoir linkage
    Signal<HydroponicsActuator *> _activateSignal;          // Activation update signal

    HydroponicsData *allocateData() const override;
    virtual void saveToData(HydroponicsData *dataOut) const override;
};


// Relay-based Binary Actuator
// This actuator acts typeAs a standard on/off switch, typically paired with a variety of
// different equipment from pumps to grow lights and heaters.
class HydroponicsRelayActuator : public HydroponicsActuator {
public:
    HydroponicsRelayActuator(Hydroponics_ActuatorType actuatorType,
                             Hydroponics_PositionIndex actuatorIndex,
                             byte outputPin,
                             bool activeLow = true,
                             int classType = Relay);
    HydroponicsRelayActuator(const HydroponicsRelayActuatorData *dataIn);
    virtual ~HydroponicsRelayActuator();

    bool enableActuator(bool override = false, float intensity = 1.0f) override;
    void disableActuator() override;
    bool getIsEnabled(float tolerance = 0.5) const override;

    bool getActiveLow() const;

protected:
    bool _enabled;                                          // Enabled state flag
    bool _activeLow;                                        // If pulling pin to a LOW state infers ACTIVE (default: true)

    void saveToData(HydroponicsData *dataOut) const override;
};


// Pump-based Relay Actuator
// This actuator acts typeAs a water pump, and typeAs such can attach to reservoirs
class HydroponicsPumpRelayActuator : public HydroponicsRelayActuator, public HydroponicsPumpObjectInterface, public HydroponicsFlowAwareInterface {
public:
    HydroponicsPumpRelayActuator(Hydroponics_ActuatorType actuatorType,
                                 Hydroponics_PositionIndex actuatorIndex,
                                 byte outputPin,
                                 bool activeLow = true,
                                 int classType = RelayPump);
    HydroponicsPumpRelayActuator(const HydroponicsPumpRelayActuatorData *dataIn);
    virtual ~HydroponicsPumpRelayActuator();

    void resolveLinks() override;

    bool canPump(float volume, Hydroponics_UnitsType volumeUnits = Hydroponics_UnitsType_Undefined) override;
    void pump(float volume, Hydroponics_UnitsType volumeUnits = Hydroponics_UnitsType_Undefined) override;

    void setReservoir(HydroponicsIdentity reservoirId) override;
    void setReservoir(shared_ptr<HydroponicsReservoir> reservoir) override;
    shared_ptr<HydroponicsReservoir> getReservoir() override;

    void setOutputReservoir(HydroponicsIdentity outputReservoirId) override;
    void setOutputReservoir(shared_ptr<HydroponicsReservoir> outputReservoir) override;
    shared_ptr<HydroponicsReservoir> getOutputReservoir() override;

    void setFlowRateUnits(Hydroponics_UnitsType flowRateUnits);
    Hydroponics_UnitsType getFlowRateUnits() const;

    void setContinuousFlowRate(float contFlowRate, Hydroponics_UnitsType contFlowRateUnits = Hydroponics_UnitsType_Undefined) override;
    void setContinuousFlowRate(HydroponicsSingleMeasurement contFlowRate) override;
    const HydroponicsSingleMeasurement &getContinuousFlowRate() const override;

    void setFlowRateSensor(HydroponicsIdentity flowRateSensorId) override;
    void setFlowRateSensor(shared_ptr<HydroponicsSensor> flowRateSensor) override;
    shared_ptr<HydroponicsSensor> getFlowRateSensor() override;

    void setInstantaneousFlowRate(float instFlowRate, Hydroponics_UnitsType instFlowRateUnits = Hydroponics_UnitsType_Undefined) override;
    void setInstantaneousFlowRate(HydroponicsSingleMeasurement instFlowRate) override;
    const HydroponicsSingleMeasurement &getInstantaneousFlowRate() const override;

protected:
    HydroponicsDLinkObject<HydroponicsReservoir> _outputReservoir; // Output reservoir linkage
    HydroponicsDLinkObject<HydroponicsSensor> _flowRateSensor; // Flow rate sensor linkage
    Hydroponics_UnitsType _flowRateUnits;                   // Flow rate units preferred
    HydroponicsSingleMeasurement _contFlowRate;             // Continuous flow rate
    HydroponicsSingleMeasurement _instFlowRate;             // Instantaneous flow rate

    void saveToData(HydroponicsData *dataOut) const override;

    void attachFlowRateSensor();
    void detachFlowRateSensor();
    void handleFlowRateMeasure(HydroponicsMeasurement *measurement);
};


// PWM-based Variable Actuator
// This actuator acts typeAs a variable range dial, typically paired with a device that supports
// PWM throttling of some kind, such typeAs a powered exhaust fan, or variable level LEDs.
class HydroponicsPWMActuator : public HydroponicsActuator {
public:
    HydroponicsPWMActuator(Hydroponics_ActuatorType actuatorType,
                           Hydroponics_PositionIndex actuatorIndex,
                           byte outputPin,
                           byte outputBitResolution = 8,
                           int classType = PWM);
    HydroponicsPWMActuator(const HydroponicsPWMActuatorData *dataIn);
    virtual ~HydroponicsPWMActuator();

    bool enableActuator(bool override = false, float intensity = 1.0f) override;
    void disableActuator() override;
    bool getIsEnabled(float tolerance = 0.5) const override;

    float getPWMAmount() const;
    int getPWMAmount(int) const;
    void setPWMAmount(float amount);
    void setPWMAmount(int amount);

    HydroponicsBitResolution getPWMResolution() const;

protected:
    bool _enabled;                                          // Enabled state flag
    float _pwmAmount;                                       // Current set PWM amount
    HydroponicsBitResolution _pwmResolution;                // PWM output resolution

    void saveToData(HydroponicsData *dataOut) const override;

    void applyPWM();
};


// Actuator Serialization Data
struct HydroponicsActuatorData : public HydroponicsObjectData
{
    byte outputPin;
    char railName[HYDRUINO_NAME_MAXSIZE];
    char reservoirName[HYDRUINO_NAME_MAXSIZE];

    HydroponicsActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Relay Actuator Serialization Data
struct HydroponicsRelayActuatorData : public HydroponicsActuatorData
{
    bool activeLow;

    HydroponicsRelayActuatorData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Pump Relay Actuator Serialization Data
struct HydroponicsPumpRelayActuatorData : public HydroponicsRelayActuatorData
{
    char outputReservoirName[HYDRUINO_NAME_MAXSIZE];
    char flowRateSensorName[HYDRUINO_NAME_MAXSIZE];
    Hydroponics_UnitsType flowRateUnits;
    HydroponicsMeasurementData contFlowRate;

    HydroponicsPumpRelayActuatorData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};

// PWM Actuator Serialization Data
struct HydroponicsPWMActuatorData : public HydroponicsActuatorData
{
    byte outputBitResolution;

    HydroponicsPWMActuatorData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroponicsActuators_H
