/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Power Rails 
*/

#include "Hydroponics.h"

HydroponicsRail *newRailObjectFromData(const HydroponicsRailData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectectData(), SFP(HStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectectData()) {
        switch (dataIn->id.object.classType) {
            case 0: // Simple
                return new HydroponicsSimpleRail((const HydroponicsSimpleRailData *)dataIn);
            case 1: // Regulated
                return new HydroponicsRegulatedRail((const HydroponicsRegulatedRailData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroponicsRail::HydroponicsRail(Hydroponics_RailType railType, Hydroponics_PositionIndex railIndex, int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(railType, railIndex)), classType((typeof(classType))classTypeIn),
      _powerUnits(Hydroponics_UnitsType_Power_Wattage), _limitState(Hydroponics_TriggerState_Undefined)
{
    _links = new Map<Hydroponics_KeyType, Pair<HydroponicsObject *, int8_t>::type, HYDRUINO_OBJ_LINKS_MAXSIZE>::type();
}

HydroponicsRail::HydroponicsRail(const HydroponicsRailData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _limitState(Hydroponics_TriggerState_Undefined),
      _powerUnits(definedUnitsElse(dataIn->powerUnits, Hydroponics_UnitsType_Power_Wattage))
{
    _links = new Map<Hydroponics_KeyType, Pair<HydroponicsObject *, int8_t>::type, HYDRUINO_OBJ_LINKS_MAXSIZE>::type();
}

HydroponicsRail::~HydroponicsRail()
{
    if (_links) {
        auto actuators = linksFilterActuators<HYDRUINO_OBJ_LINKS_MAXSIZE>(*_links);
        for (auto iter = actuators.begin(); iter != actuators.end(); ++iter) { removeLinkage(*iter); }
    }
}

void HydroponicsRail::update()
{
    HydroponicsObject::update();

    handleLimit(triggerStateFromBool(getCapacity() >= 1.0f - FLT_EPSILON));
}

bool HydroponicsRail::addLinkage(HydroponicsObject *object)
{
    if (HydroponicsObject::addLinkage(object)) {
        if (object->isActuatorType()) {
            HYDRUINO_HARD_ASSERT(isSimpleClass() || isRegulatedClass(), HStr_Err_OperationFailure);
            if (isSimpleClass()) {
                auto methodSlot = MethodSlot<HydroponicsSimpleRail, HydroponicsActuator *>((HydroponicsSimpleRail *)this, &HydroponicsSimpleRail::handleActivation);
                ((HydroponicsActuator *)object)->getActivationSignal().attach(methodSlot);
            } else if (isRegulatedClass()) {
                auto methodSlot = MethodSlot<HydroponicsRegulatedRail, HydroponicsActuator *>((HydroponicsRegulatedRail *)this, &HydroponicsRegulatedRail::handleActivation);
                ((HydroponicsActuator *)object)->getActivationSignal().attach(methodSlot);
            }
        }
        return true;
    }
    return false;
}

bool HydroponicsRail::removeLinkage(HydroponicsObject *object)
{
    if (HydroponicsObject::removeLinkage(object)) {
        if (((HydroponicsObject *)object)->isActuatorType()) {
            HYDRUINO_HARD_ASSERT(isSimpleClass() || isRegulatedClass(), HStr_Err_OperationFailure);
            if (isSimpleClass()) {
                auto methodSlot = MethodSlot<HydroponicsSimpleRail, HydroponicsActuator *>((HydroponicsSimpleRail *)this, &HydroponicsSimpleRail::handleActivation);
                ((HydroponicsActuator *)object)->getActivationSignal().detach(methodSlot);
            } else if (isRegulatedClass()) {
                auto methodSlot = MethodSlot<HydroponicsRegulatedRail, HydroponicsActuator *>((HydroponicsRegulatedRail *)this, &HydroponicsRegulatedRail::handleActivation);
                ((HydroponicsActuator *)object)->getActivationSignal().detach(methodSlot);
            }
        }
        return true;
    }
    return false;
}

void HydroponicsRail::setPowerUnits(Hydroponics_UnitsType powerUnits)
{
    if (_powerUnits != powerUnits) {
        _powerUnits = powerUnits;
    }
}

Hydroponics_UnitsType HydroponicsRail::getPowerUnits() const
{
    return definedUnitsElse(_powerUnits, Hydroponics_UnitsType_Power_Wattage);
}

float HydroponicsRail::getRailVoltage() const
{
    return getRailVoltageFromType(_id.objTypeAs.railType);
}

Signal<HydroponicsRail *> &HydroponicsRail::getCapacitySignal()
{
    return _capacitySignal;
}

HydroponicsData *HydroponicsRail::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void HydroponicsRail::saveToData(HydroponicsData *dataOut)
{
    HydroponicsObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;

    ((HydroponicsRailData *)dataOut)->powerUnits = _powerUnits;
}

void HydroponicsRail::handleLimit(Hydroponics_TriggerState limitState)
{
    if (limitState == Hydroponics_TriggerState_Disabled || limitState == Hydroponics_TriggerState_Undefined) { return; }

    if (_limitState != limitState) {
        _limitState = limitState;

        if (_limitState == Hydroponics_TriggerState_NotTriggered) {
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<HydroponicsRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}


HydroponicsSimpleRail::HydroponicsSimpleRail(Hydroponics_RailType railType,
                                             Hydroponics_PositionIndex railIndex,
                                             int maxActiveAtOnce,
                                             int classType)
    : HydroponicsRail(railType, railIndex, classType), _activeCount(0), _maxActiveAtOnce(maxActiveAtOnce)
{ ; }

HydroponicsSimpleRail::HydroponicsSimpleRail(const HydroponicsSimpleRailData *dataIn)
    : HydroponicsRail(dataIn), _activeCount(0), _maxActiveAtOnce(dataIn->maxActiveAtOnce)
{ ; }

bool HydroponicsSimpleRail::canActivate(HydroponicsActuator *actuator)
{
    return _activeCount < _maxActiveAtOnce;
}

float HydroponicsSimpleRail::getCapacity()
{
    return _activeCount / (float)_maxActiveAtOnce;
}

void HydroponicsSimpleRail::saveToData(HydroponicsData *dataOut)
{
    HydroponicsRail::saveToData(dataOut);

    ((HydroponicsSimpleRailData *)dataOut)->maxActiveAtOnce = _maxActiveAtOnce;
}

void HydroponicsSimpleRail::handleActivation(HydroponicsActuator *actuator)
{
    bool activeCountBefore = _activeCount;

    if (actuator->isEnabled()) {
        _activeCount++;
    } else {
        _activeCount--;
    }

    if (_activeCount < activeCountBefore) {
        #ifndef HYDRUINO_DISABLE_MULTITASKING
            scheduleSignalFireOnce<HydroponicsRail *>(getSharedPtr(), _capacitySignal, this);
        #else
            _capacitySignal.fire(this);
        #endif
    }
}


HydroponicsRegulatedRail::HydroponicsRegulatedRail(Hydroponics_RailType railType,
                                                   Hydroponics_PositionIndex railIndex,
                                                   float maxPower,
                                                   int classType)
    : HydroponicsRail(railType, railIndex, classType), _maxPower(maxPower), _powerUsage(this), _limitTrigger(this)
{
    _powerUsage.setMeasurementUnits(getPowerUnits(), getRailVoltage());
    _powerUsage.setHandleMethod(&HydroponicsRegulatedRail::handlePower);

    _limitTrigger.setHandleMethod(&HydroponicsRail::handleLimit);
}

HydroponicsRegulatedRail::HydroponicsRegulatedRail(const HydroponicsRegulatedRailData *dataIn)
    : HydroponicsRail(dataIn),
      _maxPower(dataIn->maxPower),
      _powerUsage(this), _limitTrigger(this)
{
    _powerUsage.setMeasurementUnits(HydroponicsRail::getPowerUnits(), getRailVoltage());
    _powerUsage.setHandleMethod(&HydroponicsRegulatedRail::handlePower);
    _powerUsage.setObject(dataIn->powerSensor);

    _limitTrigger.setHandleMethod(&HydroponicsRail::handleLimit);
    _limitTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->limitTrigger)));
    HYDRUINO_SOFT_ASSERT(_limitTrigger, SFP(HStr_Err_AllocationFailure));
}

void HydroponicsRegulatedRail::update()
{
    HydroponicsRail::update();

    _powerUsage.updateIfNeeded(true);

    _limitTrigger.updateIfNeeded();
}

void HydroponicsRegulatedRail::handleLowMemory()
{
    HydroponicsRail::handleLowMemory();

    if (_limitTrigger) { _limitTrigger->handleLowMemory(); }
}

bool HydroponicsRegulatedRail::canActivate(HydroponicsActuator *actuator)
{
    if (_limitTrigger.resolve() && triggerStateToBool(_limitTrigger.getTriggerState())) { return false; }

    HydroponicsSingleMeasurement powerReq = actuator->getContinuousPowerUsage();
    convertUnits(&powerReq, getPowerUnits(), getRailVoltage());

    return _powerUsage.getMeasurementValue() + powerReq.value < _maxPower - FLT_EPSILON;
}

float HydroponicsRegulatedRail::getCapacity()
{
    if (_limitTrigger.resolve() && triggerStateToBool(_limitTrigger.getTriggerState())) { return 1.0f; }
    float retVal = _powerUsage.getMeasurementValue() / _maxPower;
    return constrain(retVal, 0.0f, 1.0f);
}

void HydroponicsRegulatedRail::setPowerUnits(Hydroponics_UnitsType powerUnits)
{
    if (_powerUnits != powerUnits) {
        _powerUnits = powerUnits;

        _powerUsage.setMeasurementUnits(getPowerUnits(), getRailVoltage());
    }
}

HydroponicsSensorAttachment &HydroponicsRegulatedRail::getPowerUsage(bool poll)
{
    _powerUsage.updateIfNeeded(poll);
    return _powerUsage;
}

void HydroponicsRegulatedRail::saveToData(HydroponicsData *dataOut)
{
    HydroponicsRail::saveToData(dataOut);

    ((HydroponicsRegulatedRailData *)dataOut)->maxPower = roundForExport(_maxPower, 1);
    if (_powerUsage.getId()) {
        strncpy(((HydroponicsRegulatedRailData *)dataOut)->powerSensor, _powerUsage.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_limitTrigger) {
        _limitTrigger->saveToData(&(((HydroponicsRegulatedRailData *)dataOut)->limitTrigger));
    }
}

void HydroponicsRegulatedRail::handleActivation(HydroponicsActuator *actuator)
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
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<HydroponicsRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}

void HydroponicsRegulatedRail::handlePower(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        float capacityBefore = getCapacity();

        getPowerUsage().setMeasurement(getAsSingleMeasurement(measurement, _powerUsage.getMeasurementRow(), _maxPower, _powerUnits));

        if (getCapacity() < capacityBefore - FLT_EPSILON) {
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<HydroponicsRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}


HydroponicsRailData::HydroponicsRailData()
    : HydroponicsObjectData(), powerUnits(Hydroponics_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void HydroponicsRailData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsObjectData::toJSONObject(objectOut);

    if (powerUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HStr_Key_PowerUnits)] = unitsTypeToSymbol(powerUnits); }
}

void HydroponicsRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);

    powerUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_PowerUnits)]);
}

