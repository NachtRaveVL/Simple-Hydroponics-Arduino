/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydroponics Reservoirs
*/

#include "Hydroponics.h"

HydroponicsReservoir *newReservoirObjectFromData(const HydroponicsReservoirData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
            case (int8_t)HydroponicsReservoir::Fluid:
                return new HydroponicsFluidReservoir((const HydroponicsFluidReservoirData *)dataIn);
            case (int8_t)HydroponicsReservoir::Feed:
                return new HydroponicsFeedReservoir((const HydroponicsFeedReservoirData *)dataIn);
            case (int8_t)HydroponicsReservoir::Pipe:
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

void HydroponicsReservoir::update()
{
    HydroponicsObject::update();

    handleFilled(triggerStateFromBool(isFilled()));

    handleEmpty(triggerStateFromBool(isEmpty()));
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

Signal<HydroponicsReservoir *, HYDRUINO_RESERVOIR_STATE_SLOTS> &HydroponicsReservoir::getFilledSignal()
{
    return _filledSignal;
}

Signal<HydroponicsReservoir *, HYDRUINO_RESERVOIR_STATE_SLOTS> &HydroponicsReservoir::getEmptySignal()
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

void HydroponicsReservoir::handleFilled(Hydroponics_TriggerState filledState)
{
    if (filledState == Hydroponics_TriggerState_Disabled || filledState == Hydroponics_TriggerState_Undefined) { return; }

    if (_filledState != filledState) {
        _filledState = filledState;

        if (triggerStateToBool(_filledState)) {
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<HydroponicsReservoir *>(getSharedPtr(), _filledSignal, this);
            #else
                _filledSignal.fire(this);
            #endif
        }
    }
}

void HydroponicsReservoir::handleEmpty(Hydroponics_TriggerState emptyState)
{
    if (emptyState == Hydroponics_TriggerState_Disabled || emptyState == Hydroponics_TriggerState_Undefined) { return; }

    if (_emptyState != emptyState) {
        _emptyState = emptyState;

        if (triggerStateToBool(_emptyState)) {
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<HydroponicsReservoir *>(getSharedPtr(), _emptySignal, this);
            #else
                _emptySignal.fire(this);
            #endif
        }
    }
}


HydroponicsFluidReservoir::HydroponicsFluidReservoir(Hydroponics_ReservoirType reservoirType,
                                                     Hydroponics_PositionIndex reservoirIndex,
                                                     float maxVolume,
                                                     int classType)
    : HydroponicsReservoir(reservoirType, reservoirIndex, classType),
      _maxVolume(maxVolume), _waterVolume(this), _filledTrigger(this), _emptyTrigger(this)
{
    allocateLinkages(getReservoirType() == Hydroponics_ReservoirType_FeedWater ? HYDRUINO_FEEDRES_LINKS_BASESIZE : HYDRUINO_FLUIDRES_LINKS_BASESIZE);

    _filledTrigger.setHandleMethod(&HydroponicsFluidReservoir::handleFilled);
    _emptyTrigger.setHandleMethod(&HydroponicsFluidReservoir::handleEmpty);
}

HydroponicsFluidReservoir::HydroponicsFluidReservoir(const HydroponicsFluidReservoirData *dataIn)
    : HydroponicsReservoir(dataIn),
      _maxVolume(dataIn->maxVolume), _waterVolume(this), _filledTrigger(this), _emptyTrigger(this)
{
    allocateLinkages(getReservoirType() == Hydroponics_ReservoirType_FeedWater ? HYDRUINO_FEEDRES_LINKS_BASESIZE : HYDRUINO_FLUIDRES_LINKS_BASESIZE);

    _waterVolume.setObject(dataIn->volumeSensor);

    _filledTrigger.setHandleMethod(&HydroponicsFluidReservoir::handleFilled);
    _filledTrigger = newTriggerObjectFromSubData(&(dataIn->filledTrigger));
    HYDRUINO_SOFT_ASSERT(_filledTrigger, SFP(HStr_Err_AllocationFailure));

    _emptyTrigger.setHandleMethod(&HydroponicsFluidReservoir::handleEmpty);
    _emptyTrigger = newTriggerObjectFromSubData(&(dataIn->emptyTrigger));
    HYDRUINO_SOFT_ASSERT(_emptyTrigger, SFP(HStr_Err_AllocationFailure));
}

void HydroponicsFluidReservoir::update()
{
    HydroponicsReservoir::update();

    _waterVolume.updateIfNeeded(true);

    _filledTrigger.updateIfNeeded();
    _emptyTrigger.updateIfNeeded();
}

void HydroponicsFluidReservoir::handleLowMemory()
{
    HydroponicsObject::handleLowMemory();

    if (_filledTrigger) { _filledTrigger->handleLowMemory(); }
    if (_emptyTrigger) { _emptyTrigger->handleLowMemory(); }
}

bool HydroponicsFluidReservoir::isFilled()
{
    if (_filledTrigger.resolve() && triggerStateToBool(_filledTrigger.getTriggerState())) { return true; }
    return _waterVolume.getMeasurementValue() >= (_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_FRACTION_FILLED
                                                                                                                     : _maxVolume) - FLT_EPSILON;
}

bool HydroponicsFluidReservoir::isEmpty()
{
    if (_emptyTrigger.resolve() && triggerStateToBool(_emptyTrigger.getTriggerState())) { return true; }
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

HydroponicsSensorAttachment &HydroponicsFluidReservoir::getWaterVolume(bool poll)
{
    _waterVolume.updateIfNeeded(poll);
    return _waterVolume;
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

void HydroponicsFluidReservoir::handleFilled(Hydroponics_TriggerState filledState)
{
    HydroponicsReservoir::handleFilled(filledState);

    if (triggerStateToBool(_filledState) && !getWaterVolumeSensor()) {
        getWaterVolume().setMeasurement(_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_FRACTION_FILLED
                                                                                                           : _maxVolume, _volumeUnits);
    }
}

void HydroponicsFluidReservoir::handleEmpty(Hydroponics_TriggerState emptyState)
{
    HydroponicsReservoir::handleEmpty(emptyState);

    if (triggerStateToBool(_emptyState) && !getWaterVolumeSensor()) {
        getWaterVolume().setMeasurement(_id.objTypeAs.reservoirType == Hydroponics_ReservoirType_FeedWater ? _maxVolume * HYDRUINO_FEEDRES_FRACTION_EMPTY
                                                                                                           : 0, _volumeUnits);
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
       _waterPHBalancer(this), _waterTDSBalancer(this), _waterTempBalancer(this), _airTempBalancer(this), _airCO2Balancer(this)
{ ; }

HydroponicsFeedReservoir::HydroponicsFeedReservoir(const HydroponicsFeedReservoirData *dataIn)
    : HydroponicsFluidReservoir(dataIn),
      _lastChangeDate(dataIn->lastChangeDate), _lastPruningDate(dataIn->lastPruningDate),
      _lastFeedingDate(dataIn->lastFeedingDate), _numFeedingsToday(dataIn->numFeedingsToday),
      _tdsUnits(definedUnitsElse(dataIn->tdsUnits, Hydroponics_UnitsType_Concentration_TDS)),
      _tempUnits(definedUnitsElse(dataIn->tempUnits, defaultTemperatureUnits())),
      _waterPH(this), _waterTDS(this), _waterTemp(this), _airTemp(this), _airCO2(this),
      _waterPHBalancer(this), _waterTDSBalancer(this), _waterTempBalancer(this), _airTempBalancer(this), _airCO2Balancer(this)
{
    if (_lastFeedingDate) {
        auto lastFeeding = DateTime((uint32_t)(_lastFeedingDate + (getHydroponicsInstance() ? getHydroponicsInstance()->getTimeZoneOffset() * SECS_PER_HOUR : 0)));
        auto currTime = getCurrentTime();

        if (currTime.year() != lastFeeding.year() ||
            currTime.month() != lastFeeding.month() ||
            currTime.day() != lastFeeding.day()) {
            _numFeedingsToday = 0;
        }
    } else { _numFeedingsToday = 0; }

    _waterPH.setObject(dataIn->waterPHSensor);
    _waterTDS.setObject(dataIn->waterTDSSensor);
    _waterTemp.setObject(dataIn->waterTempSensor);

    _airTemp.setObject(dataIn->airTempSensor);
    _airCO2.setObject(dataIn->airCO2Sensor);
}

void HydroponicsFeedReservoir::update()
{
    HydroponicsFluidReservoir::update();

    _waterPH.updateIfNeeded(true);
    _waterTDS.updateIfNeeded(true);
    _waterTemp.updateIfNeeded(true);
    _airTemp.updateIfNeeded(true);
    _airCO2.updateIfNeeded(true);

    _waterPHBalancer.updateIfNeeded();
    _waterTDSBalancer.updateIfNeeded();
    _waterTempBalancer.updateIfNeeded();
    _airTempBalancer.updateIfNeeded();
    _airCO2Balancer.updateIfNeeded();
}

void HydroponicsFeedReservoir::handleLowMemory()
{
    HydroponicsFluidReservoir::handleLowMemory();

    if (_waterPHBalancer && !_waterPHBalancer->isEnabled()) { _waterPHBalancer.setObject(nullptr); }
    if (_waterTDSBalancer && !_waterTDSBalancer->isEnabled()) { _waterTDSBalancer.setObject(nullptr); }
    if (_waterTempBalancer && !_waterTempBalancer->isEnabled()) { _waterTempBalancer.setObject(nullptr); }

    if (_airTempBalancer && !_airTempBalancer->isEnabled()) { _airTempBalancer.setObject(nullptr); }
    if (_airCO2Balancer && !_airCO2Balancer->isEnabled()) { _airCO2Balancer.setObject(nullptr); }
}

void HydroponicsFeedReservoir::setTDSUnits(Hydroponics_UnitsType tdsUnits)
{
    if (_tdsUnits != tdsUnits) {
        _tdsUnits = tdsUnits;

        _waterTDS.setMeasurementUnits(getTDSUnits());
    }
}

void HydroponicsFeedReservoir::setTemperatureUnits(Hydroponics_UnitsType tempUnits)
{
    if (_tempUnits != tempUnits) {
        _tempUnits = tempUnits;

        _waterTemp.setMeasurementUnits(getTemperatureUnits());
        _airTemp.setMeasurementUnits(getTemperatureUnits());
    }
}

HydroponicsSensorAttachment &HydroponicsFeedReservoir::getWaterPH(bool poll)
{
    _waterPH.updateIfNeeded(poll);
    return _waterPH;
}

HydroponicsSensorAttachment &HydroponicsFeedReservoir::getWaterTDS(bool poll)
{
    _waterTDS.updateIfNeeded(poll);
    return _waterTDS;
}

HydroponicsSensorAttachment &HydroponicsFeedReservoir::getWaterTemperature(bool poll)
{
    _waterTemp.updateIfNeeded(poll);
    return _waterTemp;
}

HydroponicsSensorAttachment &HydroponicsFeedReservoir::getAirTemperature(bool poll)
{
    _airTemp.updateIfNeeded(poll);
    return _airTemp;
}

HydroponicsSensorAttachment &HydroponicsFeedReservoir::getAirCO2(bool poll)
{
    _airCO2.updateIfNeeded(poll);
    return _airCO2;
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

bool HydroponicsInfiniteReservoir::isFilled()
{
    return _alwaysFilled;
}

bool HydroponicsInfiniteReservoir::isEmpty()
{
    return !_alwaysFilled;
}

HydroponicsSensorAttachment &HydroponicsInfiniteReservoir::getWaterVolume(bool poll)
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

    if (volumeUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HStr_Key_VolumeUnits)] = unitsTypeToSymbol(volumeUnits); }
}

void HydroponicsReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);

    volumeUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_VolumeUnits)]);
}

