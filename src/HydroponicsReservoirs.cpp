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
      _volumeUnits(defaultWaterVolumeUnits()),
      _filledState(Hydroponics_TriggerState_NotTriggered), _emptyState(Hydroponics_TriggerState_NotTriggered)
{ ; }

HydroponicsReservoir::HydroponicsReservoir(const HydroponicsReservoirData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _volumeUnits(definedUnitsElse(dataIn->volumeUnits, defaultWaterVolumeUnits())),
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
        scheduleSignalFireOnce<HydroponicsReservoir *>(getSharedPtr(), _filledSignal, this);
    }
    if (_emptyState != triggerStateFromBool(getIsEmpty())) {
        _emptyState = triggerStateFromBool(getIsEmpty());
        scheduleSignalFireOnce<HydroponicsReservoir *>(getSharedPtr(), _emptySignal, this);
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

void HydroponicsReservoir::setVolumeUnits(Hydroponics_UnitsType volumeUnits)
{
    if (_volumeUnits != volumeUnits) {
        _volumeUnits = volumeUnits;
    }
}

Hydroponics_UnitsType HydroponicsReservoir::getVolumeUnits() const
{
    return _volumeUnits;
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
    ((HydroponicsReservoirData *)dataOut)->volumeUnits = _volumeUnits;
}


HydroponicsFluidReservoir::HydroponicsFluidReservoir(Hydroponics_ReservoirType reservoirType,
                                                     Hydroponics_PositionIndex reservoirIndex,
                                                     float maxVolume,
                                                     int classType)
    : HydroponicsReservoir(reservoirType, reservoirIndex, classType),
      _maxVolume(maxVolume), _needsVolumeUpdate(true),
      _filledTrigger(nullptr), _emptyTrigger(nullptr)
{ ; }

HydroponicsFluidReservoir::HydroponicsFluidReservoir(const HydroponicsFluidReservoirData *dataIn)
    : HydroponicsReservoir(dataIn), _needsVolumeUpdate(true),
      _maxVolume(dataIn->maxVolume),
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

        convertUnits(&_waterVolume, _volumeUnits);
    }
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
    _waterVolume.units = definedUnitsElse(waterVolumeUnits, _volumeUnits, defaultWaterVolumeUnits());
    _waterVolume.updateTimestamp();
    _waterVolume.updateFrame(1);

    convertUnits(&_waterVolume, _volumeUnits);
}

void HydroponicsFluidReservoir::setWaterVolume(HydroponicsSingleMeasurement waterVolume)
{
    _waterVolume = waterVolume;
    _waterVolume.setMinFrame(1);

    convertUnits(&_waterVolume, _volumeUnits);
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

    ((HydroponicsFluidReservoirData *)dataOut)->maxVolume = roundForExport(_maxVolume, 1);
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
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &handleFilledTrigger);
        _filledTrigger->getTriggerSignal().attach(methodSlot);
    }
}

void HydroponicsFluidReservoir::detachFilledTrigger()
{
    HYDRUINO_SOFT_ASSERT(_filledTrigger, F("Filled trigger not linked, failure detaching"));
    if (_filledTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &handleFilledTrigger);
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
        scheduleSignalFireOnce<HydroponicsReservoir *>(getSharedPtr(), _filledSignal, this);
    }
}

void HydroponicsFluidReservoir::attachEmptyTrigger()
{
    HYDRUINO_SOFT_ASSERT(_emptyTrigger, F("Empty trigger not linked, failure attaching"));
    if (_emptyTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &handleEmptyTrigger);
        _emptyTrigger->getTriggerSignal().attach(methodSlot);
    }
}

void HydroponicsFluidReservoir::detachEmptyTrigger()
{
    HYDRUINO_SOFT_ASSERT(_emptyTrigger, F("Empty trigger not linked, failure detaching"));
    if (_emptyTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &handleEmptyTrigger);
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
        scheduleSignalFireOnce<HydroponicsReservoir *>(getSharedPtr(), _emptySignal, this);
    }
}

void HydroponicsFluidReservoir::attachWaterVolumeSensor()
{
    HYDRUINO_SOFT_ASSERT(getVolumeSensor(), F("Volume sensor not linked, failure attaching"));
    if (getVolumeSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleWaterVolumeMeasure);
        _volumeSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFluidReservoir::detachWaterVolumeSensor()
{
    HYDRUINO_SOFT_ASSERT(getVolumeSensor(), F("Volume sensor not linked, failure detaching"));
    if (getVolumeSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleWaterVolumeMeasure);
        _volumeSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFluidReservoir::handleWaterVolumeMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        _needsVolumeUpdate = false;
        setWaterVolume(singleMeasurementAt(measurement, 0, _maxVolume, _volumeUnits));
    }
}


