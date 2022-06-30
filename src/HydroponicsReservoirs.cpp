/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Reservoirs
*/

#include "Hydroponics.h"

HydroponicsReservoir *newReservoirObjectFromData(const HydroponicsReservoirData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjData(), F("Invalid data"));

    if (dataIn && dataIn->isObjData()) {
        switch(dataIn->id.object.classType) {
            case 0: // Fluid
                return new HydroponicsFluidReservoir((const HydroponicsFluidReservoirData *)dataIn);
            case 1: // Pipe
                return new HydroponicsInfiniteReservoir((const HydroponicsInfiniteReservoirData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroponicsReservoir::HydroponicsReservoir(Hydroponics_ReservoirType reservoirType, Hydroponics_PositionIndex reservoirIndex, int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(reservoirType, reservoirIndex)), classType((typeof(classType))classTypeIn)
{ ; }

HydroponicsReservoir::HydroponicsReservoir(const HydroponicsReservoirData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType))
{ ; }

HydroponicsReservoir::~HydroponicsReservoir()
{
    //discardFromTaskManager(&_filledSignal);
    //discardFromTaskManager(&_emptySignal);
    {   auto actuators = getActuators();
        for (auto iter = actuators.begin(); iter != actuators.end(); ++iter) { removeActuator(iter->second); }
    }
    {   auto sensors = getSensors();
        for (auto iter = sensors.begin(); iter != sensors.end(); ++iter) { removeSensor(iter->second); }
    }
    {   auto crops = getCrops();
        for (auto iter = crops.begin(); iter != crops.end(); ++iter) { removeCrop(iter->second); }
    }
}

void HydroponicsReservoir::update()
{
    HydroponicsObject::update();
}

void HydroponicsReservoir::resolveLinks()
{
    HydroponicsObject::resolveLinks();
}

void HydroponicsReservoir::handleLowMemory()
{
    HydroponicsObject::handleLowMemory();
}

bool HydroponicsReservoir::addActuator(HydroponicsActuator *actuator)
{
    return addLinkage(actuator);
}

bool HydroponicsReservoir::removeActuator(HydroponicsActuator *actuator)
{
    return removeLinkage(actuator);
}

bool HydroponicsReservoir::hasActuator(HydroponicsActuator *actuator) const
{
    return hasLinkage(actuator);
}

arx::map<Hydroponics_KeyType, HydroponicsActuator *> HydroponicsReservoir::getActuators() const
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

bool HydroponicsReservoir::addSensor(HydroponicsSensor *sensor)
{
    return addLinkage(sensor);
}

bool HydroponicsReservoir::removeSensor(HydroponicsSensor *sensor)
{
    return removeLinkage(sensor);
}

bool HydroponicsReservoir::hasSensor(HydroponicsSensor *sensor) const
{
    return hasLinkage(sensor);
}

arx::map<Hydroponics_KeyType, HydroponicsSensor *> HydroponicsReservoir::getSensors() const
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

bool HydroponicsReservoir::addCrop(HydroponicsCrop *crop)
{
    return addLinkage(crop);
}

bool HydroponicsReservoir::removeCrop(HydroponicsCrop *crop)
{
    return removeLinkage(crop);
}

bool HydroponicsReservoir::hasCrop(HydroponicsCrop *crop) const
{
    return hasLinkage(crop);
}

arx::map<Hydroponics_KeyType, HydroponicsCrop *> HydroponicsReservoir::getCrops() const
{
    arx::map<Hydroponics_KeyType, HydroponicsCrop *> retVal;
    for (auto iter = _links.begin(); iter != _links.end(); ++iter) {
        auto obj = iter->second;
        if (obj && obj->isCropType()) {
            retVal.insert(iter->first, (HydroponicsCrop *)obj);
        }
    }
    return retVal;
}

Hydroponics_ReservoirType HydroponicsReservoir::getReservoirType() const
{
    return _id.objTypeAs.reservoirType;
}

Hydroponics_PositionIndex HydroponicsReservoir::getReservoirIndex() const
{
    return _id.posIndex;
}

Signal<HydroponicsReservoir *> &HydroponicsReservoir::getFilledSignal()
{
    return _filledSignal;
}

Signal<HydroponicsReservoir *> &HydroponicsReservoir::getEmptySignal()
{
    return _emptySignal;
}

HydroponicsData *HydroponicsReservoir::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void HydroponicsReservoir::saveToData(HydroponicsData *dataOut) const
{
    HydroponicsObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
}


HydroponicsFluidReservoir::HydroponicsFluidReservoir(Hydroponics_ReservoirType reservoirType,
                                                     Hydroponics_PositionIndex reservoirIndex,
                                                     float maxVolume,
                                                     int channel,
                                                     int classType)
    : HydroponicsReservoir(reservoirType, reservoirIndex, classType),
      _maxVolume(maxVolume), _volumeUnits(defaultLiquidVolumeUnits()), _channel(channel),
      _filledTrigger(nullptr), _emptyTrigger(nullptr)
{ ; }

HydroponicsFluidReservoir::HydroponicsFluidReservoir(const HydroponicsFluidReservoirData *dataIn)
    : HydroponicsReservoir(dataIn),
      _maxVolume(dataIn->maxVolume), _volumeUnits(dataIn->volumeUnits), _channel(dataIn->channel),
      _volumeSensor(dataIn->volumeSensorName),
      _filledTrigger(newTriggerObjectFromSubData(&(dataIn->filledTrigger))),
      _emptyTrigger(newTriggerObjectFromSubData(&(dataIn->emptyTrigger)))
{ ; }

HydroponicsFluidReservoir::~HydroponicsFluidReservoir()
{
    if (_volumeSensor) { detachVolumeSensor(); }
    if (_filledTrigger) { delete _filledTrigger; _filledTrigger = nullptr; }
    if (_emptyTrigger) { delete _emptyTrigger; _emptyTrigger = nullptr; }
}

void HydroponicsFluidReservoir::update()
{
    HydroponicsReservoir::update();

    if (_filledTrigger) { _filledTrigger->update(); }
    if (_emptyTrigger) { _emptyTrigger->update(); }
}

void HydroponicsFluidReservoir::resolveLinks()
{
    HydroponicsReservoir::resolveLinks();

    if (_volumeSensor.needsResolved()) { getVolumeSensor(); }
    if (_filledTrigger) { _filledTrigger->resolveLinks(); }
    if (_emptyTrigger) { _emptyTrigger->resolveLinks(); }
}

void HydroponicsFluidReservoir::handleLowMemory()
{
    HydroponicsReservoir::handleLowMemory();

    if (_filledTrigger) { _filledTrigger->handleLowMemory(); }
    if (_emptyTrigger) { _emptyTrigger->handleLowMemory(); }
}

bool HydroponicsFluidReservoir::canActivate(HydroponicsActuator *actuator) const
{
    bool doEmptyCheck;

    switch (actuator->getActuatorType()) {
        case Hydroponics_ActuatorType_WaterPump:
        case Hydroponics_ActuatorType_PeristalticPump: {
            doEmptyCheck = (actuator->getReservoir().get() == this);
        } break;

        case Hydroponics_ActuatorType_WaterAerator:
        case Hydroponics_ActuatorType_WaterHeater: {
            doEmptyCheck = true;
        } break;

        default:
            return true;
    }

    return doEmptyCheck ? !getIsEmpty() : !getIsFull();
}

bool HydroponicsFluidReservoir::getIsFull() const
{
    return _filledTrigger ? _filledTrigger->getTriggerState() == Hydroponics_TriggerState_Triggered
                          : _volume.value >= _maxVolume - FLT_EPSILON;
}

bool HydroponicsFluidReservoir::getIsEmpty() const
{
    return _emptyTrigger ? _emptyTrigger->getTriggerState() == Hydroponics_TriggerState_Triggered
                         : _volume.value <= (_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_RES_FEED_EMPTY_FRACTION
                                                                                                             : 0) + FLT_EPSILON;
}

void HydroponicsFluidReservoir::setVolumeUnits(Hydroponics_UnitsType volumeUnits)
{
    _volumeUnits = volumeUnits;
}

Hydroponics_UnitsType HydroponicsFluidReservoir::getVolumeUnits() const
{
    return _volumeUnits;
}

void HydroponicsFluidReservoir::setChannelNumber(int channel)
{
    _channel = channel;
}

int HydroponicsFluidReservoir::getChannelNumber() const
{
    return _channel;
}

void HydroponicsFluidReservoir::setVolumeSensor(HydroponicsIdentity volumeSensorId)
{
    if (_volumeSensor != volumeSensorId) {
        if (_volumeSensor) { detachVolumeSensor(); }
        _volumeSensor = volumeSensorId;
    }
}

void HydroponicsFluidReservoir::setVolumeSensor(shared_ptr<HydroponicsSensor> volumeSensor)
{
    if (_volumeSensor != volumeSensor) {
        if (_volumeSensor) { detachVolumeSensor(); }
        _volumeSensor = volumeSensor;
        if (_volumeSensor) { attachVolumeSensor(); }
    }
}

shared_ptr<HydroponicsSensor> HydroponicsFluidReservoir::getVolumeSensor()
{
    if (_volumeSensor.resolveIfNeeded()) { attachVolumeSensor(); }
    return _volumeSensor.getObj();
}

void HydroponicsFluidReservoir::setLiquidVolume(float liquidVolume, Hydroponics_UnitsType liquidVolumeUnits)
{
    _volume.value = liquidVolume;
    _volume.units = liquidVolumeUnits != Hydroponics_UnitsType_Undefined ? liquidVolumeUnits : defaultLiquidVolumeUnits();
}

void HydroponicsFluidReservoir::setLiquidVolume(HydroponicsSingleMeasurement liquidVolume)
{
    _volume = liquidVolume;
}

const HydroponicsSingleMeasurement &HydroponicsFluidReservoir::getLiquidVolume() const
{
    return _volume;
}

void HydroponicsFluidReservoir::setFilledTrigger(HydroponicsTrigger *filledTrigger)
{
    if (_filledTrigger != filledTrigger) {
        if (_filledTrigger) { delete _filledTrigger; }
        _filledTrigger = filledTrigger;
    }
}

const HydroponicsTrigger *HydroponicsFluidReservoir::getFilledTrigger() const
{
    return _filledTrigger;
}

void HydroponicsFluidReservoir::setEmptyTrigger(HydroponicsTrigger *emptyTrigger)
{
    if (_emptyTrigger != emptyTrigger) {
        if (_emptyTrigger) { delete _emptyTrigger; }
        _emptyTrigger = emptyTrigger;
    }
}

const HydroponicsTrigger *HydroponicsFluidReservoir::getEmptyTrigger() const
{
    return _emptyTrigger;
}

void HydroponicsFluidReservoir::saveToData(HydroponicsData *dataOut) const
{
    HydroponicsReservoir::saveToData(dataOut);

    ((HydroponicsFluidReservoirData *)dataOut)->maxVolume = _maxVolume;
    ((HydroponicsFluidReservoirData *)dataOut)->volumeUnits = _volumeUnits;
    ((HydroponicsFluidReservoirData *)dataOut)->channel = _channel;
    if (_volumeSensor.getId()) {
        strncpy(((HydroponicsFluidReservoirData *)dataOut)->volumeSensorName, _volumeSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_filledTrigger) {
        _filledTrigger->saveToData(&(((HydroponicsFluidReservoirData *)dataOut)->filledTrigger));
    }
    if (_emptyTrigger) {
        _emptyTrigger->saveToData(&(((HydroponicsFluidReservoirData *)dataOut)->emptyTrigger));
    }
}

void HydroponicsFluidReservoir::attachVolumeSensor()
{
    HYDRUINO_SOFT_ASSERT(_volumeSensor, F("Volume sensor not linked, failure attaching"));
    if (_volumeSensor) {
        auto methodSlot = MethodSlot<HydroponicsFluidReservoir, HydroponicsMeasurement *>(this, &handleVolumeMeasure);
        _volumeSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFluidReservoir::detachVolumeSensor()
{
    HYDRUINO_SOFT_ASSERT(_volumeSensor, F("Volume sensor not linked, failure detaching"));
    if (_volumeSensor) {
        auto methodSlot = MethodSlot<HydroponicsFluidReservoir, HydroponicsMeasurement *>(this, &handleVolumeMeasure);
        _volumeSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFluidReservoir::handleVolumeMeasure(HydroponicsMeasurement *measurement)
{
    if (measurement) {
        if (measurement->isBinaryType()) {
            setLiquidVolume(((HydroponicsBinaryMeasurement *)measurement)->state ? _maxVolume : 0.0f, _volumeUnits);
        } else if (measurement->isSingleType()) {
            setLiquidVolume(*((HydroponicsSingleMeasurement *)measurement));
        } else if (measurement->isDoubleType()) {
            setLiquidVolume(((HydroponicsDoubleMeasurement *)measurement)->asSingleMeasurement(0)); // TODO: Correct row reference, based on sensor
        } else if (measurement->isTripleType()) {
            setLiquidVolume(((HydroponicsTripleMeasurement *)measurement)->asSingleMeasurement(0)); // TODO: Correct row reference, based on sensor
        }
    }
}


HydroponicsInfiniteReservoir::HydroponicsInfiniteReservoir(Hydroponics_ReservoirType reservoirType,
                                                           Hydroponics_PositionIndex reservoirIndex,
                                                           bool alwaysFilled,
                                                           int classType)
    : HydroponicsReservoir(reservoirType, reservoirIndex, classType), _alwaysFilled(alwaysFilled)
{ ; }

HydroponicsInfiniteReservoir::HydroponicsInfiniteReservoir(const HydroponicsInfiniteReservoirData *dataIn)
    : HydroponicsReservoir(dataIn), _alwaysFilled(dataIn->alwaysFilled)
{ ; }

HydroponicsInfiniteReservoir::~HydroponicsInfiniteReservoir()
{ ; }

bool HydroponicsInfiniteReservoir::canActivate(HydroponicsActuator *actuator) const
{
    return actuator->getReservoir().get() == this ? getIsFull() : getIsEmpty();
}

bool HydroponicsInfiniteReservoir::getIsFull() const
{
    return _alwaysFilled;
}

bool HydroponicsInfiniteReservoir::getIsEmpty() const
{
    return !_alwaysFilled;
}

void HydroponicsInfiniteReservoir::saveToData(HydroponicsData *dataOut) const
{
    HydroponicsReservoir::saveToData(dataOut);

    ((HydroponicsInfiniteReservoirData *)dataOut)->alwaysFilled = _alwaysFilled;
}


HydroponicsReservoirData::HydroponicsReservoirData()
    : HydroponicsObjectData()
{
    _size = sizeof(*this);
}

void HydroponicsReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsObjectData::toJSONObject(objectOut);
}

void HydroponicsReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);
}

HydroponicsFluidReservoirData::HydroponicsFluidReservoirData()
    : HydroponicsReservoirData(), maxVolume(0), volumeUnits(Hydroponics_UnitsType_Undefined), channel(-1), volumeSensorName{0}
{
    _size = sizeof(*this);
}

void HydroponicsFluidReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsReservoirData::toJSONObject(objectOut);

    objectOut[F("maxVolume")] = maxVolume;
    if (volumeUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("volumeUnits")] = volumeUnits; }
    if (channel != -1) { objectOut[F("channel")] = channel; }
    if (volumeSensorName[0]) { objectOut[F("volumeSensorName")] = stringFromChars(volumeSensorName, HYDRUINO_NAME_MAXSIZE); }
    if (filledTrigger.type != -1) {
        JsonObject filledTriggerObj = objectOut.createNestedObject(F("filledTrigger"));
        filledTrigger.toJSONObject(filledTriggerObj);
    }
    if (emptyTrigger.type != -1) {
        JsonObject emptyTriggerObj = objectOut.createNestedObject(F("emptyTrigger"));
        emptyTrigger.toJSONObject(emptyTriggerObj);
    }
}

void HydroponicsFluidReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsReservoirData::fromJSONObject(objectIn);

    maxVolume = objectIn[F("maxVolume")] | maxVolume;
    volumeUnits = objectIn[F("volumeUnits")] | volumeUnits;
    channel = objectIn[F("channel")] | channel;
    const char *volumeSensorNameStr = objectIn[F("volumeSensorName")];
    if (volumeSensorNameStr && volumeSensorNameStr[0]) { strncpy(volumeSensorName, volumeSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    JsonObjectConst filledTriggerObj = objectIn[F("filledTrigger")];
    if (!filledTriggerObj.isNull()) { filledTrigger.fromJSONObject(filledTriggerObj); }
    JsonObjectConst emptyTriggerObj = objectIn[F("emptyTrigger")];
    if (!emptyTriggerObj.isNull()) { emptyTrigger.fromJSONObject(emptyTriggerObj); }
}

HydroponicsInfiniteReservoirData::HydroponicsInfiniteReservoirData()
    : HydroponicsReservoirData(), alwaysFilled(true)
{
    _size = sizeof(*this);
}

void HydroponicsInfiniteReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsReservoirData::toJSONObject(objectOut);

    objectOut[F("alwaysFilled")] = alwaysFilled;
}

void HydroponicsInfiniteReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsReservoirData::fromJSONObject(objectIn);

    alwaysFilled = objectIn[F("alwaysFilled")] | alwaysFilled;
}
