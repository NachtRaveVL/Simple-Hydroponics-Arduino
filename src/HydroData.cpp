/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Data
*/

#include "Hydruino.h"
#include "HydroCoreLogic.h"

size_t serializeDataToBinaryStream(const HydroData *data, Stream *streamOut, size_t skipBytes)
{
    return streamOut->write((const uint8_t *)((intptr_t)data + skipBytes), data->_size - skipBytes);
}

size_t deserializeDataFromBinaryStream(HydroData *data, Stream *streamIn, size_t skipBytes)
{
    return streamIn->readBytes((uint8_t *)((intptr_t)data + skipBytes), data->_size - skipBytes);
}

HydroData *newDataFromBinaryStream(Stream *streamIn)
{
    HydroData baseDecode;
    const size_t baseSize = baseDecode._size;
    size_t readBytes = deserializeDataFromBinaryStream(&baseDecode, streamIn, sizeof(void*));
    const size_t serializedSize = baseDecode._size;
    HYDRO_SOFT_ASSERT(readBytes == baseSize - sizeof(void*) && serializedSize >= baseSize, SFP(HStr_Err_ImportFailure));

    if (readBytes == baseSize - sizeof(void*) && serializedSize >= baseSize) {
        HydroData *data = _allocateDataFromBaseDecode(baseDecode);
        HYDRO_SOFT_ASSERT(data, SFP(HStr_Err_AllocationFailure));

        if (data) {
            auto readPlan = hydroBinaryDataReadPlan(serializedSize, data->_size, baseSize);
            if (readPlan.copyBytes) {
                readBytes += streamIn->readBytes((uint8_t *)data + baseSize, readPlan.copyBytes);
            }

            size_t skippedBytes = 0;
            uint8_t skipBuffer[16];
            while (skippedBytes < readPlan.skipBytes) {
                size_t skipBytes = readPlan.skipBytes - skippedBytes;
                if (skipBytes > sizeof(skipBuffer)) { skipBytes = sizeof(skipBuffer); }
                size_t skipped = streamIn->readBytes(skipBuffer, skipBytes);
                skippedBytes += skipped;
                if (skipped != skipBytes) { break; }
            }

            HYDRO_SOFT_ASSERT(readBytes == baseSize - sizeof(void*) + readPlan.copyBytes &&
                               skippedBytes == readPlan.skipBytes, SFP(HStr_Err_ImportFailure));
            if (readBytes == baseSize - sizeof(void*) + readPlan.copyBytes && skippedBytes == readPlan.skipBytes) {
                data->migrateFromBinaryVersion(baseDecode._version);
                return data;
            }

            delete data;
        }
    }

    return nullptr;
}

HydroData *newDataFromJSONObject(JsonObjectConst &objectIn)
{
    HydroData baseDecode;
    baseDecode.fromJSONObject(objectIn);

    HydroData *data = _allocateDataFromBaseDecode(baseDecode);
    HYDRO_SOFT_ASSERT(data, SFP(HStr_Err_AllocationFailure));

    if (data) {
        data->fromJSONObject(objectIn);
        return data;
    }

    return nullptr;
}


HydroData::HydroData()
    : id{.chars={'\000','\000','\000','\000'}}, _version(1), _revision(-1)
{
    _size = sizeof(*this);
}

HydroData::HydroData(char id0, char id1, char id2, char id3, uint8_t version, uint8_t revision)
    : id{.chars={id0,id1,id2,id3}}, _version(version), _revision((int8_t)revision)
{
    _size = sizeof(*this);
    HYDRO_HARD_ASSERT(isStandardData(), SFP(HStr_Err_InvalidParameter));
}

HydroData::HydroData(hid_t idType, hid_t objType, hposi_t posIndex, hid_t classType, uint8_t version, uint8_t revision)
    : id{.object={idType,objType,posIndex,classType}}, _version(version), _revision((int8_t)revision)
{
    _size = sizeof(*this);
}

HydroData::HydroData(const HydroIdentity &id)
    : HydroData(id.type, id.objTypeAs.idType, id.posIndex, -1, 1, 0)
{
    _size = sizeof(*this);
}

void HydroData::toJSONObject(JsonObject &objectOut) const
{
    if (isStandardData()) {
        objectOut[SFP(HStr_Key_Type)] = charsToString(id.chars, sizeof(id.chars));
    } else {
        int8_t typeVals[4] = {id.object.idType, id.object.objType, id.object.posIndex, id.object.classType};
        objectOut[SFP(HStr_Key_Type)] = commaStringFromArray(typeVals, 4);
    }
    if (_version > 1) { objectOut[SFP(HStr_Key_Version)] = _version; }
    if (getRevision() > 1) { objectOut[SFP(HStr_Key_Revision)] = getRevision(); }
}

void HydroData::fromJSONObject(JsonObjectConst &objectIn)
{
    JsonVariantConst idVar = objectIn[SFP(HStr_Key_Type)];
    const char *idStr = idVar.as<const char *>();
    if (idStr && idStr[0] == 'H') {
        strncpy(id.chars, idStr, 4);
    } else if (idStr) {
        int8_t typeVals[4];
        commaStringToArray(idStr, typeVals, 4);
        id.object.idType = typeVals[0];
        id.object.objType = typeVals[1];
        id.object.posIndex = typeVals[2];
        id.object.classType = typeVals[3];
    }
    _version = objectIn[SFP(HStr_Key_Version)] | _version;
    _revision = objectIn[SFP(HStr_Key_Revision)] | _revision;
    _revision = abs(_revision);
}


HydroSubData::HydroSubData()
    : type(hid_none)
{ ; }

HydroSubData::HydroSubData(hid_t dataType)
    : type(dataType)
{ ; }

void HydroSubData::toJSONObject(JsonObject &objectOut) const
{
    if (type != hid_none) { objectOut[SFP(HStr_Key_Type)] = type; }
}

void HydroSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    type = objectIn[SFP(HStr_Key_Type)] | type;
}