HydroponicsFeedReservoir::HydroponicsFeedReservoir(Hydroponics_PositionIndex reservoirIndex,
                                                   float maxVolume,
                                                   DateTime lastChangeDate,
                                                   DateTime lastPruningDate,
                                                   int classType)
    : HydroponicsFluidReservoir(Hydroponics_ReservoirType_FeedWater, reservoirIndex, maxVolume, classType),
       _lastChangeDate(lastChangeDate.unixtime()), _lastPruningDate(lastPruningDate.unixtime()), _lastFeedingDate(0), _numFeedingsToday(0),
       _tdsUnits(Hydroponics_UnitsType_Concentration_EC), _tempUnits(defaultTemperatureUnits()),
       _needsWaterPHUpdate(true), _needsWaterTDSUpdate(true), _needsWaterTempUpdate(true), _needsAirTempUpdate(true), _needsAirCO2Update(true),
       _waterPHBalancer(nullptr), _waterTDSBalancer(nullptr), _waterTempBalancer(nullptr), _airTempBalancer(nullptr), _airCO2Balancer(nullptr)
{ ; }

HydroponicsFeedReservoir::HydroponicsFeedReservoir(const HydroponicsFeedReservoirData *dataIn)
    : HydroponicsFluidReservoir(dataIn),
      _lastChangeDate(dataIn->lastChangeDate), _lastPruningDate(dataIn->lastPruningDate),
      _lastFeedingDate(dataIn->lastFeedingDate), _numFeedingsToday(dataIn->numFeedingsToday),
      _tdsUnits(definedUnitsElse(dataIn->tdsUnits, Hydroponics_UnitsType_Concentration_EC)),
      _tempUnits(definedUnitsElse(dataIn->tempUnits, defaultTemperatureUnits())),
      _waterPHSensor(dataIn->waterPHSensorName), _waterTDSSensor(dataIn->waterTDSSensorName), _waterTempSensor(dataIn->waterTempSensorName),
      _airTempSensor(dataIn->airTempSensorName), _airCO2Sensor(dataIn->airCO2SensorName),
      _needsWaterPHUpdate(true), _needsWaterTDSUpdate(true), _needsWaterTempUpdate(true), _needsAirTempUpdate(true), _needsAirCO2Update(true),
      _waterPHBalancer(nullptr), _waterTDSBalancer(nullptr), _waterTempBalancer(nullptr), _airTempBalancer(nullptr), _airCO2Balancer(nullptr)
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
    if (_waterPHSensor) { detachWaterPHSensor(); }
    if (_waterTDSSensor) { detachWaterTDSSensor(); }
    if (_waterTempSensor) { detachWaterTempSensor(); }
    if (_airTempSensor) { detachAirTempSensor(); }
    if (_airCO2Sensor) { detachAirCO2Sensor(); }
    if (_waterPHBalancer) { delete _waterPHBalancer; _waterPHBalancer = nullptr; }
    if (_waterTDSBalancer) { delete _waterTDSBalancer; _waterTDSBalancer = nullptr; }
    if (_waterTempBalancer) { delete _waterTempBalancer; _waterTempBalancer = nullptr; }
    if (_airTempBalancer) { delete _airTempBalancer; _airTempBalancer = nullptr; }
    if (_airCO2Balancer) { delete _airCO2Balancer; _airCO2Balancer = nullptr; }
}

void HydroponicsFeedReservoir::update()
{
    HydroponicsFluidReservoir::update();

    if (_waterPHBalancer) { _waterPHBalancer->update(); }
    if (_waterTDSBalancer) { _waterTDSBalancer->update(); }
    if (_waterTempBalancer) { _waterTempBalancer->update(); }
    if (_airTempBalancer) { _airTempBalancer->update(); }
    if (_airCO2Balancer) { _airCO2Balancer->update(); }

    if (_needsWaterPHUpdate && getWaterPHSensor()) {
        handleWaterPHMeasure(_waterPHSensor->getLatestMeasurement());
    }
    if (_needsWaterTDSUpdate && getWaterTDSSensor()) {
        handleWaterTDSMeasure(_waterTDSSensor->getLatestMeasurement());
    }
    if (_needsWaterTempUpdate && getWaterTempSensor()) {
        handleWaterTempMeasure(_waterTempSensor->getLatestMeasurement());
    }
    if (_needsAirTempUpdate && getAirTempSensor()) {
        handleAirTempMeasure(_airTempSensor->getLatestMeasurement());
    }
    if (_needsAirCO2Update && getAirCO2Sensor()) {
        handleAirCO2Measure(_airCO2Sensor->getLatestMeasurement());
    }
}

