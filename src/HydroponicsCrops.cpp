/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops
*/

#include "Hydroponics.h"

HydroponicsCrop *newCropObjectFromData(const HydroponicsCropData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), F("Invalid data"));

    if (dataIn && dataIn->isObjectData()) {
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
                                 DateTime sowDate,
                                 int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(cropType, cropIndex)), classType((typeof(classType))classTypeIn),
      _substrateType(substrateType), _sowDate(sowDate.unixtime()), _cropsData(nullptr), _growWeek(0), _feedingWeight(1.0f),
      _cropPhase(Hydroponics_CropPhase_Undefined), _feedingState(Hydroponics_TriggerState_NotTriggered)
{
    recalcGrowWeekAndPhase();
    attachCustomCrop();
}

HydroponicsCrop::HydroponicsCrop(const HydroponicsCropData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _substrateType(dataIn->substrateType), _sowDate(dataIn->sowDate), _feedReservoir(dataIn->feedReservoirName),
      _cropsData(nullptr), _growWeek(0), _feedingWeight(dataIn->feedingWeight),
      _cropPhase(Hydroponics_CropPhase_Undefined), _feedingState(Hydroponics_TriggerState_NotTriggered)
{
    recalcGrowWeekAndPhase();
    attachCustomCrop();
}

HydroponicsCrop::~HydroponicsCrop()
{
    detachCustomCrop();
    if (_cropsData) { returnCropsLibData(); }
    if (_feedReservoir) { _feedReservoir->removeCrop(this); }
}

void HydroponicsCrop::update()
{
    HydroponicsObject::update();

    auto feedingState = triggerStateFromBool(getNeedsFeeding());
    if (_feedingState != feedingState) {
        _feedingState = feedingState;
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

void HydroponicsCrop::notifyFeedingBegan()
{ ; }

void HydroponicsCrop::notifyFeedingEnded()
{ ; }

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

arx::map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE> HydroponicsCrop::getSensors() const
{
    return linksFilterSensors(_links);
}

void HydroponicsCrop::setFeedReservoir(HydroponicsIdentity reservoirId)
{
    if (_feedReservoir != reservoirId) {
        if (_feedReservoir) { _feedReservoir->removeCrop(this); }
        _feedReservoir = reservoirId;
    }
}

void HydroponicsCrop::setFeedReservoir(shared_ptr<HydroponicsFeedReservoir> reservoir)
{
    if (_feedReservoir != reservoir) {
        if (_feedReservoir) { _feedReservoir->removeCrop(this); }
        _feedReservoir = reservoir;
        if (_feedReservoir) { _feedReservoir->addCrop(this); }
    }
}

shared_ptr<HydroponicsFeedReservoir> HydroponicsCrop::getFeedReservoir()
{
    if (_feedReservoir.resolveIfNeeded()) { _feedReservoir->addCrop(this); }
    return static_pointer_cast<HydroponicsFeedReservoir>(_feedReservoir.getObj());
}

void HydroponicsCrop::setFeedingWeight(float weight)
{
    _feedingWeight = weight;
}

float HydroponicsCrop::getFeedingWeight() const
{
    return _feedingWeight;
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

DateTime HydroponicsCrop::getSowDate() const
{
    return DateTime((uint32_t)_sowDate);
}

const HydroponicsCropsLibData *HydroponicsCrop::getCropsLibData() const
{
    return _cropsData;
}

int HydroponicsCrop::getGrowWeek() const
{
    return _growWeek;
}

int HydroponicsCrop::getTotalGrowWeeks() const
{
    return _totalGrowWeeks;
}

Hydroponics_CropPhase HydroponicsCrop::getCropPhase() const
{
    return _cropPhase;
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
        strncpy(((HydroponicsCropData *)dataOut)->feedReservoirName, _feedReservoir.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    ((HydroponicsCropData *)dataOut)->feedingWeight = _feedingWeight;
}

void HydroponicsCrop::handleFeedingState()
{
    scheduleSignalFireOnce<HydroponicsCrop *>(getSharedPtr(), _feedingSignal, this);
}

void HydroponicsCrop::recalcGrowWeekAndPhase()
{
    TimeSpan dateSpan = DateTime(now()) - DateTime((uint32_t)_sowDate);
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

void HydroponicsCrop::attachCustomCrop()
{
    if (getCropType() >= Hydroponics_CropType_CustomCrop1 && getCropType() < Hydroponics_CropType_CustomCropCount) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_CropType>(this, &handleCustomCropUpdated);
        getCropsLibraryInstance()->getCustomCropSignal().attach(methodSlot);
    }
}

void HydroponicsCrop::detachCustomCrop()
{
    if (getCropType() >= Hydroponics_CropType_CustomCrop1 && getCropType() < Hydroponics_CropType_CustomCropCount) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_CropType>(this, &handleCustomCropUpdated);
        getCropsLibraryInstance()->getCustomCropSignal().detach(methodSlot);
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
      _lastFeedingTime(),
      _feedTimingMins{timeOn.totalseconds() / SECS_PER_MIN, timeOff.totalseconds() / SECS_PER_MIN}
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

void HydroponicsTimedCrop::notifyFeedingBegan()
{
    HydroponicsCrop::notifyFeedingBegan();

    _lastFeedingTime = now();
}

void HydroponicsTimedCrop::setFeedTimeOn(TimeSpan timeOn)
{
    _feedTimingMins[0] = timeOn.totalseconds() / SECS_PER_MIN;
}

TimeSpan HydroponicsTimedCrop::getFeedTimeOn() const
{
    return TimeSpan(_feedTimingMins[0] * SECS_PER_MIN);
}

void HydroponicsTimedCrop::setFeedTimeOff(TimeSpan timeOff)
{
    _feedTimingMins[1] = timeOff.totalseconds() / SECS_PER_MIN;
}

TimeSpan HydroponicsTimedCrop::getFeedTimeOff() const
{
    return TimeSpan(_feedTimingMins[1] * SECS_PER_MIN);
}

void HydroponicsTimedCrop::saveToData(HydroponicsData *dataOut)
{
    HydroponicsCrop::saveToData(dataOut);

    ((HydroponicsTimedCropData *)dataOut)->lastFeedingTime = _lastFeedingTime;
    ((HydroponicsTimedCropData *)dataOut)->feedTimingMins[0] = _feedTimingMins[0];
    ((HydroponicsTimedCropData *)dataOut)->feedTimingMins[1] = _feedTimingMins[1];
}


HydroponicsAdaptiveCrop::HydroponicsAdaptiveCrop(Hydroponics_CropType cropType,
                                               Hydroponics_PositionIndex cropIndex,
                                               Hydroponics_SubstrateType substrateType,
                                               DateTime sowDate,
                                               int classType)
    : HydroponicsCrop(cropType, cropIndex, substrateType, sowDate, classType),
      _moistureUnits(Hydroponics_UnitsType_Concentration_EC), _needsSoilMoisture(true), _feedingTrigger(nullptr)
{ ; }

HydroponicsAdaptiveCrop::HydroponicsAdaptiveCrop(const HydroponicsAdaptiveCropData *dataIn)
    : HydroponicsCrop(dataIn), _needsSoilMoisture(true),
      _moistureUnits(definedUnitsElse(dataIn->moistureUnits, Hydroponics_UnitsType_Concentration_EC)),
      _moistureSensor(dataIn->moistureSensorName),
      _feedingTrigger(newTriggerObjectFromSubData(&(dataIn->feedingTrigger)))
{ ; }

HydroponicsAdaptiveCrop::~HydroponicsAdaptiveCrop()
{
    if (_moistureSensor) { detachSoilMoistureSensor(); }
    if (_feedingTrigger) { detachFeedingTrigger(); delete _feedingTrigger; _feedingTrigger = nullptr; }
}

void HydroponicsAdaptiveCrop::update()
{
    HydroponicsCrop::update();

    if (_feedingTrigger) { _feedingTrigger->update(); }

    if (_needsSoilMoisture && getMoistureSensor()) {
        handleSoilMoistureMeasure(_moistureSensor->getLatestMeasurement());
    }
}

void HydroponicsAdaptiveCrop::resolveLinks()
{
    HydroponicsCrop::resolveLinks();

    if (_moistureSensor.needsResolved()) { getMoistureSensor(); }
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

void HydroponicsAdaptiveCrop::setMoistureUnits(Hydroponics_UnitsType moistureUnits)
{
    if (_moistureUnits != moistureUnits) {
        _moistureUnits = moistureUnits;

        convertUnits(&_soilMoisture, _moistureUnits);
    }
}

Hydroponics_UnitsType HydroponicsAdaptiveCrop::getMoistureUnits() const
{
    return definedUnitsElse(_moistureUnits, Hydroponics_UnitsType_Concentration_EC);
}

void HydroponicsAdaptiveCrop::setMoistureSensor(HydroponicsIdentity moistureSensorId)
{
    if (_moistureSensor != moistureSensorId) {
        if (_moistureSensor) { detachSoilMoistureSensor(); }
        _moistureSensor = moistureSensorId;
        _needsSoilMoisture = true;
    }
}

void HydroponicsAdaptiveCrop::setMoistureSensor(shared_ptr<HydroponicsSensor> moistureSensor)
{
    if (_moistureSensor != moistureSensor) {
        if (_moistureSensor) { detachSoilMoistureSensor(); }
        _moistureSensor = moistureSensor;
        if (_moistureSensor) { attachSoilMoistureSensor(); }
        _needsSoilMoisture = true;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsAdaptiveCrop::getMoistureSensor()
{
    if (_moistureSensor.resolveIfNeeded()) { attachSoilMoistureSensor(); }
    return _moistureSensor.getObj();
}

void HydroponicsAdaptiveCrop::setSoilMoisture(float soilMoisture, Hydroponics_UnitsType soilMoistureUnits)
{
    _soilMoisture.value = soilMoisture;
    _soilMoisture.units = soilMoistureUnits;
    _soilMoisture.updateTimestamp();
    _soilMoisture.updateFrame(1);

    convertUnits(&_soilMoisture, _moistureUnits);
    _needsSoilMoisture = false;
}

void HydroponicsAdaptiveCrop::setSoilMoisture(HydroponicsSingleMeasurement soilMoisture)
{
    _soilMoisture = soilMoisture;
    _soilMoisture.setMinFrame(1);

    convertUnits(&_soilMoisture, _moistureUnits);
    _needsSoilMoisture = false;
}

const HydroponicsSingleMeasurement &HydroponicsAdaptiveCrop::getSoilMoisture()
{
    if (_needsSoilMoisture && getMoistureSensor()) {
        handleSoilMoistureMeasure(_moistureSensor->getLatestMeasurement());
    }
    return _soilMoisture;
}

void HydroponicsAdaptiveCrop::setFeedingTrigger(HydroponicsTrigger *feedingTrigger)
{
    if (_feedingTrigger != feedingTrigger) {
        if (_feedingTrigger) { detachFeedingTrigger(); delete _feedingTrigger; }
        _feedingTrigger = feedingTrigger;
        if (_feedingTrigger) { attachFeedingTrigger(); }
    }
}

const HydroponicsTrigger *HydroponicsAdaptiveCrop::getFeedingTrigger() const
{
    return _feedingTrigger;
}

void HydroponicsAdaptiveCrop::saveToData(HydroponicsData *dataOut)
{
    HydroponicsCrop::saveToData(dataOut);

    ((HydroponicsAdaptiveCropData *)dataOut)->moistureUnits = _moistureUnits;
    if (_moistureSensor.getId()) {
        strncpy(((HydroponicsAdaptiveCropData *)dataOut)->moistureSensorName, _moistureSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_feedingTrigger) {
        _feedingTrigger->saveToData(&(((HydroponicsAdaptiveCropData *)dataOut)->feedingTrigger));
    }
}

void HydroponicsAdaptiveCrop::attachSoilMoistureSensor()
{
    HYDRUINO_SOFT_ASSERT(getMoistureSensor(), F("Moisture sensor not linked, failure attaching"));
    if (getMoistureSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleSoilMoistureMeasure);
        _moistureSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsAdaptiveCrop::detachSoilMoistureSensor()
{
    HYDRUINO_SOFT_ASSERT(getMoistureSensor(), F("Moisture sensor not linked, failure detaching"));
    if (getMoistureSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleSoilMoistureMeasure);
        _moistureSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsAdaptiveCrop::handleSoilMoistureMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        setSoilMoisture(singleMeasurementAt(measurement, 0));
    }
}

void HydroponicsAdaptiveCrop::attachFeedingTrigger()
{
    HYDRUINO_SOFT_ASSERT(_feedingTrigger, F("Feeding trigger not linked, failure attaching"));
    if (_feedingTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &handleFeedingTrigger);
        _feedingTrigger->getTriggerSignal().attach(methodSlot);
    }
}

void HydroponicsAdaptiveCrop::detachFeedingTrigger()
{
    HYDRUINO_SOFT_ASSERT(_feedingTrigger, F("Feeding trigger not linked, failure detaching"));
    if (_feedingTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &handleFeedingTrigger);
        _feedingTrigger->getTriggerSignal().detach(methodSlot);
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
    : HydroponicsObjectData(), substrateType(Hydroponics_SubstrateType_Undefined), sowDate(0), feedReservoirName{0}, feedingWeight(1.0f)
{
    _size = sizeof(*this);
}

void HydroponicsCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsObjectData::toJSONObject(objectOut);

    if (substrateType != Hydroponics_SubstrateType_Undefined) { objectOut[F("substrateType")] = substrateTypeToString(substrateType); }
    if (sowDate) { objectOut[F("sowDate")] = sowDate; }
    if (feedReservoirName[0]) { objectOut[F("feedReservoirName")] = stringFromChars(feedReservoirName, HYDRUINO_NAME_MAXSIZE); }
    if (!isFPEqual(feedingWeight, 1.0f)) { objectOut[F("feedingWeight")] = feedingWeight; }
}

void HydroponicsCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);
    substrateType = substrateTypeFromString(objectIn[F("substrateType")]);
    sowDate = objectIn[F("sowDate")] | sowDate;
    const char *feedReservoirNameStr = objectIn[F("feedReservoirName")];
    if (feedReservoirNameStr && feedReservoirNameStr[0]) { strncpy(feedReservoirName, feedReservoirNameStr, HYDRUINO_NAME_MAXSIZE); }
    feedingWeight = objectIn[F("feedingWeight")] | feedingWeight;
}

HydroponicsTimedCropData::HydroponicsTimedCropData()
    : HydroponicsCropData(), lastFeedingTime(0), feedTimingMins{0}
{
    _size = sizeof(*this);
}

void HydroponicsTimedCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsCropData::toJSONObject(objectOut);

    if (lastFeedingTime) { objectOut[F("lastFeedingTime")] = lastFeedingTime; }
    objectOut[F("feedTimingMins")] = commaStringFromArray(feedTimingMins, 2);
}

void HydroponicsTimedCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsCropData::fromJSONObject(objectIn);
    lastFeedingTime = objectIn[F("lastFeedingTime")] | lastFeedingTime;
    JsonVariantConst feedTimingMinsVar = objectIn[F("feedTimingMins")];
    commaStringToArray(feedTimingMinsVar, feedTimingMins, 2);
    feedTimingMins[0] = feedTimingMinsVar[F("on")] | feedTimingMinsVar[0] | feedTimingMins[0];
    feedTimingMins[1] = feedTimingMinsVar[F("off")] | feedTimingMinsVar[1] | feedTimingMins[1];
}

HydroponicsAdaptiveCropData::HydroponicsAdaptiveCropData()
    : HydroponicsCropData(), moistureUnits(Hydroponics_UnitsType_Undefined), moistureSensorName{0}
{
    _size = sizeof(*this);
}

void HydroponicsAdaptiveCropData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsCropData::toJSONObject(objectOut);

    if (moistureUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("moistureUnits")] = unitsTypeToSymbol(moistureUnits); }
    if (moistureSensorName[0]) { objectOut[F("moistureSensorName")] = stringFromChars(moistureSensorName, HYDRUINO_NAME_MAXSIZE); }
    if (feedingTrigger.type != -1) {
        JsonObject feedingTriggerObj = objectOut.createNestedObject(F("feedingTrigger"));
        feedingTrigger.toJSONObject(feedingTriggerObj);
    }
}

void HydroponicsAdaptiveCropData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsCropData::fromJSONObject(objectIn);

    moistureUnits = unitsTypeFromSymbol(objectIn[F("moistureUnits")]);
    const char *moistureSensorNameStr = objectIn[F("moistureSensorName")];
    if (moistureSensorNameStr && moistureSensorNameStr[0]) { strncpy(moistureSensorName, moistureSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    JsonObjectConst feedingTriggerObj = objectIn[F("feedingTrigger")];
    if (!feedingTriggerObj.isNull()) { feedingTrigger.fromJSONObject(feedingTriggerObj); }
}
