/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops
*/

#include "Hydroponics.h"

HydroponicsCrop *newCropObjectFromData(const HydroponicsCropData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectectData(), SFP(HS_Err_InvalidParameter));

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
    if (getCropType() >= Hydroponics_CropType_CustomCrop1 && getCropType() < Hydroponics_CropType_CustomCrop1 + Hydroponics_CropType_CustomCropCount) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_CropType>(this, &HydroponicsCrop::handleCustomCropUpdated);
        getCropsLibraryInstance()->getCustomCropSignal().attach(methodSlot);
    }
    recalcGrowWeekAndPhase();
    _feedReservoir = dataIn->feedReservoir;
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

    _feedReservoir.resolveIfNeeded();

    auto feedingState = triggerStateFromBool(needsFeeding());
    if (_feedingState != feedingState) {
        _feedingState = feedingState;
        handleFeedingState();
    }
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

HydroponicsAttachment<HydroponicsFeedReservoir> &HydroponicsCrop::getFeedingReservoir()
{
    _feedReservoir.resolveIfNeeded();
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

void HydroponicsCrop::handleFeedingState()
{
    #ifndef HYDRUINO_DISABLE_MULTITASKING
        scheduleSignalFireOnce<HydroponicsCrop *>(getSharedPtr(), _feedingSignal, this);
    #else
        _feedingSignal.fire(this);
    #endif
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
        returnCropsLibData(); // force re-checkout
        recalcGrowWeekAndPhase();
        auto scheduler = getSchedulerInstance();
        if (scheduler) { scheduler->setNeedsScheduling(); }
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

HydroponicsTimedCrop::~HydroponicsTimedCrop()
{ ; }

bool HydroponicsTimedCrop::needsFeeding() const
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
      _moistureUnits(Hydroponics_UnitsType_Concentration_EC), _soilMoisture(this), _feedingTrigger(nullptr)
{
    _soilMoisture.setMeasurementUnits(getMoistureUnits());
}

HydroponicsAdaptiveCrop::HydroponicsAdaptiveCrop(const HydroponicsAdaptiveCropData *dataIn)
    : HydroponicsCrop(dataIn), _soilMoisture(this),
      _moistureUnits(definedUnitsElse(dataIn->moistureUnits, Hydroponics_UnitsType_Concentration_EC)),
      _feedingTrigger(newTriggerObjectFromSubData(&(dataIn->feedingTrigger)))
{
    _soilMoisture.setMeasurementUnits(getMoistureUnits());
    _soilMoisture = dataIn->moistureSensor;
}

HydroponicsAdaptiveCrop::~HydroponicsAdaptiveCrop()
{
    if (_feedingTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &HydroponicsAdaptiveCrop::handleFeedingTrigger);
        _feedingTrigger->getTriggerSignal().detach(methodSlot);
        delete _feedingTrigger; _feedingTrigger = nullptr;
    }
}

void HydroponicsAdaptiveCrop::update()
{
    HydroponicsCrop::update();

    if (_feedingTrigger) { _feedingTrigger->update(); }

    _soilMoisture.updateMeasurementIfNeeded();
}

void HydroponicsAdaptiveCrop::handleLowMemory()
{
    HydroponicsCrop::handleLowMemory();

    if (_feedingTrigger) { _feedingTrigger->handleLowMemory(); }
}

bool HydroponicsAdaptiveCrop::needsFeeding() const
{
    return ((_feedingTrigger ? _feedingTrigger->getTriggerState() : _feedingState) == Hydroponics_TriggerState_Triggered);
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

HydroponicsSensorAttachment &HydroponicsAdaptiveCrop::getSoilMoisture()
{
    _soilMoisture.updateMeasurementIfNeeded();
    return _soilMoisture;
}

void HydroponicsAdaptiveCrop::setFeedingTrigger(HydroponicsTrigger *feedingTrigger)
{
    if (_feedingTrigger != feedingTrigger) {
        if (_feedingTrigger) {
            auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &HydroponicsAdaptiveCrop::handleFeedingTrigger);
            _feedingTrigger->getTriggerSignal().detach(methodSlot);
            delete _feedingTrigger; _feedingTrigger = nullptr;
        }
        _feedingTrigger = feedingTrigger;
        if (_feedingTrigger) {
            auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &HydroponicsAdaptiveCrop::handleFeedingTrigger);
            _feedingTrigger->getTriggerSignal().attach(methodSlot);
        }
    }
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

void HydroponicsAdaptiveCrop::handleFeedingTrigger(Hydroponics_TriggerState triggerState)
{
    if (triggerState != Hydroponics_TriggerState_Undefined && triggerState != Hydroponics_TriggerState_Disabled) {
        _feedingState = triggerState;
        handleFeedingState();
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

    if (substrateType != Hydroponics_SubstrateType_Undefined) { objectOut[SFP(HS_Key_SubstrateType)] = substrateTypeToString(substrateType); }
    if (sowDate) { objectOut[SFP(HS_Key_SowDate)] = sowDate; }
    if (feedReservoir[0]) { objectOut[SFP(HS_Key_FeedReservoir)] = charsToString(feedReservoir, HYDRUINO_NAME_MAXSIZE); }
    if (!isFPEqual(feedingWeight, 1.0f)) { objectOut[SFP(HS_Key_FeedingWeight)] = feedingWeight; }
}

void HydroponicsCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);
    substrateType = substrateTypeFromString(objectIn[SFP(HS_Key_SubstrateType)]);
    sowDate = objectIn[SFP(HS_Key_SowDate)] | sowDate;
    const char *feedReservoirStr = objectIn[SFP(HS_Key_FeedReservoir)];
    if (feedReservoirStr && feedReservoirStr[0]) { strncpy(feedReservoir, feedReservoirStr, HYDRUINO_NAME_MAXSIZE); }
    feedingWeight = objectIn[SFP(HS_Key_FeedingWeight)] | feedingWeight;
}