HydroponicsSimpleRailData::HydroponicsSimpleRailData()
    : HydroponicsRailData(), maxActiveAtOnce(2)
{
    _size = sizeof(*this);
}

void HydroponicsSimpleRailData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsRailData::toJSONObject(objectOut);

    if (maxActiveAtOnce != 2) { objectOut[SFP(HStr_Key_MaxActiveAtOnce)] = maxActiveAtOnce; }
}

void HydroponicsSimpleRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsRailData::fromJSONObject(objectIn);

    maxActiveAtOnce = objectIn[SFP(HStr_Key_MaxActiveAtOnce)] | maxActiveAtOnce;
}

HydroponicsRegulatedRailData::HydroponicsRegulatedRailData()
    : HydroponicsRailData(), maxPower(0), powerSensor{0}, limitTrigger()
{
    _size = sizeof(*this);
}

void HydroponicsRegulatedRailData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsRailData::toJSONObject(objectOut);

    objectOut[SFP(HStr_Key_MaxPower)] = maxPower;
    if (powerSensor[0]) { objectOut[SFP(HStr_Key_PowerSensor)] = charsToString(powerSensor, HYDRUINO_NAME_MAXSIZE); }
    if (limitTrigger.type != -1) {
        JsonObject limitTriggerObj = objectOut.createNestedObject(SFP(HStr_Key_LimitTrigger));
        limitTrigger.toJSONObject(limitTriggerObj);
    }
}

void HydroponicsRegulatedRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsRailData::fromJSONObject(objectIn);

    maxPower = objectIn[SFP(HStr_Key_MaxPower)] | maxPower;
    const char *powerSensorStr = objectIn[SFP(HStr_Key_PowerSensor)];
    if (powerSensorStr && powerSensorStr[0]) { strncpy(powerSensor, powerSensorStr, HYDRUINO_NAME_MAXSIZE); }
    JsonObjectConst limitTriggerObj = objectIn[SFP(HStr_Key_LimitTrigger)];
    if (!limitTriggerObj.isNull()) { limitTrigger.fromJSONObject(limitTriggerObj); }
}
