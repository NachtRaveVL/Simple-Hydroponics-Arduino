/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydroponics Additives Market
*/

#include "Hydroponics.h"

HydroponicsAdditivesMarket hydroAdditives;

bool HydroponicsAdditivesMarket::setCustomAdditiveData(const HydroponicsCustomAdditiveData *customAdditiveData)
{
    HYDRUINO_SOFT_ASSERT(customAdditiveData, SFP(HStr_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(!customAdditiveData || (customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
                                                 customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount), SFP(HStr_Err_InvalidParameter));

    if (customAdditiveData && customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
        customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
        auto iter = _additives.find(customAdditiveData->reservoirType);
        bool retVal = false;

        if (iter == _additives.end()) {
            auto additiveData = new HydroponicsCustomAdditiveData();

            HYDRUINO_SOFT_ASSERT(additiveData, SFP(HStr_Err_AllocationFailure));
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

bool HydroponicsAdditivesMarket::dropCustomAdditiveData(const HydroponicsCustomAdditiveData *customAdditiveData)
{
    HYDRUINO_HARD_ASSERT(customAdditiveData, SFP(HStr_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(!customAdditiveData || (customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
                                                 customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount), SFP(HStr_Err_InvalidParameter));

    if (customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
        customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
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

const HydroponicsCustomAdditiveData *HydroponicsAdditivesMarket::getCustomAdditiveData(Hydroponics_ReservoirType reservoirType) const
{
    HYDRUINO_SOFT_ASSERT(reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
                         reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount, SFP(HStr_Err_InvalidParameter));

    if (reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
        reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
        auto iter = _additives.find(reservoirType);

        if (iter != _additives.end()) {
            return iter->second;
        }
    }
    return nullptr;
}
