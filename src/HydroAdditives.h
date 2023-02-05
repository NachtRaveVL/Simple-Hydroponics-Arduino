/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Additives
*/

#ifndef HydroAdditives_H
#define HydroAdditives_H

class HydroAdditives;

#include "Hydruino.h"

// Additives Storage
// Stores custom user additive data, which is used to define feed nutrient dosing levels
// through the growing cycle.
class HydroAdditives {
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
};

#endif // /ifndef HydroAdditives_H














