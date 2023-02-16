/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Crops
*/

#include "Hydruino.h"

HydroCrop *newCropObjectFromData(const HydroCropData *dataIn)
{
    if (dataIn && isValidType(dataIn->id.object.idType)) return nullptr;
    HYDRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
            case (int8_t)HydroCrop::Timed:
                return new HydroTimedCrop((const HydroTimedCropData *)dataIn);
            case (int8_t)HydroCrop::Adaptive:
                return new HydroAdaptiveCrop((const HydroAdaptiveCropData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroCrop::HydroCrop(Hydro_CropType cropType, hposi_t cropIndex, Hydro_SubstrateType substrateType, DateTime sowTime, int classTypeIn)
    : HydroObject(HydroIdentity(cropType, cropIndex)), classType((typeof(classType))classTypeIn),
      _substrateType(substrateType), _sowTime(unixTime(sowTime)), _feedReservoir(this), _cropsData(nullptr), _growWeek(0), _feedingWeight(1.0f),
      _cropPhase(Hydro_CropPhase_Undefined), _feedingState(Hydro_TriggerState_NotTriggered)
{
    allocateLinkages(HYDRO_CROPS_LINKS_BASESIZE);

    recalcGrowthParams();
}

HydroCrop::HydroCrop(const HydroCropData *dataIn)
    : HydroObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _substrateType(dataIn->substrateType), _sowTime(dataIn->sowTime), _feedReservoir(this),
      _cropsData(nullptr), _growWeek(0), _feedingWeight(dataIn->feedingWeight),
      _cropPhase(Hydro_CropPhase_Undefined), _feedingState(Hydro_TriggerState_NotTriggered)
{
    allocateLinkages(HYDRO_CROPS_LINKS_BASESIZE);

    recalcGrowthParams();
}

HydroCrop::~HydroCrop()
{
    if (_cropsData) { returnCropsLibData(); }
}

void HydroCrop::update()
{
    HydroObject::update();

    _feedReservoir.resolve();

    handleFeeding(triggerStateFromBool(needsFeeding(true)));
}

void HydroCrop::handleLowMemory()
{
    HydroObject::handleLowMemory();

    returnCropsLibData();
}

void HydroCrop::notifyFeedingBegan()
{ ; }

void HydroCrop::notifyFeedingEnded()
{ ; }

HydroAttachment &HydroCrop::getFeed()
{
    return _feedReservoir;
}

void HydroCrop::setFeedingWeight(float weight)
{
    if (!isFPEqual(_feedingWeight, weight)) {
        _feedingWeight = weight;

        getScheduler()->setNeedsScheduling();
    }
}

Signal<HydroCrop *, HYDRO_FEEDING_SIGNAL_SLOTS> &HydroCrop::getFeedingSignal()
{
    return _feedingSignal;
}

void HydroCrop::notifyDayChanged()
{
    recalcGrowthParams();
}

HydroData *HydroCrop::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void HydroCrop::saveToData(HydroData *dataOut)
{
    HydroObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
    ((HydroCropData *)dataOut)->substrateType = _substrateType;
    ((HydroCropData *)dataOut)->sowTime = _sowTime;
    if (_feedReservoir.getId()) {
        strncpy(((HydroCropData *)dataOut)->feedReservoir, _feedReservoir.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    ((HydroCropData *)dataOut)->feedingWeight = _feedingWeight;
}

void HydroCrop::handleFeeding(Hydro_TriggerState feedingState)
{
    if (feedingState == Hydro_TriggerState_Disabled || feedingState == Hydro_TriggerState_Undefined) { return; }

    if (_feedingState != feedingState) {
        _feedingState = feedingState;

        #ifdef HYDRO_USE_MULTITASKING
            scheduleSignalFireOnce<HydroCrop *>(getSharedPtr(), _feedingSignal, this);
        #else
            _feedingSignal.fire(this);
        #endif
    }
}

void HydroCrop::recalcGrowthParams()
{
    TimeSpan dateSpan(unixNow() - _sowTime);
    _growWeek = dateSpan.days() / DAYS_PER_WEEK;

    if (!_cropsData) { checkoutCropsLibData(); }
    HYDRO_SOFT_ASSERT(_cropsData, F("Invalid crops lib data, unable to update growth cycle"));

    if (_cropsData) {
        _totalGrowWeeks = _cropsData->totalGrowWeeks;
        _cropPhase = Hydro_CropPhase_Seedling;
        for (int phaseIndex = 0; phaseIndex < (int)Hydro_CropPhase_MainCount; ++phaseIndex) {
            if (_growWeek > _cropsData->phaseDurationWeeks[phaseIndex]) {
                _cropPhase = (Hydro_CropPhase)(phaseIndex + 1);
            } else { break; }
        }
    }
}

void HydroCrop::checkoutCropsLibData()
{
    if (!_cropsData) {
        _cropsData = hydroCropsLib.checkoutCropsData(_id.objTypeAs.cropType);
    }
}

void HydroCrop::returnCropsLibData()
{
    if (_cropsData) {
        hydroCropsLib.returnCropsData(_cropsData); _cropsData = nullptr;
    }
}

void HydroCrop::handleCustomCropUpdated(Hydro_CropType cropType)
{
    if (getCropType() == cropType) {
        returnCropsLibData(); // forces re-checkout
        recalcGrowthParams();

        if (getScheduler()) {
            getScheduler()->setNeedsScheduling();
        }
    }
}


HydroTimedCrop::HydroTimedCrop(Hydro_CropType cropType, hposi_t cropIndex, Hydro_SubstrateType substrateType, DateTime sowTime, TimeSpan timeOn, TimeSpan timeOff, int classType)
    : HydroCrop(cropType, cropIndex, substrateType, sowTime, classType),
      _lastFeedingTime(),
      _feedTimingMins{timeOn.totalseconds() / SECS_PER_MIN, timeOff.totalseconds() / SECS_PER_MIN}
{ ; }

HydroTimedCrop::HydroTimedCrop(const HydroTimedCropData *dataIn)
    : HydroCrop(dataIn),
      _lastFeedingTime(dataIn->lastFeedingTime),
      _feedTimingMins{dataIn->feedTimingMins[0], dataIn->feedTimingMins[1]}
{ ; }

bool HydroTimedCrop::needsFeeding(bool poll)
{
    time_t time = unixNow();
    return time >= _lastFeedingTime + ((_feedTimingMins[0] + _feedTimingMins[1]) * SECS_PER_MIN) ||
           time < _lastFeedingTime + (_feedTimingMins[0] * SECS_PER_MIN);
}

void HydroTimedCrop::notifyFeedingBegan()
{
    HydroCrop::notifyFeedingBegan();

    _lastFeedingTime = unixNow();
}

void HydroTimedCrop::setFeedTimeOn(TimeSpan timeOn)
{
    _feedTimingMins[0] = timeOn.totalseconds() / SECS_PER_MIN;
}

void HydroTimedCrop::setFeedTimeOff(TimeSpan timeOff)
{
    _feedTimingMins[1] = timeOff.totalseconds() / SECS_PER_MIN;
}

void HydroTimedCrop::saveToData(HydroData *dataOut)
{
    HydroCrop::saveToData(dataOut);

    ((HydroTimedCropData *)dataOut)->lastFeedingTime = _lastFeedingTime;
    ((HydroTimedCropData *)dataOut)->feedTimingMins[0] = _feedTimingMins[0];
    ((HydroTimedCropData *)dataOut)->feedTimingMins[1] = _feedTimingMins[1];
}


HydroAdaptiveCrop::HydroAdaptiveCrop(Hydro_CropType cropType, hposi_t cropIndex, Hydro_SubstrateType substrateType, DateTime sowTime, int classType)
    : HydroCrop(cropType, cropIndex, substrateType, sowTime, classType),
      HydroConcentrateUnitsInterface(defaultConcentrateUnits()),
      _soilMoisture(this), _feedingTrigger(this)
{
    _soilMoisture.setMeasureUnits(getConcentrateUnits());

    _feedingTrigger.setHandleMethod(&HydroCrop::handleFeeding);
}

HydroAdaptiveCrop::HydroAdaptiveCrop(const HydroAdaptiveCropData *dataIn)
    : HydroCrop(dataIn), _soilMoisture(this), _feedingTrigger(this),
    HydroConcentrateUnitsInterface(definedUnitsElse(dataIn->concentrateUnits, defaultConcentrateUnits()))
{
    _soilMoisture.setMeasureUnits(definedUnitsElse(dataIn->concentrateUnits, getConcentrateUnits()));
    _soilMoisture.setObject(dataIn->moistureSensor);

    _feedingTrigger.setHandleMethod(&HydroCrop::handleFeeding);
    _feedingTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->feedingTrigger)));
    HYDRO_SOFT_ASSERT(_feedingTrigger, SFP(HStr_Err_AllocationFailure));
}

void HydroAdaptiveCrop::update()
{
    HydroCrop::update();

    _soilMoisture.updateIfNeeded(true);

    _feedingTrigger.updateIfNeeded();
}

void HydroAdaptiveCrop::handleLowMemory()
{
    HydroCrop::handleLowMemory();

    if (_feedingTrigger) { _feedingTrigger->handleLowMemory(); }
}

bool HydroAdaptiveCrop::needsFeeding(bool poll)
{
    if (_feedingTrigger.resolve() && triggerStateToBool(_feedingTrigger.getTriggerState(poll))) { return true; }
    return _soilMoisture.resolve() && _soilMoisture.getMeasurementValue(poll) <= 0; // FIXME: Default value for soil moisture.
}

void HydroAdaptiveCrop::setConcentrateUnits(Hydro_UnitsType concentrateUnits)
{
    if (_concUnits != concentrateUnits) {
        _concUnits = concentrateUnits;

        _soilMoisture.setMeasureUnits(getConcentrateUnits());
    }
}

HydroSensorAttachment &HydroAdaptiveCrop::getSoilMoisture()
{
    return _soilMoisture;
}

HydroTriggerAttachment &HydroAdaptiveCrop::getFeeding()
{
    return _feedingTrigger;
}

void HydroAdaptiveCrop::saveToData(HydroData *dataOut)
{
    HydroCrop::saveToData(dataOut);

    ((HydroAdaptiveCropData *)dataOut)->concentrateUnits = _concUnits;
    if (_soilMoisture.getId()) {
        strncpy(((HydroAdaptiveCropData *)dataOut)->moistureSensor, _soilMoisture.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_feedingTrigger) {
        _feedingTrigger->saveToData(&(((HydroAdaptiveCropData *)dataOut)->feedingTrigger));
    }
}


HydroCropData::HydroCropData()
    : HydroObjectData(), substrateType(Hydro_SubstrateType_Undefined), sowTime(0), feedReservoir{0}, feedingWeight(1.0f)
{
    _size = sizeof(*this);
}

void HydroCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroObjectData::toJSONObject(objectOut);

    if (substrateType != Hydro_SubstrateType_Undefined) { objectOut[SFP(HStr_Key_SubstrateType)] = substrateTypeToString(substrateType); }
    if (sowTime) { objectOut[SFP(HStr_Key_SowTime)] = sowTime; }
    if (feedReservoir[0]) { objectOut[SFP(HStr_Key_FeedReservoir)] = charsToString(feedReservoir, HYDRO_NAME_MAXSIZE); }
    if (!isFPEqual(feedingWeight, 1.0f)) { objectOut[SFP(HStr_Key_FeedingWeight)] = feedingWeight; }
}

void HydroCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroObjectData::fromJSONObject(objectIn);
    substrateType = substrateTypeFromString(objectIn[SFP(HStr_Key_SubstrateType)]);
    sowTime = objectIn[SFP(HStr_Key_SowTime)] | sowTime;
    const char *feedReservoirStr = objectIn[SFP(HStr_Key_FeedReservoir)];
    if (feedReservoirStr && feedReservoirStr[0]) { strncpy(feedReservoir, feedReservoirStr, HYDRO_NAME_MAXSIZE); }
    feedingWeight = objectIn[SFP(HStr_Key_FeedingWeight)] | feedingWeight;
}

HydroTimedCropData::HydroTimedCropData()
    : HydroCropData(), lastFeedingTime(0), feedTimingMins{0}
{
    _size = sizeof(*this);
}

void HydroTimedCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroCropData::toJSONObject(objectOut);

    if (lastFeedingTime) { objectOut[SFP(HStr_Key_LastFeedingTime)] = lastFeedingTime; }
    objectOut[SFP(HStr_Key_FeedTimingMins)] = commaStringFromArray(feedTimingMins, 2);
}

void HydroTimedCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroCropData::fromJSONObject(objectIn);
    lastFeedingTime = objectIn[SFP(HStr_Key_LastFeedingTime)] | lastFeedingTime;
    JsonVariantConst feedTimingMinsVar = objectIn[SFP(HStr_Key_FeedTimingMins)];
    commaStringToArray(feedTimingMinsVar, feedTimingMins, 2);
}

HydroAdaptiveCropData::HydroAdaptiveCropData()
    : HydroCropData(), concentrateUnits(Hydro_UnitsType_Undefined), moistureSensor{0}
{
    _size = sizeof(*this);
}

void HydroAdaptiveCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroCropData::toJSONObject(objectOut);

    if (concentrateUnits != Hydro_UnitsType_Undefined) { objectOut[SFP(HStr_Key_ConcentrateUnits)] = unitsTypeToSymbol(concentrateUnits); }
    if (moistureSensor[0]) { objectOut[SFP(HStr_Key_MoistureSensor)] = charsToString(moistureSensor, HYDRO_NAME_MAXSIZE); }
    if (isValidType(feedingTrigger.type)) {
        JsonObject feedingTriggerObj = objectOut.createNestedObject(SFP(HStr_Key_FeedingTrigger));
        feedingTrigger.toJSONObject(feedingTriggerObj);
    }
}

void HydroAdaptiveCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroCropData::fromJSONObject(objectIn);

    concentrateUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_ConcentrateUnits)]);
    const char *moistureSensorStr = objectIn[SFP(HStr_Key_MoistureSensor)];
    if (moistureSensorStr && moistureSensorStr[0]) { strncpy(moistureSensor, moistureSensorStr, HYDRO_NAME_MAXSIZE); }
    JsonObjectConst feedingTriggerObj = objectIn[SFP(HStr_Key_FeedingTrigger)];
    if (!feedingTriggerObj.isNull()) { feedingTrigger.fromJSONObject(feedingTriggerObj); }
}
