/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops
*/

#include "Hydroponics.h"

HydroponicsCrop::HydroponicsCrop(Hydroponics_CropType cropType,
                                 Hydroponics_PositionIndex cropIndex,
                                 Hydroponics_SubstrateType substrateType,
                                 time_t sowDate,
                                 int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(cropType, cropIndex)), classType((typeof(classType))classTypeIn),
      _substrateType(substrateType), _sowDate(sowDate),
      _cropsLibData(nullptr), _growWeek(0), _cropPhase(Hydroponics_CropPhase_Undefined)
{
    recalcGrowWeekAndPhase();
}

HydroponicsCrop::~HydroponicsCrop()
{
    if (_cropsLibData) { returnCropsLibData(); }
    if (_feedReservoir) { _feedReservoir->removeCrop(this); }
}

void HydroponicsCrop::update()
{
    HydroponicsObject::update();
    recalcGrowWeekAndPhase();
}

void HydroponicsCrop::resolveLinks()
{
    HydroponicsObject::resolveLinks();
    if (_feedReservoir.needsResolved()) { getFeedReservoir(); }
}

void HydroponicsCrop::handleLowMem()
{
    HydroponicsObject::handleLowMem();
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

arx::map<Hydroponics_KeyType, HydroponicsSensor *> HydroponicsCrop::getSensors()
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
    return _id.as.cropType;
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
    return _cropsLibData;
}

int HydroponicsCrop::getGrowWeek() const
{
    return _growWeek;
}

Hydroponics_CropPhase HydroponicsCrop::getCropPhase() const
{
    return _cropPhase;
}

void HydroponicsCrop::recalcGrowWeekAndPhase()
{
    TimeSpan dateSpan = DateTime(now()) - DateTime(_sowDate);
    _growWeek = dateSpan.days() / DAYS_PER_WEEK;

    if (!_cropsLibData) { checkoutCropsLibData(); }
    HYDRUINO_SOFT_ASSERT(_cropsLibData, "Invalid crops lib data, unable to update growth cycle");

    if (_cropsLibData) {
        for (int phaseIndex = 0; phaseIndex < (int)Hydroponics_CropPhase_Count; ++phaseIndex) {
            if (_growWeek >= _cropsLibData->phaseBeginWeek[phaseIndex]) {
                _cropPhase = (Hydroponics_CropPhase)phaseIndex;
            } else { break; }
        }
    }
}

void HydroponicsCrop::checkoutCropsLibData()
{
    if (!_cropsLibData) {
        auto cropLib = HydroponicsCropsLibrary::getInstance();
        if (cropLib) {
            _cropsLibData = cropLib->checkoutCropData(_id.as.cropType);
        }
    }
}

void HydroponicsCrop::returnCropsLibData()
{
    if (_cropsLibData) {
        auto cropLib = HydroponicsCropsLibrary::getInstance();
        if (cropLib) {
            cropLib->returnCropData(_cropsLibData); _cropsLibData = nullptr;
        }
    }
}


HydroponicsSimpleCrop::HydroponicsSimpleCrop(Hydroponics_CropType cropType,
                                             Hydroponics_PositionIndex cropIndex,
                                             Hydroponics_SubstrateType substrateType,
                                             time_t sowDate,
                                             int classType)
    : HydroponicsCrop(cropType, cropIndex, substrateType, sowDate, classType)
{ ; }

HydroponicsSimpleCrop::~HydroponicsSimpleCrop()
{ ; }