HydroponicsFluidReservoirData::HydroponicsFluidReservoirData()
    : HydroponicsReservoirData(), maxVolume(0), volumeSensor{0}
{
    _size = sizeof(*this);
}

void HydroponicsFluidReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsReservoirData::toJSONObject(objectOut);

    objectOut[SFP(HStr_Key_MaxVolume)] = maxVolume;
    if (volumeSensor[0]) { objectOut[SFP(HStr_Key_VolumeSensor)] = charsToString(volumeSensor, HYDRUINO_NAME_MAXSIZE); }
    if (filledTrigger.type != -1) {
        JsonObject filledTriggerObj = objectOut.createNestedObject(SFP(HStr_Key_FilledTrigger));
        filledTrigger.toJSONObject(filledTriggerObj);
    }
    if (emptyTrigger.type != -1) {
        JsonObject emptyTriggerObj = objectOut.createNestedObject(SFP(HStr_Key_EmptyTrigger));
        emptyTrigger.toJSONObject(emptyTriggerObj);
    }
}

void HydroponicsFluidReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsReservoirData::fromJSONObject(objectIn);

    maxVolume = objectIn[SFP(HStr_Key_MaxVolume)] | maxVolume;
    const char *volumeSensorStr = objectIn[SFP(HStr_Key_VolumeSensor)];
    if (volumeSensorStr && volumeSensorStr[0]) { strncpy(volumeSensor, volumeSensorStr, HYDRUINO_NAME_MAXSIZE); }
    JsonObjectConst filledTriggerObj = objectIn[SFP(HStr_Key_FilledTrigger)];
    if (!filledTriggerObj.isNull()) { filledTrigger.fromJSONObject(filledTriggerObj); }
    JsonObjectConst emptyTriggerObj = objectIn[SFP(HStr_Key_EmptyTrigger)];
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

    if (lastChangeDate) { objectOut[SFP(HStr_Key_LastChangeDate)] = lastChangeDate; }
    if (lastPruningDate) { objectOut[SFP(HStr_Key_LastPruningDate)] = lastPruningDate; }
    if (lastFeedingDate) { objectOut[SFP(HStr_Key_LastFeedingDate)] = lastFeedingDate; }
    if (numFeedingsToday > 0) { objectOut[SFP(HStr_Key_NumFeedingsToday)] = numFeedingsToday; }
    if (tdsUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HStr_Key_TDSUnits)] = unitsTypeToSymbol(tdsUnits); }
    if (tempUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HStr_Key_TempUnits)] = unitsTypeToSymbol(tempUnits); }
    if (waterPHSensor[0]) { objectOut[SFP(HStr_Key_PHSensor)] = charsToString(waterPHSensor, HYDRUINO_NAME_MAXSIZE); }
    if (waterTDSSensor[0]) { objectOut[SFP(HStr_Key_TDSSensor)] = charsToString(waterTDSSensor, HYDRUINO_NAME_MAXSIZE); }
    if (waterTempSensor[0]) {
        objectOut[(airTempSensor[0] ? SFP(HStr_Key_WaterTemperatureSensor) : SFP(HStr_Key_TemperatureSensor))] = charsToString(waterTempSensor, HYDRUINO_NAME_MAXSIZE);
    }
    if (airTempSensor[0]) { objectOut[SFP(HStr_Key_AirTemperatureSensor)] = charsToString(airTempSensor, HYDRUINO_NAME_MAXSIZE); }
    if (airCO2Sensor[0]) { objectOut[SFP(HStr_Key_CO2Sensor)] = charsToString(airCO2Sensor, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsFeedReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsFluidReservoirData::fromJSONObject(objectIn);

    lastChangeDate = objectIn[SFP(HStr_Key_LastChangeDate)] | lastChangeDate;
    lastPruningDate = objectIn[SFP(HStr_Key_LastPruningDate)] | lastPruningDate;
    lastFeedingDate = objectIn[SFP(HStr_Key_LastFeedingDate)] | lastFeedingDate;
    numFeedingsToday = objectIn[SFP(HStr_Key_NumFeedingsToday)] | numFeedingsToday;
    tdsUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_TDSUnits)]);
    tempUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_TempUnits)]);
    const char *waterPHSensorStr = objectIn[SFP(HStr_Key_PHSensor)];
    if (waterPHSensorStr && waterPHSensorStr[0]) { strncpy(waterPHSensor, waterPHSensorStr, HYDRUINO_NAME_MAXSIZE); }
    const char *waterTDSSensorStr = objectIn[SFP(HStr_Key_TDSSensor)];
    if (waterTDSSensorStr && waterTDSSensorStr[0]) { strncpy(waterTDSSensor, waterTDSSensorStr, HYDRUINO_NAME_MAXSIZE); }
    const char *waterTempSensorStr = objectIn[SFP(HStr_Key_WaterTemperatureSensor)] | objectIn[SFP(HStr_Key_TemperatureSensor)];
    if (waterTempSensorStr && waterTempSensorStr[0]) { strncpy(waterTempSensor, waterTempSensorStr, HYDRUINO_NAME_MAXSIZE); }
    const char *airTempSensorStr = objectIn[SFP(HStr_Key_AirTemperatureSensor)];
    if (airTempSensorStr && airTempSensorStr[0]) { strncpy(airTempSensor, airTempSensorStr, HYDRUINO_NAME_MAXSIZE); }
    const char *airCO2SensorStr = objectIn[SFP(HStr_Key_CO2Sensor)];
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

    objectOut[SFP(HStr_Key_AlwaysFilled)] = alwaysFilled;
}

void HydroponicsInfiniteReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsReservoirData::fromJSONObject(objectIn);

    alwaysFilled = objectIn[SFP(HStr_Key_AlwaysFilled)] | alwaysFilled;
}