void HydroponicsFeedReservoir::resolveLinks()
{
    HydroponicsFluidReservoir::resolveLinks();

    if (_waterPHSensor.needsResolved()) { getWaterPHSensor(); }
    if (_waterTDSSensor.needsResolved()) { getWaterTDSSensor(); }
    if (_waterTempSensor.needsResolved()) { getWaterTempSensor(); }
    if (_airTempSensor.needsResolved()) { getAirTempSensor(); }
    if (_airCO2Sensor.needsResolved()) { getAirCO2Sensor(); }
    if (_waterPHBalancer) { _waterPHBalancer->resolveLinks(); }
    if (_waterTDSBalancer) { _waterTDSBalancer->resolveLinks(); }
    if (_waterTempBalancer) { _waterTempBalancer->resolveLinks(); }
    if (_airTempBalancer) { _airTempBalancer->resolveLinks(); }
    if (_airCO2Balancer) { _airCO2Balancer->resolveLinks(); }
}

void HydroponicsFeedReservoir::handleLowMemory()
{
    HydroponicsFluidReservoir::handleLowMemory();

    if (_waterPHBalancer && !_waterPHBalancer->getIsEnabled()) { setWaterPHBalancer(nullptr); }
    if (_waterTDSBalancer && !_waterTDSBalancer->getIsEnabled()) { setWaterTDSBalancer(nullptr); }
    if (_waterTempBalancer && !_waterTempBalancer->getIsEnabled()) { setWaterTempBalancer(nullptr); }
    if (_airTempBalancer && !_airTempBalancer->getIsEnabled()) { setAirTempBalancer(nullptr); }
    if (_airCO2Balancer && !_airCO2Balancer->getIsEnabled()) { setAirCO2Balancer(nullptr); }
}

void HydroponicsFeedReservoir::setTDSUnits(Hydroponics_UnitsType tdsUnits)
{
    if (_tdsUnits != tdsUnits) {
        _tdsUnits = tdsUnits;

        convertUnits(&_waterTDS, _tdsUnits);
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

        convertUnits(&_waterTemp, _tempUnits);
        convertUnits(&_airTemp, _tempUnits);
    }
}

Hydroponics_UnitsType HydroponicsFeedReservoir::getTemperatureUnits() const
{
    return _tempUnits;
}

void HydroponicsFeedReservoir::setWaterPHSensor(HydroponicsIdentity waterPHSensorId)
{
    if (_waterPHSensor != waterPHSensorId) {
        if (_waterPHSensor) { detachWaterPHSensor(); }
        _waterPHSensor = waterPHSensorId;
        _needsWaterPHUpdate = true;
    }
}

