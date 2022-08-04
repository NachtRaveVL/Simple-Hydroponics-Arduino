/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops
*/

#include "Hydroponics.h"

HydroponicsCrop *newCropObjectFromData(const HydroponicsCropData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectectData(), SFP(HStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectectData()) {
        switch (dataIn->id.object.classType) {
            case 0: // Timed
                return new HydroponicsTimedCrop((const HydroponicsTimedCropData *)dataIn);
            case 1: // Adaptive
                return new HydroponicsAdaptiveCrop((const HydroponicsAdaptiveCropData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroponicsCrop::HydroponicsCrop(Hydroponics_CropType cropType,
                                 Hydroponics_PositionIndex cropIndex,
                                 Hydroponics_SubstrateType substrateType,
                                 DateTime sowDate,
                                 int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(cropType, cropIndex)), classType((typeof(classType))classTypeIn),
      _substrateType(substrateType), _sowDate(sowDate.unixtime()), _feedReservoir(this), _cropsData(nullptr), _growWeek(0), _feedingWeight(1.0f),
      _cropPhase(Hydroponics_CropPhase_Undefined), _feedingState(Hydroponics_TriggerState_NotTriggered)
{
    _links = new Map<Hydroponics_KeyType, Pair<HydroponicsObject *, int8_t>, HYDRUINO_OBJ_LINKS_MAXSIZE>();

    if (getCropType() >= Hydroponics_CropType_CustomCrop1 && getCropType() < Hydroponics_CropType_CustomCrop1 + Hydroponics_CropType_CustomCropCount) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_CropType>(this, &HydroponicsCrop::handleCustomCropUpdated);
        getCropsLibraryInstance()->getCustomCropSignal().attach(methodSlot);
    }

    recalcGrowWeekAndPhase();
}

HydroponicsCrop::HydroponicsCrop(const HydroponicsCropData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _substrateType(dataIn->substrateType), _sowDate(dataIn->sowDate), _feedReservoir(this),
      _cropsData(nullptr), _growWeek(0), _feedingWeight(dataIn->feedingWeight),
      _cropPhase(Hydroponics_CropPhase_Undefined), _feedingState(Hydroponics_TriggerState_NotTriggered)
{
    _links = new Map<Hydroponics_KeyType, Pair<HydroponicsObject *, int8_t>, HYDRUINO_OBJ_LINKS_MAXSIZE>();

    if (getCropType() >= Hydroponics_CropType_CustomCrop1 && getCropType() < Hydroponics_CropType_CustomCrop1 + Hydroponics_CropType_CustomCropCount) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_CropType>(this, &HydroponicsCrop::handleCustomCropUpdated);
        getCropsLibraryInstance()->getCustomCropSignal().attach(methodSlot);
    }
    _feedReservoir.setObject(dataIn->feedReservoir);

    recalcGrowWeekAndPhase();
}

HydroponicsCrop::~HydroponicsCrop()
{
    if (getCropType() >= Hydroponics_CropType_CustomCrop1 && getCropType() < Hydroponics_CropType_CustomCrop1 + Hydroponics_CropType_CustomCropCount) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_CropType>(this, &HydroponicsCrop::handleCustomCropUpdated);
        getCropsLibraryInstance()->getCustomCropSignal().detach(methodSlot);
    }
    if (_cropsData) { returnCropsLibData(); }
}

void HydroponicsCrop::update()
{
    HydroponicsObject::update();

    _feedReservoir.resolve();

    handleFeeding(triggerStateFromBool(needsFeeding()));
}

void HydroponicsCrop::handleLowMemory()
{
    HydroponicsObject::handleLowMemory();

    returnCropsLibData();
}

void HydroponicsCrop::notifyFeedingBegan()
{ ; }

void HydroponicsCrop::notifyFeedingEnded()
{ ; }

HydroponicsAttachment &HydroponicsCrop::getFeedingReservoir(bool resolve)
{
    if (resolve) { _feedReservoir.resolve(); }
    return _feedReservoir;
}

void HydroponicsCrop::setFeedingWeight(float weight)
{
    if (!isFPEqual(_feedingWeight, weight)) {
        _feedingWeight = weight;

        getSchedulerInstance()->setNeedsScheduling();
    }
}

Signal<HydroponicsCrop *> &HydroponicsCrop::getFeedingSignal()
{
    return _feedingSignal;
}

void HydroponicsCrop::notifyDayChanged()
{
    recalcGrowWeekAndPhase();
}

HydroponicsData *HydroponicsCrop::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void HydroponicsCrop::saveToData(HydroponicsData *dataOut)
{
    HydroponicsObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
    ((HydroponicsCropData *)dataOut)->substrateType = _substrateType;
    ((HydroponicsCropData *)dataOut)->sowDate = _sowDate;
    if (_feedReservoir.getId()) {
        strncpy(((HydroponicsCropData *)dataOut)->feedReservoir, _feedReservoir.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    ((HydroponicsCropData *)dataOut)->feedingWeight = _feedingWeight;
}

void HydroponicsCrop::handleFeeding(Hydroponics_TriggerState feedingState)
{
    if (feedingState == Hydroponics_TriggerState_Disabled || feedingState == Hydroponics_TriggerState_Undefined) { return; }

    if (_feedingState != feedingState) {
        _feedingState = feedingState;

        #ifndef HYDRUINO_DISABLE_MULTITASKING
            scheduleSignalFireOnce<HydroponicsCrop *>(getSharedPtr(), _feedingSignal, this);
        #else
            _feedingSignal.fire(this);
        #endif
    }
}

void HydroponicsCrop::recalcGrowWeekAndPhase()
{
    TimeSpan dateSpan(unixNow() - _sowDate);
    _growWeek = dateSpan.days() / DAYS_PER_WEEK;

    if (!_cropsData) { checkoutCropsLibData(); }
    HYDRUINO_SOFT_ASSERT(_cropsData, F("Invalid crops lib data, unable to update growth cycle"));

    if (_cropsData) {
        _totalGrowWeeks = _cropsData->totalGrowWeeks;
        _cropPhase = Hydroponics_CropPhase_Seedling;
        for (int phaseIndex = 0; phaseIndex < (int)Hydroponics_CropPhase_MainCount; ++phaseIndex) {
            if (_growWeek > _cropsData->phaseDurationWeeks[phaseIndex]) {
                _cropPhase = (Hydroponics_CropPhase)(phaseIndex + 1);
            } else { break; }
        }
    }
}

void HydroponicsCrop::checkoutCropsLibData()
{
    if (!_cropsData) {
        _cropsData = getCropsLibraryInstance()->checkoutCropsData(_id.objTypeAs.cropType);
    }
}

void HydroponicsCrop::returnCropsLibData()
{
    if (_cropsData) {
        getCropsLibraryInstance()->returnCropsData(_cropsData); _cropsData = nullptr;
    }
}

void HydroponicsCrop::handleCustomCropUpdated(Hydroponics_CropType cropType)
{
    if (getCropType() == cropType) {
        returnCropsLibData(); // forces re-checkout
        recalcGrowWeekAndPhase();
        if (getSchedulerInstance()) { getSchedulerInstance()->setNeedsScheduling(); }
    }
}


HydroponicsTimedCrop::HydroponicsTimedCrop(Hydroponics_CropType cropType,
                                           Hydroponics_PositionIndex cropIndex,
                                           Hydroponics_SubstrateType substrateType,
                                           DateTime sowDate,
                                           TimeSpan timeOn, TimeSpan timeOff,
                                           int classType)
    : HydroponicsCrop(cropType, cropIndex, substrateType, sowDate, classType),
      _lastFeedingDate(),
      _feedTimingMins{timeOn.totalseconds() / SECS_PER_MIN, timeOff.totalseconds() / SECS_PER_MIN}
{ ; }

HydroponicsTimedCrop::HydroponicsTimedCrop(const HydroponicsTimedCropData *dataIn)
    : HydroponicsCrop(dataIn),
      _lastFeedingDate(dataIn->lastFeedingDate),
      _feedTimingMins{dataIn->feedTimingMins[0], dataIn->feedTimingMins[1]}
{ ; }

bool HydroponicsTimedCrop::needsFeeding()
{
    return unixNow() >= _lastFeedingDate + ((_feedTimingMins[0] + _feedTimingMins[1]) * SECS_PER_MIN) ||
           unixNow() < _lastFeedingDate + (_feedTimingMins[0] * SECS_PER_MIN);
}

void HydroponicsTimedCrop::notifyFeedingBegan()
{
    HydroponicsCrop::notifyFeedingBegan();

    _lastFeedingDate = unixNow();
}

void HydroponicsTimedCrop::setFeedTimeOn(TimeSpan timeOn)
{
    _feedTimingMins[0] = timeOn.totalseconds() / SECS_PER_MIN;
}

void HydroponicsTimedCrop::setFeedTimeOff(TimeSpan timeOff)
{
    _feedTimingMins[1] = timeOff.totalseconds() / SECS_PER_MIN;
}

void HydroponicsTimedCrop::saveToData(HydroponicsData *dataOut)
{
    HydroponicsCrop::saveToData(dataOut);

    ((HydroponicsTimedCropData *)dataOut)->lastFeedingDate = _lastFeedingDate;
    ((HydroponicsTimedCropData *)dataOut)->feedTimingMins[0] = _feedTimingMins[0];
    ((HydroponicsTimedCropData *)dataOut)->feedTimingMins[1] = _feedTimingMins[1];
}


HydroponicsAdaptiveCrop::HydroponicsAdaptiveCrop(Hydroponics_CropType cropType,
                                                 Hydroponics_PositionIndex cropIndex,
                                                 Hydroponics_SubstrateType substrateType,
                                                 DateTime sowDate,
                                                 int classType)
    : HydroponicsCrop(cropType, cropIndex, substrateType, sowDate, classType),
      _moistureUnits(Hydroponics_UnitsType_Concentration_EC), _soilMoisture(this), _feedingTrigger(this)
{
    _soilMoisture.setMeasurementUnits(getMoistureUnits());

    _feedingTrigger.setHandleMethod(&HydroponicsCrop::handleFeeding);
}

HydroponicsAdaptiveCrop::HydroponicsAdaptiveCrop(const HydroponicsAdaptiveCropData *dataIn)
    : HydroponicsCrop(dataIn), _soilMoisture(this), _feedingTrigger(this),
      _moistureUnits(definedUnitsElse(dataIn->moistureUnits, Hydroponics_UnitsType_Concentration_EC))
{
    _soilMoisture.setMeasurementUnits(definedUnitsElse(dataIn->moistureUnits, getMoistureUnits()));
    _soilMoisture.setObject(dataIn->moistureSensor);

    _feedingTrigger.setHandleMethod(&HydroponicsCrop::handleFeeding);
    _feedingTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->feedingTrigger)));
    HYDRUINO_SOFT_ASSERT(_feedingTrigger, SFP(HStr_Err_AllocationFailure));
}

void HydroponicsAdaptiveCrop::update()
{
    HydroponicsCrop::update();

    _soilMoisture.updateIfNeeded(true);

    _feedingTrigger.updateIfNeeded();
}

void HydroponicsAdaptiveCrop::handleLowMemory()
{
    HydroponicsCrop::handleLowMemory();

    if (_feedingTrigger) { _feedingTrigger->handleLowMemory(); }
}

bool HydroponicsAdaptiveCrop::needsFeeding()
{
    return _feedingTrigger.resolve() && triggerStateToBool(_feedingTrigger.getTriggerState());
}

void HydroponicsAdaptiveCrop::setMoistureUnits(Hydroponics_UnitsType moistureUnits)
{
    if (_moistureUnits != moistureUnits) {
        _moistureUnits = moistureUnits;

        _soilMoisture.setMeasurementUnits(getMoistureUnits());
    }
}

Hydroponics_UnitsType HydroponicsAdaptiveCrop::getMoistureUnits() const
{
    return definedUnitsElse(_moistureUnits, Hydroponics_UnitsType_Concentration_EC);
}

HydroponicsSensorAttachment &HydroponicsAdaptiveCrop::getSoilMoisture(bool poll)
{
    _soilMoisture.updateIfNeeded(poll);
    return _soilMoisture;
}

void HydroponicsAdaptiveCrop::saveToData(HydroponicsData *dataOut)
{
    HydroponicsCrop::saveToData(dataOut);

    ((HydroponicsAdaptiveCropData *)dataOut)->moistureUnits = _moistureUnits;
    if (_soilMoisture.getId()) {
        strncpy(((HydroponicsAdaptiveCropData *)dataOut)->moistureSensor, _soilMoisture.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_feedingTrigger) {
        _feedingTrigger->saveToData(&(((HydroponicsAdaptiveCropData *)dataOut)->feedingTrigger));
    }
}


HydroponicsCropData::HydroponicsCropData()
    : HydroponicsObjectData(), substrateType(Hydroponics_SubstrateType_Undefined), sowDate(0), feedReservoir{0}, feedingWeight(1.0f)
{
    _size = sizeof(*this);
}

void HydroponicsCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsObjectData::toJSONObject(objectOut);

    if (substrateType != Hydroponics_SubstrateType_Undefined) { objectOut[SFP(HStr_Key_SubstrateType)] = substrateTypeToString(substrateType); }
    if (sowDate) { objectOut[SFP(HStr_Key_SowDate)] = sowDate; }
    if (feedReservoir[0]) { objectOut[SFP(HStr_Key_FeedReservoir)] = charsToString(feedReservoir, HYDRUINO_NAME_MAXSIZE); }
    if (!isFPEqual(feedingWeight, 1.0f)) { objectOut[SFP(HStr_Key_FeedingWeight)] = feedingWeight; }
}

void HydroponicsCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);
    substrateType = substrateTypeFromString(objectIn[SFP(HStr_Key_SubstrateType)]);
    sowDate = objectIn[SFP(HStr_Key_SowDate)] | sowDate;
    const char *feedReservoirStr = objectIn[SFP(HStr_Key_FeedReservoir)];
    if (feedReservoirStr && feedReservoirStr[0]) { strncpy(feedReservoir, feedReservoirStr, HYDRUINO_NAME_MAXSIZE); }
    feedingWeight = objectIn[SFP(HStr_Key_FeedingWeight)] | feedingWeight;
}

