/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Additives Market
*/

#ifndef HydroAdditivesMarket_H
#define HydroAdditivesMarket_H

class HydroAdditivesMarket;

#include "Hydruino.h"

// Additives
// The Additive Market stores custom user additive data, which is used to define feed
// nutrient dosing levels through the growing cycle.
class HydroAdditivesMarket {
public:
    // Sets custom additive data, returning success flag.
    bool setCustomAdditiveData(const HydroCustomAdditiveData *customAdditiveData);
    
    // Drops custom additive data, returning success flag.
    bool dropCustomAdditiveData(const HydroCustomAdditiveData *customAdditiveData);

    // Returns custom additive data (if any), else nullptr.
    const HydroCustomAdditiveData *getCustomAdditiveData(Hydro_ReservoirType reservoirType) const;

    // Returns if there are custom additives data stored
    inline bool hasCustomAdditives() const { return _additives.size(); }

protected:
    Map<Hydro_ReservoirType, HydroCustomAdditiveData *, Hydro_ReservoirType_CustomAdditiveCount> _additives; // Loaded custom additives data

    friend class Hydruino;
    friend class HydroScheduler;
};

extern HydroAdditivesMarket hydroAdditives;

#endif // /ifndef HydroAdditivesMarket_H