HydroponicsTimedCropData::HydroponicsTimedCropData()
    : HydroponicsCropData(), lastFeedingDate(0), feedTimingMins{0}
{
    _size = sizeof(*this);
}

void HydroponicsTimedCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsCropData::toJSONObject(objectOut);

    if (lastFeedingDate) { objectOut[SFP(HS_Key_LastFeedingDate)] = lastFeedingDate; }
    objectOut[SFP(HS_Key_FeedTimingMins)] = commaStringFromArray(feedTimingMins, 2);
}

void HydroponicsTimedCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsCropData::fromJSONObject(objectIn);
    lastFeedingDate = objectIn[SFP(HS_Key_LastFeedingDate)] | lastFeedingDate;
    JsonVariantConst feedTimingMinsVar = objectIn[SFP(HS_Key_FeedTimingMins)];
    commaStringToArray(feedTimingMinsVar, feedTimingMins, 2);
    feedTimingMins[0] = feedTimingMinsVar[F("on")] | feedTimingMinsVar[0] | feedTimingMins[0];
    feedTimingMins[1] = feedTimingMinsVar[F("off")] | feedTimingMinsVar[1] | feedTimingMins[1];
}

HydroponicsAdaptiveCropData::HydroponicsAdaptiveCropData()
    : HydroponicsCropData(), moistureUnits(Hydroponics_UnitsType_Undefined), moistureSensor{0}
{
    _size = sizeof(*this);
}

void HydroponicsAdaptiveCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsCropData::toJSONObject(objectOut);

    if (moistureUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HS_Key_MoistureUnits)] = unitsTypeToSymbol(moistureUnits); }
    if (moistureSensor[0]) { objectOut[SFP(HS_Key_MoistureSensor)] = charsToString(moistureSensor, HYDRUINO_NAME_MAXSIZE); }
    if (feedingTrigger.type != -1) {
        JsonObject feedingTriggerObj = objectOut.createNestedObject(SFP(HS_Key_FeedingTrigger));
        feedingTrigger.toJSONObject(feedingTriggerObj);
    }
}

void HydroponicsAdaptiveCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsCropData::fromJSONObject(objectIn);

    moistureUnits = unitsTypeFromSymbol(objectIn[SFP(HS_Key_MoistureUnits)]);
    const char *moistureSensorStr = objectIn[SFP(HS_Key_MoistureSensor)];
    if (moistureSensorStr && moistureSensorStr[0]) { strncpy(moistureSensor, moistureSensorStr, HYDRUINO_NAME_MAXSIZE); }
    JsonObjectConst feedingTriggerObj = objectIn[SFP(HS_Key_FeedingTrigger)];
    if (!feedingTriggerObj.isNull()) { feedingTrigger.fromJSONObject(feedingTriggerObj); }
}
