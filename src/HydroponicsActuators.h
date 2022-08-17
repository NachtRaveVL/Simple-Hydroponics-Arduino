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

// Function pointer to pinMode for binary actuators. Allows an intermediary, such as a port extender or multiplexer, to be used to enable/disable actuator relay pins. By default uses pinMode.
extern void (*hy_act_pinMode)(pintype_t,uint8_t);

// Function pointer to digitalWrite for binary actuators. Allows an intermediary, such as a port extender or multiplexer, to be used to enable/disable actuator relay pins. By default uses digitalWrite.
extern void (*hy_act_digitalWrite)(pintype_t,uint8_t);


// Hydroponics Actuator Base
// This is the base class for all actuators, which defines how the actuator is identified,
// where it lives, and what it's attached to.
class HydroponicsActuator : public HydroponicsObject, public HydroponicsActuatorObjectInterface, public HydroponicsRailAttachmentInterface, public HydroponicsReservoirAttachmentInterface {
public:
    const enum : signed char { Relay, RelayPump, VariablePWM, Unknown = -1 } classType; // Actuator class type (custom RTTI)
    inline bool isRelayClass() const { return classType == Relay; }
    inline bool isRelayPumpClass() const { return classType == RelayPump; }
    inline bool isVariablePWMClass() const { return classType == VariablePWM; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroponicsActuator(Hydroponics_ActuatorType actuatorType,
                        Hydroponics_PositionIndex actuatorIndex,
                        pintype_t outputPin = -1,
                        int classType = Unknown);
    HydroponicsActuator(const HydroponicsActuatorData *dataIn);

    virtual void update() override;

    virtual bool enableActuator(float intensity = 1.0f, bool force = false) = 0;
    virtual bool getCanEnable() override;
    virtual bool isEnabled(float tolerance = 0.0f) const = 0;

    virtual void setContinuousPowerUsage(float contPowerUsage, Hydroponics_UnitsType contPowerUsageUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setContinuousPowerUsage(HydroponicsSingleMeasurement contPowerUsage) override;
    virtual const HydroponicsSingleMeasurement &getContinuousPowerUsage() override;

    virtual HydroponicsAttachment &getParentRail(bool resolve = true) override;
    virtual HydroponicsAttachment &getParentReservoir(bool resolve = true) override;

    inline pintype_t getOutputPin() const { return _outputPin; }
    inline Hydroponics_ActuatorType getActuatorType() const { return _id.objTypeAs.actuatorType; }
    inline Hydroponics_PositionIndex getActuatorIndex() const { return _id.posIndex; }

    Signal<HydroponicsActuator *> &getActivationSignal();

protected:
    pintype_t _outputPin;                                   // Output pin
    bool _enabled;                                          // Enabled state flag
    HydroponicsSingleMeasurement _contPowerUsage;           // Continuous power draw
    HydroponicsAttachment _rail;                            // Power rail attachment
    HydroponicsAttachment _reservoir;                       // Reservoir attachment
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
                             pintype_t outputPin,
                             bool activeLow = true,
                             int classType = Relay);
    HydroponicsRelayActuator(const HydroponicsRelayActuatorData *dataIn);
    virtual ~HydroponicsRelayActuator();

    virtual bool enableActuator(float intensity = 1.0f, bool force = false) override;
    virtual void disableActuator() override;
    virtual bool isEnabled(float tolerance = 0.0f) const override;

    inline bool getActiveLow() const { return _activeLow; }

protected:
    bool _activeLow;                                        // If pulling pin to a LOW state infers ACTIVE status (default: true)

    virtual void saveToData(HydroponicsData *dataOut) override;
};


// Pump-based Relay Actuator
// This actuator acts as a water pump, and as such can attach to reservoirs
class HydroponicsPumpRelayActuator : public HydroponicsRelayActuator, public HydroponicsPumpObjectInterface, public HydroponicsFlowSensorAttachmentInterface {
public:
    HydroponicsPumpRelayActuator(Hydroponics_ActuatorType actuatorType,
                                 Hydroponics_PositionIndex actuatorIndex,
                                 pintype_t outputPin,
                                 bool activeLow = true,
                                 int classType = RelayPump);
    HydroponicsPumpRelayActuator(const HydroponicsPumpRelayActuatorData *dataIn);