void HydroponicsFeedReservoir::setWaterPHSensor(shared_ptr<HydroponicsSensor> waterPHSensor)
{
    if (_waterPHSensor != waterPHSensor) {
        if (_waterPHSensor) { detachWaterPHSensor(); }
        _waterPHSensor = waterPHSensor;
        if (_waterPHSensor) { attachWaterPHSensor(); }
        _needsWaterPHUpdate = true;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsFeedReservoir::getWaterPHSensor()
{
    if (_waterPHSensor.resolveIfNeeded()) { attachWaterPHSensor(); }
    return _waterPHSensor.getObj();
}

void HydroponicsFeedReservoir::setWaterPH(float waterPH, Hydroponics_UnitsType waterPHUnits)
{
    _waterPH.value = waterPH;
    _waterPH.units = definedUnitsElse(waterPHUnits, Hydroponics_UnitsType_pHScale_0_14);
    _waterPH.updateTimestamp();
    _waterPH.updateFrame(1);

    convertUnits(&_waterPH, Hydroponics_UnitsType_pHScale_0_14);
}

void HydroponicsFeedReservoir::setWaterPH(HydroponicsSingleMeasurement waterPH)
{
    _waterPH = waterPH;
    _waterPH.setMinFrame(1);

    convertUnits(&_waterPH, Hydroponics_UnitsType_pHScale_0_14);
}

const HydroponicsSingleMeasurement &HydroponicsFeedReservoir::getWaterPH()
{
    if (_needsWaterPHUpdate && getWaterPHSensor()) {
        handleWaterPHMeasure(_waterPHSensor->getLatestMeasurement());
    }
    return _waterPH;
}

void HydroponicsFeedReservoir::setWaterTDSSensor(HydroponicsIdentity waterTDSSensorId)
{
    if (_waterTDSSensor != waterTDSSensorId) {
        if (_waterTDSSensor) { detachWaterTDSSensor(); }
        _waterTDSSensor = waterTDSSensorId;
        _needsWaterTDSUpdate = true;
    }
}

void HydroponicsFeedReservoir::setWaterTDSSensor(shared_ptr<HydroponicsSensor> waterTDSSensor)
{
    if (_waterTDSSensor != waterTDSSensor) {
        if (_waterTDSSensor) { detachWaterTDSSensor(); }
        _waterTDSSensor = waterTDSSensor;
        if (_waterTDSSensor) { attachWaterTDSSensor(); }
        _needsWaterTDSUpdate = true;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsFeedReservoir::getWaterTDSSensor()
{
    if (_waterTDSSensor.resolveIfNeeded()) { attachWaterTDSSensor(); }
    return _waterTDSSensor.getObj();
}

void HydroponicsFeedReservoir::setWaterTDS(float waterTDS, Hydroponics_UnitsType waterTDSUnits)
{
    _waterTDS.value = waterTDS;
    _waterTDS.units = definedUnitsElse(waterTDSUnits, _tdsUnits, Hydroponics_UnitsType_Concentration_EC);
    _waterTDS.updateTimestamp();
    _waterTDS.updateFrame(1);

    convertUnits(&_waterTDS, _tdsUnits);
}

void HydroponicsFeedReservoir::setWaterTDS(HydroponicsSingleMeasurement waterTDS)
{
    _waterTDS = waterTDS;
    _waterTDS.setMinFrame(1);

    convertUnits(&_waterTDS, _tdsUnits);
}

const HydroponicsSingleMeasurement &HydroponicsFeedReservoir::getWaterTDS()
{
    if (_needsWaterTDSUpdate && getWaterTDSSensor()) {
        handleWaterTDSMeasure(_waterTDSSensor->getLatestMeasurement());
    }
    return _waterTDS;
}

void HydroponicsFeedReservoir::setWaterTempSensor(HydroponicsIdentity waterTempSensorId)
{
    if (_waterTempSensor != waterTempSensorId) {
        if (_waterTempSensor) { detachWaterTempSensor(); }
        _waterTempSensor = waterTempSensorId;
        _needsWaterTempUpdate = true;
    }
}

void HydroponicsFeedReservoir::setWaterTempSensor(shared_ptr<HydroponicsSensor> waterTempSensor)
{
    if (_waterTempSensor != waterTempSensor) {
        if (_waterTempSensor) { detachWaterTempSensor(); }
        _waterTempSensor = waterTempSensor;
        if (_waterTempSensor) { attachWaterTempSensor(); }
        _needsWaterTempUpdate = true;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsFeedReservoir::getWaterTempSensor()
{
    if (_waterTempSensor.resolveIfNeeded()) { attachWaterTempSensor(); }
    return _waterTempSensor.getObj();
}

void HydroponicsFeedReservoir::setWaterTemperature(float waterTemperature, Hydroponics_UnitsType waterTempUnits)
{
    _waterTemp.value = waterTemperature;
    _waterTemp.units = definedUnitsElse(waterTempUnits, _tempUnits, defaultTemperatureUnits());
    _waterTemp.updateTimestamp();
    _waterTemp.updateFrame(1);

    convertUnits(&_waterTemp, _tempUnits);
}

void HydroponicsFeedReservoir::setWaterTemperature(HydroponicsSingleMeasurement waterTemperature)
{
    _waterTemp = waterTemperature;
    _waterTemp.setMinFrame(1);

    convertUnits(&_waterTemp, _tempUnits);
}

const HydroponicsSingleMeasurement &HydroponicsFeedReservoir::getWaterTemperature()
{
    if (_needsWaterTempUpdate && getWaterTempSensor()) {
        handleWaterTempMeasure(_waterTempSensor->getLatestMeasurement());
    }
    return _waterTemp;
}

void HydroponicsFeedReservoir::setAirTempSensor(HydroponicsIdentity airTempSensorId)
{
    if (_airTempSensor != airTempSensorId) {
        if (_airTempSensor) { detachAirTempSensor(); }
        _airTempSensor = airTempSensorId;
        _needsAirTempUpdate = true;
    }
}

void HydroponicsFeedReservoir::setAirTempSensor(shared_ptr<HydroponicsSensor> airTempSensor)
{
    if (_airTempSensor != airTempSensor) {
        if (_airTempSensor) { detachAirTempSensor(); }
        _airTempSensor = airTempSensor;
        if (_airTempSensor) { attachAirTempSensor(); }
        _needsAirTempUpdate = true;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsFeedReservoir::getAirTempSensor()
{
    if (_airTempSensor.resolveIfNeeded()) { attachAirTempSensor(); }
    return _airTempSensor.getObj();
}

void HydroponicsFeedReservoir::setAirTemperature(float airTemperature, Hydroponics_UnitsType airTempUnits)
{
    _airTemp.value = airTemperature;
    _airTemp.units = definedUnitsElse(airTempUnits, _tempUnits);
    _airTemp.updateTimestamp();
    _airTemp.updateFrame(1);

    convertUnits(&_airTemp, _tempUnits);
}

void HydroponicsFeedReservoir::setAirTemperature(HydroponicsSingleMeasurement airTemperature)
{
    _airTemp = airTemperature;
    _airTemp.setMinFrame(1);

    convertUnits(&_airTemp, _tempUnits);
}

const HydroponicsSingleMeasurement &HydroponicsFeedReservoir::getAirTemperature()
{
    if (_needsAirTempUpdate && getAirTempSensor()) {
        handleAirTempMeasure(_airTempSensor->getLatestMeasurement());
    }
    return _airTemp;
}

void HydroponicsFeedReservoir::setAirCO2Sensor(HydroponicsIdentity airCO2SensorId)
{
    if (_airCO2Sensor != airCO2SensorId) {
        if (_airCO2Sensor) { detachAirCO2Sensor(); }
        _airCO2Sensor = airCO2SensorId;
        _needsAirCO2Update = true;
    }
}

void HydroponicsFeedReservoir::setAirCO2Sensor(shared_ptr<HydroponicsSensor> airCO2Sensor)
{
    if (_airCO2Sensor != airCO2Sensor) {
        if (_airCO2Sensor) { detachAirCO2Sensor(); }
        _airCO2Sensor = airCO2Sensor;
        if (_airCO2Sensor) { attachAirCO2Sensor(); }
        _needsAirCO2Update = true;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsFeedReservoir::getAirCO2Sensor()
{
    if (_airCO2Sensor.resolveIfNeeded()) { attachAirCO2Sensor(); }
    return _airCO2Sensor.getObj();
}

void HydroponicsFeedReservoir::setAirCO2(float airCO2, Hydroponics_UnitsType airCO2Units)
{
    _airCO2.value = airCO2;
    _airCO2.units = definedUnitsElse(airCO2Units, Hydroponics_UnitsType_Concentration_PPM);
    _airCO2.updateTimestamp();
    _airCO2.updateFrame(1);

    convertUnits(&_airCO2, Hydroponics_UnitsType_Concentration_PPM);
}

void HydroponicsFeedReservoir::setAirCO2(HydroponicsSingleMeasurement airCO2)
{
    _airCO2 = airCO2;
    _airCO2.setMinFrame(1);

    convertUnits(&_airCO2, Hydroponics_UnitsType_Concentration_PPM);
}

const HydroponicsSingleMeasurement &HydroponicsFeedReservoir::getAirCO2()
{
    if (_needsAirCO2Update && getAirCO2Sensor()) {
        handleAirCO2Measure(_airCO2Sensor->getLatestMeasurement());
    }
    return _airCO2;
}

HydroponicsBalancer *HydroponicsFeedReservoir::setWaterPHBalancer(float phSetpoint, Hydroponics_UnitsType phSetpointUnits)
{
    if (!_waterPHBalancer && getWaterPHSensor()) {
        _waterPHBalancer = new HydroponicsTimedDosingBalancer(getWaterPHSensor(), phSetpoint, HYDRUINO_CROP_PH_RANGE_HALF, _maxVolume, _volumeUnits);
        HYDRUINO_SOFT_ASSERT(_waterPHBalancer, F("Failure allocating pH balancer"));
    }
    if (_waterPHBalancer) {
        _waterPHBalancer->setTargetSetpoint(phSetpoint);
        _waterPHBalancer->setTargetUnits(phSetpointUnits);
    }
    return _waterPHBalancer;
}

void HydroponicsFeedReservoir::setWaterPHBalancer(HydroponicsBalancer *phBalancer)
{
    if (_waterPHBalancer != phBalancer) {
        if (_waterPHBalancer) { delete _waterPHBalancer; }
        _waterPHBalancer = phBalancer;
    }
}

HydroponicsBalancer *HydroponicsFeedReservoir::getWaterPHBalancer() const
{
    return _waterPHBalancer;
}

HydroponicsBalancer *HydroponicsFeedReservoir::setWaterTDSBalancer(float tdsSetpoint, Hydroponics_UnitsType tdsSetpointUnits)
{
    if (!_waterTDSBalancer && getWaterTDSSensor()) {
        _waterTDSBalancer = new HydroponicsTimedDosingBalancer(getWaterTDSSensor(), tdsSetpoint, HYDRUINO_CROP_EC_RANGE_HALF, _maxVolume, _volumeUnits);
        HYDRUINO_SOFT_ASSERT(_waterTDSBalancer, F("Failure allocating TDS balancer"));
    }
    if (_waterTDSBalancer) {
        _waterTDSBalancer->setTargetSetpoint(tdsSetpoint);
        _waterTDSBalancer->setTargetUnits(tdsSetpointUnits);
    }
    return _waterTDSBalancer;
}

void HydroponicsFeedReservoir::setWaterTDSBalancer(HydroponicsBalancer *tdsBalancer)
{
    if (_waterTDSBalancer != tdsBalancer) {
        if (_waterTDSBalancer) { delete _waterTDSBalancer; }
        _waterTDSBalancer = tdsBalancer;
    }
}

HydroponicsBalancer *HydroponicsFeedReservoir::getWaterTDSBalancer() const
{
    return _waterTDSBalancer;
}

HydroponicsBalancer *HydroponicsFeedReservoir::setWaterTempBalancer(float tempSetpoint, Hydroponics_UnitsType tempSetpointUnits)
{
    if (!_waterTempBalancer && getWaterTempSensor()) {
        auto tempRangeQuad = HYDRUINO_CROP_TEMP_RANGE_HALF * 0.5f;
        _waterTempBalancer = new HydroponicsLinearEdgeBalancer(getWaterTempSensor(), tempSetpoint, HYDRUINO_CROP_TEMP_RANGE_HALF, -tempRangeQuad * 0.5f, tempRangeQuad);
        HYDRUINO_SOFT_ASSERT(_waterTempBalancer, F("Failure allocating TDS balancer"));
    }
    if (_waterTempBalancer) {
        _waterTempBalancer->setTargetSetpoint(tempSetpoint);
        _waterTempBalancer->setTargetUnits(tempSetpointUnits);
    }
    return _waterTempBalancer;
}

void HydroponicsFeedReservoir::setWaterTempBalancer(HydroponicsBalancer *waterTempBalancer)
{
    if (_waterTempBalancer != waterTempBalancer) {
        if (_waterTempBalancer) { delete _waterTempBalancer; }
        _waterTempBalancer = waterTempBalancer;
    }
}

HydroponicsBalancer *HydroponicsFeedReservoir::getWaterTempBalancer() const
{
    return _waterTempBalancer;
}

HydroponicsBalancer *HydroponicsFeedReservoir::setAirTempBalancer(float tempSetpoint, Hydroponics_UnitsType tempSetpointUnits)
{
    if (!_airTempBalancer && getAirTempSensor()) {
        auto tempRangeQuad = HYDRUINO_CROP_TEMP_RANGE_HALF * 0.5f;
        _airTempBalancer = new HydroponicsLinearEdgeBalancer(getAirTempSensor(), tempSetpoint, HYDRUINO_CROP_TEMP_RANGE_HALF, -tempRangeQuad * 0.5f, tempRangeQuad);
        HYDRUINO_SOFT_ASSERT(_waterTempBalancer, F("Failure allocating TDS balancer"));
    }
    if (_airTempBalancer) {
        _airTempBalancer->setTargetSetpoint(tempSetpoint);
        _airTempBalancer->setTargetUnits(tempSetpointUnits);
    }
    return _airTempBalancer;
}

void HydroponicsFeedReservoir::setAirTempBalancer(HydroponicsBalancer *airTempBalancer)
{
    if (_airTempBalancer != airTempBalancer) {
        if (_airTempBalancer) { delete _airTempBalancer; }
        _airTempBalancer = airTempBalancer;
    }
}

HydroponicsBalancer *HydroponicsFeedReservoir::getAirTempBalancer() const
{
    return _airTempBalancer;
}

HydroponicsBalancer *HydroponicsFeedReservoir::setAirCO2Balancer(float co2Setpoint, Hydroponics_UnitsType co2SetpointUnits)
{
    if (!_airCO2Balancer && getAirCO2Sensor()) {
        auto co2RangeQuad = HYDRUINO_CROP_CO2_RANGE_HALF * 0.5f;
        _airCO2Balancer = new HydroponicsLinearEdgeBalancer(getAirCO2Sensor(), co2Setpoint, HYDRUINO_CROP_CO2_RANGE_HALF, -co2RangeQuad * 0.5f, co2RangeQuad);
        HYDRUINO_SOFT_ASSERT(_waterTempBalancer, F("Failure allocating TDS balancer"));
    }
    if (_airCO2Balancer) {
        _airCO2Balancer->setTargetSetpoint(co2Setpoint);
        _airCO2Balancer->setTargetUnits(co2SetpointUnits);
    }
    return _airCO2Balancer;
}

void HydroponicsFeedReservoir::setAirCO2Balancer(HydroponicsBalancer *co2Balancer)
{
    if (_airCO2Balancer != co2Balancer) {
        if (_airCO2Balancer) { delete _airCO2Balancer; }
        _airCO2Balancer = co2Balancer;
    }
}

HydroponicsBalancer *HydroponicsFeedReservoir::getAirCO2Balancer() const
{
    return _airCO2Balancer;
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
    if (_waterPHSensor.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->waterPHSensorName, _waterPHSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_waterTDSSensor.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->waterTDSSensorName, _waterTDSSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_waterTempSensor.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->waterTempSensorName, _waterTempSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_airTempSensor.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->airTempSensorName, _airTempSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_airCO2Sensor.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->airCO2SensorName, _airCO2Sensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}

void HydroponicsFeedReservoir::attachWaterPHSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterPHSensor(), F("PH sensor not linked, failure attaching"));
    if (getWaterPHSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleWaterPHMeasure);
        _waterPHSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFeedReservoir::detachWaterPHSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterPHSensor(), F("PH sensor not linked, failure detaching"));
    if (getWaterPHSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleWaterPHMeasure);
        _waterPHSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFeedReservoir::handleWaterPHMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        _needsWaterPHUpdate = false;
        setWaterPH(singleMeasurementAt(measurement, 0));
    }
}

void HydroponicsFeedReservoir::attachWaterTDSSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterTDSSensor(), F("TDS sensor not linked, failure attaching"));
    if (getWaterTDSSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleWaterTDSMeasure);
        _waterTDSSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFeedReservoir::detachWaterTDSSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterTDSSensor(), F("TDS sensor not linked, failure detaching"));
    if (getWaterTDSSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleWaterTDSMeasure);
        _waterTDSSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFeedReservoir::handleWaterTDSMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        _needsWaterTDSUpdate = false;
        setWaterTDS(singleMeasurementAt(measurement, 0));
    }
}

void HydroponicsFeedReservoir::attachWaterTempSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterTempSensor(), F("Temperature sensor not linked, failure attaching"));
    if (getWaterTempSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleWaterTempMeasure);
        _waterTempSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFeedReservoir::detachWaterTempSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterTempSensor(), F("Temperature sensor not linked, failure detaching"));
    if (getWaterTempSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleWaterTempMeasure);
        _waterTempSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFeedReservoir::handleWaterTempMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        _needsWaterTempUpdate = false;
        setWaterTemperature(singleMeasurementAt(measurement, 0));
    }
}

