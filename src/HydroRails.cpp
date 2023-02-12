/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Power Rails 
*/

#include "Hydruino.h"

HydroRail *newRailObjectFromData(const HydroRailData *dataIn)
{
    if (dataIn && isValidType(dataIn->id.object.idType)) return nullptr;
    HYDRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
            case (int8_t)HydroRail::Simple:
                return new HydroSimpleRail((const HydroSimpleRailData *)dataIn);
            case (int8_t)HydroRail::Regulated:
                return new HydroRegulatedRail((const HydroRegulatedRailData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroRail::HydroRail(Hydro_RailType railType, hposi_t railIndex, int classTypeIn)
    : HydroObject(HydroIdentity(railType, railIndex)), classType((typeof(classType))classTypeIn),
      _powerUnits(defaultPowerUnits()), _limitState(Hydro_TriggerState_Undefined)
{
    allocateLinkages(HYDRO_RAILS_LINKS_BASESIZE);
}

HydroRail::HydroRail(const HydroRailData *dataIn)
    : HydroObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _limitState(Hydro_TriggerState_Undefined),
      _powerUnits(definedUnitsElse(dataIn->powerUnits, defaultPowerUnits()))
{
    allocateLinkages(HYDRO_RAILS_LINKS_BASESIZE);
}

HydroRail::~HydroRail()
{
    if (_links) {
        auto actuators = linksFilterActuators(getLinkages());
        for (auto iter = actuators.begin(); iter != actuators.end(); ++iter) { removeLinkage(*iter); }
    }
}

void HydroRail::update()
{
    HydroObject::update();

    handleLimit(triggerStateFromBool(getCapacity() >= 1.0f - FLT_EPSILON));
}

bool HydroRail::addLinkage(HydroObject *object)
{
    if (HydroObject::addLinkage(object)) {
        if (object->isActuatorType()) {
            HYDRO_HARD_ASSERT(isSimpleClass() || isRegulatedClass(), HStr_Err_OperationFailure);
            if (isSimpleClass()) {
                auto methodSlot = MethodSlot<HydroSimpleRail, HydroActuator *>((HydroSimpleRail *)this, &HydroSimpleRail::handleActivation);
                ((HydroActuator *)object)->getActivationSignal().attach(methodSlot);
            } else if (isRegulatedClass()) {
                auto methodSlot = MethodSlot<HydroRegulatedRail, HydroActuator *>((HydroRegulatedRail *)this, &HydroRegulatedRail::handleActivation);
                ((HydroActuator *)object)->getActivationSignal().attach(methodSlot);
            }
        }
        return true;
    }
    return false;
}

bool HydroRail::removeLinkage(HydroObject *object)
{
    if (HydroObject::removeLinkage(object)) {
        if (((HydroObject *)object)->isActuatorType()) {
            HYDRO_HARD_ASSERT(isSimpleClass() || isRegulatedClass(), HStr_Err_OperationFailure);
            if (isSimpleClass()) {
                auto methodSlot = MethodSlot<HydroSimpleRail, HydroActuator *>((HydroSimpleRail *)this, &HydroSimpleRail::handleActivation);
                ((HydroActuator *)object)->getActivationSignal().detach(methodSlot);
            } else if (isRegulatedClass()) {
                auto methodSlot = MethodSlot<HydroRegulatedRail, HydroActuator *>((HydroRegulatedRail *)this, &HydroRegulatedRail::handleActivation);
                ((HydroActuator *)object)->getActivationSignal().detach(methodSlot);
            }
        }
        return true;
    }
    return false;
}

void HydroRail::setPowerUnits(Hydro_UnitsType powerUnits)
{
    if (_powerUnits != powerUnits) {
        _powerUnits = powerUnits;
    }
}

Hydro_UnitsType HydroRail::getPowerUnits() const
{
    return definedUnitsElse(_powerUnits, defaultPowerUnits());
}

float HydroRail::getRailVoltage() const
{
    return getRailVoltageFromType(_id.objTypeAs.railType);
}

Signal<HydroRail *, HYDRO_RAIL_SIGNAL_SLOTS> &HydroRail::getCapacitySignal()
{
    return _capacitySignal;
}

HydroData *HydroRail::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void HydroRail::saveToData(HydroData *dataOut)
{
    HydroObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;

    ((HydroRailData *)dataOut)->powerUnits = _powerUnits;
}

void HydroRail::handleLimit(Hydro_TriggerState limitState)
{
    if (limitState == Hydro_TriggerState_Disabled || limitState == Hydro_TriggerState_Undefined) { return; }

    if (_limitState != limitState) {
        _limitState = limitState;

        if (_limitState == Hydro_TriggerState_NotTriggered) {
            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<HydroRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}


HydroSimpleRail::HydroSimpleRail(Hydro_RailType railType, hposi_t railIndex, int maxActiveAtOnce, int classType)
    : HydroRail(railType, railIndex, classType), _activeCount(0), _maxActiveAtOnce(maxActiveAtOnce)
{ ; }

HydroSimpleRail::HydroSimpleRail(const HydroSimpleRailData *dataIn)
    : HydroRail(dataIn), _activeCount(0), _maxActiveAtOnce(dataIn->maxActiveAtOnce)
{ ; }

bool HydroSimpleRail::canActivate(HydroActuator *actuator)
{
    return _activeCount < _maxActiveAtOnce;
}

float HydroSimpleRail::getCapacity()
{
    return _activeCount / (float)_maxActiveAtOnce;
}

void HydroSimpleRail::saveToData(HydroData *dataOut)
{
    HydroRail::saveToData(dataOut);

    ((HydroSimpleRailData *)dataOut)->maxActiveAtOnce = _maxActiveAtOnce;
}

void HydroSimpleRail::handleActivation(HydroActuator *actuator)
{
    bool activeCountBefore = _activeCount;

    if (actuator->isEnabled()) {
        _activeCount++;
    } else {
        _activeCount--;
    }

    if (_activeCount < activeCountBefore) {
        #ifdef HYDRO_USE_MULTITASKING
            scheduleSignalFireOnce<HydroRail *>(getSharedPtr(), _capacitySignal, this);
        #else
            _capacitySignal.fire(this);
        #endif
    }
}


HydroRegulatedRail::HydroRegulatedRail(Hydro_RailType railType, hposi_t railIndex, float maxPower, int classType)
    : HydroRail(railType, railIndex, classType), _maxPower(maxPower), _powerUsage(this), _limitTrigger(this)
{
    _powerUsage.setMeasureUnits(getPowerUnits(), getRailVoltage());
    _powerUsage.setHandleMethod(&HydroRegulatedRail::handlePower);

    _limitTrigger.setHandleMethod(&HydroRail::handleLimit);
}

HydroRegulatedRail::HydroRegulatedRail(const HydroRegulatedRailData *dataIn)
    : HydroRail(dataIn),
      _maxPower(dataIn->maxPower),
      _powerUsage(this), _limitTrigger(this)
{
    _powerUsage.setMeasureUnits(HydroRail::getPowerUnits(), getRailVoltage());
    _powerUsage.setHandleMethod(&HydroRegulatedRail::handlePower);
    _powerUsage.setObject(dataIn->powerSensor);

    _limitTrigger.setHandleMethod(&HydroRail::handleLimit);
    _limitTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->limitTrigger)));
    HYDRO_SOFT_ASSERT(_limitTrigger, SFP(HStr_Err_AllocationFailure));
}

void HydroRegulatedRail::update()
{
    HydroRail::update();

    _powerUsage.updateIfNeeded(true);

    _limitTrigger.updateIfNeeded();
}

void HydroRegulatedRail::handleLowMemory()
{
    HydroRail::handleLowMemory();

    if (_limitTrigger) { _limitTrigger->handleLowMemory(); }
}

bool HydroRegulatedRail::canActivate(HydroActuator *actuator)
{
    if (_limitTrigger.resolve() && triggerStateToBool(_limitTrigger.getTriggerState())) { return false; }

    HydroSingleMeasurement powerReq = actuator->getContinuousPowerUsage();
    convertUnits(&powerReq, getPowerUnits(), getRailVoltage());

    return _powerUsage.getMeasurementValue() + powerReq.value < _maxPower - FLT_EPSILON;
}

float HydroRegulatedRail::getCapacity()
{
    if (_limitTrigger.resolve() && triggerStateToBool(_limitTrigger.getTriggerState())) { return 1.0f; }
    float retVal = _powerUsage.getMeasurementValue() / _maxPower;
    return constrain(retVal, 0.0f, 1.0f);
}

void HydroRegulatedRail::setPowerUnits(Hydro_UnitsType powerUnits)
{
    if (_powerUnits != powerUnits) {
        _powerUnits = powerUnits;

        _powerUsage.setMeasureUnits(getPowerUnits(), getRailVoltage());
    }
}

HydroSensorAttachment &HydroRegulatedRail::getPowerUsage(bool poll)
{
    _powerUsage.updateIfNeeded(poll);
    return _powerUsage;
}

void HydroRegulatedRail::saveToData(HydroData *dataOut)
{
    HydroRail::saveToData(dataOut);

    ((HydroRegulatedRailData *)dataOut)->maxPower = roundForExport(_maxPower, 1);
    if (_powerUsage.getId()) {
        strncpy(((HydroRegulatedRailData *)dataOut)->powerSensor, _powerUsage.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_limitTrigger) {
        _limitTrigger->saveToData(&(((HydroRegulatedRailData *)dataOut)->limitTrigger));
    }
}

void HydroRegulatedRail::handleActivation(HydroActuator *actuator)
{
    if (!getPowerUsage() && actuator) {
        auto powerReq = actuator->getContinuousPowerUsage();
        auto powerUsage = getPowerUsage().getMeasurement(true);
        bool enabled = actuator->isEnabled();

        convertUnits(&powerReq, getPowerUnits(), getRailVoltage());

        if (enabled) {
            powerUsage.value += powerReq.value;
        } else {
            powerUsage.value -= powerReq.value;
        }

        getPowerUsage().setMeasurement(powerUsage);

        if (!enabled) {
            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<HydroRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}

void HydroRegulatedRail::handlePower(const HydroMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        float capacityBefore = getCapacity();

        getPowerUsage().setMeasurement(getAsSingleMeasurement(measurement, _powerUsage.getMeasureRow(), _maxPower, _powerUnits));

        if (getCapacity() < capacityBefore - FLT_EPSILON) {
            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<HydroRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}


HydroRailData::HydroRailData()
    : HydroObjectData(), powerUnits(Hydro_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void HydroRailData::toJSONObject(JsonObject &objectOut) const
{
    HydroObjectData::toJSONObject(objectOut);

    if (powerUnits != Hydro_UnitsType_Undefined) { objectOut[SFP(HStr_Key_PowerUnits)] = unitsTypeToSymbol(powerUnits); }
}

void HydroRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroObjectData::fromJSONObject(objectIn);

    powerUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_PowerUnits)]);
}

HydroSimpleRailData::HydroSimpleRailData()
    : HydroRailData(), maxActiveAtOnce(2)
{
    _size = sizeof(*this);
}

void HydroSimpleRailData::toJSONObject(JsonObject &objectOut) const
{
    HydroRailData::toJSONObject(objectOut);

    if (maxActiveAtOnce != 2) { objectOut[SFP(HStr_Key_MaxActiveAtOnce)] = maxActiveAtOnce; }
}

void HydroSimpleRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroRailData::fromJSONObject(objectIn);

    maxActiveAtOnce = objectIn[SFP(HStr_Key_MaxActiveAtOnce)] | maxActiveAtOnce;
}

HydroRegulatedRailData::HydroRegulatedRailData()
    : HydroRailData(), maxPower(0), powerSensor{0}, limitTrigger()
{
    _size = sizeof(*this);
}

void HydroRegulatedRailData::toJSONObject(JsonObject &objectOut) const
{
    HydroRailData::toJSONObject(objectOut);

    objectOut[SFP(HStr_Key_MaxPower)] = maxPower;
    if (powerSensor[0]) { objectOut[SFP(HStr_Key_PowerSensor)] = charsToString(powerSensor, HYDRO_NAME_MAXSIZE); }
    if (isValidType(limitTrigger.type)) {
        JsonObject limitTriggerObj = objectOut.createNestedObject(SFP(HStr_Key_LimitTrigger));
        limitTrigger.toJSONObject(limitTriggerObj);
    }
}

void HydroRegulatedRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroRailData::fromJSONObject(objectIn);

    maxPower = objectIn[SFP(HStr_Key_MaxPower)] | maxPower;
    const char *powerSensorStr = objectIn[SFP(HStr_Key_PowerSensor)];
    if (powerSensorStr && powerSensorStr[0]) { strncpy(powerSensor, powerSensorStr, HYDRO_NAME_MAXSIZE); }
    JsonObjectConst limitTriggerObj = objectIn[SFP(HStr_Key_LimitTrigger)];
    if (!limitTriggerObj.isNull()) { limitTrigger.fromJSONObject(limitTriggerObj); }
}
