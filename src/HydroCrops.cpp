/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Crops
*/

#include "Hydruino.h"

HydroCrop *newCropObjectFromData(const HydroCropData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
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


HydroCrop::HydroCrop(Hydro_CropType cropType,
                                 Hydro_PositionIndex cropIndex,
                                 Hydro_SubstrateType substrateType,
                                 DateTime sowDate,
                                 int classTypeIn)
    : HydroObject(HydroIdentity(cropType, cropIndex)), classType((typeof(classType))classTypeIn),
      _substrateType(substrateType), _sowDate(sowDate.unixtime()), _feedReservoir(this), _cropsData(nullptr), _growWeek(0), _feedingWeight(1.0f),
      _cropPhase(Hydro_CropPhase_Undefined), _feedingState(Hydro_TriggerState_NotTriggered)
{
    allocateLinkages(HYDRO_CROPS_LINKS_BASESIZE);

    recalcCropGrowthParams();
}

HydroCrop::HydroCrop(const HydroCropData *dataIn)
    : HydroObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _substrateType(dataIn->substrateType), _sowDate(dataIn->sowDate), _feedReservoir(this),
      _cropsData(nullptr), _growWeek(0), _feedingWeight(dataIn->feedingWeight),
      _cropPhase(Hydro_CropPhase_Undefined), _feedingState(Hydro_TriggerState_NotTriggered)
{
    allocateLinkages(HYDRO_CROPS_LINKS_BASESIZE);

    recalcCropGrowthParams();
}

HydroCrop::~HydroCrop()
{
    if (_cropsData) { returnCropsLibData(); }
}

void HydroCrop::update()
{
    HydroObject::update();

    _feedReservoir.resolve();

    handleFeeding(triggerStateFromBool(getNeedsFeeding()));
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

HydroAttachment &HydroCrop::getFeedingReservoir(bool resolve)
{
    if (resolve) { _feedReservoir.resolve(); }
    return _feedReservoir;
}

void HydroCrop::setFeedingWeight(float weight)
{
    if (!isFPEqual(_feedingWeight, weight)) {
        _feedingWeight = weight;

        getSchedulerInstance()->setNeedsScheduling();
    }
}

Signal<HydroCrop *, HYDRO_FEEDING_SIGNAL_SLOTS> &HydroCrop::getFeedingSignal()
{
    return _feedingSignal;
}

void HydroCrop::notifyDayChanged()
{
    recalcCropGrowthParams();
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
    ((HydroCropData *)dataOut)->sowDate = _sowDate;
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

void HydroCrop::recalcCropGrowthParams()
{
    TimeSpan dateSpan(unixNow() - _sowDate);
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
        recalcCropGrowthParams();

        if (getSchedulerInstance()) {
            getSchedulerInstance()->setNeedsScheduling();
        }
    }
}


HydroTimedCrop::HydroTimedCrop(Hydro_CropType cropType,
                                           Hydro_PositionIndex cropIndex,
                                           Hydro_SubstrateType substrateType,
                                           DateTime sowDate,
                                           TimeSpan timeOn, TimeSpan timeOff,
                                           int classType)
    : HydroCrop(cropType, cropIndex, substrateType, sowDate, classType),
      _lastFeedingDate(),
      _feedTimingMins{timeOn.totalseconds() / SECS_PER_MIN, timeOff.totalseconds() / SECS_PER_MIN}
{ ; }

HydroTimedCrop::HydroTimedCrop(const HydroTimedCropData *dataIn)
    : HydroCrop(dataIn),
      _lastFeedingDate(dataIn->lastFeedingDate),
      _feedTimingMins{dataIn->feedTimingMins[0], dataIn->feedTimingMins[1]}
{ ; }

bool HydroTimedCrop::getNeedsFeeding()
{
    return unixNow() >= _lastFeedingDate + ((_feedTimingMins[0] + _feedTimingMins[1]) * SECS_PER_MIN) ||
           unixNow() < _lastFeedingDate + (_feedTimingMins[0] * SECS_PER_MIN);
}

void HydroTimedCrop::notifyFeedingBegan()
{
    HydroCrop::notifyFeedingBegan();

    _lastFeedingDate = unixNow();
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

    ((HydroTimedCropData *)dataOut)->lastFeedingDate = _lastFeedingDate;
    ((HydroTimedCropData *)dataOut)->feedTimingMins[0] = _feedTimingMins[0];
    ((HydroTimedCropData *)dataOut)->feedTimingMins[1] = _feedTimingMins[1];
}


HydroAdaptiveCrop::HydroAdaptiveCrop(Hydro_CropType cropType,
                                                 Hydro_PositionIndex cropIndex,
                                                 Hydro_SubstrateType substrateType,
                                                 DateTime sowDate,
                                                 int classType)
    : HydroCrop(cropType, cropIndex, substrateType, sowDate, classType),
      _moistureUnits(Hydro_UnitsType_Concentration_EC), _soilMoisture(this), _feedingTrigger(this)
{
    _soilMoisture.setMeasurementUnits(getMoistureUnits());

    _feedingTrigger.setHandleMethod(&HydroCrop::handleFeeding);
}

HydroAdaptiveCrop::HydroAdaptiveCrop(const HydroAdaptiveCropData *dataIn)
    : HydroCrop(dataIn), _soilMoisture(this), _feedingTrigger(this),
      _moistureUnits(definedUnitsElse(dataIn->moistureUnits, Hydro_UnitsType_Concentration_EC))
{
    _soilMoisture.setMeasurementUnits(definedUnitsElse(dataIn->moistureUnits, getMoistureUnits()));
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

bool HydroAdaptiveCrop::getNeedsFeeding()
{
    return _feedingTrigger.resolve() && triggerStateToBool(_feedingTrigger.getTriggerState());
}

void HydroAdaptiveCrop::setMoistureUnits(Hydro_UnitsType moistureUnits)
{
    if (_moistureUnits != moistureUnits) {
        _moistureUnits = moistureUnits;

        _soilMoisture.setMeasurementUnits(getMoistureUnits());
    }
}

Hydro_UnitsType HydroAdaptiveCrop::getMoistureUnits() const
{
    return definedUnitsElse(_moistureUnits, Hydro_UnitsType_Concentration_EC);
}

HydroSensorAttachment &HydroAdaptiveCrop::getSoilMoisture(bool poll)
{
    _soilMoisture.updateIfNeeded(poll);
    return _soilMoisture;
}

void HydroAdaptiveCrop::saveToData(HydroData *dataOut)
{
    HydroCrop::saveToData(dataOut);

    ((HydroAdaptiveCropData *)dataOut)->moistureUnits = _moistureUnits;
    if (_soilMoisture.getId()) {
        strncpy(((HydroAdaptiveCropData *)dataOut)->moistureSensor, _soilMoisture.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_feedingTrigger) {
        _feedingTrigger->saveToData(&(((HydroAdaptiveCropData *)dataOut)->feedingTrigger));
    }
}


HydroCropData::HydroCropData()
    : HydroObjectData(), substrateType(Hydro_SubstrateType_Undefined), sowDate(0), feedReservoir{0}, feedingWeight(1.0f)
{
    _size = sizeof(*this);
}

void HydroCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroObjectData::toJSONObject(objectOut);

    if (substrateType != Hydro_SubstrateType_Undefined) { objectOut[SFP(HStr_Key_SubstrateType)] = substrateTypeToString(substrateType); }
    if (sowDate) { objectOut[SFP(HStr_Key_SowDate)] = sowDate; }
    if (feedReservoir[0]) { objectOut[SFP(HStr_Key_FeedReservoir)] = charsToString(feedReservoir, HYDRO_NAME_MAXSIZE); }
    if (!isFPEqual(feedingWeight, 1.0f)) { objectOut[SFP(HStr_Key_FeedingWeight)] = feedingWeight; }
}

void HydroCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroObjectData::fromJSONObject(objectIn);
    substrateType = substrateTypeFromString(objectIn[SFP(HStr_Key_SubstrateType)]);
    sowDate = objectIn[SFP(HStr_Key_SowDate)] | sowDate;
    const char *feedReservoirStr = objectIn[SFP(HStr_Key_FeedReservoir)];
    if (feedReservoirStr && feedReservoirStr[0]) { strncpy(feedReservoir, feedReservoirStr, HYDRO_NAME_MAXSIZE); }
    feedingWeight = objectIn[SFP(HStr_Key_FeedingWeight)] | feedingWeight;
}

HydroTimedCropData::HydroTimedCropData()
    : HydroCropData(), lastFeedingDate(0), feedTimingMins{0}
{
    _size = sizeof(*this);
}

void HydroTimedCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroCropData::toJSONObject(objectOut);

    if (lastFeedingDate) { objectOut[SFP(HStr_Key_LastFeedingDate)] = lastFeedingDate; }
    objectOut[SFP(HStr_Key_FeedTimingMins)] = commaStringFromArray(feedTimingMins, 2);
}

void HydroTimedCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroCropData::fromJSONObject(objectIn);
    lastFeedingDate = objectIn[SFP(HStr_Key_LastFeedingDate)] | lastFeedingDate;
    JsonVariantConst feedTimingMinsVar = objectIn[SFP(HStr_Key_FeedTimingMins)];
    commaStringToArray(feedTimingMinsVar, feedTimingMins, 2);
}

