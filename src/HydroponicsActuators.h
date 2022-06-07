/*  Arduino Controller for Simple Hydroponics.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Actuators
*/

#ifndef HydroponicsActuators_H
#define HydroponicsActuators_H

class HydroponicsActuator;
class HydroponicsRelayActuator;
class HydroponicsPWMActuator;

#include "Hydroponics.h"

// TODO
class HydroponicsActuator {
public:
    HydroponicsActuator(byte outputPin,
                        Hydroponics_ActuatorType actuatorType,
                        Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_Undefined);
    virtual ~HydroponicsActuator();

    virtual void disableActuator() = 0;
    virtual void enableActuator() = 0;
    void enableActuatorUntil(time_t disableDate);
    inline void enableActuatorFor(time_t enableTime) { enableActuatorUntil(now() + enableTime); }

    virtual void update();

    byte getOutputPin() const;
    Hydroponics_ActuatorType getActuatorType() const;
    Hydroponics_FluidReservoir getFluidReservoir() const;
    bool getIsActuatorEnabled() const;
    time_t getActuatorEnabledUntil() const;

protected:
    byte _outputPin;                                        // TODO
    Hydroponics_ActuatorType _actuatorType;                 // TODO
    Hydroponics_FluidReservoir _fluidReservoir;             // TODO
    bool _enabled;                                          // TODO
    time_t _enabledUntil;                                   // TODO
};


// TODO
class HydroponicsRelayActuator : public HydroponicsActuator {
public:
    HydroponicsRelayActuator(byte outputPin,
                             Hydroponics_ActuatorType actuatorType,
                             Hydroponics_RelayRail relayRail,
                             Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_Undefined,
                             bool activeLow = true);
    virtual ~HydroponicsRelayActuator();

    virtual void disableActuator();
    virtual void enableActuator();

    Hydroponics_RelayRail getRelayRail() const;
    bool getActiveLow() const;

protected:
    Hydroponics_RelayRail _relayRail;                       // TODO
    bool _activeLow;                                        // TODO
};


// TODO
class HydroponicsPWMActuator : public HydroponicsActuator {
public:
    HydroponicsPWMActuator(byte outputPin,
                           Hydroponics_ActuatorType actuatorType,
                           Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_Undefined,
                           byte writeBitResolution = 8);
    virtual ~HydroponicsPWMActuator();

    virtual void disableActuator();
    virtual void enableActuator();

    float getPWMAmount() const;
    int getPWMAmount(int) const;
    void setPWMAmount(float amount);
    void setPWMAmount(int amount);

    bool getIsActuatorEnabled(float tolerance) const;
    int getPWMMaxAmount() const;
    int getPWMBitResolution() const;

protected:
    float _pwmAmount;                                       // TODO
    int _pwmMaxAmount;                                      // TODO
    byte _pwmBitRes;                                        // TODO

    void applyPWM();
};

#endif // /ifndef HydroponicsActuators_H
