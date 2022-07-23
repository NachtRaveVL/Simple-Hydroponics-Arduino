/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Reservoirs
*/

#include "Hydroponics.h"

HydroponicsReservoir *newReservoirObjectFromData(const HydroponicsReservoirData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HS_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
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
      _volumeUnits(defaultLiquidVolumeUnits()),
      _filledState(Hydroponics_TriggerState_Disabled), _emptyState(Hydroponics_TriggerState_Disabled)
{ ; }

HydroponicsReservoir::HydroponicsReservoir(const HydroponicsReservoirData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _volumeUnits(definedUnitsElse(dataIn->volumeUnits, defaultLiquidVolumeUnits())),
      _filledState(Hydroponics_TriggerState_Disabled), _emptyState(Hydroponics_TriggerState_Disabled)
{ ; }

HydroponicsReservoir::~HydroponicsReservoir()
{ ; }

void HydroponicsReservoir::update()
{
    HydroponicsObject::update();

    auto filledState = triggerStateFromBool(getIsFilled());
    if (_filledState != filledState) {
        _filledState = filledState;
        handleFilledState();
    }
    auto emptyState = triggerStateFromBool(getIsEmpty());
    if (_emptyState != emptyState) {
        _emptyState = emptyState;
        handleEmptyState();
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

    if (getActuatorIsPumpFromType(actuator->getActuatorType())) {
        doEmptyCheck = (actuator->getReservoir().get() == this);
    } else if (getActuatorInWaterFromType(actuator->getActuatorType())) {
        doEmptyCheck = true;
    } else {
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
    return definedUnitsElse(_volumeUnits, defaultLiquidVolumeUnits());
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

void HydroponicsReservoir::handleFilledState()
{
    if (_emptyState == Hydroponics_TriggerState_Triggered) {
        #ifndef HYDRUINO_DISABLE_MULTITASKING
            scheduleSignalFireOnce<HydroponicsReservoir *>(getSharedPtr(), _emptySignal, this);
        #else
            _emptySignal.fire(this);
        #endif
    }
}

void HydroponicsReservoir::handleEmptyState()
{
    if (_filledState == Hydroponics_TriggerState_Triggered) {
        #ifndef HYDRUINO_DISABLE_MULTITASKING
            scheduleSignalFireOnce<HydroponicsReservoir *>(getSharedPtr(), _filledSignal, this);
        #else
            _filledSignal.fire(this);
        #endif
    }
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
      _volumeSensor(dataIn->volumeSensor),
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
    if (_filledTrigger) { return _filledTrigger->getTriggerState() == Hydroponics_TriggerState_Triggered; }
    return _waterVolume.value >= (_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_FRACTION_FILLED
                                                                                                     : _maxVolume) - FLT_EPSILON;
}

bool HydroponicsFluidReservoir::getIsEmpty() const
{
    if (_emptyTrigger) { return _emptyTrigger->getTriggerState() == Hydroponics_TriggerState_Triggered; }
    return _waterVolume.value <= (_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_FRACTION_EMPTY
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
    _waterVolume.units = waterVolumeUnits;
    _waterVolume.updateTimestamp();
    _waterVolume.updateFrame(1);

    convertUnits(&_waterVolume, getVolumeUnits());
    _needsVolumeUpdate = false;
}

void HydroponicsFluidReservoir::setWaterVolume(HydroponicsSingleMeasurement waterVolume)
{
    _waterVolume = waterVolume;
    _waterVolume.setMinFrame(1);

    convertUnits(&_waterVolume, getVolumeUnits());
    _needsVolumeUpdate = false;
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
        strncpy(((HydroponicsFluidReservoirData *)dataOut)->volumeSensor, _volumeSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_filledTrigger) {
        _filledTrigger->saveToData(&(((HydroponicsFluidReservoirData *)dataOut)->filledTrigger));
    }
    if (_emptyTrigger) {
        _emptyTrigger->saveToData(&(((HydroponicsFluidReservoirData *)dataOut)->emptyTrigger));
    }
}

void HydroponicsFluidReservoir::handleFilledState()
{
    if (_filledState == Hydroponics_TriggerState_Triggered && !getVolumeSensor()) {
        setWaterVolume(_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_FRACTION_FILLED
                                                                                          : _maxVolume, _volumeUnits);
    }
    HydroponicsReservoir::handleFilledState();
}

void HydroponicsFluidReservoir::handleEmptyState()
{
    if (_emptyState == Hydroponics_TriggerState_Triggered && !getVolumeSensor()) {
        setWaterVolume(_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_FRACTION_EMPTY
                                                                                          : 0, _volumeUnits);
    }
    HydroponicsReservoir::handleEmptyState();
}

void HydroponicsFluidReservoir::attachFilledTrigger()
{
    HYDRUINO_SOFT_ASSERT(_filledTrigger, SFP(HS_Err_MissingLinkage));
    if (_filledTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &HydroponicsFluidReservoir::handleFilledTrigger);
        _filledTrigger->getTriggerSignal().attach(methodSlot);
    }
}

void HydroponicsFluidReservoir::detachFilledTrigger()
{
    HYDRUINO_SOFT_ASSERT(_filledTrigger, SFP(HS_Err_MissingLinkage));
    if (_filledTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &HydroponicsFluidReservoir::handleFilledTrigger);
        _filledTrigger->getTriggerSignal().detach(methodSlot);
    }
}

void HydroponicsFluidReservoir::handleFilledTrigger(Hydroponics_TriggerState triggerState)
{
    if (triggerState != Hydroponics_TriggerState_Undefined && triggerState != Hydroponics_TriggerState_Disabled && _filledState != triggerState) {
        _filledState = triggerState;
        handleFilledState();
    }
}

void HydroponicsFluidReservoir::attachEmptyTrigger()
{
    HYDRUINO_SOFT_ASSERT(_emptyTrigger, SFP(HS_Err_MissingLinkage));
    if (_emptyTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &HydroponicsFluidReservoir::handleEmptyTrigger);
        _emptyTrigger->getTriggerSignal().attach(methodSlot);
    }
}

void HydroponicsFluidReservoir::detachEmptyTrigger()
{
    HYDRUINO_SOFT_ASSERT(_emptyTrigger, SFP(HS_Err_MissingLinkage));
    if (_emptyTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &HydroponicsFluidReservoir::handleEmptyTrigger);
        _emptyTrigger->getTriggerSignal().detach(methodSlot);
    }
}

void HydroponicsFluidReservoir::handleEmptyTrigger(Hydroponics_TriggerState triggerState)
{
    if (triggerState != Hydroponics_TriggerState_Undefined && triggerState != Hydroponics_TriggerState_Disabled && _emptyState != triggerState) {
        _emptyState = triggerState;
        handleEmptyState();
    }
}

void HydroponicsFluidReservoir::attachWaterVolumeSensor()
{
    HYDRUINO_SOFT_ASSERT(getVolumeSensor(), SFP(HS_Err_MissingLinkage));
    if (getVolumeSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsFluidReservoir::handleWaterVolumeMeasure);
        _volumeSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFluidReservoir::detachWaterVolumeSensor()
{
    HYDRUINO_SOFT_ASSERT(getVolumeSensor(), SFP(HS_Err_MissingLinkage));
    if (getVolumeSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsFluidReservoir::handleWaterVolumeMeasure);
        _volumeSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFluidReservoir::handleWaterVolumeMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        setWaterVolume(getAsSingleMeasurement(measurement, 0, _maxVolume, _volumeUnits));
    }
}


HydroponicsFeedReservoir::HydroponicsFeedReservoir(Hydroponics_PositionIndex reservoirIndex,
                                                   float maxVolume,
                                                   DateTime lastChangeDate,
                                                   DateTime lastPruningDate,
                                                   int classType)
    : HydroponicsFluidReservoir(Hydroponics_ReservoirType_FeedWater, reservoirIndex, maxVolume, classType),
       _lastChangeDate(lastChangeDate.unixtime()), _lastPruningDate(lastPruningDate.unixtime()), _lastFeedingDate(0), _numFeedingsToday(0),
       _tdsUnits(Hydroponics_UnitsType_Concentration_TDS), _tempUnits(defaultTemperatureUnits()),
       _needsWaterPHUpdate(true), _needsWaterTDSUpdate(true), _needsWaterTempUpdate(true), _needsAirTempUpdate(true), _needsAirCO2Update(true),
       _waterPHBalancer(nullptr), _waterTDSBalancer(nullptr), _waterTempBalancer(nullptr), _airTempBalancer(nullptr), _airCO2Balancer(nullptr)
{ ; }

HydroponicsFeedReservoir::HydroponicsFeedReservoir(const HydroponicsFeedReservoirData *dataIn)
    : HydroponicsFluidReservoir(dataIn),
      _lastChangeDate(dataIn->lastChangeDate), _lastPruningDate(dataIn->lastPruningDate),
      _lastFeedingDate(dataIn->lastFeedingDate), _numFeedingsToday(dataIn->numFeedingsToday),
      _tdsUnits(definedUnitsElse(dataIn->tdsUnits, Hydroponics_UnitsType_Concentration_TDS)),
      _tempUnits(definedUnitsElse(dataIn->tempUnits, defaultTemperatureUnits())),
      _waterPHSensor(dataIn->waterPHSensor), _waterTDSSensor(dataIn->waterTDSSensor), _waterTempSensor(dataIn->waterTempSensor),
      _airTempSensor(dataIn->airTempSensor), _airCO2Sensor(dataIn->airCO2Sensor),
      _needsWaterPHUpdate(true), _needsWaterTDSUpdate(true), _needsWaterTempUpdate(true), _needsAirTempUpdate(true), _needsAirCO2Update(true),
      _waterPHBalancer(nullptr), _waterTDSBalancer(nullptr), _waterTempBalancer(nullptr), _airTempBalancer(nullptr), _airCO2Balancer(nullptr)
{
    if (_lastFeedingDate) {
        auto hydroponics = getHydroponicsInstance();
        auto lastFeeding = DateTime((uint32_t)(_lastFeedingDate + (hydroponics ? hydroponics->getTimeZoneOffset() * SECS_PER_HOUR : 0)));
        auto currTime = getCurrentTime();

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

        convertUnits(&_waterTDS, getTDSUnits());
    }
}

Hydroponics_UnitsType HydroponicsFeedReservoir::getTDSUnits() const
{
    return definedUnitsElse(_tdsUnits, Hydroponics_UnitsType_Concentration_TDS);
}

void HydroponicsFeedReservoir::setTemperatureUnits(Hydroponics_UnitsType tempUnits)
{
    if (_tempUnits != tempUnits) {
        _tempUnits = tempUnits;

        convertUnits(&_waterTemp, getTemperatureUnits());
        convertUnits(&_airTemp, getTemperatureUnits());
    }
}

Hydroponics_UnitsType HydroponicsFeedReservoir::getTemperatureUnits() const
{
    return definedUnitsElse(_tempUnits, defaultTemperatureUnits());
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
    _waterPH.units = waterPHUnits;
    _waterPH.updateTimestamp();
    _waterPH.updateFrame(1);

    convertUnits(&_waterPH, Hydroponics_UnitsType_Alkalinity_pH_0_14);
    _needsWaterPHUpdate = false;
}

void HydroponicsFeedReservoir::setWaterPH(HydroponicsSingleMeasurement waterPH)
{
    _waterPH = waterPH;
    _waterPH.setMinFrame(1);

    convertUnits(&_waterPH, Hydroponics_UnitsType_Alkalinity_pH_0_14);
    _needsWaterPHUpdate = false;
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
    _waterTDS.units = waterTDSUnits;
    _waterTDS.updateTimestamp();
    _waterTDS.updateFrame(1);

    convertUnits(&_waterTDS, getTDSUnits());
    _needsWaterTDSUpdate = false;
}

void HydroponicsFeedReservoir::setWaterTDS(HydroponicsSingleMeasurement waterTDS)
{
    _waterTDS = waterTDS;
    _waterTDS.setMinFrame(1);

    convertUnits(&_waterTDS, getTDSUnits());
    _needsWaterTDSUpdate = false;
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
    _waterTemp.units = waterTempUnits;
    _waterTemp.updateTimestamp();
    _waterTemp.updateFrame(1);

    convertUnits(&_waterTemp, getTemperatureUnits());
    _needsWaterTempUpdate = false;
}

void HydroponicsFeedReservoir::setWaterTemperature(HydroponicsSingleMeasurement waterTemperature)
{
    _waterTemp = waterTemperature;
    _waterTemp.setMinFrame(1);

    convertUnits(&_waterTemp, getTemperatureUnits());
    _needsWaterTempUpdate = false;
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
    _airTemp.units = airTempUnits;
    _airTemp.updateTimestamp();
    _airTemp.updateFrame(1);

    convertUnits(&_airTemp, getTemperatureUnits());
    _needsAirTempUpdate = false;
}

void HydroponicsFeedReservoir::setAirTemperature(HydroponicsSingleMeasurement airTemperature)
{
    _airTemp = airTemperature;
    _airTemp.setMinFrame(1);

    convertUnits(&_airTemp, getTemperatureUnits());
    _needsAirTempUpdate = false;
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
    _airCO2.units = airCO2Units;
    _airCO2.updateTimestamp();
    _airCO2.updateFrame(1);

    convertUnits(&_airCO2, Hydroponics_UnitsType_Concentration_PPM);
    _needsAirCO2Update = false;
}

void HydroponicsFeedReservoir::setAirCO2(HydroponicsSingleMeasurement airCO2)
{
    _airCO2 = airCO2;
    _airCO2.setMinFrame(1);

    convertUnits(&_airCO2, Hydroponics_UnitsType_Concentration_PPM);
    _needsAirCO2Update = false;
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
        _waterPHBalancer = new HydroponicsTimedDosingBalancer(getWaterPHSensor(), phSetpoint, HYDRUINO_RANGE_PH_HALF, _maxVolume, _volumeUnits);
        HYDRUINO_SOFT_ASSERT(_waterPHBalancer, SFP(HS_Err_AllocationFailure));
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
        _waterTDSBalancer = new HydroponicsTimedDosingBalancer(getWaterTDSSensor(), tdsSetpoint, HYDRUINO_RANGE_EC_HALF, _maxVolume, _volumeUnits);
        HYDRUINO_SOFT_ASSERT(_waterTDSBalancer, SFP(HS_Err_AllocationFailure));
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
        auto tempRangeQuad = HYDRUINO_RANGE_TEMP_HALF * 0.5f;
        _waterTempBalancer = new HydroponicsLinearEdgeBalancer(getWaterTempSensor(), tempSetpoint, HYDRUINO_RANGE_TEMP_HALF, -tempRangeQuad * 0.5f, tempRangeQuad);
        HYDRUINO_SOFT_ASSERT(_waterTempBalancer, SFP(HS_Err_AllocationFailure));
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
        auto tempRangeQuad = HYDRUINO_RANGE_TEMP_HALF * 0.5f;
        _airTempBalancer = new HydroponicsLinearEdgeBalancer(getAirTempSensor(), tempSetpoint, HYDRUINO_RANGE_TEMP_HALF, -tempRangeQuad * 0.5f, tempRangeQuad);
        HYDRUINO_SOFT_ASSERT(_waterTempBalancer, SFP(HS_Err_AllocationFailure));
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
        auto co2RangeQuad = HYDRUINO_RANGE_CO2_HALF * 0.5f;
        _airCO2Balancer = new HydroponicsLinearEdgeBalancer(getAirCO2Sensor(), co2Setpoint, HYDRUINO_RANGE_CO2_HALF, -co2RangeQuad * 0.5f, co2RangeQuad);
        HYDRUINO_SOFT_ASSERT(_waterTempBalancer, SFP(HS_Err_AllocationFailure));
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
    _lastChangeDate = unixNow();
}

DateTime HydroponicsFeedReservoir::getLastPruningDate() const
{
    return DateTime((uint32_t)_lastPruningDate);
}

void HydroponicsFeedReservoir::notifyPruningCompleted()
{
    _lastPruningDate = unixNow();
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
    _lastFeedingDate = unixNow();
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
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->waterPHSensor, _waterPHSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_waterTDSSensor.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->waterTDSSensor, _waterTDSSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_waterTempSensor.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->waterTempSensor, _waterTempSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_airTempSensor.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->airTempSensor, _airTempSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_airCO2Sensor.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->airCO2Sensor, _airCO2Sensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}

void HydroponicsFeedReservoir::attachWaterPHSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterPHSensor(), SFP(HS_Err_MissingLinkage));
    if (getWaterPHSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsFeedReservoir::handleWaterPHMeasure);
        _waterPHSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFeedReservoir::detachWaterPHSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterPHSensor(), SFP(HS_Err_MissingLinkage));
    if (getWaterPHSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsFeedReservoir::handleWaterPHMeasure);
        _waterPHSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFeedReservoir::handleWaterPHMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        setWaterPH(getAsSingleMeasurement(measurement, 0));
    }
}

