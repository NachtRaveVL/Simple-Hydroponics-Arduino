/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Object
*/

#include "Hydruino.h"

HydroObject *newObjectFromData(const HydroData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.idType) {
            case (int8_t)HydroIdentity::Actuator:
                return newActuatorObjectFromData((HydroActuatorData *)dataIn);
            case (int8_t)HydroIdentity::Sensor:
                return newSensorObjectFromData((HydroSensorData *)dataIn);
            case (int8_t)HydroIdentity::Crop:
                return newCropObjectFromData((HydroCropData *)dataIn);
            case (int8_t)HydroIdentity::Reservoir:
                return newReservoirObjectFromData((HydroReservoirData *)dataIn);
            case (int8_t)HydroIdentity::Rail:
                return newRailObjectFromData((HydroRailData *)dataIn);
            default: // Unable
                return nullptr;
        }
    }

    return nullptr;
}


HydroIdentity::HydroIdentity()
    : type(Unknown), objTypeAs{.actuatorType=(Hydro_ActuatorType)-1}, posIndex(-1), keyString(), key((Hydro_KeyType)-1)
{ ; }

HydroIdentity::HydroIdentity(Hydro_KeyType key)
    : type(Unknown), objTypeAs{.actuatorType=(Hydro_ActuatorType)-1}, posIndex(-1), keyString(), key(key)
{ ; }

HydroIdentity::HydroIdentity(const char *idKeyStr)
    : type(Unknown), objTypeAs{.actuatorType=(Hydro_ActuatorType)-1}, posIndex(-1), keyString(idKeyStr), key(stringHash(idKeyStr))
{
    // TODO: Advanced string detokenization (may not be needed tho)
}

HydroIdentity::HydroIdentity(String idKey)
    : type(Unknown), objTypeAs{.actuatorType=(Hydro_ActuatorType)-1}, posIndex(-1), keyString(idKey), key(stringHash(idKey.c_str()))
{ ; }

HydroIdentity::HydroIdentity(const HydroIdentity &id, Hydro_PositionIndex positionIndex)
    : type(id.type), objTypeAs{.actuatorType=id.objTypeAs.actuatorType}, posIndex(positionIndex), keyString(), key((Hydro_KeyType)-1)
{
    regenKey();
}

HydroIdentity::HydroIdentity(Hydro_ActuatorType actuatorTypeIn, Hydro_PositionIndex positionIndex)
    : type(Actuator), objTypeAs{.actuatorType=actuatorTypeIn}, posIndex(positionIndex), keyString(), key((Hydro_KeyType)-1)
{
    regenKey();
}

HydroIdentity::HydroIdentity(Hydro_SensorType sensorTypeIn, Hydro_PositionIndex positionIndex)
    : type(Sensor), objTypeAs{.sensorType=sensorTypeIn}, posIndex(positionIndex), keyString(), key((Hydro_KeyType)-1)
{
    regenKey();
}

HydroIdentity::HydroIdentity(Hydro_CropType cropTypeIn, Hydro_PositionIndex positionIndex)
    : type(Crop), objTypeAs{.cropType=cropTypeIn}, posIndex(positionIndex), keyString(), key((Hydro_KeyType)-1)
{
    regenKey();
}

HydroIdentity::HydroIdentity(Hydro_ReservoirType reservoirTypeIn, Hydro_PositionIndex positionIndex)
    : type(Reservoir), objTypeAs{.reservoirType=reservoirTypeIn}, posIndex(positionIndex), keyString(), key((Hydro_KeyType)-1)
{
    regenKey();
}

HydroIdentity::HydroIdentity(Hydro_RailType railTypeIn, Hydro_PositionIndex positionIndex)
    : type(Rail), objTypeAs{.railType=railTypeIn}, posIndex(positionIndex), keyString(), key((Hydro_KeyType)-1)
{
    regenKey();
}

HydroIdentity::HydroIdentity(const HydroData *dataIn)
    : type((typeof(type))(dataIn->id.object.idType)),
      objTypeAs{.actuatorType=(Hydro_ActuatorType)(dataIn->id.object.objType)},
      posIndex(dataIn->id.object.posIndex),
      keyString(), key((Hydro_KeyType)-1)
{
    regenKey();
}

Hydro_KeyType HydroIdentity::regenKey()
{
    switch (type) {
        case Actuator:
            keyString = actuatorTypeToString(objTypeAs.actuatorType, true);
            break;
        case Sensor:
            keyString = sensorTypeToString(objTypeAs.sensorType, true);
            break;
        case Crop:
            keyString = cropTypeToString(objTypeAs.cropType, true);
            break;
        case Reservoir:
            keyString = reservoirTypeToString(objTypeAs.reservoirType, true);
            break;
        case Rail:
            keyString = railTypeToString(objTypeAs.railType, true);
            break;
        default: // Unable
            return key;
    }
    keyString.concat(' ');
    keyString.concat('#');
    keyString.concat(positionIndexToString(posIndex, true));
    key = stringHash(keyString);
    return key;
}


HydroObject::HydroObject(HydroIdentity id)
    : _id(id), _linksSize(0), _links(nullptr)
{ ; }

