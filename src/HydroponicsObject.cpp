/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydroponics Object
*/

#include "Hydroponics.h"

HydroponicsObject *newObjectFromData(const HydroponicsData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HStr_Err_InvalidParameter));

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
            default: // Unable
                return nullptr;
        }
    }

    return nullptr;
}


HydroponicsIdentity::HydroponicsIdentity()
    : type(Unknown), objTypeAs{.actuatorType=(Hydroponics_ActuatorType)-1}, posIndex(-1), keyString(), key((Hydroponics_KeyType)-1)
{ ; }

HydroponicsIdentity::HydroponicsIdentity(Hydroponics_KeyType key)
    : type(Unknown), objTypeAs{.actuatorType=(Hydroponics_ActuatorType)-1}, posIndex(-1), keyString(), key(key)
{ ; }

HydroponicsIdentity::HydroponicsIdentity(const char *idKeyStr)
    : type(Unknown), objTypeAs{.actuatorType=(Hydroponics_ActuatorType)-1}, posIndex(-1), keyString(idKeyStr), key(stringHash(idKeyStr))
{
    // TODO: Advanced string detokenization (may not be needed tho)
}

HydroponicsIdentity::HydroponicsIdentity(String idKey)
    : type(Unknown), objTypeAs{.actuatorType=(Hydroponics_ActuatorType)-1}, posIndex(-1), keyString(idKey), key(stringHash(idKey.c_str()))
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

HydroponicsIdentity::HydroponicsIdentity(const HydroponicsData *dataIn)
    : type((typeof(type))(dataIn->id.object.idType)),
      objTypeAs{.actuatorType=(Hydroponics_ActuatorType)(dataIn->id.object.objType)},
      posIndex(dataIn->id.object.posIndex),
      keyString(), key((Hydroponics_KeyType)-1)
{
    regenKey();
}

Hydroponics_KeyType HydroponicsIdentity::regenKey()
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


HydroponicsObject::HydroponicsObject(HydroponicsIdentity id)
    : _id(id), _linksSize(0), _links(nullptr)
{ ; }

HydroponicsObject::HydroponicsObject(const HydroponicsData *data)
    : _id(data), _linksSize(0), _links(nullptr)
{ ; }

HydroponicsObject::~HydroponicsObject()
{
    if (_links) { delete [] _links; _links = nullptr; }
}

void HydroponicsObject::update()
{ ; }

void HydroponicsObject::handleLowMemory()
{
    if (_links && !_links[_linksSize >> 1].first) { allocateLinkages(_linksSize >> 1); } // shrink /2 if too big
}

HydroponicsData *HydroponicsObject::newSaveData()
{
    auto data = allocateData();
    HYDRUINO_SOFT_ASSERT(data, SFP(HStr_Err_AllocationFailure));
    if (data) { saveToData(data); }
    return data;
}

void HydroponicsObject::allocateLinkages(size_t size)
{
    if (_linksSize != size) {
        Pair<HydroponicsObject *, int8_t> *newLinks = size ? new Pair<HydroponicsObject *, int8_t>[size] : nullptr;

        if (size) {
            HYDRUINO_HARD_ASSERT(newLinks, SFP(HStr_Err_AllocationFailure));

            int linksIndex = 0;
            if (_links) {
                for (; linksIndex < _linksSize && linksIndex < size; ++linksIndex) {
                    newLinks[linksIndex] = _links[linksIndex];
                }
            }
            for (; linksIndex < size; ++linksIndex) {
                newLinks[linksIndex] = make_pair((HydroponicsObject *)nullptr, (int8_t)0);
            }
        }

        if (_links) { delete [] _links; }
        _links = newLinks;
        _linksSize = size;
    }
}

bool HydroponicsObject::addLinkage(HydroponicsObject *obj)
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

bool HydroponicsObject::removeLinkage(HydroponicsObject *obj)
{
    if (_links) {
        for (int linksIndex = 0; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            if (_links[linksIndex].first == obj) {
                for (int linksSubIndex = linksIndex; linksSubIndex < _linksSize - 1; ++linksSubIndex) {
                    _links[linksSubIndex] = _links[linksSubIndex + 1];
                }
                _links[_linksSize - 1] = make_pair((HydroponicsObject *)nullptr, (int8_t)0);
                return true;
            }
        }
    }
    return false;
}

bool HydroponicsObject::hasLinkage(HydroponicsObject *obj) const
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

HydroponicsIdentity HydroponicsObject::getId() const
{
    return _id;
}

Hydroponics_KeyType HydroponicsObject::getKey() const
{
    return _id.key;
}

String HydroponicsObject::getKeyString() const
{
    return _id.keyString;
}

SharedPtr<HydroponicsObjInterface> HydroponicsObject::getSharedPtr() const
{
    return getHydroponicsInstance() ? hy_static_ptr_cast<HydroponicsObjInterface>(getHydroponicsInstance()->objectById(_id))
                                    : SharedPtr<HydroponicsObjInterface>((HydroponicsObjInterface *)this);
}

#ifdef HYDRUINO_USE_VIRTMEM

void* HydroponicsObject::operator new(size_t size)
{
    return (void *)(getVirtualAllocator()->allocRaw(size));
}

void HydroponicsObject::operator delete(void* ptr)
{
    getVirtualAllocator()->freeRaw((VPtrNum)ptr);
}

#endif

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
    return HydroponicsIdentity(getKey());
}

Hydroponics_KeyType HydroponicsSubObject::getKey() const
{
    return (Hydroponics_KeyType)(intptr_t)this;
}

String HydroponicsSubObject::getKeyString() const
{
    return addressToString((uintptr_t)this);
}

SharedPtr<HydroponicsObjInterface> HydroponicsSubObject::getSharedPtr() const
{
    return SharedPtr<HydroponicsObjInterface>((HydroponicsObjInterface *)this);
}

bool HydroponicsSubObject::addLinkage(HydroponicsObject *obj)
{
    return false;
}

bool HydroponicsSubObject::removeLinkage(HydroponicsObject *obj)
{
    return false;
}

#ifdef HYDRUINO_USE_VIRTMEM

void* HydroponicsSubObject::operator new(size_t size)
{
    return (void *)(getVirtualAllocator()->allocRaw(size));
}

void HydroponicsSubObject::operator delete(void* ptr)
{
    getVirtualAllocator()->freeRaw((VPtrNum)ptr);
}

#endif


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
