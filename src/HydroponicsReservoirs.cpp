/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Reservoirs
*/

#include "Hydroponics.h"

HydroponicsReservoir *newReservoirObjectFromData(const HydroponicsReservoirData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), F("Invalid data"));

    if (dataIn && dataIn->isObjectData()) {
        switch(dataIn->id.object.classType) {
            case 0: // Fluid
                return new HydroponicsFluidReservoir((const HydroponicsFluidReservoirData *)dataIn);
            case 1: // Feed
                return new HydroponicsFeedReservoir((const HydroponicsFeedReservoirData *)dataIn);
            case 2: // Pipe
                return new HydroponicsInfiniteReservoir((const HydroponicsInfiniteReservoirData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroponicsReservoir::HydroponicsReservoir(Hydroponics_ReservoirType reservoirType, Hydroponics_PositionIndex reservoirIndex, int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(reservoirType, reservoirIndex)), classType((typeof(classType))classTypeIn),
      _filledState(Hydroponics_TriggerState_NotTriggered), _emptyState(Hydroponics_TriggerState_NotTriggered)
{ ; }

HydroponicsReservoir::HydroponicsReservoir(const HydroponicsReservoirData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _filledState(Hydroponics_TriggerState_NotTriggered), _emptyState(Hydroponics_TriggerState_NotTriggered)
{ ; }

HydroponicsReservoir::~HydroponicsReservoir()
{
    //discardFromTaskManager(&_filledSignal);
    //discardFromTaskManager(&_emptySignal);
}

void HydroponicsReservoir::update()
{
    HydroponicsObject::update();

    if (_filledState != triggerStateFromBool(getIsFilled())) {
        _filledState = triggerStateFromBool(getIsFilled());
        scheduleSignalFireOnce<HydroponicsReservoir *>(_filledSignal, this);
    }
    if (_emptyState != triggerStateFromBool(getIsEmpty())) {
        _emptyState = triggerStateFromBool(getIsEmpty());
        scheduleSignalFireOnce<HydroponicsReservoir *>(_emptySignal, this);
    }
}

void HydroponicsReservoir::resolveLinks()
{
    HydroponicsObject::resolveLinks();
}

void HydroponicsReservoir::handleLowMemory()
{
    HydroponicsObject::handleLowMemory();
}

bool HydroponicsReservoir::canActivate(HydroponicsActuator *actuator)
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

    if (doEmptyCheck) {
        return !getIsEmpty();
    } else {
        return !getIsFilled();
    }
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

arx::map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE> HydroponicsReservoir::getActuators() const
{
    return linksFilterActuators(_links);
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

arx::map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE> HydroponicsReservoir::getSensors() const
{
    return linksFilterSensors(_links);
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

arx::map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE> HydroponicsReservoir::getCrops() const
{
    return linksFilterReservoirs(_links);
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

void HydroponicsReservoir::saveToData(HydroponicsData *dataOut)
{
    HydroponicsObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
}


HydroponicsFluidReservoir::HydroponicsFluidReservoir(Hydroponics_ReservoirType reservoirType,
                                                     Hydroponics_PositionIndex reservoirIndex,
                                                     float maxVolume,
                                                     int classType)
    : HydroponicsReservoir(reservoirType, reservoirIndex, classType),
      _maxVolume(maxVolume), _volumeUnits(defaultWaterVolumeUnits()), _needsVolumeUpdate(true),
      _filledTrigger(nullptr), _emptyTrigger(nullptr)
{ ; }

HydroponicsFluidReservoir::HydroponicsFluidReservoir(const HydroponicsFluidReservoirData *dataIn)
    : HydroponicsReservoir(dataIn), _needsVolumeUpdate(true),
      _maxVolume(dataIn->maxVolume), _volumeUnits(dataIn->volumeUnits),
      _volumeSensor(dataIn->volumeSensorName),
      _filledTrigger(newTriggerObjectFromSubData(&(dataIn->filledTrigger))),
      _emptyTrigger(newTriggerObjectFromSubData(&(dataIn->emptyTrigger)))
{
    if (_filledTrigger) { attachFilledTrigger(); }
    if (_emptyTrigger) { attachEmptyTrigger(); }
}

HydroponicsFluidReservoir::~HydroponicsFluidReservoir()
{
    if (_volumeSensor) { detachWaterVolumeSensor(); }
    if (_filledTrigger) { detachFilledTrigger(); delete _filledTrigger; _filledTrigger = nullptr; }
    if (_emptyTrigger) { detachEmptyTrigger(); delete _emptyTrigger; _emptyTrigger = nullptr; }
}

void HydroponicsFluidReservoir::update()
{
    HydroponicsReservoir::update();

    if (_filledTrigger) { _filledTrigger->update(); }
    if (_emptyTrigger) { _emptyTrigger->update(); }

    if (_needsVolumeUpdate && getVolumeSensor()) {
        handleWaterVolumeMeasure(_volumeSensor->getLatestMeasurement());
    }
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

bool HydroponicsFluidReservoir::getIsFilled() const
{
    return _filledTrigger ? _filledTrigger->getTriggerState() == Hydroponics_TriggerState_Triggered
                          : _waterVolume.value >= (_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_FILLED_FRACTION
                                                                                                                      : _maxVolume) - FLT_EPSILON;
}

bool HydroponicsFluidReservoir::getIsEmpty() const
{
    return _emptyTrigger ? _emptyTrigger->getTriggerState() == Hydroponics_TriggerState_Triggered
                         : _waterVolume.value <= (_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_EMPTY_FRACTION
                                                                                                                     : 0) + FLT_EPSILON;
}

void HydroponicsFluidReservoir::setVolumeUnits(Hydroponics_UnitsType volumeUnits)
{
    if (_volumeUnits != volumeUnits) {
        _volumeUnits = volumeUnits;

        convertStdUnits(&_waterVolume.value, &_waterVolume.units, _volumeUnits);
    }
}

Hydroponics_UnitsType HydroponicsFluidReservoir::getVolumeUnits() const
{
    return _volumeUnits;
}

void HydroponicsFluidReservoir::setVolumeSensor(HydroponicsIdentity volumeSensorId)
{
    if (_volumeSensor != volumeSensorId) {
        if (_volumeSensor) { detachWaterVolumeSensor(); }
        _volumeSensor = volumeSensorId;
        _needsVolumeUpdate = true;
    }
}

void HydroponicsFluidReservoir::setVolumeSensor(shared_ptr<HydroponicsSensor> volumeSensor)
{
    if (_volumeSensor != volumeSensor) {
        if (_volumeSensor) { detachWaterVolumeSensor(); }
        _volumeSensor = volumeSensor;
        if (_volumeSensor) { attachWaterVolumeSensor(); }
        _needsVolumeUpdate = true;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsFluidReservoir::getVolumeSensor()
{
    if (_volumeSensor.resolveIfNeeded()) { attachWaterVolumeSensor(); }
    return _volumeSensor.getObj();
}

void HydroponicsFluidReservoir::setWaterVolume(float waterVolume, Hydroponics_UnitsType waterVolumeUnits)
{
    _waterVolume.value = waterVolume;
    _waterVolume.units = waterVolumeUnits != Hydroponics_UnitsType_Undefined ? waterVolumeUnits
                                                                             : (_volumeUnits != Hydroponics_UnitsType_Undefined ? _volumeUnits
                                                                                                                                : defaultWaterVolumeUnits());
    _waterVolume.updateTimestamp();
    _waterVolume.updateFrame(1);

    convertStdUnits(&_waterVolume.value, &_waterVolume.units, _volumeUnits);
}

void HydroponicsFluidReservoir::setWaterVolume(HydroponicsSingleMeasurement waterVolume)
{
    _waterVolume = waterVolume;

    convertStdUnits(&_waterVolume.value, &_waterVolume.units, _volumeUnits);
}

const HydroponicsSingleMeasurement &HydroponicsFluidReservoir::getWaterVolume()
{
    return _waterVolume;
}

void HydroponicsFluidReservoir::setFilledTrigger(HydroponicsTrigger *filledTrigger)
{
    if (_filledTrigger != filledTrigger) {
        if (_filledTrigger) { detachFilledTrigger(); delete _filledTrigger; }
        _filledTrigger = filledTrigger;
        if (_filledTrigger) { attachFilledTrigger(); }
    }
}

const HydroponicsTrigger *HydroponicsFluidReservoir::getFilledTrigger() const
{
    return _filledTrigger;
}

void HydroponicsFluidReservoir::setEmptyTrigger(HydroponicsTrigger *emptyTrigger)
{
    if (_emptyTrigger != emptyTrigger) {
        if (_emptyTrigger) { detachEmptyTrigger(); delete _emptyTrigger; }
        _emptyTrigger = emptyTrigger;
        if (_emptyTrigger) { attachEmptyTrigger(); }
    }
}

const HydroponicsTrigger *HydroponicsFluidReservoir::getEmptyTrigger() const
{
    return _emptyTrigger;
}

float HydroponicsFluidReservoir::getMaxVolume()
{
    return _maxVolume;
}

void HydroponicsFluidReservoir::saveToData(HydroponicsData *dataOut)
{
    HydroponicsReservoir::saveToData(dataOut);

    ((HydroponicsFluidReservoirData *)dataOut)->maxVolume = _maxVolume;
    ((HydroponicsFluidReservoirData *)dataOut)->volumeUnits = _volumeUnits;
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

void HydroponicsFluidReservoir::attachFilledTrigger()
{
    HYDRUINO_SOFT_ASSERT(_filledTrigger, F("Filled trigger not linked, failure attaching"));
    if (_filledTrigger) {
        auto methodSlot = MethodSlot<HydroponicsFluidReservoir, Hydroponics_TriggerState>(this, &handleFilledTrigger);
        _filledTrigger->getTriggerSignal().attach(methodSlot);
    }
}

void HydroponicsFluidReservoir::detachFilledTrigger()
{
    HYDRUINO_SOFT_ASSERT(_filledTrigger, F("Filled trigger not linked, failure detaching"));
    if (_filledTrigger) {
        auto methodSlot = MethodSlot<HydroponicsFluidReservoir, Hydroponics_TriggerState>(this, &handleFilledTrigger);
        _filledTrigger->getTriggerSignal().detach(methodSlot);
    }
}

void HydroponicsFluidReservoir::handleFilledTrigger(Hydroponics_TriggerState triggerState)
{
    if (triggerState == Hydroponics_TriggerState_Triggered && !getVolumeSensor()) {
        setWaterVolume(_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_FILLED_FRACTION
                                                                                          : _maxVolume, _volumeUnits);
    }
    if (triggerState != Hydroponics_TriggerState_Disabled && _filledState != triggerState) {
        _filledState = triggerState;
        scheduleSignalFireOnce<HydroponicsReservoir *>(_filledSignal, this);
    }
}

void HydroponicsFluidReservoir::attachEmptyTrigger()
{
    HYDRUINO_SOFT_ASSERT(_emptyTrigger, F("Empty trigger not linked, failure attaching"));
    if (_emptyTrigger) {
        auto methodSlot = MethodSlot<HydroponicsFluidReservoir, Hydroponics_TriggerState>(this, &handleEmptyTrigger);
        _emptyTrigger->getTriggerSignal().attach(methodSlot);
    }
}

void HydroponicsFluidReservoir::detachEmptyTrigger()
{
    HYDRUINO_SOFT_ASSERT(_emptyTrigger, F("Empty trigger not linked, failure detaching"));
    if (_emptyTrigger) {
        auto methodSlot = MethodSlot<HydroponicsFluidReservoir, Hydroponics_TriggerState>(this, &handleEmptyTrigger);
        _emptyTrigger->getTriggerSignal().detach(methodSlot);
    }
}

void HydroponicsFluidReservoir::handleEmptyTrigger(Hydroponics_TriggerState triggerState)
{
    if (triggerState == Hydroponics_TriggerState_Triggered && !getVolumeSensor()) {
        setWaterVolume(_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_EMPTY_FRACTION
                                                                                          : 0, _volumeUnits);
    }
    if (triggerState != Hydroponics_TriggerState_Disabled && _emptyState != triggerState) {
        _emptyState = triggerState;
        scheduleSignalFireOnce<HydroponicsReservoir *>(_emptySignal, this);
    }
}

void HydroponicsFluidReservoir::attachWaterVolumeSensor()
{
    HYDRUINO_SOFT_ASSERT(getVolumeSensor(), F("Volume sensor not linked, failure attaching"));
    if (getVolumeSensor()) {
        auto methodSlot = MethodSlot<HydroponicsFluidReservoir, const HydroponicsMeasurement *>(this, &handleWaterVolumeMeasure);
        _volumeSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFluidReservoir::detachWaterVolumeSensor()
{
    HYDRUINO_SOFT_ASSERT(getVolumeSensor(), F("Volume sensor not linked, failure detaching"));
    if (getVolumeSensor()) {
        auto methodSlot = MethodSlot<HydroponicsFluidReservoir, const HydroponicsMeasurement *>(this, &handleWaterVolumeMeasure);
        _volumeSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFluidReservoir::handleWaterVolumeMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        _needsVolumeUpdate = false;
        setWaterVolume(singleMeasurementAt(measurement, 0, _maxVolume, _volumeUnits)); // TODO: Correct row reference, based on sensor
    }
}


HydroponicsFeedReservoir::HydroponicsFeedReservoir(Hydroponics_PositionIndex reservoirIndex,
                                                   float maxVolume,
                                                   DateTime lastChangeDate,
                                                   DateTime lastPruningDate,
                                                   int classType)
    : HydroponicsFluidReservoir(Hydroponics_ReservoirType_FeedWater, reservoirIndex, maxVolume, classType),
       _lastChangeDate(lastChangeDate.unixtime()), _lastPruningDate(lastPruningDate.unixtime()), _lastFeedingDate(0), _numFeedingsToday(0),
       _tdsUnits(defaultConcentrationUnits()), _needsPHUpdate(true), _needsTDSUpdate(true), _needsTempUpdate(true),
       _phBalancer(nullptr), _tdsBalancer(nullptr), _tempBalancer(nullptr)
{ ; }

HydroponicsFeedReservoir::HydroponicsFeedReservoir(const HydroponicsFeedReservoirData *dataIn)
    : HydroponicsFluidReservoir(dataIn), _needsPHUpdate(true), _needsTDSUpdate(true), _needsTempUpdate(true),
      _lastChangeDate(dataIn->lastChangeDate), _lastPruningDate(dataIn->lastPruningDate),
      _lastFeedingDate(dataIn->lastFeedingDate), _numFeedingsToday(dataIn->numFeedingsToday),
      _tdsUnits(dataIn->tdsUnits),
      _phSensor(dataIn->phSensorName), _tdsSensor(dataIn->tdsSensorName), _tempSensor(dataIn->tempSensorName),
      _phBalancer(nullptr), _tdsBalancer(nullptr), _tempBalancer(nullptr)
{
    if (_lastFeedingDate) {
        DateTime lastFeeding = DateTime((uint32_t)_lastFeedingDate);
        auto hydroponics = getHydroponicsInstance();
        lastFeeding = lastFeeding + TimeSpan(0, hydroponics ? hydroponics->getTimeZoneOffset() : 0, 0, 0);
        DateTime currTime = getCurrentTime();

        if (currTime.year() != lastFeeding.year() ||
            currTime.month() != lastFeeding.month() ||
            currTime.day() != lastFeeding.day()) {
            _numFeedingsToday = 0;
        }
    } else { _numFeedingsToday = 0; }
}

HydroponicsFeedReservoir::~HydroponicsFeedReservoir()
{
    if (_phSensor) { detachPHSensor(); }
    if (_tdsSensor) { detachTDSSensor(); }
    if (_tempSensor) { detachWaterTempSensor(); }
    if (_phBalancer) { delete _phBalancer; _phBalancer = nullptr; }
    if (_tdsBalancer) { delete _tdsBalancer; _tdsBalancer = nullptr; }
    if (_tempBalancer) { delete _tempBalancer; _tempBalancer = nullptr; }
}

void HydroponicsFeedReservoir::update()
{
    HydroponicsFluidReservoir::update();

    if (_phBalancer) { _phBalancer->update(); }
    if (_tdsBalancer) { _tdsBalancer->update(); }
    if (_tempBalancer) { _tempBalancer->update(); }

    if (_needsPHUpdate && getWaterPHSensor()) {
        handlePHMeasure(_phSensor->getLatestMeasurement());
    }
    if (_needsTDSUpdate && getWaterTDSSensor()) {
        handleTDSMeasure(_tdsSensor->getLatestMeasurement());
    }
    if (_needsTempUpdate && getWaterTempSensor()) {
        handleWaterTempMeasure(_tempSensor->getLatestMeasurement());
    }
}

void HydroponicsFeedReservoir::resolveLinks()
{
    HydroponicsFluidReservoir::resolveLinks();

    if (_phSensor.needsResolved()) { getWaterPHSensor(); }
    if (_tdsSensor.needsResolved()) { getWaterTDSSensor(); }
    if (_tempSensor.needsResolved()) { getWaterTempSensor(); }
    if (_phBalancer) { _phBalancer->resolveLinks(); }
    if (_tdsBalancer) { _tdsBalancer->resolveLinks(); }
    if (_tempBalancer) { _tempBalancer->resolveLinks(); }
}

void HydroponicsFeedReservoir::handleLowMemory()
{
    HydroponicsFluidReservoir::handleLowMemory();

    if (_phBalancer && !_phBalancer->getIsEnabled()) { setWaterPHBalancer(nullptr); }
    if (_tdsBalancer && !_tdsBalancer->getIsEnabled()) { setWaterTDSBalancer(nullptr); }
    if (_tempBalancer && !_tempBalancer->getIsEnabled()) { setWaterTempBalancer(nullptr); }
}

void HydroponicsFeedReservoir::setTDSUnits(Hydroponics_UnitsType tdsUnits)
{
    if (_tdsUnits != tdsUnits) {
        _tdsUnits = tdsUnits;

        convertStdUnits(&_waterTDS.value, &_waterTDS.units, _tdsUnits);
    }
}

Hydroponics_UnitsType HydroponicsFeedReservoir::getTDSUnits() const
{
    return _tdsUnits;
}

void HydroponicsFeedReservoir::setTemperatureUnits(Hydroponics_UnitsType tempUnits)
{
    if (_tempUnits != tempUnits) {
        _tempUnits = tempUnits;

        convertStdUnits(&_waterTemp.value, &_waterTemp.units, _tempUnits);
    }
}

Hydroponics_UnitsType HydroponicsFeedReservoir::getTemperatureUnits() const
{
    return _tempUnits;
}

void HydroponicsFeedReservoir::setWaterPHSensor(HydroponicsIdentity phSensorId)
{
    if (_phSensor != phSensorId) {
        if (_phSensor) { detachPHSensor(); }
        _phSensor = phSensorId;
        _needsPHUpdate = true;
    }
}

void HydroponicsFeedReservoir::setWaterPHSensor(shared_ptr<HydroponicsSensor> phSensor)
{
    if (_phSensor != phSensor) {
        if (_phSensor) { detachPHSensor(); }
        _phSensor = phSensor;
        if (_phSensor) { attachPHSensor(); }
        _needsPHUpdate = true;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsFeedReservoir::getWaterPHSensor()
{
    if (_phSensor.resolveIfNeeded()) { attachPHSensor(); }
    return _phSensor.getObj();
}

void HydroponicsFeedReservoir::setWaterPH(float waterPH, Hydroponics_UnitsType waterPHUnits)
{
    _waterPH.value = waterPH;
    _waterPH.units = waterPHUnits != Hydroponics_UnitsType_Undefined ? waterPHUnits : Hydroponics_UnitsType_pHScale_0_14;
    _waterPH.updateTimestamp();
    _waterPH.updateFrame(1);

    convertStdUnits(&_waterPH.value, &_waterPH.units, Hydroponics_UnitsType_pHScale_0_14);
}

void HydroponicsFeedReservoir::setWaterPH(HydroponicsSingleMeasurement waterPH)
{
    _waterPH = waterPH;

    convertStdUnits(&_waterPH.value, &_waterPH.units, Hydroponics_UnitsType_pHScale_0_14);
}

const HydroponicsSingleMeasurement &HydroponicsFeedReservoir::getWaterPH()
{
    if (_needsPHUpdate && getWaterPHSensor()) {
        handlePHMeasure(_phSensor->getLatestMeasurement());
    }
    return _waterPH;
}

void HydroponicsFeedReservoir::setWaterTDSSensor(HydroponicsIdentity tdsSensorId)
{
    if (_tdsSensor != tdsSensorId) {
        if (_tdsSensor) { detachTDSSensor(); }
        _tdsSensor = tdsSensorId;
        _needsTDSUpdate = true;
    }
}

void HydroponicsFeedReservoir::setWaterTDSSensor(shared_ptr<HydroponicsSensor> tdsSensor)
{
    if (_tdsSensor != tdsSensor) {
        if (_tdsSensor) { detachTDSSensor(); }
        _tdsSensor = tdsSensor;
        if (_tdsSensor) { attachTDSSensor(); }
        _needsTDSUpdate = true;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsFeedReservoir::getWaterTDSSensor()
{
    if (_tdsSensor.resolveIfNeeded()) { attachTDSSensor(); }
    return _tdsSensor.getObj();
}

void HydroponicsFeedReservoir::setWaterTDS(float waterTDS, Hydroponics_UnitsType waterTDSUnits)
{
    _waterTDS.value = waterTDS;
    _waterTDS.units = waterTDSUnits != Hydroponics_UnitsType_Undefined ? waterTDSUnits
                                                                       : (_tdsUnits != Hydroponics_UnitsType_Undefined ? _tdsUnits
                                                                                                                       : defaultConcentrationUnits());
    _waterTDS.updateTimestamp();
    _waterTDS.updateFrame(1);

    convertStdUnits(&_waterTDS.value, &_waterTDS.units, _tdsUnits);
}

void HydroponicsFeedReservoir::setWaterTDS(HydroponicsSingleMeasurement waterTDS)
{
    _waterTDS = waterTDS;

    convertStdUnits(&_waterTDS.value, &_waterTDS.units, _tdsUnits);
}

const HydroponicsSingleMeasurement &HydroponicsFeedReservoir::getWaterTDS()
{
    if (_needsTDSUpdate && getWaterTDSSensor()) {
        handleTDSMeasure(_tdsSensor->getLatestMeasurement());
    }
    return _waterTDS;
}

void HydroponicsFeedReservoir::setWaterTempSensor(HydroponicsIdentity waterTempSensorId)
{
    if (_tempSensor != waterTempSensorId) {
        if (_tempSensor) { detachWaterTempSensor(); }
        _tempSensor = waterTempSensorId;
        _needsTempUpdate = true;
    }
}

void HydroponicsFeedReservoir::setWaterTempSensor(shared_ptr<HydroponicsSensor> waterTempSensor)
{
    if (_tempSensor != waterTempSensor) {
        if (_tempSensor) { detachWaterTempSensor(); }
        _tempSensor = waterTempSensor;
        if (_tempSensor) { attachWaterTempSensor(); }
        _needsTempUpdate = true;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsFeedReservoir::getWaterTempSensor()
{
    if (_tempSensor.resolveIfNeeded()) { attachWaterTempSensor(); }
    return _tempSensor.getObj();
}

void HydroponicsFeedReservoir::setWaterTemperature(float waterTemperature, Hydroponics_UnitsType waterTempUnits)
{
    _waterTemp.value = waterTemperature;
    _waterTemp.units = waterTempUnits != Hydroponics_UnitsType_Undefined ? waterTempUnits
                                                                         : (_tempUnits != Hydroponics_UnitsType_Undefined ? _tempUnits
                                                                                                                          : defaultTemperatureUnits());
    _waterTemp.updateTimestamp();
    _waterTemp.updateFrame(1);

    convertStdUnits(&_waterTemp.value, &_waterTemp.units, _tempUnits);
}

void HydroponicsFeedReservoir::setWaterTemperature(HydroponicsSingleMeasurement waterTemperature)
{
    _waterTemp = waterTemperature;

    convertStdUnits(&_waterTemp.value, &_waterTemp.units, _tempUnits);
}

const HydroponicsSingleMeasurement &HydroponicsFeedReservoir::getWaterTemperature()
{
    if (_needsTempUpdate && getWaterTempSensor()) {
        handleWaterTempMeasure(_tempSensor->getLatestMeasurement());
    }
    return _waterTemp;
}

HydroponicsBalancer *HydroponicsFeedReservoir::setWaterPHBalancer(float phSetpoint, Hydroponics_UnitsType phSetpointUnits)
{
    if (!_phBalancer && getWaterPHSensor()) {
        _phBalancer = new HydroponicsTimedDosingBalancer(getWaterPHSensor(), phSetpoint, HYDRUINO_CROP_PH_RANGE_HALF, _maxVolume, _volumeUnits);
        HYDRUINO_SOFT_ASSERT(_phBalancer, F("Failure allocating pH balancer"));
    }
    if (_phBalancer) {
        _phBalancer->setTargetSetpoint(phSetpoint);
        _phBalancer->setTargetUnits(phSetpointUnits);
    }
    return _phBalancer;
}

void HydroponicsFeedReservoir::setWaterPHBalancer(HydroponicsBalancer *phBalancer)
{
    if (_phBalancer != phBalancer) {
        if (_phBalancer) { delete _phBalancer; }
        _phBalancer = phBalancer;
    }
}

HydroponicsBalancer *HydroponicsFeedReservoir::getWaterPHBalancer() const
{
    return _phBalancer;
}

HydroponicsBalancer *HydroponicsFeedReservoir::setWaterTDSBalancer(float tdsSetpoint, Hydroponics_UnitsType tdsSetpointUnits)
{
    if (!_tdsBalancer && getWaterTDSSensor()) {
        _tdsBalancer = new HydroponicsTimedDosingBalancer(getWaterTDSSensor(), tdsSetpoint, HYDRUINO_CROP_EC_RANGE_HALF, _maxVolume, _volumeUnits);
        HYDRUINO_SOFT_ASSERT(_tdsBalancer, F("Failure allocating TDS balancer"));
    }
    if (_tdsBalancer) {
        _tdsBalancer->setTargetSetpoint(tdsSetpoint);
        _tdsBalancer->setTargetUnits(tdsSetpointUnits);
    }
    return _tdsBalancer;
}

void HydroponicsFeedReservoir::setWaterTDSBalancer(HydroponicsBalancer *tdsBalancer)
{
    if (_tdsBalancer != tdsBalancer) {
        if (_tdsBalancer) { delete _tdsBalancer; }
        _tdsBalancer = tdsBalancer;
    }
}

HydroponicsBalancer *HydroponicsFeedReservoir::getWaterTDSBalancer() const
{
    return _tdsBalancer;
}

HydroponicsBalancer *HydroponicsFeedReservoir::setWaterTempBalancer(float tempSetpoint, Hydroponics_UnitsType tempSetpointUnits)
{
    if (!_tempBalancer && getWaterTempSensor()) {
        auto tempRangeQuad = HYDRUINO_CROP_TEMP_RANGE_HALF * 0.5f;
        _tempBalancer = new HydroponicsLinearEdgeBalancer(getWaterTempSensor(), tempSetpoint, HYDRUINO_CROP_TEMP_RANGE_HALF, tempRangeQuad * 0.5f, tempRangeQuad);
        HYDRUINO_SOFT_ASSERT(_tempBalancer, F("Failure allocating TDS balancer"));
    }
    if (_tempBalancer) {
        _tempBalancer->setTargetSetpoint(tempSetpoint);
        _tempBalancer->setTargetUnits(tempSetpointUnits);
    }
    return _tempBalancer;
}

void HydroponicsFeedReservoir::setWaterTempBalancer(HydroponicsBalancer *tempBalancer)
{
    if (_tempBalancer != tempBalancer) {
        if (_tempBalancer) { delete _tempBalancer; }
        _tempBalancer = tempBalancer;
    }
}

HydroponicsBalancer *HydroponicsFeedReservoir::getWaterTempBalancer() const
{
    return _tempBalancer;
}

Hydroponics_PositionIndex HydroponicsFeedReservoir::getChannelNumber() const
{
    return _id.posIndex;
}

DateTime HydroponicsFeedReservoir::getLastWaterChangeDate() const
{
    return DateTime((uint32_t)_lastChangeDate);
}

void HydroponicsFeedReservoir::notifyWaterChanged()
{
    _lastChangeDate = now();
}

DateTime HydroponicsFeedReservoir::getLastPruningDate() const
{
    return DateTime((uint32_t)_lastPruningDate);
}

void HydroponicsFeedReservoir::notifyPruningCompleted()
{
    _lastPruningDate = now();
}

DateTime HydroponicsFeedReservoir::getLastFeeding() const
{
    return DateTime((uint32_t)_lastFeedingDate);
}

int HydroponicsFeedReservoir::getFeedingsToday() const
{
    return _numFeedingsToday;
}

void HydroponicsFeedReservoir::notifyFeedingBegan()
{
    _numFeedingsToday++;
    _lastFeedingDate = now();
}

void HydroponicsFeedReservoir::notifyFeedingEnded()
{ ; }

void HydroponicsFeedReservoir::notifyDayChanged()
{
    _numFeedingsToday = 0;
}

void HydroponicsFeedReservoir::saveToData(HydroponicsData *dataOut)
{
    HydroponicsFluidReservoir::saveToData(dataOut);

    ((HydroponicsFeedReservoirData *)dataOut)->lastChangeDate = _lastChangeDate;
    ((HydroponicsFeedReservoirData *)dataOut)->lastPruningDate = _lastPruningDate;
    ((HydroponicsFeedReservoirData *)dataOut)->lastFeedingDate = _lastFeedingDate;
    ((HydroponicsFeedReservoirData *)dataOut)->numFeedingsToday = _numFeedingsToday;
    ((HydroponicsFeedReservoirData *)dataOut)->tdsUnits = _tdsUnits;
    ((HydroponicsFeedReservoirData *)dataOut)->tempUnits = _tempUnits;
    if (_phSensor.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->phSensorName, _phSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_tdsSensor.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->tdsSensorName, _tdsSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_tempSensor.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->tempSensorName, _tempSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}

void HydroponicsFeedReservoir::attachPHSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterPHSensor(), F("PH sensor not linked, failure attaching"));
    if (getWaterPHSensor()) {
        auto methodSlot = MethodSlot<HydroponicsFeedReservoir, const HydroponicsMeasurement *>(this, &handlePHMeasure);
        _phSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFeedReservoir::detachPHSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterPHSensor(), F("PH sensor not linked, failure detaching"));
    if (getWaterPHSensor()) {
        auto methodSlot = MethodSlot<HydroponicsFeedReservoir, const HydroponicsMeasurement *>(this, &handlePHMeasure);
        _phSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFeedReservoir::handlePHMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        _needsPHUpdate = false;
        setWaterPH(singleMeasurementAt(measurement, 0)); // TODO: Correct row reference, based on sensor
    }
}

void HydroponicsFeedReservoir::attachTDSSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterTDSSensor(), F("TDS sensor not linked, failure attaching"));
    if (getWaterTDSSensor()) {
        auto methodSlot = MethodSlot<HydroponicsFeedReservoir, const HydroponicsMeasurement *>(this, &handleTDSMeasure);
        _tdsSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFeedReservoir::detachTDSSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterTDSSensor(), F("TDS sensor not linked, failure detaching"));
    if (getWaterTDSSensor()) {
        auto methodSlot = MethodSlot<HydroponicsFeedReservoir, const HydroponicsMeasurement *>(this, &handleTDSMeasure);
        _tdsSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFeedReservoir::handleTDSMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        _needsTDSUpdate = false;
        setWaterTDS(singleMeasurementAt(measurement, 0)); // TODO: Correct row reference, based on sensor
    }
}

void HydroponicsFeedReservoir::attachWaterTempSensor()
{
    HYDRUINO_SOFT_ASSERT(_tempSensor, F("Temperature sensor not linked, failure attaching"));
    if (_tempSensor) {
        auto methodSlot = MethodSlot<HydroponicsFeedReservoir, const HydroponicsMeasurement *>(this, &handleWaterTempMeasure);
        _tempSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFeedReservoir::detachWaterTempSensor()
{
    HYDRUINO_SOFT_ASSERT(_tempSensor, F("Temperature sensor not linked, failure detaching"));
    if (_tempSensor) {
        auto methodSlot = MethodSlot<HydroponicsFeedReservoir, const HydroponicsMeasurement *>(this, &handleWaterTempMeasure);
        _tempSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFeedReservoir::handleWaterTempMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        _needsTempUpdate = false;
        setWaterTemperature(singleMeasurementAt(measurement, 0)); // TODO: Correct row reference, based on sensor
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

bool HydroponicsInfiniteReservoir::getIsFilled() const
{
    return _alwaysFilled;
}

bool HydroponicsInfiniteReservoir::getIsEmpty() const
{
    return !_alwaysFilled;
}

void HydroponicsInfiniteReservoir::saveToData(HydroponicsData *dataOut)
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
    : HydroponicsReservoirData(), maxVolume(0), volumeUnits(Hydroponics_UnitsType_Undefined), volumeSensorName{0}
{
    _size = sizeof(*this);
}

void HydroponicsFluidReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsReservoirData::toJSONObject(objectOut);

    objectOut[F("maxVolume")] = maxVolume;
    if (volumeUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("volumeUnits")] = volumeUnits; }
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
    const char *volumeSensorNameStr = objectIn[F("volumeSensorName")];
    if (volumeSensorNameStr && volumeSensorNameStr[0]) { strncpy(volumeSensorName, volumeSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    JsonObjectConst filledTriggerObj = objectIn[F("filledTrigger")];
    if (!filledTriggerObj.isNull()) { filledTrigger.fromJSONObject(filledTriggerObj); }
    JsonObjectConst emptyTriggerObj = objectIn[F("emptyTrigger")];
    if (!emptyTriggerObj.isNull()) { emptyTrigger.fromJSONObject(emptyTriggerObj); }
}

HydroponicsFeedReservoirData::HydroponicsFeedReservoirData()
    : HydroponicsFluidReservoirData(), lastChangeDate(0), lastPruningDate(0), lastFeedingDate(0), numFeedingsToday(0),
      tdsUnits(Hydroponics_UnitsType_Undefined), tempUnits(Hydroponics_UnitsType_Undefined),
      phSensorName{0}, tdsSensorName{0}, tempSensorName{0}
{
    _size = sizeof(*this);
}

void HydroponicsFeedReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsFluidReservoirData::toJSONObject(objectOut);

    if (lastChangeDate) { objectOut[F("lastChangeDate")] = lastChangeDate; }
    if (lastPruningDate) { objectOut[F("lastPruningDate")] = lastPruningDate; }
    if (lastFeedingDate) { objectOut[F("lastFeedingDate")] = lastFeedingDate; }
    if (numFeedingsToday > 0) { objectOut[F("numFeedingsToday")] = numFeedingsToday; }
    if (tdsUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("tdsUnits")] = tdsUnits; }
    if (tempUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("tempUnits")] = tempUnits; }
    if (phSensorName[0]) { objectOut[F("phSensorName")] = stringFromChars(phSensorName, HYDRUINO_NAME_MAXSIZE); }
    if (tdsSensorName[0]) { objectOut[F("tdsSensorName")] = stringFromChars(tdsSensorName, HYDRUINO_NAME_MAXSIZE); }
    if (tempSensorName[0]) { objectOut[F("tempSensorName")] = stringFromChars(tempSensorName, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsFeedReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsFluidReservoirData::fromJSONObject(objectIn);

    lastChangeDate = objectIn[F("lastChangeDate")] | lastChangeDate;
    lastPruningDate = objectIn[F("lastPruningDate")] | lastPruningDate;
    lastFeedingDate = objectIn[F("lastFeedingDate")] | lastFeedingDate;
    numFeedingsToday = objectIn[F("numFeedingsToday")] | numFeedingsToday;
    tdsUnits = objectIn[F("tdsUnits")] | tdsUnits;
    tempUnits = objectIn[F("tempUnits")] | tempUnits;
    const char *phSensorNameStr = objectIn[F("phSensorName")];
    if (phSensorNameStr && phSensorNameStr[0]) { strncpy(phSensorName, phSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    const char *tdsSensorNameStr = objectIn[F("tdsSensorName")];
    if (tdsSensorNameStr && tdsSensorNameStr[0]) { strncpy(tdsSensorName, tdsSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    const char *tempSensorNameStr = objectIn[F("tempSensorName")];
    if (tempSensorNameStr && tempSensorNameStr[0]) { strncpy(tempSensorName, tempSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
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
