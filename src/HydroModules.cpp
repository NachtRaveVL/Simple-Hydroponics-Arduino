/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Controller Modules
*/

#include "Hydruino.h"

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
            if (getScheduler()) {
                getScheduler()->setNeedsScheduling();
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
            if (getScheduler()) {
                getScheduler()->setNeedsScheduling();
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


bool HydroObjectRegistration::registerObject(SharedPtr<HydroObject> obj)
{
    HYDRO_SOFT_ASSERT(obj->getId().posIndex >= 0 && obj->getId().posIndex < HYDRO_POS_MAXSIZE, SFP(HStr_Err_InvalidParameter));
    if (obj && _objects.find(obj->getKey()) == _objects.end()) {
        _objects[obj->getKey()] = obj;

        if (obj->isActuatorType() || obj->isCropType() || obj->isReservoirType()) {
            if (getScheduler()) {
                getScheduler()->setNeedsScheduling();
            }
        }
        if (obj->isSensorType()) {
            if (getPublisher()) {
                getPublisher()->setNeedsTabulation();
            }
        }

        return true;
    }
    return false;
}

bool HydroObjectRegistration::unregisterObject(SharedPtr<HydroObject> obj)
{
    auto iter = _objects.find(obj->getKey());
    if (iter != _objects.end()) {
        _objects.erase(iter);

        if (obj->isActuatorType() || obj->isCropType() || obj->isReservoirType()) {
            if (getScheduler()) {
                getScheduler()->setNeedsScheduling();
            }
        }
        if (obj->isSensorType()) {
            if (getPublisher()) {
                getPublisher()->setNeedsTabulation();
            }
        }

        return true;
    }
    return false;
}

SharedPtr<HydroObject> HydroObjectRegistration::objectById(HydroIdentity id) const
{
    if (id.posIndex == HYDRO_POS_SEARCH_FROMBEG) {
        while (++id.posIndex < HYDRO_POS_MAXSIZE) {
            auto iter = _objects.find(id.regenKey());
            if (iter != _objects.end()) {
                if (id.keyString == iter->second->getKeyString()) {
                    return iter->second;
                } else {
                    objectById_Col(id);
                }
            }
        }
    } else if (id.posIndex == HYDRO_POS_SEARCH_FROMEND) {
        while (--id.posIndex >= 0) {
            auto iter = _objects.find(id.regenKey());
            if (iter != _objects.end()) {
                if (id.keyString == iter->second->getKeyString()) {
                    return iter->second;
                } else {
                    objectById_Col(id);
                }
            }
        }
    } else {
        auto iter = _objects.find(id.key);
        if (iter != _objects.end()) {
            if (id.keyString == iter->second->getKeyString()) {
                return iter->second;
            } else {
                objectById_Col(id);
            }
        }
    }

    return nullptr;
}

SharedPtr<HydroObject> HydroObjectRegistration::objectById_Col(const HydroIdentity &id) const
{
    HYDRO_SOFT_ASSERT(false, F("Hashing collision")); // exhaustive search must be performed, publishing may miss values

    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (id.keyString == iter->second->getKeyString()) {
            return iter->second;
        }
    }

    return nullptr;
}

hposi_t HydroObjectRegistration::firstPosition(HydroIdentity id, bool taken)
{
    if (id.posIndex != HYDRO_POS_SEARCH_FROMEND) {
        id.posIndex = HYDRO_POS_SEARCH_FROMBEG;
        while (++id.posIndex < HYDRO_POS_MAXSIZE) {
            auto iter = _objects.find(id.regenKey());
            if (taken == (iter != _objects.end())) {
                return id.posIndex;
            }
        }
    } else {
        id.posIndex = HYDRO_POS_SEARCH_FROMEND;
        while (--id.posIndex >= 0) {
            auto iter = _objects.find(id.regenKey());
            if (taken == (iter != _objects.end())) {
                return id.posIndex;
            }
        }
    }

    return -1;
}


bool HydroPinHandlers::tryGetPinLock(pintype_t pin, millis_t wait)
{
    millis_t start = millis();
    while (1) {
        auto iter = _pinLocks.find(pin);
        if (iter == _pinLocks.end()) {
            _pinLocks[pin] = true;
            return (_pinLocks.find(pin) != _pinLocks.end());
        }
        else if (millis() - start >= wait) { return false; }
        else { yield(); }
    }
}

void HydroPinHandlers::deactivatePinMuxers()
{
    for (auto iter = _pinMuxers.begin(); iter != _pinMuxers.end(); ++iter) {
        iter->second->deactivate();
    }
}

OneWire *HydroPinHandlers::getOneWireForPin(pintype_t pin)
{
    auto wireIter = _pinOneWire.find(pin);
    if (wireIter != _pinOneWire.end()) {
        return wireIter->second;
    } else {
        OneWire *oneWire = new OneWire(pin);
        if (oneWire) {
            _pinOneWire[pin] = oneWire;
            if (_pinOneWire.find(pin) != _pinOneWire.end()) { return oneWire; }
            else if (oneWire) { delete oneWire; }
        } else if (oneWire) { delete oneWire; }
    }
    return nullptr;
}

void HydroPinHandlers::dropOneWireForPin(pintype_t pin)
{
    auto wireIter = _pinOneWire.find(pin);
    if (wireIter != _pinOneWire.end()) {
        if (wireIter->second) {
            wireIter->second->depower();
            delete wireIter->second;
        }
        _pinOneWire.erase(wireIter);
    }
}
