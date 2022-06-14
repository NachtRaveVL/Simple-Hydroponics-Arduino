/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Reservoirs
*/

#ifndef HydroponicsReservoirs_H
#define HydroponicsReservoirs_H

class HydroponicsReservoir;
class HydroponicsWaterReservoir;
class HydroponicsFluidSolution;
class HydroponicsDrainagePipe;

#include "Hydroponics.h"

// TODO
class HydroponicsReservoir : public HydroponicsObject {
public:
    HydroponicsReservoir(Hydroponics_ReservoirType reservoirType,
                         Hydroponics_PositionIndex reservoirIndex);
    virtual ~HydroponicsReservoir();

    Hydroponics_ReservoirType getReservoirType() const;
    Hydroponics_PositionIndex getReservoirIndex() const;

protected:
};

class HydroponicsWaterReservoir : public HydroponicsReservoir {
public:
protected:
};

class HydroponicsFluidSolution : public HydroponicsReservoir {
public:
protected:
};

class HydroponicsDrainagePipe : public HydroponicsReservoir {
public:
protected:
};

#endif // /ifndef HydroponicsReservoirs_H
