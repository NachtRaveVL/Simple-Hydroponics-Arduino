/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Power Rails 
*/

#include "Hydroponics.h"

HydroponicsRail *newRailObjectFromData(const HydroponicsRailData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HS_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
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
      _powerUnits(Hydroponics_UnitsType_Power_Wattage)
{ ; }

HydroponicsRail::HydroponicsRail(const HydroponicsRailData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _powerUnits(definedUnitsElse(dataIn->powerUnits, Hydroponics_UnitsType_Power_Wattage))
{ ; }

HydroponicsRail::~HydroponicsRail()
{ ; }

void HydroponicsRail::update()
{
    HydroponicsObject::update();

    auto limitState = triggerStateFromBool(getCapacity() >= 1.0f - FLT_EPSILON);
    if (_limitState != limitState) {
        _limitState = limitState;
        handleLimitState();
    }
}

void HydroponicsRail::resolveLinks()
{
    HydroponicsObject::resolveLinks();
}

void HydroponicsRail::handleLowMemory()
{
    HydroponicsObject::handleLowMemory();
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

bool HydroponicsRail::addActuator(HydroponicsActuator *actuator)
{
    return addLinkage(actuator);
}

bool HydroponicsRail::removeActuator(HydroponicsActuator *actuator)
{
    return removeLinkage(actuator);
}

bool HydroponicsRail::hasActuator(HydroponicsActuator *actuator) const
{
    return hasLinkage(actuator);
}

bool HydroponicsRail::addSensor(HydroponicsSensor *sensor)
{
    return addLinkage(sensor);
}

bool HydroponicsRail::removeSensor(HydroponicsSensor *sensor)
{
    return removeLinkage(sensor);
}

bool HydroponicsRail::hasSensor(HydroponicsSensor *sensor) const
{
    return hasLinkage(sensor);
}

Hydroponics_RailType HydroponicsRail::getRailType() const
{
    return _id.objTypeAs.railType;
}

Hydroponics_PositionIndex HydroponicsRail::getRailIndex() const
{
    return _id.posIndex;
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

void HydroponicsRail::handleLimitState()
{
    if (_limitState == Hydroponics_TriggerState_NotTriggered) {
        #ifndef HYDRUINO_DISABLE_MULTITASKING
            scheduleSignalFireOnce<HydroponicsRail *>(getSharedPtr(), _capacitySignal, this);
        #else
            _capacitySignal.fire(this);
        #endif
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

HydroponicsSimpleRail::~HydroponicsSimpleRail()
{
    {   auto actuators = linksFilterActuators(_links);
        for (auto iter = actuators.begin(); iter != actuators.end(); ++iter) { removeActuator((HydroponicsActuator *)(*iter)); }
    }
}

bool HydroponicsSimpleRail::canActivate(HydroponicsActuator *actuator)
{
    return _activeCount < _maxActiveAtOnce;
}

float HydroponicsSimpleRail::getCapacity() const
{
    return _activeCount / (float)_maxActiveAtOnce;
}

bool HydroponicsSimpleRail::addActuator(HydroponicsActuator *actuator)
{
    if (HydroponicsRail::addActuator(actuator)) {
        auto methodSlot = MethodSlot<typeof(*this), HydroponicsActuator *>(this, &HydroponicsSimpleRail::handleActivation);
        actuator->getActivationSignal().attach(methodSlot);
        return true;
    }
    return false;
}

bool HydroponicsSimpleRail::removeActuator(HydroponicsActuator *actuator)
{
    if (HydroponicsRail::removeActuator(actuator)) {
        auto methodSlot = MethodSlot<typeof(*this), HydroponicsActuator *>(this, &HydroponicsSimpleRail::handleActivation);
        actuator->getActivationSignal().detach(methodSlot);
        return true;
    }
    return false;
}

int HydroponicsSimpleRail::getActiveCount()
{
    return _activeCount;
}

void HydroponicsSimpleRail::saveToData(HydroponicsData *dataOut)
{
    HydroponicsRail::saveToData(dataOut);

    ((HydroponicsSimpleRailData *)dataOut)->maxActiveAtOnce = _maxActiveAtOnce;
}

void HydroponicsSimpleRail::handleActivation(HydroponicsActuator *actuator)
{
    bool activeCountBefore = _activeCount;

    if (actuator->getIsEnabled()) {
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
                             int classType = Regulated)
    : HydroponicsRail(railType, railIndex, classType), _maxPower(maxPower), _needsPowerUpdate(true)
{ ; }

HydroponicsRegulatedRail::HydroponicsRegulatedRail(const HydroponicsRegulatedRailData *dataIn)
    : HydroponicsRail(dataIn), _needsPowerUpdate(true),
      _maxPower(dataIn->maxPower),
      _powerSensor(dataIn->powerSensor), _limitTrigger(newTriggerObjectFromSubData(&(dataIn->limitTrigger)))
{
    if (_limitTrigger) { attachLimitTrigger(); }
}

HydroponicsRegulatedRail::~HydroponicsRegulatedRail()
{
    {   auto actuators = linksFilterActuators(_links);
        for (auto iter = actuators.begin(); iter != actuators.end(); ++iter) { removeActuator((HydroponicsActuator *)(*iter)); }
    }
    if (_powerSensor) { detachPowerSensor(); }
    if (_limitTrigger) { detachLimitTrigger(); delete _limitTrigger; _limitTrigger = nullptr; }
}

void HydroponicsRegulatedRail::update()
{
    HydroponicsRail::update();

    if (_limitTrigger) { _limitTrigger->update(); }

    if (_needsPowerUpdate && getPowerSensor()) {
        handlePowerMeasure(_powerSensor->getLatestMeasurement());
    }
}

void HydroponicsRegulatedRail::resolveLinks()
{
    HydroponicsRail::resolveLinks();

    if (_powerSensor.needsResolved()) { getPowerSensor(); }
    if (_limitTrigger) { _limitTrigger->resolveLinks(); }
}

void HydroponicsRegulatedRail::handleLowMemory()
{
    HydroponicsRail::handleLowMemory();

    if (_limitTrigger) { _limitTrigger->handleLowMemory(); }
}

bool HydroponicsRegulatedRail::canActivate(HydroponicsActuator *actuator)
{
    if (_limitTrigger && _limitTrigger->getTriggerState() == Hydroponics_TriggerState_Triggered) { return false; }

    HydroponicsSingleMeasurement powerReq = actuator->getContinuousPowerDraw();
    convertUnits(&powerReq, getPowerUnits(), getRailVoltage());

    return _powerDraw.value + powerReq.value < _maxPower - FLT_EPSILON;
}

float HydroponicsRegulatedRail::getCapacity() const
{
    if (_limitTrigger && _limitTrigger->getTriggerState() == Hydroponics_TriggerState_Triggered) { return 1.0f; }
    return constrain(_powerDraw.value / _maxPower, 0.0f, 1.0f);
}

void HydroponicsRegulatedRail::setPowerUnits(Hydroponics_UnitsType powerUnits)
{
    if (_powerUnits != powerUnits) {
        _powerUnits = powerUnits;

        convertUnits(&_powerDraw, getPowerUnits(), getRailVoltage());
    }
}

bool HydroponicsRegulatedRail::addActuator(HydroponicsActuator *actuator)
{
    if (HydroponicsRail::addActuator(actuator)) {
        auto methodSlot = MethodSlot<typeof(*this), HydroponicsActuator *>(this, &HydroponicsRegulatedRail::handleActivation);
        actuator->getActivationSignal().attach(methodSlot);
        return true;
    }
    return false;
}

bool HydroponicsRegulatedRail::removeActuator(HydroponicsActuator *actuator)
{
    if (HydroponicsRail::removeActuator(actuator)) {
        auto methodSlot = MethodSlot<typeof(*this), HydroponicsActuator *>(this, &HydroponicsRegulatedRail::handleActivation);
        actuator->getActivationSignal().detach(methodSlot);
        return true;
    }
    return false;
}

void HydroponicsRegulatedRail::setPowerSensor(HydroponicsIdentity powerSensorId)
{
    if (_powerSensor != powerSensorId) {
        if (_powerSensor) { detachPowerSensor(); }
        _powerSensor = powerSensorId;
        _needsPowerUpdate = true;
    }
}

void HydroponicsRegulatedRail::setPowerSensor(shared_ptr<HydroponicsSensor> powerSensor)
{
    if (_powerSensor != powerSensor) {
        if (_powerSensor) { detachPowerSensor(); }
        _powerSensor = powerSensor;
        if (_powerSensor) { attachPowerSensor(); }
        _needsPowerUpdate = true;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsRegulatedRail::getPowerSensor()
{
    if (_powerSensor.resolveIfNeeded()) { attachPowerSensor(); }
    return _powerSensor.getObj();
}

void HydroponicsRegulatedRail::setPowerDraw(float powerDraw, Hydroponics_UnitsType powerDrawUnits)
{
    _powerDraw.value = powerDraw;
    _powerDraw.units = powerDrawUnits;
    _powerDraw.updateTimestamp();
    _powerDraw.updateFrame(1);

    convertUnits(&_powerDraw, getPowerUnits(), getRailVoltage());
    _needsPowerUpdate = false;
}

void HydroponicsRegulatedRail::setPowerDraw(HydroponicsSingleMeasurement powerDraw)
{
    _powerDraw = powerDraw;
    _powerDraw.setMinFrame(1);

    convertUnits(&_powerDraw, getPowerUnits(), getRailVoltage());
    _needsPowerUpdate = false;
}

const HydroponicsSingleMeasurement &HydroponicsRegulatedRail::getPowerDraw()
{
    if (_needsPowerUpdate && getPowerSensor()) {
        handlePowerMeasure(_powerSensor->getLatestMeasurement());
    }
    return _powerDraw;
}

void HydroponicsRegulatedRail::setLimitTrigger(HydroponicsTrigger *limitTrigger)
{
    if (_limitTrigger != limitTrigger) {
        if (_limitTrigger) { detachLimitTrigger(); delete _limitTrigger; }
        _limitTrigger = limitTrigger;
        if (_limitTrigger) { attachLimitTrigger(); }
    }
}

const HydroponicsTrigger *HydroponicsRegulatedRail::getLimitTrigger() const
{
    return _limitTrigger;
}

float HydroponicsRegulatedRail::getMaxPower() const
{
    return _maxPower;
}

void HydroponicsRegulatedRail::saveToData(HydroponicsData *dataOut)
{
    HydroponicsRail::saveToData(dataOut);

    ((HydroponicsRegulatedRailData *)dataOut)->maxPower = roundForExport(_maxPower, 1);
    if (_powerSensor.getId()) {
        strncpy(((HydroponicsRegulatedRailData *)dataOut)->powerSensor, _powerSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_limitTrigger) {
        _limitTrigger->saveToData(&(((HydroponicsRegulatedRailData *)dataOut)->limitTrigger));
    }
}

void HydroponicsRegulatedRail::handleActivation(HydroponicsActuator *actuator)
{
    if (!getPowerSensor() && actuator) {
        auto powerReq = actuator->getContinuousPowerDraw();
        auto powerDraw = getPowerDraw();
        bool enabled = actuator->getIsEnabled();

        convertUnits(&powerReq, getPowerUnits(), getRailVoltage());

        if (enabled) {
            powerDraw.value += powerReq.value;
        } else {
            powerDraw.value -= powerReq.value;
        }

        setPowerDraw(powerDraw);

        if (!enabled) {
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<HydroponicsRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}

void HydroponicsRegulatedRail::attachPowerSensor()
{
    HYDRUINO_SOFT_ASSERT(getPowerSensor(), SFP(HS_Err_MissingLinkage));
    if (getPowerSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsRegulatedRail::handlePowerMeasure);
        _powerSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsRegulatedRail::detachPowerSensor()
{
    HYDRUINO_SOFT_ASSERT(getPowerSensor(), SFP(HS_Err_MissingLinkage));
    if (getPowerSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsRegulatedRail::handlePowerMeasure);
        _powerSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsRegulatedRail::handlePowerMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        float capacityBefore = getCapacity();

        setPowerDraw(getAsSingleMeasurement(measurement, 0, _maxPower, _powerUnits));

        if (getCapacity() < capacityBefore - FLT_EPSILON) {
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<HydroponicsRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}

void HydroponicsRegulatedRail::attachLimitTrigger()
{
    HYDRUINO_SOFT_ASSERT(_limitTrigger, SFP(HS_Err_MissingLinkage));
    if (_limitTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &HydroponicsRegulatedRail::handleLimitTrigger);
        _limitTrigger->getTriggerSignal().attach(methodSlot);
    }
}

void HydroponicsRegulatedRail::detachLimitTrigger()
{
    HYDRUINO_SOFT_ASSERT(_limitTrigger, SFP(HS_Err_MissingLinkage));
    if (_limitTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &HydroponicsRegulatedRail::handleLimitTrigger);
        _limitTrigger->getTriggerSignal().detach(methodSlot);
    }
}

void HydroponicsRegulatedRail::handleLimitTrigger(Hydroponics_TriggerState triggerState)
{
    if (triggerState != Hydroponics_TriggerState_Undefined && triggerState != Hydroponics_TriggerState_Disabled && _limitState != triggerState) {
        _limitState = triggerState;
        handleLimitState();
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

    if (powerUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HS_Key_PowerUnits)] = unitsTypeToSymbol(powerUnits); }
}

void HydroponicsRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);

    powerUnits = unitsTypeFromSymbol(objectIn[SFP(HS_Key_PowerUnits)]);
}

HydroponicsSimpleRailData::HydroponicsSimpleRailData()
    : HydroponicsRailData(), maxActiveAtOnce(2)
{
    _size = sizeof(*this);
}

void HydroponicsSimpleRailData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsRailData::toJSONObject(objectOut);

    if (maxActiveAtOnce != 2) { objectOut[SFP(HS_Key_MaxActiveAtOnce)] = maxActiveAtOnce; }
}

void HydroponicsSimpleRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsRailData::fromJSONObject(objectIn);

    maxActiveAtOnce = objectIn[SFP(HS_Key_MaxActiveAtOnce)] | maxActiveAtOnce;
}

HydroponicsRegulatedRailData::HydroponicsRegulatedRailData()
    : HydroponicsRailData(), maxPower(0), powerSensor{0}, limitTrigger()
{
    _size = sizeof(*this);
}

void HydroponicsRegulatedRailData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsRailData::toJSONObject(objectOut);

    objectOut[SFP(HS_Key_MaxPower)] = maxPower;
    if (powerSensor[0]) { objectOut[SFP(HS_Key_PowerSensor)] = stringFromChars(powerSensor, HYDRUINO_NAME_MAXSIZE); }
    if (limitTrigger.type != -1) {
        JsonObject limitTriggerObj = objectOut.createNestedObject(SFP(HS_Key_LimitTrigger));
        limitTrigger.toJSONObject(limitTriggerObj);
    }
}

void HydroponicsRegulatedRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsRailData::fromJSONObject(objectIn);

    maxPower = objectIn[SFP(HS_Key_MaxPower)] | maxPower;
    const char *powerSensorStr = objectIn[SFP(HS_Key_PowerSensor)];
    if (powerSensorStr && powerSensorStr[0]) { strncpy(powerSensor, powerSensorStr, HYDRUINO_NAME_MAXSIZE); }
    JsonObjectConst limitTriggerObj = objectIn[SFP(HS_Key_LimitTrigger)];
    if (!limitTriggerObj.isNull()) { limitTrigger.fromJSONObject(limitTriggerObj); }
}
