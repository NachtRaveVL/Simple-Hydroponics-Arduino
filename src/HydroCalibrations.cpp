/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Calibrations
*/

#include "Hydruino.h"

HydroCalibrations hydroCalibrations;

const HydroCalibrationData *HydroCalibrations::getUserCalibrationData(hkey_t key) const
{
    auto iter = _calibrationData.find(key);
    if (iter != _calibrationData.end()) {
        return iter->second;
    }
    return nullptr;
}

bool HydroCalibrations::setUserCalibrationData(const HydroCalibrationData *calibrationData)
{
    HYDRO_SOFT_ASSERT(calibrationData, SFP(HStr_Err_InvalidParameter));

    if (calibrationData) {
        hkey_t key = stringHash(calibrationData->ownerName);
        auto iter = _calibrationData.find(key);
        bool retVal = false;

        if (iter == _calibrationData.end()) {
            auto calibData = new HydroCalibrationData();

            HYDRO_SOFT_ASSERT(calibData, SFP(HStr_Err_AllocationFailure));
            if (calibData) {
                *calibData = *calibrationData;
                _calibrationData[key] = calibData;
                retVal = (_calibrationData.find(key) != _calibrationData.end());
            }
        } else {
            *(iter->second) = *calibrationData;
            retVal = true;
        }

        return retVal;
    }
    return false;
}

bool HydroCalibrations::dropUserCalibrationData(const HydroCalibrationData *calibrationData)
{
    HYDRO_HARD_ASSERT(calibrationData, SFP(HStr_Err_InvalidParameter));
    hkey_t key = stringHash(calibrationData->ownerName);
    auto iter = _calibrationData.find(key);

    if (iter != _calibrationData.end()) {
        if (iter->second) { delete iter->second; }
        _calibrationData.erase(iter);

        return true;
    }

    return false;
}
