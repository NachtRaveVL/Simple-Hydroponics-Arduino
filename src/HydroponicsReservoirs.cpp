/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Reservoirs
*/

#include "Hydroponics.h"

HydroponicsReservoir *newReservoirObjectFromData(const HydroponicsReservoirData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectectData(), SFP(HS_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectectData()) {
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

    auto filledState = triggerStateFromBool(isFilled());
    if (_filledState != filledState) {
        _filledState = filledState;
        handleFilledState();
    }
    auto emptyState = triggerStateFromBool(isEmpty());
    if (_emptyState != emptyState) {
        _emptyState = emptyState;
        handleEmptyState();
    }
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
        return !isEmpty();
    } else {
        return !isFilled();
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
      _maxVolume(maxVolume), _waterVolume(this),
      _filledTrigger(nullptr), _emptyTrigger(nullptr)
{ ; }

HydroponicsFluidReservoir::HydroponicsFluidReservoir(const HydroponicsFluidReservoirData *dataIn)
    : HydroponicsReservoir(dataIn),
      _maxVolume(dataIn->maxVolume),
      _waterVolume(this, dataIn->volumeSensor),
      _filledTrigger(newTriggerObjectFromSubData(&(dataIn->filledTrigger))),
      _emptyTrigger(newTriggerObjectFromSubData(&(dataIn->emptyTrigger)))
{
    if (_filledTrigger) { attachFilledTrigger(); }
    if (_emptyTrigger) { attachEmptyTrigger(); }
}

HydroponicsFluidReservoir::~HydroponicsFluidReservoir()
{
    if (_filledTrigger) { detachFilledTrigger(); delete _filledTrigger; _filledTrigger = nullptr; }
    if (_emptyTrigger) { detachEmptyTrigger(); delete _emptyTrigger; _emptyTrigger = nullptr; }
}

void HydroponicsFluidReservoir::update()
{
    HydroponicsReservoir::update();

    if (_filledTrigger) { _filledTrigger->update(); }
    if (_emptyTrigger) { _emptyTrigger->update(); }

    _waterVolume.updateMeasurementIfNeeded();
}

void HydroponicsFluidReservoir::handleLowMemory()
{
    HydroponicsReservoir::handleLowMemory();

    if (_filledTrigger) { _filledTrigger->handleLowMemory(); }
    if (_emptyTrigger) { _emptyTrigger->handleLowMemory(); }
}

bool HydroponicsFluidReservoir::isFilled()
{
    if (_filledTrigger) { return _filledTrigger->getTriggerState() == Hydroponics_TriggerState_Triggered; }
    return _waterVolume.getMeasurementValue() >= (_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_FRACTION_FILLED
                                                                                                                     : _maxVolume) - FLT_EPSILON;
}

bool HydroponicsFluidReservoir::isEmpty()
{
    if (_emptyTrigger) { return _emptyTrigger->getTriggerState() == Hydroponics_TriggerState_Triggered; }
    return _waterVolume.getMeasurementValue() <= (_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_FRACTION_EMPTY
                                                                                                                     : 0) + FLT_EPSILON;
}

void HydroponicsFluidReservoir::setVolumeUnits(Hydroponics_UnitsType volumeUnits)
{
    if (_volumeUnits != volumeUnits) {
        _volumeUnits = volumeUnits;

        _waterVolume.setMeasurementUnits(volumeUnits);
    }
}

HydroponicsSensorAttachment &HydroponicsFluidReservoir::getWaterVolume()
{
    _waterVolume.updateMeasurementIfNeeded();
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
    if (_waterVolume.getId()) {
        strncpy(((HydroponicsFluidReservoirData *)dataOut)->volumeSensor, _waterVolume.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
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
    if (_filledState == Hydroponics_TriggerState_Triggered && !getWaterVolume()) {
        getWaterVolume().setMeasurement(_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_FRACTION_FILLED
                                                                                                           : _maxVolume, _volumeUnits);
    }
    HydroponicsReservoir::handleFilledState();
}

void HydroponicsFluidReservoir::handleEmptyState()
{
    if (_emptyState == Hydroponics_TriggerState_Triggered && !getWaterVolume()) {
        getWaterVolume().setMeasurement(_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_FRACTION_EMPTY
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


HydroponicsFeedReservoir::HydroponicsFeedReservoir(Hydroponics_PositionIndex reservoirIndex,
                                                   float maxVolume,
                                                   DateTime lastChangeDate,
                                                   DateTime lastPruningDate,
                                                   int classType)
    : HydroponicsFluidReservoir(Hydroponics_ReservoirType_FeedWater, reservoirIndex, maxVolume, classType),
       _lastChangeDate(lastChangeDate.unixtime()), _lastPruningDate(lastPruningDate.unixtime()), _lastFeedingDate(0), _numFeedingsToday(0),
       _tdsUnits(Hydroponics_UnitsType_Concentration_TDS), _tempUnits(defaultTemperatureUnits()),
       _waterPH(this), _waterTDS(this), _waterTemp(this), _airTemp(this), _airCO2(this),
       _waterPHBalancer(nullptr), _waterTDSBalancer(nullptr), _waterTempBalancer(nullptr), _airTempBalancer(nullptr), _airCO2Balancer(nullptr)
{ ; }

HydroponicsFeedReservoir::HydroponicsFeedReservoir(const HydroponicsFeedReservoirData *dataIn)
    : HydroponicsFluidReservoir(dataIn),
      _lastChangeDate(dataIn->lastChangeDate), _lastPruningDate(dataIn->lastPruningDate),
      _lastFeedingDate(dataIn->lastFeedingDate), _numFeedingsToday(dataIn->numFeedingsToday),
      _tdsUnits(definedUnitsElse(dataIn->tdsUnits, Hydroponics_UnitsType_Concentration_TDS)),
      _tempUnits(definedUnitsElse(dataIn->tempUnits, defaultTemperatureUnits())),
      _waterPH(this, dataIn->waterPHSensor), _waterTDS(this, dataIn->waterTDSSensor), _waterTemp(this, dataIn->waterTempSensor),
      _airTemp(this, dataIn->airTempSensor), _airCO2(this, dataIn->airCO2Sensor),
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

    _waterPH.updateMeasurementIfNeeded();
    _waterTDS.updateMeasurementIfNeeded();
    _waterTemp.updateMeasurementIfNeeded();
    _airTemp.updateMeasurementIfNeeded();
    _airCO2.updateMeasurementIfNeeded();
}

void HydroponicsFeedReservoir::handleLowMemory()
{
    HydroponicsFluidReservoir::handleLowMemory();

    if (_waterPHBalancer && !_waterPHBalancer->isEnabled()) { setWaterPHBalancer(nullptr); }
    if (_waterTDSBalancer && !_waterTDSBalancer->isEnabled()) { setWaterTDSBalancer(nullptr); }
    if (_waterTempBalancer && !_waterTempBalancer->isEnabled()) { setWaterTemperatureBalancer(nullptr); }
    if (_airTempBalancer && !_airTempBalancer->isEnabled()) { setAirTemperatureBalancer(nullptr); }
    if (_airCO2Balancer && !_airCO2Balancer->isEnabled()) { setAirCO2Balancer(nullptr); }
}

void HydroponicsFeedReservoir::setTDSUnits(Hydroponics_UnitsType tdsUnits)
{
    if (_tdsUnits != tdsUnits) {
        _tdsUnits = tdsUnits;

        _waterTDS.setMeasurementUnits(getTDSUnits());
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

        _waterTemp.setMeasurementUnits(getTemperatureUnits());
        _airTemp.setMeasurementUnits(getTemperatureUnits());
    }
}

Hydroponics_UnitsType HydroponicsFeedReservoir::getTemperatureUnits() const
{
    return definedUnitsElse(_tempUnits, defaultTemperatureUnits());
}

HydroponicsSensorAttachment &HydroponicsFeedReservoir::getWaterPH()
{
    _waterPH.updateMeasurementIfNeeded();
    return _waterPH;
}

HydroponicsSensorAttachment &HydroponicsFeedReservoir::getWaterTDS()
{
    _waterTDS.updateMeasurementIfNeeded();
    return _waterTDS;
}

HydroponicsSensorAttachment &HydroponicsFeedReservoir::getWaterTemperature()
{
    _waterTemp.updateMeasurementIfNeeded();
    return _waterTemp;
}

HydroponicsSensorAttachment &HydroponicsFeedReservoir::getAirTemperature()
{
    _airTemp.updateMeasurementIfNeeded();
    return _airTemp;
}

HydroponicsSensorAttachment &HydroponicsFeedReservoir::getAirCO2()
{
    _airCO2.updateMeasurementIfNeeded();
    return _airCO2;
}

HydroponicsBalancer *HydroponicsFeedReservoir::setWaterPHBalancer(float phSetpoint, Hydroponics_UnitsType phSetpointUnits)
{
    if (!_waterPHBalancer && getWaterPHSensor()) {
        _waterPHBalancer = new HydroponicsTimedDosingBalancer(_waterPH.getObject(), phSetpoint, HYDRUINO_RANGE_PH_HALF, _maxVolume, _volumeUnits);
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
        _waterTDSBalancer = new HydroponicsTimedDosingBalancer(getWaterTDS().getObject(), tdsSetpoint, HYDRUINO_RANGE_EC_HALF, _maxVolume, _volumeUnits);
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

HydroponicsBalancer *HydroponicsFeedReservoir::setWaterTemperatureBalancer(float tempSetpoint, Hydroponics_UnitsType tempSetpointUnits)
{
    if (!_waterTempBalancer && getWaterTemperatureSensor()) {
        auto tempRangeQuad = HYDRUINO_RANGE_TEMP_HALF * 0.5f;
        _waterTempBalancer = new HydroponicsLinearEdgeBalancer(getWaterTemperatureSensor(), tempSetpoint, HYDRUINO_RANGE_TEMP_HALF, -tempRangeQuad * 0.5f, tempRangeQuad);
        HYDRUINO_SOFT_ASSERT(_waterTempBalancer, SFP(HS_Err_AllocationFailure));
    }
    if (_waterTempBalancer) {
        _waterTempBalancer->setTargetSetpoint(tempSetpoint);
        _waterTempBalancer->setTargetUnits(tempSetpointUnits);
    }
    return _waterTempBalancer;
}

void HydroponicsFeedReservoir::setWaterTemperatureBalancer(HydroponicsBalancer *waterTempBalancer)
{
    if (_waterTempBalancer != waterTempBalancer) {
        if (_waterTempBalancer) { delete _waterTempBalancer; }
        _waterTempBalancer = waterTempBalancer;
    }
}

HydroponicsBalancer *HydroponicsFeedReservoir::getWaterTemperatureBalancer() const
{
    return _waterTempBalancer;
}

HydroponicsBalancer *HydroponicsFeedReservoir::setAirTemperatureBalancer(float tempSetpoint, Hydroponics_UnitsType tempSetpointUnits)
{
    if (!_airTempBalancer && getAirTemperatureSensor()) {
        auto tempRangeQuad = HYDRUINO_RANGE_TEMP_HALF * 0.5f;
        _airTempBalancer = new HydroponicsLinearEdgeBalancer(getAirTemperatureSensor(), tempSetpoint, HYDRUINO_RANGE_TEMP_HALF, -tempRangeQuad * 0.5f, tempRangeQuad);
        HYDRUINO_SOFT_ASSERT(_waterTempBalancer, SFP(HS_Err_AllocationFailure));
    }
    if (_airTempBalancer) {
        _airTempBalancer->setTargetSetpoint(tempSetpoint);
        _airTempBalancer->setTargetUnits(tempSetpointUnits);
    }
    return _airTempBalancer;
}

void HydroponicsFeedReservoir::setAirTemperatureBalancer(HydroponicsBalancer *airTempBalancer)
{
    if (_airTempBalancer != airTempBalancer) {
        if (_airTempBalancer) { delete _airTempBalancer; }
        _airTempBalancer = airTempBalancer;
    }
}

HydroponicsBalancer *HydroponicsFeedReservoir::getAirTemperatureBalancer() const
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
    if (_waterPH.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->waterPHSensor, _waterPH.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_waterTDS.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->waterTDSSensor, _waterTDS.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_waterTemp.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->waterTempSensor, _waterTemp.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_airTemp.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->airTempSensor, _airTemp.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_airCO2.getId()) {
        strncpy(((HydroponicsFeedReservoirData *)dataOut)->airCO2Sensor, _airCO2.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}


HydroponicsInfiniteReservoir::HydroponicsInfiniteReservoir(Hydroponics_ReservoirType reservoirType,
                                                           Hydroponics_PositionIndex reservoirIndex,
                                                           bool alwaysFilled,
                                                           int classType)
    : HydroponicsReservoir(reservoirType, reservoirIndex, classType), _alwaysFilled(alwaysFilled), _waterVolume(this)
{ ; }

HydroponicsInfiniteReservoir::HydroponicsInfiniteReservoir(const HydroponicsInfiniteReservoirData *dataIn)
    : HydroponicsReservoir(dataIn), _alwaysFilled(dataIn->alwaysFilled), _waterVolume(this)
{ ; }

HydroponicsInfiniteReservoir::~HydroponicsInfiniteReservoir()
{ ; }

bool HydroponicsInfiniteReservoir::isFilled()
{
    return _alwaysFilled;
}

bool HydroponicsInfiniteReservoir::isEmpty()
{
    return !_alwaysFilled;
}

HydroponicsSensorAttachment &HydroponicsInfiniteReservoir::getWaterVolume()
{
    _waterVolume.setMeasurement(HydroponicsSingleMeasurement(
        _alwaysFilled ? FLT_UNDEF : 0.0f,
        getVolumeUnits(),
        unixNow(),
        getHydroponicsInstance() ? getHydroponicsInstance()->getPollingFrame() : 1
    ));
    return _waterVolume;
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
    if (volumeSensor[0]) { objectOut[SFP(HS_Key_VolumeSensor)] = charsToString(volumeSensor, HYDRUINO_NAME_MAXSIZE); }
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
    if (waterPHSensor[0]) { objectOut[SFP(HS_Key_PHSensor)] = charsToString(waterPHSensor, HYDRUINO_NAME_MAXSIZE); }
    if (waterTDSSensor[0]) { objectOut[SFP(HS_Key_TDSSensor)] = charsToString(waterTDSSensor, HYDRUINO_NAME_MAXSIZE); }
    if (waterTempSensor[0]) {
        objectOut[(airTempSensor[0] ? SFP(HS_Key_WaterTemperatureSensor) : SFP(HS_Key_TemperatureSensor))] = charsToString(waterTempSensor, HYDRUINO_NAME_MAXSIZE);
    }
    if (airTempSensor[0]) { objectOut[SFP(HS_Key_AirTemperatureSensor)] = charsToString(airTempSensor, HYDRUINO_NAME_MAXSIZE); }
    if (airCO2Sensor[0]) { objectOut[SFP(HS_Key_CO2Sensor)] = charsToString(airCO2Sensor, HYDRUINO_NAME_MAXSIZE); }
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
    const char *waterTempSensorStr = objectIn[SFP(HS_Key_WaterTemperatureSensor)] | objectIn[SFP(HS_Key_TemperatureSensor)];
    if (waterTempSensorStr && waterTempSensorStr[0]) { strncpy(waterTempSensor, waterTempSensorStr, HYDRUINO_NAME_MAXSIZE); }
    const char *airTempSensorStr = objectIn[SFP(HS_Key_AirTemperatureSensor)];
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