void HydroponicsFeedReservoir::attachAirTempSensor()
{
    HYDRUINO_SOFT_ASSERT(getAirTempSensor(), F("Air temperature sensor not linked, failure attaching"));
    if (getAirTempSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleAirTempMeasure);
        _airTempSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFeedReservoir::detachAirTempSensor()
{
    HYDRUINO_SOFT_ASSERT(getAirTempSensor(), F("Air temperature sensor not linked, failure detaching"));
    if (getAirTempSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleAirTempMeasure);
        _airTempSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFeedReservoir::handleAirTempMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        _needsAirTempUpdate = false;
        setAirTemperature(singleMeasurementAt(measurement, 0));
    }
}

void HydroponicsFeedReservoir::attachAirCO2Sensor()
{
    HYDRUINO_SOFT_ASSERT(getAirCO2Sensor(), F("CO2 sensor not linked, failure attaching"));
    if (getAirCO2Sensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleAirCO2Measure);
        _airCO2Sensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFeedReservoir::detachAirCO2Sensor()
{
    HYDRUINO_SOFT_ASSERT(getAirCO2Sensor(), F("CO2 sensor not linked, failure detaching"));
    if (getAirCO2Sensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleAirCO2Measure);
        _airCO2Sensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFeedReservoir::handleAirCO2Measure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        _needsAirCO2Update = false;
        setAirCO2(singleMeasurementAt(measurement, 0));
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

void HydroponicsInfiniteReservoir::setWaterVolume(float waterVolume, Hydroponics_UnitsType waterVolumeUnits = Hydroponics_UnitsType_Undefined)
{ ; }

void HydroponicsInfiniteReservoir::setWaterVolume(HydroponicsSingleMeasurement waterVolume)
{ ; }

const HydroponicsSingleMeasurement &HydroponicsInfiniteReservoir::getWaterVolume()
{
    return HydroponicsSingleMeasurement(_alwaysFilled ? __FLT_MAX__ : 0.0f, _volumeUnits, now(), 1);
}

void HydroponicsInfiniteReservoir::saveToData(HydroponicsData *dataOut)
{
    HydroponicsReservoir::saveToData(dataOut);

    ((HydroponicsInfiniteReservoirData *)dataOut)->alwaysFilled = _alwaysFilled;
}


HydroponicsReservoirData::HydroponicsReservoirData()
    : HydroponicsObjectData(), volumeUnits(Hydroponics_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void HydroponicsReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsObjectData::toJSONObject(objectOut);

    if (volumeUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("volumeUnits")] = volumeUnits; }
}

void HydroponicsReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);

    volumeUnits = objectIn[F("volumeUnits")] | volumeUnits;
}

