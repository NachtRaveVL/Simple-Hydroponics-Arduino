/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Additives
*/

#include "Hydruino.h"

bool HydroAdditives::setCustomAdditiveData(const HydroCustomAdditiveData *customAdditiveData)
{
    HYDRO_SOFT_ASSERT(customAdditiveData, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT(!customAdditiveData || (customAdditiveData->reservoirType >= Hydro_ReservoirType_CustomAdditive1 &&
                                                 customAdditiveData->reservoirType < Hydro_ReservoirType_CustomAdditive1 + Hydro_ReservoirType_CustomAdditiveCount), SFP(HStr_Err_InvalidParameter));

    if (customAdditiveData && customAdditiveData->reservoirType >= Hydro_ReservoirType_CustomAdditive1 &&
        customAdditiveData->reservoirType < Hydro_ReservoirType_CustomAdditive1 + Hydro_ReservoirType_CustomAdditiveCount) {
        auto iter = _additives.find(customAdditiveData->reservoirType);
        bool retVal = false;

        if (iter == _additives.end()) {
            auto additiveData = new HydroCustomAdditiveData();

            HYDRO_SOFT_ASSERT(additiveData, SFP(HStr_Err_AllocationFailure));
            if (additiveData) {
                *additiveData = *customAdditiveData;
                _additives[customAdditiveData->reservoirType] = additiveData;
                retVal = (_additives.find(customAdditiveData->reservoirType) != _additives.end());
            }
        } else {
            *(iter->second) = *customAdditiveData;
            retVal = true;
        }

        if (retVal) {
            if (getSchedulerInstance()) {
                getSchedulerInstance()->setNeedsScheduling();
            }
            return true;
        }
    }
    return false;
}

bool HydroAdditives::dropCustomAdditiveData(const HydroCustomAdditiveData *customAdditiveData)
{
    HYDRO_HARD_ASSERT(customAdditiveData, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT(!customAdditiveData || (customAdditiveData->reservoirType >= Hydro_ReservoirType_CustomAdditive1 &&
                                                 customAdditiveData->reservoirType < Hydro_ReservoirType_CustomAdditive1 + Hydro_ReservoirType_CustomAdditiveCount), SFP(HStr_Err_InvalidParameter));

    if (customAdditiveData->reservoirType >= Hydro_ReservoirType_CustomAdditive1 &&
        customAdditiveData->reservoirType < Hydro_ReservoirType_CustomAdditive1 + Hydro_ReservoirType_CustomAdditiveCount) {
        auto iter = _additives.find(customAdditiveData->reservoirType);
        bool retVal = false;

        if (iter != _additives.end()) {
            if (iter->second) { delete iter->second; }
            _additives.erase(iter);
            retVal = true;
        }

        if (retVal) {
            if (getSchedulerInstance()) {
                getSchedulerInstance()->setNeedsScheduling();
            }
            return true;
        }
    }
    return false;
}

const HydroCustomAdditiveData *HydroAdditives::getCustomAdditiveData(Hydro_ReservoirType reservoirType) const
{
    HYDRO_SOFT_ASSERT(reservoirType >= Hydro_ReservoirType_CustomAdditive1 &&
                         reservoirType < Hydro_ReservoirType_CustomAdditive1 + Hydro_ReservoirType_CustomAdditiveCount, SFP(HStr_Err_InvalidParameter));

    if (reservoirType >= Hydro_ReservoirType_CustomAdditive1 &&
        reservoirType < Hydro_ReservoirType_CustomAdditive1 + Hydro_ReservoirType_CustomAdditiveCount) {
        auto iter = _additives.find(reservoirType);

        if (iter != _additives.end()) {
            return iter->second;
        }
    }
    return nullptr;
}