HydroAdaptiveCropData::HydroAdaptiveCropData()
    : HydroCropData(), moistureUnits(Hydro_UnitsType_Undefined), moistureSensor{0}
{
    _size = sizeof(*this);
}

void HydroAdaptiveCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroCropData::toJSONObject(objectOut);

    if (moistureUnits != Hydro_UnitsType_Undefined) { objectOut[SFP(HStr_Key_MoistureUnits)] = unitsTypeToSymbol(moistureUnits); }
    if (moistureSensor[0]) { objectOut[SFP(HStr_Key_MoistureSensor)] = charsToString(moistureSensor, HYDRO_NAME_MAXSIZE); }
    if (feedingTrigger.type != -1) {
        JsonObject feedingTriggerObj = objectOut.createNestedObject(SFP(HStr_Key_FeedingTrigger));
        feedingTrigger.toJSONObject(feedingTriggerObj);
    }
}

void HydroAdaptiveCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroCropData::fromJSONObject(objectIn);

    moistureUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_MoistureUnits)]);
    const char *moistureSensorStr = objectIn[SFP(HStr_Key_MoistureSensor)];
    if (moistureSensorStr && moistureSensorStr[0]) { strncpy(moistureSensor, moistureSensorStr, HYDRO_NAME_MAXSIZE); }
    JsonObjectConst feedingTriggerObj = objectIn[SFP(HStr_Key_FeedingTrigger)];
    if (!feedingTriggerObj.isNull()) { feedingTrigger.fromJSONObject(feedingTriggerObj); }
}
