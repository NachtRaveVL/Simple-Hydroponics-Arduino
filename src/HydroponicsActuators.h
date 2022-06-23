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

#include "Hydroponics.h"

// Hydroponics Actuator Base
// This is the base class for all actuators, which defines how the actuator is identified,
// where it lives, and what it's attached to.
class HydroponicsActuator : public HydroponicsObject {
public:
    const enum { Relay, RelayPump, PWM, Unknown = -1 } classType;// Actuator class type (custom RTTI)
    inline bool isRelayClass() { return classType == Relay; }
    inline bool isRelayPumpClass() { return classType == RelayPump; }
    inline bool isPWMClass() { return classType == PWM; }
    inline bool isUnknownClass() { return classType == Unknown; }
    inline bool isAnyPumpClass() { return isRelayPumpClass(); }
    inline bool isAnyRelayClass() { return isRelayClass() || isRelayPumpClass(); }

    HydroponicsActuator(Hydroponics_ActuatorType actuatorType,
                        Hydroponics_PositionIndex actuatorIndex,
                        byte outputPin = -1,
                        int classType = Unknown);
    virtual ~HydroponicsActuator();

    virtual void resolveLinks() override;
    virtual void update() override;

    virtual void disableActuator() = 0;
    virtual void enableActuator() = 0;
    void enableActuatorUntil(time_t disableDate);           // Will be refactored in future
    inline void enableActuatorFor(time_t enableTime) { enableActuatorUntil(now() + enableTime); }

    void setRail(HydroponicsIdentity powerRailId);
    void setRail(shared_ptr<HydroponicsRail> powerRail);
    shared_ptr<HydroponicsRail> getRail();

    void setReservoir(HydroponicsIdentity reservoirId);
    void setReservoir(shared_ptr<HydroponicsReservoir> reservoir);
    shared_ptr<HydroponicsReservoir> getReservoir();

    byte getOutputPin() const;
    Hydroponics_ActuatorType getActuatorType() const;
    Hydroponics_PositionIndex getActuatorIndex() const;
    bool getIsActuatorEnabled() const;
    time_t getActuatorEnabledUntil() const;                 // Will be refactored in future

    Signal<HydroponicsActuator *> &getActivationSignal();   // Signal for activation updates

protected:
    byte _outputPin;                                        // Output pin
    bool _enabled;                                          // Enabled flag
    time_t _enabledUntil;                                   // Enabled until date, else 0 = disabled // Will be refactored in future

    HydroponicsDLinkObject<HydroponicsRail> _powerRail;     // Power rail linkage
    HydroponicsDLinkObject<HydroponicsReservoir> _reservoir; // Reservoir linkage

    Signal<HydroponicsActuator *> _activateSignal;          // Activation signal
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
    virtual ~HydroponicsRelayActuator();

    void disableActuator() override;
    void enableActuator() override;

    bool getActiveLow() const;

protected:
    bool _activeLow;                                        // If pulling pin to a LOW state infers ACTIVE (default: true)
};


// Pump-based Relay Actuator
// This actuator acts as a water pump, and as such can attach to reservoirs
class HydroponicsPumpRelayActuator : public HydroponicsRelayActuator, public HydroponicsPumpObjectInterface {
public:
    HydroponicsPumpRelayActuator(Hydroponics_ActuatorType actuatorType,
                                 Hydroponics_PositionIndex actuatorIndex,
                                 byte outputPin,
                                 bool activeLow = true,
                                 int classType = RelayPump);
    virtual ~HydroponicsPumpRelayActuator();

    void resolveLinks() override;

    void setContinuousFlowRate(float flowRate, Hydroponics_UnitsType flowRateUnits = Hydroponics_UnitsType_Undefined) override;
    void setContinuousFlowRate(HydroponicsSingleMeasurement flowRate) override;
    const HydroponicsSingleMeasurement &getContinuousFlowRate() const override;

    void setFlowRateSensor(HydroponicsIdentity flowRateSensorId) override;
    void setFlowRateSensor(shared_ptr<HydroponicsSensor> flowRateSensor) override;
    shared_ptr<HydroponicsSensor> getFlowRateSensor() override;
    const HydroponicsSingleMeasurement &getInstantaneousFlowRate() const override;

    void setOutputReservoir(HydroponicsIdentity outputReservoirId) override;
    void setOutputReservoir(shared_ptr<HydroponicsReservoir> outputReservoir) override;
    shared_ptr<HydroponicsReservoir> getOutputReservoir() override;

protected:
    HydroponicsSingleMeasurement _contFlowRate;             // Continuous flow rate
    HydroponicsSingleMeasurement _instFlowRate;             // Instantaneous flow rate

    HydroponicsDLinkObject<HydroponicsSensor> _flowRateSensor; // Flow rate sensor linkage
    HydroponicsDLinkObject<HydroponicsReservoir> _outputReservoir; // Output reservoir linkage

    void attachFlowRateSensor();
    void detachFlowRateSensor();
    void handleFlowRateMeasure(HydroponicsMeasurement *measurement);
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
    virtual ~HydroponicsPWMActuator();

    void disableActuator() override;
    void enableActuator() override;

    float getPWMAmount() const;
    int getPWMAmount(int) const;
    void setPWMAmount(float amount);
    void setPWMAmount(int amount);

    bool getIsActuatorEnabled(float tolerance) const;
    HydroponicsBitResolution getPWMResolution() const;

protected:
    float _pwmAmount;                                       // Current set PWM amount
    HydroponicsBitResolution _pwmResolution;                // PWM output resolution

    void applyPWM();
};

#endif // /ifndef HydroponicsActuators_H
