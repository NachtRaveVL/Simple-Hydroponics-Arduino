/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Power Rails 
*/

#include "Hydroponics.h"

HydroponicsRail *newRailObjectFromData(const HydroponicsRailData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjData(), F("Invalid data"));

    if (dataIn && dataIn->isObjData()) {
        switch(dataIn->id.object.classType) {
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
    : HydroponicsObject(HydroponicsIdentity(railType, railIndex)), classType((typeof(classType))classTypeIn)
{ ; }

HydroponicsRail::HydroponicsRail(const HydroponicsRailData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType))
{ ; }

HydroponicsRail::~HydroponicsRail()
{
    //discardFromTaskManager(&_capacitySignal);
    {   auto actuators = getActuators();
        for (auto iter = actuators.begin(); iter != actuators.end(); ++iter) { removeActuator(iter->second); }
    }
    {   auto sensors = getSensors();
        for (auto iter = sensors.begin(); iter != sensors.end(); ++iter) { removeSensor(iter->second); }
    }
}

void HydroponicsRail::update()
{
    HydroponicsObject::update();
}

void HydroponicsRail::resolveLinks()
{
    HydroponicsObject::resolveLinks();
}

void HydroponicsRail::handleLowMemory()
{
    HydroponicsObject::handleLowMemory();
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

arx::map<Hydroponics_KeyType, HydroponicsActuator *> HydroponicsRail::getActuators() const
{
    arx::map<Hydroponics_KeyType, HydroponicsActuator *> retVal;
    for (auto iter = _links.begin(); iter != _links.end(); ++iter) {
        auto obj = iter->second;
        if (obj && obj->isActuatorType()) {
            retVal.insert(iter->first, (HydroponicsActuator *)obj);
        }
    }
    return retVal;
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

arx::map<Hydroponics_KeyType, HydroponicsSensor *> HydroponicsRail::getSensors() const
{
    arx::map<Hydroponics_KeyType, HydroponicsSensor *> retVal;
    for (auto iter = _links.begin(); iter != _links.end(); ++iter) {
        auto obj = iter->second;
        if (obj && obj->isSensorType()) {
            retVal.insert(iter->first, (HydroponicsSensor *)obj);
        }
    }
    return retVal;
}

Hydroponics_RailType HydroponicsRail::getRailType() const
{
    return _id.objTypeAs.railType;
}

Hydroponics_PositionIndex HydroponicsRail::getRailIndex() const
{
    return _id.posIndex;
}

Signal<HydroponicsRail *> &HydroponicsRail::getCapacitySignal()
{
    return _capacitySignal;
}

HydroponicsData *HydroponicsRail::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void HydroponicsRail::saveToData(HydroponicsData *dataOut) const
{
    HydroponicsObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
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
{ ; }

bool HydroponicsSimpleRail::canActivate(HydroponicsActuator *actuator) const
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
        auto methodSlot = MethodSlot<HydroponicsSimpleRail, HydroponicsActuator *>(this, &handleActivation);
        actuator->getActivationSignal().attach(methodSlot);
        return true;
    }
    return false;
}

bool HydroponicsSimpleRail::removeActuator(HydroponicsActuator *actuator)
{
    if (HydroponicsRail::removeActuator(actuator)) {
        auto methodSlot = MethodSlot<HydroponicsSimpleRail, HydroponicsActuator *>(this, &handleActivation);
        actuator->getActivationSignal().detach(methodSlot);
        return true;
    }
    return false;
}

int HydroponicsSimpleRail::getActiveCount()
{
    return _activeCount;
}

void HydroponicsSimpleRail::saveToData(HydroponicsData *dataOut) const
{
    HydroponicsRail::saveToData(dataOut);

    ((HydroponicsSimpleRailData *)dataOut)->maxActiveAtOnce = _maxActiveAtOnce;
}

void HydroponicsSimpleRail::handleActivation(HydroponicsActuator *actuator)
{
    bool hadSpaceBefore = _activeCount < _maxActiveAtOnce;

    if (actuator->getIsEnabled()) {
        _activeCount++;
    } else {
        _activeCount--;
    }

    bool hasSpaceAfter = _activeCount < _maxActiveAtOnce;

    if (hadSpaceBefore != hasSpaceAfter) {
        scheduleSignalFireOnce<HydroponicsRail *>(_capacitySignal, this);
    }
}


HydroponicsRegulatedRail::HydroponicsRegulatedRail(Hydroponics_RailType railType,
                             Hydroponics_PositionIndex railIndex,
                             float maxPower,
                             int classType = Regulated)
    : HydroponicsRail(railType, railIndex, classType), _maxPower(maxPower), _powerUnits(Hydroponics_UnitsType_Power_Wattage)
{ ; }

HydroponicsRegulatedRail::HydroponicsRegulatedRail(const HydroponicsRegulatedRailData *dataIn)
    : HydroponicsRail(dataIn), _maxPower(dataIn->maxPower), _powerUnits(dataIn->powerUnits),
      _powerSensor(dataIn->powerSensorName), _limitTrigger(newTriggerObjectFromSubData(&(dataIn->limitTrigger)))
{ ; }

HydroponicsRegulatedRail::~HydroponicsRegulatedRail()
{
    if (_powerSensor) { detachPowerSensor(); }
    if (_limitTrigger) { delete _limitTrigger; _limitTrigger = nullptr; }
}

void HydroponicsRegulatedRail::update()
{
    HydroponicsRail::update();

    if (_limitTrigger) { _limitTrigger->update(); }
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

bool HydroponicsRegulatedRail::canActivate(HydroponicsActuator *actuator) const
{
    return _limitTrigger ? _limitTrigger->getTriggerState() != Hydroponics_TriggerState_Triggered : true;
}

float HydroponicsRegulatedRail::getCapacity() const
{
    return _powerDraw.value / _maxPower;
}

void HydroponicsRegulatedRail::setPowerUnits(Hydroponics_UnitsType powerUnits)
{
    _powerUnits = powerUnits;
}

Hydroponics_UnitsType HydroponicsRegulatedRail::getPowerUnits() const
{
    return _powerUnits;
}

void HydroponicsRegulatedRail::setPowerSensor(HydroponicsIdentity powerSensorId)
{
    if (_powerSensor != powerSensorId) {
        if (_powerSensor) { detachPowerSensor(); }
        _powerSensor = powerSensorId;
    }
}

void HydroponicsRegulatedRail::setPowerSensor(shared_ptr<HydroponicsSensor> powerSensor)
{
    if (_powerSensor != powerSensor) {
        if (_powerSensor) { detachPowerSensor(); }
        _powerSensor = powerSensor;
        if (_powerSensor) { attachPowerSensor(); }
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
    _powerDraw.units = powerDrawUnits != Hydroponics_UnitsType_Undefined ? powerDrawUnits : Hydroponics_UnitsType_Power_Wattage;
}

void HydroponicsRegulatedRail::setPowerDraw(HydroponicsSingleMeasurement powerDraw)
{
    _powerDraw = powerDraw;
}

const HydroponicsSingleMeasurement &HydroponicsRegulatedRail::getPowerDraw() const
{
    return _powerDraw;
}

void HydroponicsRegulatedRail::setLimitTrigger(HydroponicsTrigger *limitTrigger)
{
    if (_limitTrigger != limitTrigger) {
        if (_limitTrigger) { delete _limitTrigger; }
        _limitTrigger = limitTrigger;
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

void HydroponicsRegulatedRail::saveToData(HydroponicsData *dataOut) const
{
    HydroponicsRail::saveToData(dataOut);

    ((HydroponicsRegulatedRailData *)dataOut)->maxPower = _maxPower;
    ((HydroponicsRegulatedRailData *)dataOut)->powerUnits = _powerUnits;
    if (_powerSensor.getId()) {
        strncpy(((HydroponicsRegulatedRailData *)dataOut)->powerSensorName, _powerSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_limitTrigger) {
        _limitTrigger->saveToData(&(((HydroponicsRegulatedRailData *)dataOut)->limitTrigger));
    }
}

void HydroponicsRegulatedRail::attachPowerSensor()
{
    HYDRUINO_SOFT_ASSERT(_powerSensor, F("Power sensor not linked, failure attaching"));
    if (_powerSensor) {
        auto methodSlot = MethodSlot<HydroponicsRegulatedRail, HydroponicsMeasurement *>(this, &handlePowerMeasure);
        _powerSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsRegulatedRail::detachPowerSensor()
{
    HYDRUINO_SOFT_ASSERT(_powerSensor, F("Power sensor not linked, failure detaching"));
    if (_powerSensor) {
        auto methodSlot = MethodSlot<HydroponicsRegulatedRail, HydroponicsMeasurement *>(this, &handlePowerMeasure);
        _powerSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsRegulatedRail::handlePowerMeasure(HydroponicsMeasurement *measurement)
{
    if (measurement) {
        if (measurement->isBinaryType()) {
            setPowerDraw(((HydroponicsBinaryMeasurement *)measurement)->state ? _maxPower : 0.0f, _powerUnits);
        } else if (measurement->isSingleType()) {
            setPowerDraw(*((HydroponicsSingleMeasurement *)measurement));
        } else if (measurement->isDoubleType()) {
            setPowerDraw(((HydroponicsDoubleMeasurement *)measurement)->asSingleMeasurement(0)); // TODO: Correct row reference, based on sensor
        } else if (measurement->isTripleType()) {
            setPowerDraw(((HydroponicsTripleMeasurement *)measurement)->asSingleMeasurement(0)); // TODO: Correct row reference, based on sensor
        }
    }
}

void HydroponicsRegulatedRail::handleLimitTrigger(Hydroponics_TriggerState triggerState)
{
    scheduleSignalFireOnce<HydroponicsRail *>(_capacitySignal, this);
}


HydroponicsRailData::HydroponicsRailData()
    : HydroponicsObjectData()
{
    _size = sizeof(*this);
}

void HydroponicsRailData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsObjectData::toJSONObject(objectOut);
}

void HydroponicsRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);
}

HydroponicsSimpleRailData::HydroponicsSimpleRailData()
    : HydroponicsRailData(), maxActiveAtOnce(2)
{
    _size = sizeof(*this);
}

void HydroponicsSimpleRailData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsRailData::toJSONObject(objectOut);

    objectOut[F("maxActiveAtOnce")] = maxActiveAtOnce;
}

void HydroponicsSimpleRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsRailData::fromJSONObject(objectIn);

    maxActiveAtOnce = objectIn[F("maxActiveAtOnce")] | maxActiveAtOnce;
}

HydroponicsRegulatedRailData::HydroponicsRegulatedRailData()
    : HydroponicsRailData(), maxPower(0), powerUnits{Hydroponics_UnitsType_Undefined}, powerSensorName{0}
{
    _size = sizeof(*this);
}

void HydroponicsRegulatedRailData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsRailData::toJSONObject(objectOut);

    objectOut[F("maxPower")] = maxPower;
    if (powerUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("powerUnits")] = powerUnits; }
    if (powerSensorName[0]) { objectOut[F("powerSensorName")] = stringFromChars(powerSensorName, HYDRUINO_NAME_MAXSIZE); }
    if (limitTrigger.type != -1) {
        JsonObject limitTriggerObj = objectOut.createNestedObject(F("limitTrigger"));
        limitTrigger.toJSONObject(limitTriggerObj);
    }
}

void HydroponicsRegulatedRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsRailData::fromJSONObject(objectIn);

    maxPower = objectIn[F("maxPower")] | maxPower;
    powerUnits = objectIn[F("powerUnits")] | powerUnits;
    const char *powerSensorNameStr = objectIn[F("powerSensorName")];
    if (powerSensorNameStr && powerSensorNameStr[0]) { strncpy(powerSensorName, powerSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    JsonObjectConst limitTriggerObj = objectIn[F("limitTrigger")];
    if (!limitTriggerObj.isNull()) { limitTrigger.fromJSONObject(limitTriggerObj); }
}