HydroObject::HydroObject(const HydroData *data)
    : _id(data), _linksSize(0), _links(nullptr)
{ ; }

HydroObject::~HydroObject()
{
    if (_links) { delete [] _links; _links = nullptr; }
}

void HydroObject::update()
{ ; }

void HydroObject::handleLowMemory()
{
    if (_links && !_links[_linksSize >> 1].first) { allocateLinkages(_linksSize >> 1); } // shrink /2 if too big
}

HydroData *HydroObject::newSaveData()
{
    auto data = allocateData();
    HYDRO_SOFT_ASSERT(data, SFP(HStr_Err_AllocationFailure));
    if (data) { saveToData(data); }
    return data;
}

void HydroObject::allocateLinkages(size_t size)
{
    if (_linksSize != size) {
        Pair<HydroObject *, int8_t> *newLinks = size ? new Pair<HydroObject *, int8_t>[size] : nullptr;

        if (size) {
            HYDRO_HARD_ASSERT(newLinks, SFP(HStr_Err_AllocationFailure));

            int linksIndex = 0;
            if (_links) {
                for (; linksIndex < _linksSize && linksIndex < size; ++linksIndex) {
                    newLinks[linksIndex] = _links[linksIndex];
                }
            }
            for (; linksIndex < size; ++linksIndex) {
                newLinks[linksIndex] = make_pair((HydroObject *)nullptr, (int8_t)0);
            }
        }

        if (_links) { delete [] _links; }
        _links = newLinks;
        _linksSize = size;
    }
}

bool HydroObject::addLinkage(HydroObject *obj)
{
    if (!_links) { allocateLinkages(); }
    if (_links) {
        if (_links[_linksSize-1].first) { allocateLinkages(_linksSize << 1); } // grow *2 if too small
        int linksIndex = 0;
        for (; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            if (_links[linksIndex].first == obj) {
                _links[linksIndex].second++;
                return true;
            }
        }
        if (linksIndex < _linksSize) {
            _links[linksIndex] = make_pair(obj, (int8_t)0);
            return true;
        }
    }
    return false;
}

bool HydroObject::removeLinkage(HydroObject *obj)
{
    if (_links) {
        for (int linksIndex = 0; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            if (_links[linksIndex].first == obj) {
                for (int linksSubIndex = linksIndex; linksSubIndex < _linksSize - 1; ++linksSubIndex) {
                    _links[linksSubIndex] = _links[linksSubIndex + 1];
                }
                _links[_linksSize - 1] = make_pair((HydroObject *)nullptr, (int8_t)0);
                return true;
            }
        }
    }
    return false;
}

bool HydroObject::hasLinkage(HydroObject *obj) const
{
    if (_links) {
        for (int linksIndex = 0; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            if (_links[linksIndex].first == obj) {
                return true;
            }
        }
    }
    return false;
}

HydroIdentity HydroObject::getId() const
{
    return _id;
}

Hydro_KeyType HydroObject::getKey() const
{
    return _id.key;
}

String HydroObject::getKeyString() const
{
    return _id.keyString;
}

SharedPtr<HydroObjInterface> HydroObject::getSharedPtr() const
{
    return getHydroInstance() ? static_pointer_cast<HydroObjInterface>(getHydroInstance()->objectById(_id))
                              : SharedPtr<HydroObjInterface>((HydroObjInterface *)this);
}

HydroData *HydroObject::allocateData() const
{
    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_UnsupportedOperation));
    return new HydroData();
}

void HydroObject::saveToData(HydroData *dataOut)
{
    dataOut->id.object.idType = (int8_t)_id.type;
    dataOut->id.object.objType = (int8_t)_id.objTypeAs.actuatorType;
    dataOut->id.object.posIndex = (int8_t)_id.posIndex;
    if (_id.keyString.length()) {
        strncpy(((HydroObjectData *)dataOut)->name, _id.keyString.c_str(), HYDRO_NAME_MAXSIZE);
    }
}


HydroIdentity HydroSubObject::getId() const
{
    return HydroIdentity(getKey());
}

Hydro_KeyType HydroSubObject::getKey() const
{
    return (Hydro_KeyType)(intptr_t)this;
}

String HydroSubObject::getKeyString() const
{
    return addressToString((uintptr_t)this);
}

SharedPtr<HydroObjInterface> HydroSubObject::getSharedPtr() const
{
    return SharedPtr<HydroObjInterface>((HydroObjInterface *)this);
}

bool HydroSubObject::addLinkage(HydroObject *obj)
{
    return false;
}

bool HydroSubObject::removeLinkage(HydroObject *obj)
{
    return false;
}


HydroObjectData::HydroObjectData()
    : HydroData(), name{0}
{
    _size = sizeof(*this);
}

void HydroObjectData::toJSONObject(JsonObject &objectOut) const
{
    HydroData::toJSONObject(objectOut);

    if (name[0]) { objectOut[SFP(HStr_Key_Id)] = charsToString(name, HYDRO_NAME_MAXSIZE); }
}

void HydroObjectData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroData::fromJSONObject(objectIn);

    const char *nameStr = objectIn[SFP(HStr_Key_Id)];
    if (nameStr && nameStr[0]) { strncpy(name, nameStr, HYDRO_NAME_MAXSIZE); }
}
