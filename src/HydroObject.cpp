/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Object
*/

#include "Hydruino.h"

HydroObject *newObjectFromData(const HydroData *dataIn)
{
    if (dataIn && isValidType(dataIn->id.object.idType)) return nullptr;
    HYDRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.idType) {
            case (hid_t)HydroIdentity::Actuator:
                return newActuatorObjectFromData((HydroActuatorData *)dataIn);
            case (hid_t)HydroIdentity::Sensor:
                return newSensorObjectFromData((HydroSensorData *)dataIn);
            case (hid_t)HydroIdentity::Crop:
                return newCropObjectFromData((HydroCropData *)dataIn);
            case (hid_t)HydroIdentity::Reservoir:
                return newReservoirObjectFromData((HydroReservoirData *)dataIn);
            case (hid_t)HydroIdentity::Rail:
                return newRailObjectFromData((HydroRailData *)dataIn);
            default: // Unable
                return nullptr;
        }
    }

    return nullptr;
}


hkey_t HydroIdentity::regenKey()
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

            hposi_t linksIndex = 0;
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
        hposi_t linksIndex = 0;
        for (; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            if (_links[linksIndex].first == obj) {
                _links[linksIndex].second++;
                return true;
            }
        }
        if (linksIndex >= _linksSize) { allocateLinkages(_linksSize << 1); } // grow *2 if too small
        if (linksIndex < _linksSize) {
            _links[linksIndex] = make_pair(obj, (int8_t)1);
            return true;
        }
    }
    return false;
}

bool HydroObject::removeLinkage(HydroObject *obj)
{
    if (_links) {
        for (hposi_t linksIndex = 0; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            if (_links[linksIndex].first == obj) {
                if (--_links[linksIndex].second <= 0) {
                    for (int linksSubIndex = linksIndex; linksSubIndex < _linksSize - 1; ++linksSubIndex) {
                        _links[linksSubIndex] = _links[linksSubIndex + 1];
                    }
                    _links[_linksSize - 1] = make_pair((HydroObject *)nullptr, (int8_t)0);
                }
                return true;
            }
        }
    }
    return false;
}

bool HydroObject::hasLinkage(HydroObject *obj) const
{
    if (_links) {
        for (hposi_t linksIndex = 0; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            if (_links[linksIndex].first == obj) {
                return true;
            }
        }
    }
    return false;
}

void HydroObject::unresolveAny(HydroObject *obj)
{
    if (this == obj && _links) {
        HydroObject *lastObject = nullptr;
        for (hposi_t linksIndex = 0; linksIndex < _linksSize && _links[linksIndex].first; ++linksIndex) {
            HydroObject *object = _links[linksIndex].first;
            if (object != obj) {
                object->unresolveAny(obj); // may clobber indexing

                if (linksIndex && _links[linksIndex].first != object) {
                    while (linksIndex && _links[linksIndex].first != lastObject) { --linksIndex; }
                    object = lastObject;
                }
            }
            lastObject = object;
        }
    }
}

HydroIdentity HydroObject::getId() const
{
    return _id;
}

hkey_t HydroObject::getKey() const
{
    return _id.key;
}

String HydroObject::getKeyString() const
{
    return _id.keyString;
}

SharedPtr<HydroObjInterface> HydroObject::getSharedPtr() const
{
    return getController() ? static_pointer_cast<HydroObjInterface>(getController()->objectById(_id))
                           : SharedPtr<HydroObjInterface>((HydroObjInterface *)this);
}

bool HydroObject::isObject() const
{
    return true;
}

HydroData *HydroObject::allocateData() const
{
    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_UnsupportedOperation));
    return new HydroData();
}

void HydroObject::saveToData(HydroData *dataOut)
{
    dataOut->id.object.idType = (hid_t)_id.type;
    dataOut->id.object.objType = _id.objTypeAs.idType;
    dataOut->id.object.posIndex = _id.posIndex;
    if (_id.keyString.length()) {
        strncpy(((HydroObjectData *)dataOut)->name, _id.keyString.c_str(), HYDRO_NAME_MAXSIZE);
    }
}


void HydroSubObject::unresolveAny(HydroObject *obj)
{ ; }

HydroIdentity HydroSubObject::getId() const
{
    return HydroIdentity(getKey());
}

hkey_t HydroSubObject::getKey() const
{
    return (hkey_t)(intptr_t)this;
}

String HydroSubObject::getKeyString() const
{
    return addressToString((uintptr_t)this);
}

SharedPtr<HydroObjInterface> HydroSubObject::getSharedPtr() const
{
    return SharedPtr<HydroObjInterface>((HydroObjInterface *)this);
}

bool HydroSubObject::isObject() const
{
    return false;
}

void HydroSubObject::setParent(HydroObjInterface *parent)
{
    _parent = parent;
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
