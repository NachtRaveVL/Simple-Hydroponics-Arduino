/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Power Rails
*/

#ifndef HydroponicsRails_H
#define HydroponicsRails_H

class HydroponicsRail;
class HydroponicsSimpleRail;

#include "Hydroponics.h"

// Hydroponics Power Rail Base
// This is the base class for all power rails, which defines how the rail is identified,
// where it lives, what's attached to it, and who can activate under it.
class HydroponicsRail : public HydroponicsObject {
public:
    const enum { Simple, Unknown = -1 } classType;          // Power rail class (custom RTTI)
    inline bool isSimpleClass() { return classType == Simple; }
    inline bool isUnknownClass() { return classType == Unknown; }

    HydroponicsRail(Hydroponics_RailType railType,
                    Hydroponics_PositionIndex railIndex,
                    int classType = Unknown);
    virtual ~HydroponicsRail();

    virtual bool canActivate(shared_ptr<HydroponicsActuator> actuator) = 0;

    virtual bool addActuator(HydroponicsActuator *actuator);
    virtual bool removeActuator(HydroponicsActuator *actuator);
    inline bool hasActuator(HydroponicsActuator *actuator) const { return hasLinkage(actuator); }
    arx::map<Hydroponics_KeyType, HydroponicsActuator *> getActuators() const;

    Hydroponics_RailType getRailType() const;
    Hydroponics_PositionIndex getRailIndex() const;

    Signal<HydroponicsRail *> &getCapacitySignal();

protected:
    Signal<HydroponicsRail *> _capacitySignal;              // Capacity changed signal
};

// Simple Power Rail
// Basic power rail that tracks # of devices turned on, with a limit to how many
// can be on at the same time. Crude, but effective.
class HydroponicsSimpleRail : public HydroponicsRail
{
public:
    HydroponicsSimpleRail(Hydroponics_RailType railType,
                          Hydroponics_PositionIndex railIndex,
                          int maxActiveAtOnce = 2,
                          int classType = Simple);
    virtual ~HydroponicsSimpleRail();

    bool canActivate(shared_ptr<HydroponicsActuator> actuator) override;

    bool addActuator(HydroponicsActuator *actuator) override;
    bool removeActuator(HydroponicsActuator *actuator) override;

    int getActiveCount();

protected:
    int _activeCount;                                       // Current active count
    int _maxActiveCount;                                    // Max active count

    void handleActivation(HydroponicsActuator *actuator);
};

#endif // /ifndef HydroponicsRails_H
