/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Reservoirs
*/

#include "Hydruino.h"

HydroReservoir *newReservoirObjectFromData(const HydroReservoirData *dataIn)
{
    if (dataIn && isValidType(dataIn->id.object.idType)) return nullptr;
    HYDRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
            case (hid_t)HydroReservoir::Fluid:
                return new HydroFluidReservoir((const HydroFluidReservoirData *)dataIn);
            case (hid_t)HydroReservoir::Feed:
                return new HydroFeedReservoir((const HydroFeedReservoirData *)dataIn);
            case (hid_t)HydroReservoir::Pipe:
                return new HydroInfiniteReservoir((const HydroInfiniteReservoirData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroReservoir::HydroReservoir(Hydro_ReservoirType reservoirType, hposi_t reservoirIndex, int classTypeIn)
    : HydroObject(HydroIdentity(reservoirType, reservoirIndex)), classType((typeof(classType))classTypeIn),
      HydroVolumeUnitsInterfaceStorage(defaultVolumeUnits()),
      _filledState(Hydro_TriggerState_Disabled), _emptyState(Hydro_TriggerState_Disabled)
{ ; }

HydroReservoir::HydroReservoir(const HydroReservoirData *dataIn)
    : HydroObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      HydroVolumeUnitsInterfaceStorage(definedUnitsElse(dataIn->volumeUnits, defaultVolumeUnits())),
      _filledState(Hydro_TriggerState_Disabled), _emptyState(Hydro_TriggerState_Disabled)
{ ; }

void HydroReservoir::update()
{
    HydroObject::update();

    handleFilled(triggerStateFromBool(isFilled()));

    handleEmpty(triggerStateFromBool(isEmpty()));
}

bool HydroReservoir::canActivate(HydroActuator *actuator)
{
    bool doEmptyCheck;

    if (actuator->isPumpType()) {
        doEmptyCheck = (actuator->getParentReservoir().get() == this);
    } else if (getActuatorInWaterFromType(actuator->getActuatorType())) {
        doEmptyCheck = true;
    } else {
        return true;
    }

    if (doEmptyCheck) {
        return !isEmpty(true);
    } else {
        return !isFilled(true);
    }
}

void HydroReservoir::setVolumeUnits(Hydro_UnitsType volumeUnits)
{
    if (_volumeUnits != volumeUnits) {
        _volumeUnits = volumeUnits;
        bumpRevisionIfNeeded();
    }
}

Signal<HydroReservoir *, HYDRO_RESERVOIR_SIGNAL_SLOTS> &HydroReservoir::getFilledSignal()
{
    return _filledSignal;
}

Signal<HydroReservoir *, HYDRO_RESERVOIR_SIGNAL_SLOTS> &HydroReservoir::getEmptySignal()
{
    return _emptySignal;
}

HydroData *HydroReservoir::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void HydroReservoir::saveToData(HydroData *dataOut)
{
    HydroObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
    ((HydroReservoirData *)dataOut)->volumeUnits = _volumeUnits;
}

void HydroReservoir::handleFilled(Hydro_TriggerState filledState)
{
    if (filledState == Hydro_TriggerState_Disabled || filledState == Hydro_TriggerState_Undefined) { return; }

    if (_filledState != filledState) {
        _filledState = filledState;

        if (triggerStateToBool(_filledState)) {
            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<HydroReservoir *>(getSharedPtr(), _filledSignal, this);
            #else
                _filledSignal.fire(this);
            #endif
        }
    }
}

void HydroReservoir::handleEmpty(Hydro_TriggerState emptyState)
{
    if (emptyState == Hydro_TriggerState_Disabled || emptyState == Hydro_TriggerState_Undefined) { return; }

    if (_emptyState != emptyState) {
        _emptyState = emptyState;

        if (triggerStateToBool(_emptyState)) {
            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<HydroReservoir *>(getSharedPtr(), _emptySignal, this);
            #else
                _emptySignal.fire(this);
            #endif
        }
    }
}


HydroFluidReservoir::HydroFluidReservoir(Hydro_ReservoirType reservoirType, hposi_t reservoirIndex, float maxVolume, int classType)
    : HydroReservoir(reservoirType, reservoirIndex, classType),
      _maxVolume(maxVolume), _waterVolume(this), _filledTrigger(this), _emptyTrigger(this)
{
    allocateLinkages(getReservoirType() == Hydro_ReservoirType_FeedWater ? HYDRO_FEEDRES_LINKS_BASESIZE : HYDRO_FLUIDRES_LINKS_BASESIZE);

    _filledTrigger.setHandleMethod(&HydroFluidReservoir::handleFilled);
    _emptyTrigger.setHandleMethod(&HydroFluidReservoir::handleEmpty);
}

HydroFluidReservoir::HydroFluidReservoir(const HydroFluidReservoirData *dataIn)
    : HydroReservoir(dataIn),
      _maxVolume(dataIn->maxVolume), _waterVolume(this), _filledTrigger(this), _emptyTrigger(this)
{
    allocateLinkages(getReservoirType() == Hydro_ReservoirType_FeedWater ? HYDRO_FEEDRES_LINKS_BASESIZE : HYDRO_FLUIDRES_LINKS_BASESIZE);

    _waterVolume.initObject(dataIn->volumeSensor);

    _filledTrigger.setHandleMethod(&HydroFluidReservoir::handleFilled);
    _filledTrigger = newTriggerObjectFromSubData(&(dataIn->filledTrigger));
    HYDRO_SOFT_ASSERT(_filledTrigger, SFP(HStr_Err_AllocationFailure));

    _emptyTrigger.setHandleMethod(&HydroFluidReservoir::handleEmpty);
    _emptyTrigger = newTriggerObjectFromSubData(&(dataIn->emptyTrigger));
    HYDRO_SOFT_ASSERT(_emptyTrigger, SFP(HStr_Err_AllocationFailure));
}

void HydroFluidReservoir::update()
{
    HydroReservoir::update();

    _waterVolume.updateIfNeeded(true);

    _filledTrigger.updateIfNeeded(true);
    _emptyTrigger.updateIfNeeded(true);
}

SharedPtr<HydroObjInterface> HydroFluidReservoir::getSharedPtrFor(const HydroObjInterface *obj) const
{
    return obj->getKey() == _filledTrigger.getKey() ? _filledTrigger.getSharedPtrFor(obj) :
           obj->getKey() == _emptyTrigger.getKey() ? _emptyTrigger.getSharedPtrFor(obj) :
           HydroObject::getSharedPtrFor(obj);
}

bool HydroFluidReservoir::isFilled(bool poll)
{
    if (triggerStateToBool(_filledTrigger.getTriggerState(poll))) { return true; }
    return _waterVolume.getMeasurementValue(poll) >= (_id.objTypeAs.reservoirType == Hydro_ReservoirType_FeedWater ? _maxVolume * HYDRO_FEEDRES_FRACTION_FILLED
                                                                                                                   : _maxVolume) - FLT_EPSILON;
}

bool HydroFluidReservoir::isEmpty(bool poll)
{
    if (triggerStateToBool(_emptyTrigger.getTriggerState(poll))) { return true; }
    return _waterVolume.getMeasurementValue(poll) <= (_id.objTypeAs.reservoirType == Hydro_ReservoirType_FeedWater ? _maxVolume * HYDRO_FEEDRES_FRACTION_EMPTY
                                                                                                                   : 0) + FLT_EPSILON;
}

void HydroFluidReservoir::setVolumeUnits(Hydro_UnitsType volumeUnits)
{
    if (_volumeUnits != volumeUnits) {
        _volumeUnits = volumeUnits;

        _waterVolume.setMeasurementUnits(volumeUnits);
        bumpRevisionIfNeeded();
    }
}

HydroSensorAttachment &HydroFluidReservoir::getWaterVolumeSensorAttachment()
{
    return _waterVolume;
}

HydroTriggerAttachment &HydroFluidReservoir::getFilledTriggerAttachment()
{
    return _filledTrigger;
}

HydroTriggerAttachment &HydroFluidReservoir::getEmptyTriggerAttachment()
{
    return _emptyTrigger;
}

void HydroFluidReservoir::saveToData(HydroData *dataOut)
{
    HydroReservoir::saveToData(dataOut);

    ((HydroFluidReservoirData *)dataOut)->maxVolume = roundForExport(_maxVolume, 1);
    if (_waterVolume.isSet()) {
        strncpy(((HydroFluidReservoirData *)dataOut)->volumeSensor, _waterVolume.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_filledTrigger.isSet()) {
        _filledTrigger->saveToData(&(((HydroFluidReservoirData *)dataOut)->filledTrigger));
    }
    if (_emptyTrigger.isSet()) {
        _emptyTrigger->saveToData(&(((HydroFluidReservoirData *)dataOut)->emptyTrigger));
    }
}

void HydroFluidReservoir::handleFilled(Hydro_TriggerState filledState)
{
    HydroReservoir::handleFilled(filledState);

    if (triggerStateToBool(_filledState) && !getWaterVolumeSensor()) {
        getWaterVolumeSensorAttachment().setMeasurement(_id.objTypeAs.reservoirType == Hydro_ReservoirType_FeedWater ? _maxVolume * HYDRO_FEEDRES_FRACTION_FILLED
                                                                                                                     : _maxVolume, _volumeUnits);
    }
}

void HydroFluidReservoir::handleEmpty(Hydro_TriggerState emptyState)
{
    HydroReservoir::handleEmpty(emptyState);

    if (triggerStateToBool(_emptyState) && !getWaterVolumeSensor()) {
        getWaterVolumeSensorAttachment().setMeasurement(_id.objTypeAs.reservoirType == Hydro_ReservoirType_FeedWater ? _maxVolume * HYDRO_FEEDRES_FRACTION_EMPTY
                                                                                                                     : 0, _volumeUnits);
    }
}


HydroFeedReservoir::HydroFeedReservoir(hposi_t reservoirIndex, float maxVolume, DateTime lastChangeTime, DateTime lastPruningTime, int classType)
    : HydroFluidReservoir(Hydro_ReservoirType_FeedWater, reservoirIndex, maxVolume, classType),
      HydroAirConcentrateUnitsInterfaceStorage(Hydro_UnitsType_Concentration_PPM),
      HydroTemperatureUnitsInterfaceStorage(defaultTemperatureUnits()),
      HydroWaterConcentrateUnitsInterfaceStorage(Hydro_UnitsType_Concentration_TDS),
      _lastChangeTime(unixTime(lastChangeTime)), _lastPruningTime(unixTime(lastPruningTime)), _lastFeedingTime(0), _numFeedingsToday(0),
      _waterPH(this), _waterTDS(this), _waterTemp(this), _airTemp(this), _airCO2(this),
      _waterPHBalancer(this), _waterTDSBalancer(this), _waterTempBalancer(this), _airTempBalancer(this), _airCO2Balancer(this)
{ ; }

HydroFeedReservoir::HydroFeedReservoir(const HydroFeedReservoirData *dataIn)
    : HydroFluidReservoir(dataIn),
      HydroAirConcentrateUnitsInterfaceStorage(definedUnitsElse(dataIn->airConcentrateUnits, Hydro_UnitsType_Concentration_PPM)),
      HydroTemperatureUnitsInterfaceStorage(definedUnitsElse(dataIn->temperatureUnits, defaultTemperatureUnits())),
      HydroWaterConcentrateUnitsInterfaceStorage(definedUnitsElse(dataIn->waterConcentrateUnits, Hydro_UnitsType_Concentration_TDS)),
      _lastChangeTime(dataIn->lastChangeTime), _lastPruningTime(dataIn->lastPruningTime),
      _lastFeedingTime(dataIn->lastFeedingTime), _numFeedingsToday(dataIn->numFeedingsToday),
      _waterPH(this), _waterTDS(this), _waterTemp(this), _airTemp(this), _airCO2(this),
      _waterPHBalancer(this), _waterTDSBalancer(this), _waterTempBalancer(this), _airTempBalancer(this), _airCO2Balancer(this)
{
    setTemperatureUnits(dataIn->temperatureUnits);

    if (_lastFeedingTime) {
        auto lastFeeding = localTime(_lastFeedingTime);
        auto currTime = localNow();

        if (currTime.year() != lastFeeding.year() ||
            currTime.month() != lastFeeding.month() ||
            currTime.day() != lastFeeding.day()) {
            _numFeedingsToday = 0;
        }
    } else { _numFeedingsToday = 0; }

    _waterPH.initObject(dataIn->waterPHSensor);
    _waterTDS.initObject(dataIn->waterTDSSensor);
    _waterTemp.initObject(dataIn->waterTempSensor);

    _airTemp.initObject(dataIn->airTempSensor);
    _airCO2.initObject(dataIn->airCO2Sensor);
}

void HydroFeedReservoir::update()
{
    HydroFluidReservoir::update();

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

void HydroFeedReservoir::handleLowMemory()
{
    if (_waterPHBalancer && !_waterPHBalancer->isEnabled()) { _waterPHBalancer.setObject(nullptr); }
    if (_waterTDSBalancer && !_waterTDSBalancer->isEnabled()) { _waterTDSBalancer.setObject(nullptr); }
    if (_waterTempBalancer && !_waterTempBalancer->isEnabled()) { _waterTempBalancer.setObject(nullptr); }

    if (_airTempBalancer && !_airTempBalancer->isEnabled()) { _airTempBalancer.setObject(nullptr); }
    if (_airCO2Balancer && !_airCO2Balancer->isEnabled()) { _airCO2Balancer.setObject(nullptr); }

    HydroObject::handleLowMemory();
}

SharedPtr<HydroObjInterface> HydroFeedReservoir::getSharedPtrFor(const HydroObjInterface *obj) const
{
    return obj->getKey() == _waterPHBalancer.getKey() ? _waterPHBalancer.getSharedPtrFor(obj) :
           obj->getKey() == _waterTDSBalancer.getKey() ? _waterTDSBalancer.getSharedPtrFor(obj) :
           obj->getKey() == _waterTempBalancer.getKey() ? _waterTempBalancer.getSharedPtrFor(obj) :
           obj->getKey() == _airTempBalancer.getKey() ? _airTempBalancer.getSharedPtrFor(obj) :
           obj->getKey() == _airCO2Balancer.getKey() ? _airCO2Balancer.getSharedPtrFor(obj) :
           HydroFluidReservoir::getSharedPtrFor(obj);
}

void HydroFeedReservoir::setAirConcentrateUnits(Hydro_UnitsType airConcentrateUnits)
{
    if (_airConcUnits != airConcentrateUnits) {
        _airConcUnits = airConcentrateUnits;

        _airCO2.setMeasurementUnits(getAirConcentrateUnits());
        bumpRevisionIfNeeded();
    }
}

void HydroFeedReservoir::setTemperatureUnits(Hydro_UnitsType temperatureUnits)
{
    if (_tempUnits != temperatureUnits) {
        _tempUnits = temperatureUnits;

        _waterTemp.setMeasurementUnits(getTemperatureUnits());
        _airTemp.setMeasurementUnits(getTemperatureUnits());
        bumpRevisionIfNeeded();
    }
}

void HydroFeedReservoir::setWaterConcentrateUnits(Hydro_UnitsType waterConcentrateUnits)
{
    if (_waterConcUnits != waterConcentrateUnits) {
        _waterConcUnits = waterConcentrateUnits;

        _waterTDS.setMeasurementUnits(getWaterConcentrateUnits());
        bumpRevisionIfNeeded();
    }
}

HydroSensorAttachment &HydroFeedReservoir::getWaterPHSensorAttachment()
{
    return _waterPH;
}

HydroSensorAttachment &HydroFeedReservoir::getWaterTDSSensorAttachment()
{
    return _waterTDS;
}

HydroSensorAttachment &HydroFeedReservoir::getWaterTemperatureSensorAttachment()
{
    return _waterTemp;
}

HydroSensorAttachment &HydroFeedReservoir::getAirTemperatureSensorAttachment()
{
    return _airTemp;
}

HydroSensorAttachment &HydroFeedReservoir::getAirCO2SensorAttachment()
{
    return _airCO2;
}

void HydroFeedReservoir::saveToData(HydroData *dataOut)
{
    HydroFluidReservoir::saveToData(dataOut);

    ((HydroFeedReservoirData *)dataOut)->lastChangeTime = _lastChangeTime;
    ((HydroFeedReservoirData *)dataOut)->lastPruningTime = _lastPruningTime;
    ((HydroFeedReservoirData *)dataOut)->lastFeedingTime = _lastFeedingTime;
    ((HydroFeedReservoirData *)dataOut)->numFeedingsToday = _numFeedingsToday;
    ((HydroFeedReservoirData *)dataOut)->airConcentrateUnits = _airConcUnits;
    ((HydroFeedReservoirData *)dataOut)->temperatureUnits = _tempUnits;
    ((HydroFeedReservoirData *)dataOut)->waterConcentrateUnits = _waterConcUnits;
    if (_waterPH.isSet()) {
        strncpy(((HydroFeedReservoirData *)dataOut)->waterPHSensor, _waterPH.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_waterTDS.isSet()) {
        strncpy(((HydroFeedReservoirData *)dataOut)->waterTDSSensor, _waterTDS.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_waterTemp.isSet()) {
        strncpy(((HydroFeedReservoirData *)dataOut)->waterTempSensor, _waterTemp.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_airTemp.isSet()) {
        strncpy(((HydroFeedReservoirData *)dataOut)->airTempSensor, _airTemp.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_airCO2.isSet()) {
        strncpy(((HydroFeedReservoirData *)dataOut)->airCO2Sensor, _airCO2.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
}


HydroInfiniteReservoir::HydroInfiniteReservoir(Hydro_ReservoirType reservoirType, hposi_t reservoirIndex, bool alwaysFilled, int classType)
    : HydroReservoir(reservoirType, reservoirIndex, classType), _alwaysFilled(alwaysFilled), _waterVolume(this)
{ ; }

HydroInfiniteReservoir::HydroInfiniteReservoir(const HydroInfiniteReservoirData *dataIn)
    : HydroReservoir(dataIn), _alwaysFilled(dataIn->alwaysFilled), _waterVolume(this)
{ ; }

bool HydroInfiniteReservoir::isFilled(bool poll)
{
    return _alwaysFilled;
}

bool HydroInfiniteReservoir::isEmpty(bool poll)
{
    return !_alwaysFilled;
}

HydroSensorAttachment &HydroInfiniteReservoir::getWaterVolumeSensorAttachment()
{
    _waterVolume.setMeasurement(HydroSingleMeasurement(
        _alwaysFilled ? FLT_UNDEF : 0.0f,
        getVolumeUnits(),
        unixNow(),
        getController() ? getController()->getPollingFrame() : 1
    ));
    return _waterVolume;
}

void HydroInfiniteReservoir::saveToData(HydroData *dataOut)
{
    HydroReservoir::saveToData(dataOut);

    ((HydroInfiniteReservoirData *)dataOut)->alwaysFilled = _alwaysFilled;
}


HydroReservoirData::HydroReservoirData()
    : HydroObjectData(), volumeUnits(Hydro_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void HydroReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroObjectData::toJSONObject(objectOut);

    if (volumeUnits != Hydro_UnitsType_Undefined) { objectOut[SFP(HStr_Key_VolumeUnits)] = unitsTypeToSymbol(volumeUnits); }
}

void HydroReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroObjectData::fromJSONObject(objectIn);

    volumeUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_VolumeUnits)]);
}

HydroFluidReservoirData::HydroFluidReservoirData()
    : HydroReservoirData(), maxVolume(0), volumeSensor{0}
{
    _size = sizeof(*this);
}

void HydroFluidReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroReservoirData::toJSONObject(objectOut);

    objectOut[SFP(HStr_Key_MaxVolume)] = maxVolume;
    if (volumeSensor[0]) { objectOut[SFP(HStr_Key_VolumeSensor)] = charsToString(volumeSensor, HYDRO_NAME_MAXSIZE); }
    if (filledTrigger.isSet()) {
        JsonObject filledTriggerObj = objectOut.createNestedObject(SFP(HStr_Key_FilledTrigger));
        filledTrigger.toJSONObject(filledTriggerObj);
    }
    if (emptyTrigger.isSet()) {
        JsonObject emptyTriggerObj = objectOut.createNestedObject(SFP(HStr_Key_EmptyTrigger));
        emptyTrigger.toJSONObject(emptyTriggerObj);
    }
}

void HydroFluidReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroReservoirData::fromJSONObject(objectIn);

    maxVolume = objectIn[SFP(HStr_Key_MaxVolume)] | maxVolume;
    const char *volumeSensorStr = objectIn[SFP(HStr_Key_VolumeSensor)];
    if (volumeSensorStr && volumeSensorStr[0]) { strncpy(volumeSensor, volumeSensorStr, HYDRO_NAME_MAXSIZE); }
    JsonObjectConst filledTriggerObj = objectIn[SFP(HStr_Key_FilledTrigger)];
    if (!filledTriggerObj.isNull()) { filledTrigger.fromJSONObject(filledTriggerObj); }
    JsonObjectConst emptyTriggerObj = objectIn[SFP(HStr_Key_EmptyTrigger)];
    if (!emptyTriggerObj.isNull()) { emptyTrigger.fromJSONObject(emptyTriggerObj); }
}

HydroFeedReservoirData::HydroFeedReservoirData()
    : HydroFluidReservoirData(), lastChangeTime(0), lastPruningTime(0), lastFeedingTime(0), numFeedingsToday(0),
      airConcentrateUnits(Hydro_UnitsType_Undefined),
      temperatureUnits(Hydro_UnitsType_Undefined),
      waterConcentrateUnits(Hydro_UnitsType_Undefined),
      waterPHSensor{0}, waterTDSSensor{0}, waterTempSensor{0}, airTempSensor{0}, airCO2Sensor{0}
{
    _size = sizeof(*this);
}

void HydroFeedReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroFluidReservoirData::toJSONObject(objectOut);

    if (lastChangeTime) { objectOut[SFP(HStr_Key_LastChangeTime)] = lastChangeTime; }
    if (lastPruningTime) { objectOut[SFP(HStr_Key_LastPruningTime)] = lastPruningTime; }
    if (lastFeedingTime) { objectOut[SFP(HStr_Key_LastFeedingTime)] = lastFeedingTime; }
    if (numFeedingsToday > 0) { objectOut[SFP(HStr_Key_NumFeedingsToday)] = numFeedingsToday; }
    if (airConcentrateUnits != Hydro_UnitsType_Undefined) { objectOut[SFP(HStr_Key_AirConcentrateUnits)] = unitsTypeToSymbol(airConcentrateUnits); }
    if (temperatureUnits != Hydro_UnitsType_Undefined) { objectOut[SFP(HStr_Key_TemperatureUnits)] = unitsTypeToSymbol(temperatureUnits); }
    if (waterConcentrateUnits != Hydro_UnitsType_Undefined) {
        objectOut[(airConcentrateUnits != Hydro_UnitsType_Undefined ? SFP(HStr_Key_WaterConcentrateUnits) : SFP(HStr_Key_ConcentrateUnits))] = unitsTypeToSymbol(waterConcentrateUnits);
    }
    if (waterPHSensor[0]) { objectOut[SFP(HStr_Key_PHSensor)] = charsToString(waterPHSensor, HYDRO_NAME_MAXSIZE); }
    if (waterTDSSensor[0]) { objectOut[SFP(HStr_Key_TDSSensor)] = charsToString(waterTDSSensor, HYDRO_NAME_MAXSIZE); }
    if (waterTempSensor[0]) {
        objectOut[(airTempSensor[0] ? SFP(HStr_Key_WaterTemperatureSensor) : SFP(HStr_Key_TemperatureSensor))] = charsToString(waterTempSensor, HYDRO_NAME_MAXSIZE);
    }
    if (airTempSensor[0]) { objectOut[SFP(HStr_Key_AirTemperatureSensor)] = charsToString(airTempSensor, HYDRO_NAME_MAXSIZE); }
    if (airCO2Sensor[0]) { objectOut[SFP(HStr_Key_CO2Sensor)] = charsToString(airCO2Sensor, HYDRO_NAME_MAXSIZE); }
}

void HydroFeedReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroFluidReservoirData::fromJSONObject(objectIn);

    lastChangeTime = objectIn[SFP(HStr_Key_LastChangeTime)] | lastChangeTime;
    lastPruningTime = objectIn[SFP(HStr_Key_LastPruningTime)] | lastPruningTime;
    lastFeedingTime = objectIn[SFP(HStr_Key_LastFeedingTime)] | lastFeedingTime;
    numFeedingsToday = objectIn[SFP(HStr_Key_NumFeedingsToday)] | numFeedingsToday;
    airConcentrateUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_AirConcentrateUnits)]);
    temperatureUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_TemperatureUnits)]);
    waterConcentrateUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_WaterConcentrateUnits)] | objectIn[SFP(HStr_Key_ConcentrateUnits)]);
    const char *waterPHSensorStr = objectIn[SFP(HStr_Key_PHSensor)];
    if (waterPHSensorStr && waterPHSensorStr[0]) { strncpy(waterPHSensor, waterPHSensorStr, HYDRO_NAME_MAXSIZE); }
    const char *waterTDSSensorStr = objectIn[SFP(HStr_Key_TDSSensor)];
    if (waterTDSSensorStr && waterTDSSensorStr[0]) { strncpy(waterTDSSensor, waterTDSSensorStr, HYDRO_NAME_MAXSIZE); }
    const char *waterTempSensorStr = objectIn[SFP(HStr_Key_WaterTemperatureSensor)] | objectIn[SFP(HStr_Key_TemperatureSensor)];
    if (waterTempSensorStr && waterTempSensorStr[0]) { strncpy(waterTempSensor, waterTempSensorStr, HYDRO_NAME_MAXSIZE); }
    const char *airTempSensorStr = objectIn[SFP(HStr_Key_AirTemperatureSensor)];
    if (airTempSensorStr && airTempSensorStr[0]) { strncpy(airTempSensor, airTempSensorStr, HYDRO_NAME_MAXSIZE); }
    const char *airCO2SensorStr = objectIn[SFP(HStr_Key_CO2Sensor)];
    if (airCO2SensorStr && airCO2SensorStr[0]) { strncpy(airCO2Sensor, airCO2SensorStr, HYDRO_NAME_MAXSIZE); }
}

HydroInfiniteReservoirData::HydroInfiniteReservoirData()
    : HydroReservoirData(), alwaysFilled(true)
{
    _size = sizeof(*this);
}

void HydroInfiniteReservoirData::toJSONObject(JsonObject &objectOut) const
{
    HydroReservoirData::toJSONObject(objectOut);

    objectOut[SFP(HStr_Key_AlwaysFilled)] = alwaysFilled;
}

void HydroInfiniteReservoirData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroReservoirData::fromJSONObject(objectIn);

    alwaysFilled = objectIn[SFP(HStr_Key_AlwaysFilled)] | alwaysFilled;
}
