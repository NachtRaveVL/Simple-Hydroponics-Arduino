/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Rails
*/

#ifndef HydroponicsRails_H
#define HydroponicsRails_H

class HydroponicsRail;
class HydroponicsRelayRail;

#include "Hydroponics.h"

// TODO
class HydroponicsRail : public HydroponicsObject {
public:
    HydroponicsRail(Hydroponics_RailType railType,
                    Hydroponics_PositionIndex railIndex);
    virtual ~HydroponicsRail();

    Hydroponics_RailType getRailType() const;
    Hydroponics_PositionIndex getRailIndex() const;

protected:
};

class HydroponicsRelayRail : public HydroponicsRail
{
public:
protected:
};

#endif // /ifndef HydroponicsRails_H