void HydroponicsFeedReservoir::attachWaterTDSSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterTDSSensor(), SFP(HS_Err_MissingLinkage));
    if (getWaterTDSSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsFeedReservoir::handleWaterTDSMeasure);
        _waterTDSSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFeedReservoir::detachWaterTDSSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterTDSSensor(), SFP(HS_Err_MissingLinkage));
    if (getWaterTDSSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsFeedReservoir::handleWaterTDSMeasure);
        _waterTDSSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFeedReservoir::handleWaterTDSMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        setWaterTDS(getAsSingleMeasurement(measurement, 0));
    }
}

void HydroponicsFeedReservoir::attachWaterTempSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterTempSensor(), SFP(HS_Err_MissingLinkage));
    if (getWaterTempSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsFeedReservoir::handleWaterTempMeasure);
        _waterTempSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFeedReservoir::detachWaterTempSensor()
{
    HYDRUINO_SOFT_ASSERT(getWaterTempSensor(), SFP(HS_Err_MissingLinkage));
    if (getWaterTempSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsFeedReservoir::handleWaterTempMeasure);
        _waterTempSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFeedReservoir::handleWaterTempMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        setWaterTemperature(getAsSingleMeasurement(measurement, 0));
    }
}

void HydroponicsFeedReservoir::attachAirTempSensor()
{
    HYDRUINO_SOFT_ASSERT(getAirTempSensor(), SFP(HS_Err_MissingLinkage));
    if (getAirTempSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsFeedReservoir::handleAirTempMeasure);
        _airTempSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFeedReservoir::detachAirTempSensor()
{
    HYDRUINO_SOFT_ASSERT(getAirTempSensor(), SFP(HS_Err_MissingLinkage));
    if (getAirTempSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsFeedReservoir::handleAirTempMeasure);
        _airTempSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFeedReservoir::handleAirTempMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        setAirTemperature(getAsSingleMeasurement(measurement, 0));
    }
}