    virtual void update() override;

    virtual bool enableActuator(float intensity = 1.0f, bool force = false) override;
    virtual void disableActuator() override;

    virtual bool canPump(float volume, Hydroponics_UnitsType volumeUnits = Hydroponics_UnitsType_Undefined) override;
    virtual bool pump(float volume, Hydroponics_UnitsType volumeUnits = Hydroponics_UnitsType_Undefined) override;
    virtual bool canPump(time_t timeMillis) override;
    virtual bool pump(time_t timeMillis) override;

    virtual void setFlowRateUnits(Hydroponics_UnitsType flowRateUnits) override;
    virtual Hydroponics_UnitsType getFlowRateUnits() const override;

    virtual HydroponicsAttachment &getParentReservoir(bool resolve = true) override;
    virtual HydroponicsAttachment &getDestinationReservoir(bool resolve = true) override;

    virtual void setContinuousFlowRate(float contFlowRate, Hydroponics_UnitsType contFlowRateUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setContinuousFlowRate(HydroponicsSingleMeasurement contFlowRate) override;
    virtual const HydroponicsSingleMeasurement &getContinuousFlowRate() override;

    virtual HydroponicsSensorAttachment &getFlowRate(bool poll) override;

protected:
    Hydroponics_UnitsType _flowRateUnits;                   // Flow rate units preferred
    HydroponicsSingleMeasurement _contFlowRate;             // Continuous flow rate
    HydroponicsSensorAttachment _flowRate;                  // Flow rate sensor attachment
    HydroponicsAttachment _destReservoir;                   // Destination output reservoir
    float _pumpVolumeAcc;                                   // Accumulator for total volume of fluid pumped
    time_t _pumpTimeBegMillis;                              // Time millis pump was activated at
    time_t _pumpTimeAccMillis;                              // Time millis pump has been accumulated up to

    virtual void saveToData(HydroponicsData *dataOut) override;

    void checkPumpingReservoirs();
    void pollPumpingSensors();
    void handlePumpTime(time_t timeMillis);
};


// PWM-based Variable Actuator
// This actuator acts as a variable range dial, typically paired with a device that supports
// PWM throttling of some kind, such as a powered exhaust fan, or variable level LEDs.
class HydroponicsPWMActuator : public HydroponicsActuator {
public:
    HydroponicsPWMActuator(Hydroponics_ActuatorType actuatorType,
                           Hydroponics_PositionIndex actuatorIndex,
                           pintype_t outputPin,
#ifdef ESP_PLATFORM
#ifdef ESP32
                           uint8_t pwmChannel,
#endif
                           float pwmFrequency = 1000,
#endif
                           uint8_t outputBitRes = 8,
                           int classType = VariablePWM);
    HydroponicsPWMActuator(const HydroponicsPWMActuatorData *dataIn);
    virtual ~HydroponicsPWMActuator();

    virtual bool enableActuator(float intensity = 1.0f, bool force = false) override;
    virtual void disableActuator() override;
    virtual bool isEnabled(float tolerance = 0.0f) const override;

    inline float getPWMAmount() const { return _pwmAmount; }
    int getPWMAmount(int) const;
    void setPWMAmount(float amount);
    void setPWMAmount(int amount);

    inline HydroponicsBitResolution getPWMResolution() const { return _pwmResolution; }

protected:
    float _pwmAmount;                                       // Current set PWM amount
#ifdef ESP_PLATFORM
#ifdef ESP32
    uint8_t _pwmChannel;                                    // PWM output channel
#endif
    float _pwmFrequency;                                    // PWM output frequency
#endif
    HydroponicsBitResolution _pwmResolution;                // PWM output resolution

    virtual void saveToData(HydroponicsData *dataOut) override;

    void applyPWM();
};


// Actuator Serialization Data
struct HydroponicsActuatorData : public HydroponicsObjectData
{
    pintype_t outputPin;
    HydroponicsMeasurementData contPowerUsage;
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
#ifdef ESP_PLATFORM
    uint8_t pwmChannel;
    float pwmFrequency;
#endif
    uint8_t outputBitRes;

    HydroponicsPWMActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroponicsActuators_H
