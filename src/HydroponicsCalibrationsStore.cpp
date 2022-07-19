/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Calibrations Store
*/

#include "Hydroponics.h"

HydroponicsCalibrationsStore *HydroponicsCalibrationsStore::_instance = nullptr;

HydroponicsCalibrationsStore *HydroponicsCalibrationsStore::getInstance()
{
    if (_instance) { return _instance; }
    else {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            if (!_instance) {
                _instance = new HydroponicsCalibrationsStore();
            }
        }
        return _instance;
    }
}

const HydroponicsCalibrationData *HydroponicsCalibrationsStore::getUserCalibrationData(Hydroponics_KeyType key) const
{
    auto iter = _calibrationData.find(key);
    if (iter != _calibrationData.end()) {
        return iter->second;
    }
    return nullptr;
}

bool HydroponicsCalibrationsStore::setUserCalibrationData(const HydroponicsCalibrationData *calibrationData)
{
    HYDRUINO_SOFT_ASSERT(calibrationData, SFP(HS_Err_InvalidParameter));

    if (calibrationData) {
        Hydroponics_KeyType key = stringHash(calibrationData->sensorName);
        auto iter = _calibrationData.find(key);
        bool retVal = false;

        if (iter == _calibrationData.end()) {
            auto calibData = new HydroponicsCalibrationData();

            HYDRUINO_SOFT_ASSERT(calibData, SFP(HS_Err_AllocationFailure));
            if (calibData) {
                *calibData = *calibrationData;
                retVal = _calibrationData.insert(key, calibData).second;
            }
        } else {
            *(iter->second) = *calibrationData;
            retVal = true;
        }

        if (retVal) {
            scheduleSignalFireOnce<Hydroponics_KeyType>(_calibrationSignal, key);
            return true;
        }
    }
    return false;
}

bool HydroponicsCalibrationsStore::dropUserCalibrationData(const HydroponicsCalibrationData *calibrationData)
{
    HYDRUINO_SOFT_ASSERT(calibrationData, SFP(HS_Err_InvalidParameter));

    if (calibrationData) {
        Hydroponics_KeyType key = stringHash(calibrationData->sensorName);
        auto iter = _calibrationData.find(key);

        if (iter != _calibrationData.end()) {
            if (iter->second) { delete iter->second; }
            _calibrationData.erase(iter);

            scheduleSignalFireOnce<Hydroponics_KeyType>(_calibrationSignal, key);
            return true;
        }
    }
    return false;
}

bool HydroponicsCalibrationsStore::hasUserCalibrations() const
{
    return _calibrationData.size();
}

Signal<Hydroponics_KeyType> &HydroponicsCalibrationsStore::getUserCalibrationSignal()
{
    return _calibrationSignal;
}