void HydroponicsFeedReservoir::attachAirCO2Sensor()
{
    HYDRUINO_SOFT_ASSERT(getAirCO2Sensor(), SFP(HS_Err_MissingLinkage));
    if (getAirCO2Sensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsFeedReservoir::handleAirCO2Measure);
        _airCO2Sensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsFeedReservoir::detachAirCO2Sensor()
{
    HYDRUINO_SOFT_ASSERT(getAirCO2Sensor(), SFP(HS_Err_MissingLinkage));
    if (getAirCO2Sensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsFeedReservoir::handleAirCO2Measure);
        _airCO2Sensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsFeedReservoir::handleAirCO2Measure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        setAirCO2(getAsSingleMeasurement(measurement, 0));
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
    return HydroponicsSingleMeasurement(_alwaysFilled ? FLT_UNDEF : 0.0f, _volumeUnits, unixNow(), 1);
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

    if (volumeUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HS_Key_VolumeUnits)] = unitsTypeToSymbol(volumeUnits); }
}

void HydroponicsReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);

    volumeUnits = unitsTypeFromSymbol(objectIn[SFP(HS_Key_VolumeUnits)]);
}

HydroponicsFluidReservoirData::HydroponicsFluidReservoirData()
    : HydroponicsReservoirData(), maxVolume(0), volumeSensor{0}
{
    _size = sizeof(*this);
}

void HydroponicsFluidReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsReservoirData::toJSONObject(objectOut);

    objectOut[SFP(HS_Key_MaxVolume)] = maxVolume;
    if (volumeSensor[0]) { objectOut[SFP(HS_Key_VolumeSensor)] = stringFromChars(volumeSensor, HYDRUINO_NAME_MAXSIZE); }
    if (filledTrigger.type != -1) {
        JsonObject filledTriggerObj = objectOut.createNestedObject(SFP(HS_Key_FilledTrigger));
        filledTrigger.toJSONObject(filledTriggerObj);
    }
    if (emptyTrigger.type != -1) {
        JsonObject emptyTriggerObj = objectOut.createNestedObject(SFP(HS_Key_EmptyTrigger));
        emptyTrigger.toJSONObject(emptyTriggerObj);
    }
}

void HydroponicsFluidReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsReservoirData::fromJSONObject(objectIn);

    maxVolume = objectIn[SFP(HS_Key_MaxVolume)] | maxVolume;
    const char *volumeSensorStr = objectIn[SFP(HS_Key_VolumeSensor)];
    if (volumeSensorStr && volumeSensorStr[0]) { strncpy(volumeSensor, volumeSensorStr, HYDRUINO_NAME_MAXSIZE); }
    JsonObjectConst filledTriggerObj = objectIn[SFP(HS_Key_FilledTrigger)];
    if (!filledTriggerObj.isNull()) { filledTrigger.fromJSONObject(filledTriggerObj); }
    JsonObjectConst emptyTriggerObj = objectIn[SFP(HS_Key_EmptyTrigger)];
    if (!emptyTriggerObj.isNull()) { emptyTrigger.fromJSONObject(emptyTriggerObj); }
}

