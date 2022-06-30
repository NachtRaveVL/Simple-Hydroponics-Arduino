/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops
*/

#include "Hydroponics.h"

HydroponicsCrop *newCropObjectFromData(const HydroponicsCropData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjData(), F("Invalid data"));

    if (dataIn && dataIn->isObjData()) {
        switch(dataIn->id.object.classType) {
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
                                 time_t sowDate,
                                 int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(cropType, cropIndex)), classType((typeof(classType))classTypeIn),
      _substrateType(substrateType), _sowDate(sowDate),
      _cropsData(nullptr), _growWeek(0), _cropPhase(Hydroponics_CropPhase_Undefined), _feedingState(Hydroponics_TriggerState_NotTriggered)
{
    recalcGrowWeekAndPhase();
}

HydroponicsCrop::HydroponicsCrop(const HydroponicsCropData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _substrateType(dataIn->substrateType), _sowDate(dataIn->sowDate), _feedReservoir(dataIn->feedReservoirName),
      _cropsData(nullptr), _growWeek(0), _cropPhase(Hydroponics_CropPhase_Undefined), _feedingState(Hydroponics_TriggerState_NotTriggered)
{
    recalcGrowWeekAndPhase();
}

HydroponicsCrop::~HydroponicsCrop()
{
    //discardFromTaskManager(&_feedingSignal);
    if (_cropsData) { returnCropsLibData(); }
    if (_feedReservoir) { _feedReservoir->removeCrop(this); }
    {   auto sensors = getSensors();
        for (auto iter = sensors.begin(); iter != sensors.end(); ++iter) { removeSensor(iter->second); }
    }
}

void HydroponicsCrop::update()
{
    HydroponicsObject::update();

    recalcGrowWeekAndPhase();

    if (_feedingState == Hydroponics_TriggerState_NotTriggered && getNeedsFeeding()) {
        _feedingState = Hydroponics_TriggerState_Triggered;
        handleFeedingState();
    } else if (_feedingState == Hydroponics_TriggerState_Triggered && !getNeedsFeeding()) {
        _feedingState = Hydroponics_TriggerState_NotTriggered;
        handleFeedingState();
    }
}

void HydroponicsCrop::resolveLinks()
{
    HydroponicsObject::resolveLinks();

    if (_feedReservoir.needsResolved()) { getFeedReservoir(); }
}

void HydroponicsCrop::handleLowMemory()
{
    HydroponicsObject::handleLowMemory();

    returnCropsLibData();
}

bool HydroponicsCrop::addSensor(HydroponicsSensor *sensor)
{
    return addLinkage(sensor);
}

bool HydroponicsCrop::removeSensor(HydroponicsSensor *sensor)
{
    return removeLinkage(sensor);
}

bool HydroponicsCrop::hasSensor(HydroponicsSensor *sensor) const
{
    return hasLinkage(sensor);
}

arx::map<Hydroponics_KeyType, HydroponicsSensor *> HydroponicsCrop::getSensors() const
{
    arx::map<Hydroponics_KeyType, HydroponicsSensor *> retVal;
    for (auto iter = _links.begin(); iter != _links.end(); ++iter) {
        auto obj = iter->second;
        if (obj && obj->isSensorType()) {
            retVal.insert(iter->first, (HydroponicsSensor *)obj);
        }
    }
    return retVal;
}

void HydroponicsCrop::setFeedReservoir(HydroponicsIdentity reservoirId)
{
    if (_feedReservoir != reservoirId) {
        if (_feedReservoir) { _feedReservoir->removeCrop(this); }
        _feedReservoir = reservoirId;
    }
}

void HydroponicsCrop::setFeedReservoir(shared_ptr<HydroponicsReservoir> reservoir)
{
    if (_feedReservoir != reservoir) {
        if (_feedReservoir) { _feedReservoir->removeCrop(this); }
        _feedReservoir = reservoir;
        if (_feedReservoir) { _feedReservoir->addCrop(this); }
    }
}

shared_ptr<HydroponicsReservoir> HydroponicsCrop::getFeedReservoir()
{
    if (_feedReservoir.resolveIfNeeded()) { _feedReservoir->addCrop(this); }
    return _feedReservoir.getObj();
}

Hydroponics_CropType HydroponicsCrop::getCropType() const
{
    return _id.objTypeAs.cropType;
}

Hydroponics_PositionIndex HydroponicsCrop::getCropIndex() const
{
    return _id.posIndex;
}

Hydroponics_SubstrateType HydroponicsCrop::getSubstrateType() const
{
    return _substrateType;
}

time_t HydroponicsCrop::getSowDate() const
{
    return _sowDate;
}

const HydroponicsCropsLibData *HydroponicsCrop::getCropsLibData() const
{
    return _cropsData;
}

int HydroponicsCrop::getGrowWeek() const
{
    return _growWeek;
}

Hydroponics_CropPhase HydroponicsCrop::getCropPhase() const
{
    return _cropPhase;
}

Signal<HydroponicsCrop *> &HydroponicsCrop::getFeedingSignal()
{
    return _feedingSignal;
}

HydroponicsData *HydroponicsCrop::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void HydroponicsCrop::saveToData(HydroponicsData *dataOut) const
{
    HydroponicsObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
    ((HydroponicsCropData *)dataOut)->substrateType = _substrateType;
    ((HydroponicsCropData *)dataOut)->sowDate = _sowDate;
    if (_feedReservoir.getId()) {
        strncpy(((HydroponicsCropData *)dataOut)->feedReservoirName, _feedReservoir.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}

void HydroponicsCrop::recalcGrowWeekAndPhase()
{
    TimeSpan dateSpan = DateTime(now()) - DateTime(_sowDate);
    _growWeek = dateSpan.days() / DAYS_PER_WEEK;

    if (!_cropsData) { checkoutCropsLibData(); }
    HYDRUINO_SOFT_ASSERT(_cropsData, F("Invalid crops lib data, unable to update growth cycle"));

    if (_cropsData) {
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
        auto cropLib = getCropsLibraryInstance();
        if (cropLib) {
            _cropsData = cropLib->checkoutCropData(_id.objTypeAs.cropType);
        }
    }
}

void HydroponicsCrop::returnCropsLibData()
{
    if (_cropsData) {
        auto cropLib = getCropsLibraryInstance();
        if (cropLib) {
            cropLib->returnCropData(_cropsData); _cropsData = nullptr;
        }
    }
}

void HydroponicsCrop::handleFeedingState()
{
    scheduleSignalFireOnce<HydroponicsCrop *>(_feedingSignal, this);
}


HydroponicsTimedCrop::HydroponicsTimedCrop(Hydroponics_CropType cropType,
                                           Hydroponics_PositionIndex cropIndex,
                                           Hydroponics_SubstrateType substrateType,
                                           time_t sowDate,
                                           TimeSpan timeOn, TimeSpan timeOff,
                                           int classType)
    : HydroponicsCrop(cropType, cropIndex, substrateType, sowDate, classType),
      _lastFeedingTime(),
      _feedTimingMins{timeOn.totalseconds()/SECS_PER_MIN, timeOff.totalseconds()/SECS_PER_MIN}
{ ; }

HydroponicsTimedCrop::HydroponicsTimedCrop(const HydroponicsTimedCropData *dataIn)
    : HydroponicsCrop(dataIn),
      _lastFeedingTime(dataIn->lastFeedingTime),
      _feedTimingMins{dataIn->feedTimingMins[0], dataIn->feedTimingMins[1]}
{ ; }

HydroponicsTimedCrop::~HydroponicsTimedCrop()
{ ; }

bool HydroponicsTimedCrop::getNeedsFeeding() const
{
    return now() >= _lastFeedingTime + ((_feedTimingMins[0] + _feedTimingMins[1]) * SECS_PER_MIN) ||
           now() < _lastFeedingTime + (_feedTimingMins[0] * SECS_PER_MIN);
}

void HydroponicsTimedCrop::setFeedTimeOn(TimeSpan timeOn)
{
    _feedTimingMins[0] = timeOn.totalseconds()/SECS_PER_MIN;
}

TimeSpan HydroponicsTimedCrop::getFeedTimeOn() const
{
    return TimeSpan(_feedTimingMins[0]*SECS_PER_MIN);
}

void HydroponicsTimedCrop::setFeedTimeOff(TimeSpan timeOff)
{
    _feedTimingMins[1] = timeOff.totalseconds()/SECS_PER_MIN;
}

TimeSpan HydroponicsTimedCrop::getFeedTimeOff() const
{
    return TimeSpan(_feedTimingMins[1]*SECS_PER_MIN);
}

void HydroponicsTimedCrop::saveToData(HydroponicsData *dataOut) const
{
    HydroponicsCrop::saveToData(dataOut);

    ((HydroponicsTimedCropData *)dataOut)->lastFeedingTime = _lastFeedingTime;
    ((HydroponicsTimedCropData *)dataOut)->feedTimingMins[0] = _feedTimingMins[0];
    ((HydroponicsTimedCropData *)dataOut)->feedTimingMins[1] = _feedTimingMins[1];
}

void HydroponicsTimedCrop::handleFeedingState()
{
    HydroponicsCrop::handleFeedingState();
    if (_feedingState == Hydroponics_TriggerState_Triggered) {
        _lastFeedingTime = now();
    }
}


HydroponicsAdaptiveCrop::HydroponicsAdaptiveCrop(Hydroponics_CropType cropType,
                                               Hydroponics_PositionIndex cropIndex,
                                               Hydroponics_SubstrateType substrateType,
                                               time_t sowDate,
                                               int classType)
    : HydroponicsCrop(cropType, cropIndex, substrateType, sowDate, classType),
      _feedingTrigger(nullptr)
{ ; }

HydroponicsAdaptiveCrop::HydroponicsAdaptiveCrop(const HydroponicsAdaptiveCropData *dataIn)
    : HydroponicsCrop(dataIn),
      _feedingTrigger(newTriggerObjectFromSubData(&(dataIn->feedingTrigger)))
{ ; }

HydroponicsAdaptiveCrop::~HydroponicsAdaptiveCrop()
{
    if (_feedingTrigger) { delete _feedingTrigger; _feedingTrigger = nullptr; }
}

void HydroponicsAdaptiveCrop::update()
{
    HydroponicsCrop::update();

    if (_feedingTrigger) { _feedingTrigger->update(); }
}

void HydroponicsAdaptiveCrop::resolveLinks()
{
    HydroponicsCrop::resolveLinks();

    if (_feedingTrigger) { _feedingTrigger->resolveLinks(); }
}

void HydroponicsAdaptiveCrop::handleLowMemory()
{
    HydroponicsCrop::handleLowMemory();

    if (_feedingTrigger) { _feedingTrigger->handleLowMemory(); }
}

bool HydroponicsAdaptiveCrop::getNeedsFeeding() const
{
    return ((_feedingTrigger ? _feedingTrigger->getTriggerState() : _feedingState) == Hydroponics_TriggerState_Triggered);
}

void HydroponicsAdaptiveCrop::setFeedingTrigger(HydroponicsTrigger *feedingTrigger)
{
    if (_feedingTrigger != feedingTrigger) {
        if (_feedingTrigger) { delete _feedingTrigger; }
        _feedingTrigger = feedingTrigger;
    }
}

const HydroponicsTrigger *HydroponicsAdaptiveCrop::getFeedingTrigger() const
{
    return _feedingTrigger;
}

void HydroponicsAdaptiveCrop::saveToData(HydroponicsData *dataOut) const
{
    HydroponicsCrop::saveToData(dataOut);

    if (_feedingTrigger) {
        _feedingTrigger->saveToData(&(((HydroponicsAdaptiveCropData *)dataOut)->feedingTrigger));
    }
}


HydroponicsCropData::HydroponicsCropData()
    : HydroponicsObjectData(), substrateType(Hydroponics_SubstrateType_Undefined), sowDate(0), feedReservoirName{0}
{
    _size = sizeof(*this);
}

void HydroponicsCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsObjectData::toJSONObject(objectOut);

    if (substrateType != Hydroponics_SubstrateType_Undefined) { objectOut[F("substrateType")] = substrateType; }
    if (sowDate > DateTime((uint32_t)0).unixtime()) { objectOut[F("sowDate")] = sowDate; }
    if (feedReservoirName[0]) { objectOut[F("feedReservoirName")] = stringFromChars(feedReservoirName, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);
    substrateType = objectIn[F("substrateType")] | substrateType;
    sowDate = objectIn[F("sowDate")] | sowDate;
    const char *feedReservoirNameStr = objectIn[F("feedReservoirName")];
    if (feedReservoirNameStr && feedReservoirNameStr[0]) { strncpy(feedReservoirName, feedReservoirNameStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsTimedCropData::HydroponicsTimedCropData()
    : HydroponicsCropData(), lastFeedingTime(0), feedTimingMins{0}
{
    _size = sizeof(*this);
}

void HydroponicsTimedCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsCropData::toJSONObject(objectOut);

    if (lastFeedingTime > DateTime((uint32_t)0).unixtime()) { objectOut[F("lastFeedingTime")] = lastFeedingTime; }
    JsonObject feedTimingMinsObj = objectOut.createNestedObject(F("feedTimingMins"));
    feedTimingMinsObj[F("on")] = feedTimingMins[0];
    feedTimingMinsObj[F("off")] = feedTimingMins[1];
}

void HydroponicsTimedCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsCropData::fromJSONObject(objectIn);
    lastFeedingTime = objectIn[F("lastFeedingTime")] | lastFeedingTime;
    JsonVariantConst feedTimingMinsVar = objectIn[F("feedTimingMins")];
    feedTimingMins[0] = feedTimingMinsVar[F("on")] | feedTimingMinsVar[0] | feedTimingMins[0];
    feedTimingMins[1] = feedTimingMinsVar[F("off")] | feedTimingMinsVar[1] | feedTimingMins[1];
}

HydroponicsAdaptiveCropData::HydroponicsAdaptiveCropData()
    : HydroponicsCropData(), feedingTrigger()
{
    _size = sizeof(*this);
}

void HydroponicsAdaptiveCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsCropData::toJSONObject(objectOut);

    if (feedingTrigger.type != -1) {
        JsonObject feedingTriggerObj = objectOut.createNestedObject(F("feedingTrigger"));
        feedingTrigger.toJSONObject(feedingTriggerObj);
    }
}

void HydroponicsAdaptiveCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsCropData::fromJSONObject(objectIn);

    JsonObjectConst feedingTriggerObj = objectIn[F("feedingTrigger")];
    if (!feedingTriggerObj.isNull()) { feedingTrigger.fromJSONObject(feedingTriggerObj); }
}
