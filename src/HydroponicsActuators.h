/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Actuators
*/

#ifndef HydroponicsActuators_H
#define HydroponicsActuators_H

class HydroponicsActuator;
class HydroponicsRelayActuator;
class HydroponicsPWMActuator;

#include "Hydroponics.h"

// Hydroponics Actuator Base
// This is the base class for all actuators, which defines how the actuator is identified and
// where it lives. Other than that, consider it a pure virtual base class.
class HydroponicsActuator : public HydroponicsObject {
public:
    HydroponicsActuator(Hydroponics_ActuatorType actuatorType,
                        Hydroponics_PositionIndex actuatorIndex,
                        byte outputPin = -1);
    virtual ~HydroponicsActuator();

    virtual void disableActuator() = 0;
    virtual void enableActuator() = 0;
    void enableActuatorUntil(time_t disableDate);           // Will be refactored in future
    inline void enableActuatorFor(time_t enableTime) { enableActuatorUntil(now() + enableTime); }

    virtual void update();

    byte getOutputPin() const;
    Hydroponics_ActuatorType getActuatorType() const;
    Hydroponics_PositionIndex getActuatorIndex() const;
    bool getIsActuatorEnabled() const;
    time_t getActuatorEnabledUntil() const;                 // Will be refactored in future

protected:
    byte _outputPin;                                        // Output pin
    bool _enabled;                                          // Enabled flag
    time_t _enabledUntil;                                   // Enabled until date, else 0 = disabled // Will be refactored in future
};


// Relay-based Binary Actuator
// This actuator acts as a standard on/off switch, typically paired with a variety of
// different equipment from pumps to grow lights and heaters.
class HydroponicsRelayActuator : public HydroponicsActuator {
public:
    HydroponicsRelayActuator(Hydroponics_ActuatorType actuatorType,
                             Hydroponics_PositionIndex actuatorIndex,
                             byte outputPin,
                             bool activeLow = true);
    virtual ~HydroponicsRelayActuator();

    virtual void disableActuator();
    virtual void enableActuator();

    bool getActiveLow() const;

protected:
    bool _activeLow;                                        // If pulling pin to a LOW state infers ACTIVE (default: true)
};


// PWM-based Variable Actuator
// This actuator acts as a variable range dial, typically paired with a device that supports
// PWM throttling of some kind, such as a powered exhaust fan, or variable level LEDs.
class HydroponicsPWMActuator : public HydroponicsActuator {
public:
    HydroponicsPWMActuator(Hydroponics_ActuatorType actuatorType,
                           Hydroponics_PositionIndex actuatorIndex,
                           byte outputPin,
                           byte outputBitResolution = 8);
    virtual ~HydroponicsPWMActuator();

    virtual void disableActuator();
    virtual void enableActuator();

    float getPWMAmount() const;
    int getPWMAmount(int) const;
    void setPWMAmount(float amount);
    void setPWMAmount(int amount);

    bool getIsActuatorEnabled(float tolerance) const;
    HydroponicsBitResolution getPWMResolution() const;

protected:
    float _pwmAmount;                                       // TODO
    HydroponicsBitResolution _pwmResolution;                // PWM output resolution

    void applyPWM();
};

#endif // /ifndef HydroponicsActuators_H
