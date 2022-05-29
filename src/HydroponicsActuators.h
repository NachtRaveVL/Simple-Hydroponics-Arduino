/*  Arduino Controller for Simple Hydroponics.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Actuators
*/

#ifndef HydroponicsActuator_H
#define HydroponicsActuator_H

class HydroponicsActuator;
class HydroponicsRelayActuator;

#include "Hydroponics.h"

class HydroponicsActuator {
public:
    HydroponicsActuator(byte outputPin,
                        Hydroponics_ActuatorType actuatorType,
                        Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_Undefined);
    virtual ~HydroponicsActuator();

    virtual bool tryEnableActuator() = 0;
    virtual void disableActuator() = 0;
    void enableActuatorFor(time_t enableTime);
    inline void enableActuatorUntil(date_t disableDate) { enableActuatorFor(disableDate - now()); }

    virtual void update() = 0;

    Hydroponics_ActuatorType getActuatorType() const;
    Hydroponics_FluidReservoir getFluidReservoir() const;
    bool getIsActuatorEnabled() const;

protected:
    byte _outputPin;
    Hydroponics_ActuatorType _actuatorType;
    Hydroponics_FluidReservoir _fluidReservoir;
    int _enabledDataTODO;
};

class HydroponicsRelayActuator : public HydroponicsActuator {
public:
    HydroponicsRelayActuator(byte outputPin,
                             Hydroponics_ActuatorType actuatorType,
                             Hydroponics_RelayRail relayRail,
                             Hydroponics_FluidReservoir fluidReservoir = Hydroponics_FluidReservoir_Undefined,
                             bool activeLow = true);
    virtual ~HydroponicsRelayActuator();

    Hydroponics_RelayRail getRelayRail() const;
    bool getActiveLow() const;

protected:
    Hydroponics_RelayRail _relayRail;
    bool _activeLow;
};


#endif // /ifndef HydroponicsActuator_H
