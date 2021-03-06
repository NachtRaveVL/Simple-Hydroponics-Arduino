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
    inline bool isRelayClass() const { return classType == Relay; }
    inline bool isRelayPumpClass() const { return classType == RelayPump; }
    inline bool isPWMClass() const { return classType == PWM; }
    inline bool isUnknownClass() const { return classType <= Unknown; }
    inline bool isAnyPumpClass() const { return isRelayPumpClass(); }
    inline bool isAnyRelayClass() const { return isRelayClass() || isRelayPumpClass(); }

    HydroponicsActuator(Hydroponics_ActuatorType actuatorType,
                        Hydroponics_PositionIndex actuatorIndex,
                        byte outputPin = -1,
                        int classType = Unknown);
    HydroponicsActuator(const HydroponicsActuatorData *dataIn);
    virtual ~HydroponicsActuator();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual bool enableActuator(float intensity = 1.0f, bool override = false) = 0;
    virtual bool getCanEnable() override;
    virtual bool getIsEnabled(float tolerance = 0.0f) const = 0;

    virtual void setContinuousPowerDraw(float contPowerDraw, Hydroponics_UnitsType contPowerDrawUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setContinuousPowerDraw(HydroponicsSingleMeasurement contPowerDraw) override;
    virtual const HydroponicsSingleMeasurement &getContinuousPowerDraw() override;

    virtual void setRail(HydroponicsIdentity powerRailId) override;
    virtual void setRail(shared_ptr<HydroponicsRail> powerRail) override;
    virtual shared_ptr<HydroponicsRail> getRail() override;

    virtual void setReservoir(HydroponicsIdentity reservoirId) override;
    virtual void setReservoir(shared_ptr<HydroponicsReservoir> reservoir) override;
    virtual shared_ptr<HydroponicsReservoir> getReservoir() override;

    byte getOutputPin() const;
    Hydroponics_ActuatorType getActuatorType() const;
    Hydroponics_PositionIndex getActuatorIndex() const;

    Signal<HydroponicsActuator *> &getActivationSignal();

protected:
    byte _outputPin;                                        // Output pin
    bool _enabled;                                          // Enabled state flag
    HydroponicsSingleMeasurement _contPowerDraw;            // Continuous power draw
    HydroponicsDLinkObject<HydroponicsRail> _rail;          // Power rail linkage
    HydroponicsDLinkObject<HydroponicsReservoir> _reservoir; // Reservoir linkage
    Signal<HydroponicsActuator *> _activateSignal;          // Activation update signal

    virtual HydroponicsData *allocateData() const override;
    virtual void saveToData(HydroponicsData *dataOut) override;
};


// Relay-based Binary Actuator
// This actuator acts as a standard on/off switch, typically paired with a variety of
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

    virtual bool enableActuator(float intensity = 1.0f, bool override = false) override;
    virtual void disableActuator() override;
    virtual bool getIsEnabled(float tolerance = 0.0f) const override;

    bool getActiveLow() const;

protected:
    bool _activeLow;                                        // If pulling pin to a LOW state infers ACTIVE status (default: true)

    virtual void saveToData(HydroponicsData *dataOut) override;
};


// Pump-based Relay Actuator
// This actuator acts as a water pump, and as such can attach to reservoirs
class HydroponicsPumpRelayActuator : public HydroponicsRelayActuator, public HydroponicsPumpObjectInterface, public HydroponicsFlowAwareInterface {
public:
    HydroponicsPumpRelayActuator(Hydroponics_ActuatorType actuatorType,
                                 Hydroponics_PositionIndex actuatorIndex,
                                 byte outputPin,
                                 bool activeLow = true,
                                 int classType = RelayPump);
    HydroponicsPumpRelayActuator(const HydroponicsPumpRelayActuatorData *dataIn);
    virtual ~HydroponicsPumpRelayActuator();

    virtual void update() override;
    virtual void resolveLinks() override;

    virtual bool enableActuator(float intensity = 1.0f, bool override = false) override;
    virtual void disableActuator() override;

    virtual bool canPump(float volume, Hydroponics_UnitsType volumeUnits = Hydroponics_UnitsType_Undefined) override;
    virtual bool pump(float volume, Hydroponics_UnitsType volumeUnits = Hydroponics_UnitsType_Undefined) override;
    virtual bool canPump(time_t timeMillis) override;
    virtual bool pump(time_t timeMillis) override;

    virtual void setReservoir(HydroponicsIdentity reservoirId) override;
    virtual void setReservoir(shared_ptr<HydroponicsReservoir> reservoir) override;
    virtual shared_ptr<HydroponicsReservoir> getReservoir() override;

