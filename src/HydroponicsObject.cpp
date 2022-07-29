/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Object
*/

#include "Hydroponics.h"

HydroponicsObject *newObjectFromData(const HydroponicsData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectectData(), SFP(HStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectectData()) {
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
    : type(Unknown), objTypeAs{.actuatorType=(Hydroponics_ActuatorType)-1}, posIndex(-1), keyString(), key((Hydroponics_KeyType)-1)
{ ; }

HydroponicsIdentity::HydroponicsIdentity(const HydroponicsIdentity &id, Hydroponics_PositionIndex positionIndex)
    : type(id.type), objTypeAs{.actuatorType=id.objTypeAs.actuatorType}, posIndex(positionIndex), keyString(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_ActuatorType actuatorTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Actuator), objTypeAs{.actuatorType=actuatorTypeIn}, posIndex(positionIndex), keyString(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_SensorType sensorTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Sensor), objTypeAs{.sensorType=sensorTypeIn}, posIndex(positionIndex), keyString(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_CropType cropTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Crop), objTypeAs{.cropType=cropTypeIn}, posIndex(positionIndex), keyString(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_ReservoirType reservoirTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Reservoir), objTypeAs{.reservoirType=reservoirTypeIn}, posIndex(positionIndex), keyString(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_RailType railTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Rail), objTypeAs{.railType=railTypeIn}, posIndex(positionIndex), keyString(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(const HydroponicsSubObject *obj)
    : type(SubObject), objTypeAs{.actuatorType=(Hydroponics_ActuatorType)-1}, posIndex(-1), keyString(),
      key((Hydroponics_KeyType)(uintptr_t)obj)
{ ; }

HydroponicsIdentity::HydroponicsIdentity(const HydroponicsData *dataIn)
    : type((typeof(type))(dataIn->id.object.idType)),
      objTypeAs{.actuatorType=(Hydroponics_ActuatorType)(dataIn->id.object.objType)},
      posIndex(dataIn->id.object.posIndex),
      keyString(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(const char *idKeyStr)
    : type(Unknown), objTypeAs{.actuatorType=(Hydroponics_ActuatorType)-1}, posIndex(-1), keyString(idKeyStr), key((Hydroponics_KeyType)-1)
{
    // TODO: Advanced string detokenization (may not be needed tho)
    regenKey();
}

HydroponicsIdentity::HydroponicsIdentity(const String &idKeyString)
    : HydroponicsIdentity(idKeyString.c_str())
{ ; }

Hydroponics_KeyType HydroponicsIdentity::regenKey()
{
    String sep = String('.');
    switch (type) {
        case Actuator:
            keyString = actuatorTypeToString(objTypeAs.actuatorType, true) + sep + positionIndexToString(posIndex, true);
            break;
        case Sensor:
            keyString = sensorTypeToString(objTypeAs.sensorType, true) + sep + positionIndexToString(posIndex, true);
            break;
        case Crop:
            keyString = cropTypeToString(objTypeAs.cropType, true) + sep + positionIndexToString(posIndex, true);
            break;
        case Reservoir:
            keyString = reservoirTypeToString(objTypeAs.reservoirType, true) + sep + positionIndexToString(posIndex, true);
            break;
        case Rail:
            keyString = railTypeToString(objTypeAs.railType, true) + sep + positionIndexToString(posIndex, true);
            break;
        case SubObject:
            return key;
        default: break;
    }
    key = stringHash(keyString);
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

void HydroponicsObject::handleLowMemory()
{
    #ifdef HYDRUINO_USE_STDCPP_CONTAINERS
        _links.shrink_to_fit(); // only implemented in libstdc++
    #endif
}

HydroponicsData *HydroponicsObject::newSaveData()
{
    auto data = allocateData();
    HYDRUINO_SOFT_ASSERT(data, SFP(HStr_Err_AllocationFailure));
    if (data) { saveToData(data); }
    return data;
}

bool HydroponicsObject::addLinkage(HydroponicsObject *obj)
{
    auto iter = _links.find(obj->getKey());
    if (iter != _links.end()) {
        iter->second.second++;
    } else {
        _links[obj->getKey()] = make_pair(obj, (int8_t)1);
        return true;
    }
    return false;
}

bool HydroponicsObject::removeLinkage(HydroponicsObject *obj)
{
    auto iter = _links.find(obj->getKey());
    if (iter != _links.end()) {
        if (--iter->second.second == 0) {
            _links.erase(iter);
            return true;
        }
    }
    return false;
}

bool HydroponicsObject::hasLinkage(HydroponicsObject *obj) const
{
    return (_links.find(obj->getId()) != _links.end());
}

HydroponicsIdentity HydroponicsObject::getId() const
{
    return _id;
}

Hydroponics_KeyType HydroponicsObject::getKey() const
{
    return _id.key;
}

shared_ptr<HydroponicsObjInterface> HydroponicsObject::getSharedPtr() const
{
    return getHydroponicsInstance() ? getHydroponicsInstance()->objectById(_id) : nullptr;
}

HydroponicsData *HydroponicsObject::allocateData() const
{
    HYDRUINO_HARD_ASSERT(false, SFP(HStr_Err_UnsupportedOperation));
    return new HydroponicsData();
}

void HydroponicsObject::saveToData(HydroponicsData *dataOut)
{
    dataOut->id.object.idType = (int8_t)_id.type;
    dataOut->id.object.objType = (int8_t)_id.objTypeAs.actuatorType;
    dataOut->id.object.posIndex = (int8_t)_id.posIndex;
    if (_id.keyString.length()) {
        strncpy(((HydroponicsObjectData *)dataOut)->name, _id.keyString.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}


HydroponicsIdentity HydroponicsSubObject::getId() const
{
    return HydroponicsIdentity(this);
}

Hydroponics_KeyType HydroponicsSubObject::getKey() const
{
    return (Hydroponics_KeyType)(intptr_t)this;
}

shared_ptr<HydroponicsObjInterface> HydroponicsSubObject::getSharedPtr() const
{
    return shared_ptr<HydroponicsObjInterface>(this);
}

bool HydroponicsSubObject::addLinkage(HydroponicsObject *obj)
{
    return false;
}

bool HydroponicsSubObject::removeLinkage(HydroponicsObject *obj)
{
    return false;
}


HydroponicsObjectData::HydroponicsObjectData()
    : HydroponicsData(), name{0}
{
    _size = sizeof(*this);
}

void HydroponicsObjectData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsData::toJSONObject(objectOut);

    if (name[0]) { objectOut[SFP(HStr_Key_Id)] = charsToString(name, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsObjectData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsData::fromJSONObject(objectIn);

    const char *nameStr = objectIn[SFP(HStr_Key_Id)];
    if (nameStr && nameStr[0]) { strncpy(name, nameStr, HYDRUINO_NAME_MAXSIZE); }
}
