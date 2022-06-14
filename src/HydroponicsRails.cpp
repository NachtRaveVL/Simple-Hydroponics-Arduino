/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensorsare 
*/

#include "Hydroponics.h"

HydroponicsRail::HydroponicsRail(Hydroponics_RailType railType, Hydroponics_PositionIndex railIndex)
    : HydroponicsObject(HydroponicsIdentity(railType, railIndex))
{ ; }

HydroponicsRail::~HydroponicsRail()
{ ; }

Hydroponics_RailType HydroponicsRail::getRailType() const
{
    return _id.as.railType;
}

Hydroponics_PositionIndex HydroponicsRail::getRailIndex() const
{
    return _id.posIndex;
}