    virtual void setFlowRateUnits(Hydroponics_UnitsType flowRateUnits) override;
    virtual Hydroponics_UnitsType getFlowRateUnits() const override;

    virtual void setOutputReservoir(HydroponicsIdentity destReservoirId) override;
    virtual void setOutputReservoir(shared_ptr<HydroponicsReservoir> destReservoir) override;
    virtual shared_ptr<HydroponicsReservoir> getOutputReservoir() override;

    virtual void setContinuousFlowRate(float contFlowRate, Hydroponics_UnitsType contFlowRateUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setContinuousFlowRate(HydroponicsSingleMeasurement contFlowRate) override;
    virtual const HydroponicsSingleMeasurement &getContinuousFlowRate() override;

    virtual void setFlowRateSensor(HydroponicsIdentity flowRateSensorId) override;
    virtual void setFlowRateSensor(shared_ptr<HydroponicsSensor> flowRateSensor) override;
    virtual shared_ptr<HydroponicsSensor> getFlowRateSensor() override;

    virtual void setFlowRate(float flowRate, Hydroponics_UnitsType flowRateUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setFlowRate(HydroponicsSingleMeasurement flowRate) override;
    virtual const HydroponicsSingleMeasurement &getFlowRate() override;

protected:
    Hydroponics_UnitsType _flowRateUnits;                   // Flow rate units preferred
    HydroponicsSingleMeasurement _contFlowRate;             // Continuous flow rate
    HydroponicsSingleMeasurement _flowRate;                 // Current flow rate
    bool _needsFlowRate;                                    // Needs flow rate update tracking flag
    float _pumpVolumeAcc;                                   // Accumulator for total volume of fluid pumped
    time_t _pumpTimeBegMillis;                              // Time millis pump was activated at
    time_t _pumpTimeAccMillis;                              // Time millis pump has been accumulated up to
    HydroponicsDLinkObject<HydroponicsReservoir> _destReservoir; // Output reservoir linkage
    HydroponicsDLinkObject<HydroponicsSensor> _flowRateSensor; // Flow rate sensor linkage

    virtual void saveToData(HydroponicsData *dataOut) override;

    void checkPumpingReservoirs();
    void pulsePumpingSensors();
    void handlePumpTime(time_t timeMillis);

    void attachFlowRateSensor();
    void detachFlowRateSensor();
    void handleFlowRateMeasure(const HydroponicsMeasurement *measurement);
};


// PWM-based Variable Actuator
// This actuator acts as a variable range dial, typically paired with a device that supports
// PWM throttling of some kind, such as a powered exhaust fan, or variable level LEDs.
class HydroponicsPWMActuator : public HydroponicsActuator {
public:
    HydroponicsPWMActuator(Hydroponics_ActuatorType actuatorType,
                           Hydroponics_PositionIndex actuatorIndex,
                           byte outputPin,
                           byte outputBitResolution = 8,
                           int classType = PWM);
    HydroponicsPWMActuator(const HydroponicsPWMActuatorData *dataIn);
    virtual ~HydroponicsPWMActuator();

    virtual bool enableActuator(float intensity = 1.0f, bool override = false) override;
    virtual void disableActuator() override;
    virtual bool getIsEnabled(float tolerance = 0.0f) const override;

    float getPWMAmount() const;
    int getPWMAmount(int) const;
    void setPWMAmount(float amount);
    void setPWMAmount(int amount);

    HydroponicsBitResolution getPWMResolution() const;

protected:
    float _pwmAmount;                                       // Current set PWM amount
    HydroponicsBitResolution _pwmResolution;                // PWM output resolution

    virtual void saveToData(HydroponicsData *dataOut) override;

    void applyPWM();
};


// Actuator Serialization Data
struct HydroponicsActuatorData : public HydroponicsObjectData
{
    byte outputPin;
    HydroponicsMeasurementData contPowerDraw;
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
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Pump Relay Actuator Serialization Data
struct HydroponicsPumpRelayActuatorData : public HydroponicsRelayActuatorData
{
    Hydroponics_UnitsType flowRateUnits;
    HydroponicsMeasurementData contFlowRate;
    char destReservoir[HYDRUINO_NAME_MAXSIZE];
    char flowRateSensor[HYDRUINO_NAME_MAXSIZE];

    HydroponicsPumpRelayActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// PWM Actuator Serialization Data
struct HydroponicsPWMActuatorData : public HydroponicsActuatorData
{
    byte outputBitResolution;

    HydroponicsPWMActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroponicsActuators_H