HydroponicsFeedReservoirData::HydroponicsFeedReservoirData()
    : HydroponicsFluidReservoirData(), lastChangeDate(0), lastPruningDate(0), lastFeedingDate(0), numFeedingsToday(0),
      tdsUnits(Hydroponics_UnitsType_Undefined), tempUnits(Hydroponics_UnitsType_Undefined),
      waterPHSensor{0}, waterTDSSensor{0}, waterTempSensor{0}, airTempSensor{0}, airCO2Sensor{0}
{
    _size = sizeof(*this);
}

void HydroponicsFeedReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsFluidReservoirData::toJSONObject(objectOut);

    if (lastChangeDate) { objectOut[SFP(HS_Key_LastChangeDate)] = lastChangeDate; }
    if (lastPruningDate) { objectOut[SFP(HS_Key_LastPruningDate)] = lastPruningDate; }
    if (lastFeedingDate) { objectOut[SFP(HS_Key_LastFeedingDate)] = lastFeedingDate; }
    if (numFeedingsToday > 0) { objectOut[SFP(HS_Key_NumFeedingsToday)] = numFeedingsToday; }
    if (tdsUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HS_Key_TDSUnits)] = unitsTypeToSymbol(tdsUnits); }
    if (tempUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HS_Key_TempUnits)] = unitsTypeToSymbol(tempUnits); }
    if (waterPHSensor[0]) { objectOut[SFP(HS_Key_PHSensor)] = stringFromChars(waterPHSensor, HYDRUINO_NAME_MAXSIZE); }
    if (waterTDSSensor[0]) { objectOut[SFP(HS_Key_TDSSensor)] = stringFromChars(waterTDSSensor, HYDRUINO_NAME_MAXSIZE); }
    if (waterTempSensor[0]) {
        objectOut[(airTempSensor[0] ? SFP(HS_Key_WaterTempSensor) : SFP(HS_Key_TempSensor))] = stringFromChars(waterTempSensor, HYDRUINO_NAME_MAXSIZE);
    }
    if (airTempSensor[0]) { objectOut[SFP(HS_Key_AirTempSensor)] = stringFromChars(airTempSensor, HYDRUINO_NAME_MAXSIZE); }
    if (airCO2Sensor[0]) { objectOut[SFP(HS_Key_CO2Sensor)] = stringFromChars(airCO2Sensor, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsFeedReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsFluidReservoirData::fromJSONObject(objectIn);

    lastChangeDate = objectIn[SFP(HS_Key_LastChangeDate)] | lastChangeDate;
    lastPruningDate = objectIn[SFP(HS_Key_LastPruningDate)] | lastPruningDate;
    lastFeedingDate = objectIn[SFP(HS_Key_LastFeedingDate)] | lastFeedingDate;
    numFeedingsToday = objectIn[SFP(HS_Key_NumFeedingsToday)] | numFeedingsToday;
    tdsUnits = unitsTypeFromSymbol(objectIn[SFP(HS_Key_TDSUnits)]);
    tempUnits = unitsTypeFromSymbol(objectIn[SFP(HS_Key_TempUnits)]);
    const char *waterPHSensorStr = objectIn[SFP(HS_Key_PHSensor)];
    if (waterPHSensorStr && waterPHSensorStr[0]) { strncpy(waterPHSensor, waterPHSensorStr, HYDRUINO_NAME_MAXSIZE); }
    const char *waterTDSSensorStr = objectIn[SFP(HS_Key_TDSSensor)];
    if (waterTDSSensorStr && waterTDSSensorStr[0]) { strncpy(waterTDSSensor, waterTDSSensorStr, HYDRUINO_NAME_MAXSIZE); }
    const char *waterTempSensorStr = objectIn[SFP(HS_Key_WaterTempSensor)] | objectIn[SFP(HS_Key_TempSensor)];
    if (waterTempSensorStr && waterTempSensorStr[0]) { strncpy(waterTempSensor, waterTempSensorStr, HYDRUINO_NAME_MAXSIZE); }
    const char *airTempSensorStr = objectIn[SFP(HS_Key_AirTempSensor)];
    if (airTempSensorStr && airTempSensorStr[0]) { strncpy(airTempSensor, airTempSensorStr, HYDRUINO_NAME_MAXSIZE); }
    const char *airCO2SensorStr = objectIn[SFP(HS_Key_CO2Sensor)];
    if (airCO2SensorStr && airCO2SensorStr[0]) { strncpy(airCO2Sensor, airCO2SensorStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsInfiniteReservoirData::HydroponicsInfiniteReservoirData()
    : HydroponicsReservoirData(), alwaysFilled(true)
{
    _size = sizeof(*this);
}

void HydroponicsInfiniteReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsReservoirData::toJSONObject(objectOut);

    objectOut[SFP(HS_Key_AlwaysFilled)] = alwaysFilled;
}

void HydroponicsInfiniteReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsReservoirData::fromJSONObject(objectIn);

    alwaysFilled = objectIn[SFP(HS_Key_AlwaysFilled)] | alwaysFilled;
}
