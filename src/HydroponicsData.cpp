/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Data
*/

#include "Hydroponics.h"

size_t serializeDataToBinaryStream(HydroponicsData *data, Stream *streamOut, size_t skipBytes)
{
    return streamOut->write((const byte *)((intptr_t)data + skipBytes), data->_size - skipBytes);
}

size_t deserializeDataFromBinaryStream(HydroponicsData *data, Stream *streamIn, size_t skipBytes)
{
    return streamIn->readBytes((byte *)((intptr_t)data + skipBytes), data->_size - skipBytes);
}

HydroponicsData *newDataFromBinaryStream(Stream *streamIn)
{
    HydroponicsData baseDecode;
    size_t readBytes = deserializeDataFromBinaryStream(&baseDecode, streamIn, sizeof(void*));
    HYDRUINO_SOFT_ASSERT(readBytes == baseDecode._size - sizeof(void*), F("Failure importing data, unexpected read length"));

    if (readBytes) {
        HydroponicsData *data = _allocateDataFromBaseDecode(baseDecode);
        HYDRUINO_SOFT_ASSERT(data, F("Failure allocating data"));

        if (data) {
            readBytes += deserializeDataFromBinaryStream(data, streamIn, readBytes + sizeof(void*));
            HYDRUINO_SOFT_ASSERT(readBytes == data->_size - sizeof(void*), F("Failure importing data, unexpected read length"));

            return data;
        }
    }

    return nullptr;
}

HydroponicsData *newDataFromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsData baseDecode;
    baseDecode.fromJSONObject(objectIn);

    HydroponicsData *data = _allocateDataFromBaseDecode(baseDecode);
    HYDRUINO_SOFT_ASSERT(data, F("Failure allocating data"));

    if (data) {
        data->fromJSONObject(objectIn);
        return data;
    }

    return nullptr;
}


HydroponicsData::HydroponicsData()
    : id{.chars={'\0','\0','\0','\0'}}, _version(1), _revision(1), _modified(false)
{
    _size = sizeof(*this);
}

HydroponicsData::HydroponicsData(const char *idIn, uint8_t version, uint8_t revision)
    : id{.chars={'\0','\0','\0','\0'}}, _version(version), _revision(revision), _modified(false)
{
    _size = sizeof(*this);
    HYDRUINO_SOFT_ASSERT(idIn, F("Invalid id"));
    strncpy(id.chars, idIn, sizeof(id.chars));
}

HydroponicsData::HydroponicsData(int8_t idType, int8_t objType, int8_t posIndex, int8_t classType, uint8_t version, uint8_t revision)
    : id{.object={idType,objType,posIndex,classType}}, _version(version), _revision(revision), _modified(false)
{
    _size = sizeof(*this);
}

HydroponicsData::HydroponicsData(const HydroponicsIdentity &id)
    : HydroponicsData(id.type, (int8_t)id.objTypeAs.actuatorType, id.posIndex, -1, 1, 1)
{
    _size = sizeof(*this);
}

void HydroponicsData::toJSONObject(JsonObject &objectOut) const
{
    if (this->isStandardData()) {
        objectOut[F("type")] = stringFromChars(id.chars, sizeof(id.chars));
    } else {
        int8_t typeVals[4] = {id.object.idType, id.object.objType, id.object.posIndex, id.object.classType};
        objectOut[F("type")] = commaStringFromArray(typeVals, 4);
    }
    if (_version > 1) { objectOut[F("_ver")] = _version; }
    if (_revision > 1) { objectOut[F("_rev")] = _revision; }
}

void HydroponicsData::fromJSONObject(JsonObjectConst &objectIn)
{
    JsonVariantConst idVar = objectIn[F("type")];
    const char *idStr = idVar.as<const char *>();
    if (idStr && idStr[0] == 'H') {
        strncpy(id.chars, idStr, sizeof(id.chars));
    } else if (idStr) {
        int8_t typeVals[4];
        commaStringToArray(idStr, typeVals, 4);
        id.object.idType = typeVals[0];
        id.object.objType = typeVals[1];
        id.object.posIndex = typeVals[2];
        id.object.classType = typeVals[3];
    }
    _version = objectIn[F("_ver")] | _version;
    _revision = objectIn[F("_rev")] | _revision;
}


HydroponicsSubData::HydroponicsSubData()
    : type(-1)
{ ; }

void HydroponicsSubData::toJSONObject(JsonObject &objectOut) const
{
    if (type != -1) { objectOut[F("type")] = type; }
}

void HydroponicsSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    type = objectIn[F("type")] | type;
}
