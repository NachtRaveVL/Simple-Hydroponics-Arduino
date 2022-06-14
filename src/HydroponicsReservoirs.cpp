/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensorsare 
*/

#include "Hydroponics.h"

HydroponicsReservoir::HydroponicsReservoir(Hydroponics_ReservoirType reservoirType, Hydroponics_PositionIndex reservoirIndex)
    : HydroponicsObject(HydroponicsIdentity(reservoirType, reservoirIndex))
{ ; }

HydroponicsReservoir::~HydroponicsReservoir()
{ ; }

Hydroponics_ReservoirType HydroponicsReservoir::getReservoirType() const
{
    return _id.as.reservoirType;
}

Hydroponics_PositionIndex HydroponicsReservoir::getReservoirIndex() const
{
    return _id.posIndex;
}
