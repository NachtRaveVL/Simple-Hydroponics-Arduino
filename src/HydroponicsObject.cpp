/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Object
*/

#include "Hydroponics.h"

HydroponicsObject *newObjectFromData(const HydroponicsData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HS_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.idType) {
            case 0: // Actuator
                return newActuatorObjectFromData((HydroponicsActuatorData *)dataIn);
            case 1: // Sensor
                return newSensorObjectFromData((HydroponicsSensorData *)dataIn);
            case 2: // Crop
                return newCropObjectFromData((HydroponicsCropData *)dataIn);
            case 3: // Reservoir
                return newReservoirObjectFromData((HydroponicsReservoirData *)dataIn);
            case 4: // Rail
                return newRailObjectFromData((HydroponicsRailData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroponicsIdentity::HydroponicsIdentity()
    : type(Unknown), objTypeAs{.actuatorType=(Hydroponics_ActuatorType)-1}, posIndex(-1), keyStr(), key((Hydroponics_KeyType)-1)
{ ; }

HydroponicsIdentity::HydroponicsIdentity(const HydroponicsIdentity &id, Hydroponics_PositionIndex positionIndex)
    : type(id.type), objTypeAs{.actuatorType=id.objTypeAs.actuatorType}, posIndex(positionIndex), keyStr(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_ActuatorType actuatorTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Actuator), objTypeAs{.actuatorType=actuatorTypeIn}, posIndex(positionIndex), keyStr(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_SensorType sensorTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Sensor), objTypeAs{.sensorType=sensorTypeIn}, posIndex(positionIndex), keyStr(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_CropType cropTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Crop), objTypeAs{.cropType=cropTypeIn}, posIndex(positionIndex), keyStr(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_ReservoirType reservoirTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Reservoir), objTypeAs{.reservoirType=reservoirTypeIn}, posIndex(positionIndex), keyStr(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_RailType railTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Rail), objTypeAs{.railType=railTypeIn}, posIndex(positionIndex), keyStr(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(const HydroponicsData *dataIn)
    : type((typeof(type))(dataIn->id.object.idType)),
      objTypeAs{.actuatorType=(Hydroponics_ActuatorType)(dataIn->id.object.objType)},
      posIndex(dataIn->id.object.posIndex),
      keyStr(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(const char *name)
    : type(Unknown), objTypeAs{.actuatorType=(Hydroponics_ActuatorType)-1}, posIndex(-1), keyStr(name), key((Hydroponics_KeyType)-1)
{
    // TODO: Advanced string detokenization (may not be needed tho)
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(String name)
    : HydroponicsIdentity(name.c_str())
{ ; }

Hydroponics_KeyType HydroponicsIdentity::regenKey()
{
    String sep = String('.');
    switch(type) {
        case Actuator:
            keyStr = actuatorTypeToString(objTypeAs.actuatorType, true) + sep + positionIndexToString(posIndex, true);
            break;
        case Sensor:
            keyStr = sensorTypeToString(objTypeAs.sensorType, true) + sep + positionIndexToString(posIndex, true);
            break;
        case Crop:
            keyStr = cropTypeToString(objTypeAs.cropType, true) + sep + positionIndexToString(posIndex, true);
            break;
        case Reservoir:
            keyStr = reservoirTypeToString(objTypeAs.reservoirType, true) + sep + positionIndexToString(posIndex, true);
            break;
        case Rail:
            keyStr = railTypeToString(objTypeAs.railType, true) + sep + positionIndexToString(posIndex, true);
            break;
        default: break;
    }
    key = stringHash(keyStr);
    return key;
}


HydroponicsObject::HydroponicsObject(HydroponicsIdentity id)
    : _id(id), _links()
{ ; }

HydroponicsObject::HydroponicsObject(const HydroponicsData *data)
    : _id(data), _links()
{ ; }

HydroponicsObject::~HydroponicsObject()
{ ; }

void HydroponicsObject::update()
{ ; }

void HydroponicsObject::resolveLinks()
{ ; }

void HydroponicsObject::handleLowMemory()
{
    //_links.shrink_to_fit(); // not yet implemented library method
}

HydroponicsData *HydroponicsObject::newSaveData()
{
    auto data = allocateData();
    HYDRUINO_SOFT_ASSERT(data, SFP(HS_Err_AllocationFailure));
    if (data) { saveToData(data); }
    return data;
}

bool HydroponicsObject::hasLinkage(HydroponicsObject *obj) const
{
    return (_links.find(obj->_id) != _links.end());
}

const arx::map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE> HydroponicsObject::getLinkages() const
{
    return _links;
}

const HydroponicsIdentity &HydroponicsObject::getId() const
{
    return _id;
}

Hydroponics_KeyType HydroponicsObject::getKey() const
{
    return _id.key;
}

shared_ptr<HydroponicsObject> HydroponicsObject::getSharedPtr() const
{
    auto hydroponics = getHydroponicsInstance();
    return hydroponics ? hydroponics->objectById(_id) : nullptr;
}

bool HydroponicsObject::addLinkage(HydroponicsObject *obj)
{
    return _links.insert(obj->getKey(), obj).second;
}

bool HydroponicsObject::removeLinkage(HydroponicsObject *obj)
{
    auto iter = _links.find(obj->_id);
    if (iter != _links.end()) {
        _links.erase(iter);
        return true;
    }
    return false;
}

HydroponicsData *HydroponicsObject::allocateData() const
{
    HYDRUINO_HARD_ASSERT(false, SFP(HS_Err_UnsupportedOperation));
    return new HydroponicsData();
}

void HydroponicsObject::saveToData(HydroponicsData *dataOut)
{
    dataOut->id.object.idType = (int8_t)_id.type;
    dataOut->id.object.objType = (int8_t)_id.objTypeAs.actuatorType;
    dataOut->id.object.posIndex = (int8_t)_id.posIndex;
    if (_id.keyStr.length()) {
        strncpy(((HydroponicsObjectData *)dataOut)->name, _id.keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}


HydroponicsObjectData::HydroponicsObjectData()
    : HydroponicsData(), name{0}
{
    _size = sizeof(*this);
}

void HydroponicsObjectData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsData::toJSONObject(objectOut);

    if (name[0]) { objectOut[SFP(HS_Key_Id)] = stringFromChars(name, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsObjectData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsData::fromJSONObject(objectIn);

    const char *nameStr = objectIn[SFP(HS_Key_Id)];
    if (nameStr && nameStr[0]) { strncpy(name, nameStr, HYDRUINO_NAME_MAXSIZE); }
}