HydroponicsFluidReservoirData::HydroponicsFluidReservoirData()
    : HydroponicsReservoirData(), maxVolume(0), volumeSensorName{0}
{
    _size = sizeof(*this);
}

void HydroponicsFluidReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsReservoirData::toJSONObject(objectOut);

    objectOut[F("maxVolume")] = maxVolume;
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
      waterPHSensorName{0}, waterTDSSensorName{0}, waterTempSensorName{0}, airTempSensorName{0}, airCO2SensorName{0}
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
    if (waterPHSensorName[0]) { objectOut[F("phSensorName")] = stringFromChars(waterPHSensorName, HYDRUINO_NAME_MAXSIZE); }
    if (waterTDSSensorName[0]) { objectOut[F("tdsSensorName")] = stringFromChars(waterTDSSensorName, HYDRUINO_NAME_MAXSIZE); }
    if (waterTempSensorName[0]) {
        objectOut[(airTempSensorName[0] ? F("waterTempSensorName") : F("tempSensorName"))] = stringFromChars(waterTempSensorName, HYDRUINO_NAME_MAXSIZE);
    }
    if (airTempSensorName[0]) { objectOut[F("airTempSensorName")] = stringFromChars(airTempSensorName, HYDRUINO_NAME_MAXSIZE); }
    if (airCO2SensorName[0]) { objectOut[F("co2SensorName")] = stringFromChars(airCO2SensorName, HYDRUINO_NAME_MAXSIZE); }
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
    const char *waterPHSensorNameStr = objectIn[F("phSensorName")];
    if (waterPHSensorNameStr && waterPHSensorNameStr[0]) { strncpy(waterPHSensorName, waterPHSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    const char *waterTDSSensorNameStr = objectIn[F("tdsSensorName")];
    if (waterTDSSensorNameStr && waterTDSSensorNameStr[0]) { strncpy(waterTDSSensorName, waterTDSSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    const char *waterTempSensorNameStr = objectIn[F("waterTempSensorName")] | objectIn[F("tempSensorName")];
    if (waterTempSensorNameStr && waterTempSensorNameStr[0]) { strncpy(waterTempSensorName, waterTempSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    const char *airTempSensorNameStr = objectIn[F("airTempSensorName")];
    if (airTempSensorNameStr && airTempSensorNameStr[0]) { strncpy(airTempSensorName, airTempSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    const char *airCO2SensorNameStr = objectIn[F("co2SensorName")];
    if (airCO2SensorNameStr && airCO2SensorNameStr[0]) { strncpy(airCO2SensorName, airCO2SensorNameStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsInfiniteReservoirData::HydroponicsInfiniteReservoirData()
    : HydroponicsReservoirData(), alwaysFilled(true)
{
    _size = sizeof(*this);
}

void HydroponicsInfiniteReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsReservoirData::toJSONObject(objectOut);

    if (alwaysFilled != true) { objectOut[F("alwaysFilled")] = alwaysFilled; }
}

void HydroponicsInfiniteReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsReservoirData::fromJSONObject(objectIn);

    alwaysFilled = objectIn[F("alwaysFilled")] | alwaysFilled;
}
