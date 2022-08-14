/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Additives Market
*/

#ifndef HydroponicsAdditivesMarket_H
#define HydroponicsAdditivesMarket_H

class HydroponicsAdditivesMarket;

#include "Hydroponics.h"

// Hydroponics Additives Market
// The Additive Market stores custom user additive data, which is used to define feed
// nutrient dosing levels through the growing cycle.
class HydroponicsAdditivesMarket {
public:
    // Sets custom additive data, returning success flag.
    bool setCustomAdditiveData(const HydroponicsCustomAdditiveData *customAdditiveData);
    
    // Drops custom additive data, returning success flag.
    bool dropCustomAdditiveData(const HydroponicsCustomAdditiveData *customAdditiveData);

    // Returns custom additive data (if any), else nullptr.
    const HydroponicsCustomAdditiveData *getCustomAdditiveData(Hydroponics_ReservoirType reservoirType) const;

    // Returns if there are custom additives data stored
    inline bool hasCustomAdditives() const { return _additives.size(); }

protected:
    Map<Hydroponics_ReservoirType, HydroponicsCustomAdditiveData *, Hydroponics_ReservoirType_CustomAdditiveCount> _additives; // Loaded custom additives data

    friend class Hydroponics;
    friend class HydroponicsScheduler;
};

extern HydroponicsAdditivesMarket hydroAdditives;

#endif // /ifndef HydroponicsAdditivesMarket_H