HydroponicsTimedCropData::HydroponicsTimedCropData()
    : HydroponicsCropData(), lastFeedingDate(0), feedTimingMins{0}
{
    _size = sizeof(*this);
}

void HydroponicsTimedCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsCropData::toJSONObject(objectOut);

    if (lastFeedingDate) { objectOut[SFP(HStr_Key_LastFeedingDate)] = lastFeedingDate; }
    objectOut[SFP(HStr_Key_FeedTimingMins)] = commaStringFromArray(feedTimingMins, 2);
}

void HydroponicsTimedCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsCropData::fromJSONObject(objectIn);
    lastFeedingDate = objectIn[SFP(HStr_Key_LastFeedingDate)] | lastFeedingDate;
    JsonVariantConst feedTimingMinsVar = objectIn[SFP(HStr_Key_FeedTimingMins)];
    commaStringToArray(feedTimingMinsVar, feedTimingMins, 2);
}

HydroponicsAdaptiveCropData::HydroponicsAdaptiveCropData()
    : HydroponicsCropData(), moistureUnits(Hydroponics_UnitsType_Undefined), moistureSensor{0}
{
    _size = sizeof(*this);
}

void HydroponicsAdaptiveCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsCropData::toJSONObject(objectOut);

    if (moistureUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HStr_Key_MoistureUnits)] = unitsTypeToSymbol(moistureUnits); }
    if (moistureSensor[0]) { objectOut[SFP(HStr_Key_MoistureSensor)] = charsToString(moistureSensor, HYDRUINO_NAME_MAXSIZE); }
    if (feedingTrigger.type != -1) {
        JsonObject feedingTriggerObj = objectOut.createNestedObject(SFP(HStr_Key_FeedingTrigger));
        feedingTrigger.toJSONObject(feedingTriggerObj);
    }
}

void HydroponicsAdaptiveCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsCropData::fromJSONObject(objectIn);

    moistureUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_MoistureUnits)]);
    const char *moistureSensorStr = objectIn[SFP(HStr_Key_MoistureSensor)];
    if (moistureSensorStr && moistureSensorStr[0]) { strncpy(moistureSensor, moistureSensorStr, HYDRUINO_NAME_MAXSIZE); }
    JsonObjectConst feedingTriggerObj = objectIn[SFP(HStr_Key_FeedingTrigger)];
    if (!feedingTriggerObj.isNull()) { feedingTrigger.fromJSONObject(feedingTriggerObj); }
}
