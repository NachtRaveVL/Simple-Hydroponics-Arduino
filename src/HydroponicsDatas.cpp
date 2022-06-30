/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Data Objects
*/

#include "Hydroponics.h"

HydroponicsData *_allocateDataFromBaseDecode(const HydroponicsData &baseDecode)
{
    HydroponicsData *retVal = nullptr;

    if (baseDecode.isStdData()) {
        if (baseDecode.isSystemData()) {
            retVal = new HydroponicsSystemData();
        } else if (baseDecode.isCalibrationData()) {
            retVal = new HydroponicsCalibrationData();
        } else if (baseDecode.isCropsLibData()) {
            retVal = new HydroponicsCropsLibData();
        }
    } else if (baseDecode.isObjData()) {
        retVal = _allocateDataForObjType(baseDecode.id.object.idType, baseDecode.id.object.classType);
    }

    HYDRUINO_SOFT_ASSERT(retVal, F("Unknown data decode"));
    if (retVal) {
        retVal->id = baseDecode.id;
        HYDRUINO_SOFT_ASSERT(retVal->_version == baseDecode._version, F("Data version mismatch")); // TODO: Version updaters
        retVal->_revision = baseDecode._revision;
        return retVal;
    }
    return new HydroponicsData(baseDecode);
}

HydroponicsData *_allocateDataForObjType(int8_t idType, int8_t classType)
{
    switch (idType) {
        case 0: // Actuator
            switch(classType) {
                case 0:
                    return new HydroponicsRelayActuatorData();
                case 1:
                    return new HydroponicsPumpRelayActuatorData();
                case 2:
                    return new HydroponicsPWMActuatorData();
                default: break;
            }
            break;

        case 1: // Sensor
            switch(classType) {
                case 0:
                    return new HydroponicsBinarySensorData();
                case 1:
                    return new HydroponicsAnalogSensorData();
                case 3:
                    return new HydroponicsDHTTempHumiditySensorData();
                case 4:
                    return new HydroponicsDSTemperatureSensorData();
                case 5:
                    return new HydroponicsTMPSoilMoistureSensorData();
                default: break;
            }
            break;

        case 2: // Crop
            switch(classType) {
                case 0:
                    return new HydroponicsTimedCropData();
                    break;
                case 1:
                    return new HydroponicsAdaptiveCropData();
                default: break;
            }
            break;

        case 3: // Reservoir
            switch(classType) {
                case 0:
                    return new HydroponicsFluidReservoirData();
                case 1:
                    return new HydroponicsInfiniteReservoirData();
                default: break;
            }
            break;

        case 4: // Rail
            switch(classType) {
                case 0:
                    return new HydroponicsSimpleRailData();
                case 1:
                    return new HydroponicsRegulatedRailData();
                default: break;
            }
            break;

        default: break;
    }

    return nullptr;
}

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
    HYDRUINO_SOFT_ASSERT(readBytes == baseDecode._size - sizeof(void*), F("Failure reading data, unexpected read length"));

    if (readBytes) {
        HydroponicsData *data = _allocateDataFromBaseDecode(baseDecode);
        HYDRUINO_SOFT_ASSERT(data, F("Failure allocating data"));

        if (data) {
            readBytes += deserializeDataFromBinaryStream(data, streamIn, readBytes + sizeof(void*));
            HYDRUINO_SOFT_ASSERT(readBytes == data->_size - sizeof(void*), F("Failure reading data, unexpected read length"));

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
    if (this->isStdData()) {
        objectOut[F("id")] = stringFromChars(id.chars, sizeof(id.chars));
    } else {
        JsonObject object = objectOut.createNestedObject(F("id"));
        object[F("type")] = id.object.idType;
        object[F("obj")] = id.object.objType;
        object[F("pos")] = id.object.posIndex;
        object[F("cls")] = id.object.classType;
    }
    if (_version > 1) { objectOut[F("_ver")] = _version; }
    if (_revision > 1) { objectOut[F("_rev")] = _revision; }
}

void HydroponicsData::fromJSONObject(JsonObjectConst &objectIn)
{
    JsonVariantConst idVar = objectIn[F("id")];
    if (idVar.is<const char *>()) {
        strncpy(id.chars, idVar.as<const char *>(), sizeof(id.chars));
    } else {
        id.object.idType = idVar[F("type")];
        id.object.objType = idVar[F("obj")];
        id.object.posIndex = idVar[F("pos")];
        id.object.classType = idVar[F("cls")];
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


HydroponicsSystemData::HydroponicsSystemData()
    : HydroponicsData("HSYS", 1),
      systemMode(Hydroponics_SystemMode_Undefined), measureMode(Hydroponics_MeasurementMode_Undefined),
      dispOutMode(Hydroponics_DisplayOutputMode_Undefined), ctrlInMode(Hydroponics_ControlInputMode_Undefined),
      ctrlInputPinMap{0}, timeZoneOffset(0),
      pollingInterval(HYDRUINO_DATA_LOOP_INTERVAL)
{
    _size = sizeof(*this);
    String defaultSysNameStr(F("Hydruino"));
    strncpy(systemName, defaultSysNameStr.c_str(), HYDRUINO_NAME_MAXSIZE);
}

void HydroponicsSystemData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsData::toJSONObject(objectOut);

    objectOut[F("systemMode")] = systemMode;
    objectOut[F("measureMode")] = measureMode;
    objectOut[F("dispOutMode")] = dispOutMode;
    objectOut[F("ctrlInMode")] = ctrlInMode;
    if (systemName[0]) { objectOut[F("systemName")] = stringFromChars(systemName, HYDRUINO_NAME_MAXSIZE); }
    if (timeZoneOffset != 0) { objectOut[F("timeZoneOffset")] = timeZoneOffset; }
    if (pollingInterval != HYDRUINO_DATA_LOOP_INTERVAL) { objectOut[F("pollingInterval")] = pollingInterval; }
    if (lastWaterChangeTime > DateTime((uint32_t)0).unixtime()) { objectOut[F("lastWaterChangeTime")] = lastWaterChangeTime; }
}

void HydroponicsSystemData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsData::fromJSONObject(objectIn);

    systemMode = objectIn[F("systemMode")] | systemMode;
    measureMode = objectIn[F("measureMode")] | measureMode;
    dispOutMode = objectIn[F("dispOutMode")] | dispOutMode;
    ctrlInMode = objectIn[F("ctrlInMode")] | ctrlInMode;
    const char *systemNameStr = objectIn[F("systemName")];
    if (systemNameStr && systemNameStr[0]) { strncpy(systemName, systemNameStr, HYDRUINO_NAME_MAXSIZE); }
    timeZoneOffset = objectIn[F("timeZoneOffset")] | timeZoneOffset;
    pollingInterval = objectIn[F("pollingInterval")] | pollingInterval;
    lastWaterChangeTime = objectIn[F("lastWaterChangeTime")] | lastWaterChangeTime;
}


HydroponicsCalibrationData::HydroponicsCalibrationData()
    : HydroponicsData("HCAL", 1),
      sensorName{0}, calibUnits(Hydroponics_UnitsType_Undefined),
      multiplier(1.0f), offset(0.0f)
{
    _size = sizeof(*this);
}

HydroponicsCalibrationData::HydroponicsCalibrationData(HydroponicsIdentity sensorId, Hydroponics_UnitsType calibUnitsIn)
    : HydroponicsData("HCAL", 1),
      sensorName{0}, calibUnits(calibUnitsIn),
      multiplier(1.0f), offset(0.0f)
{
    _size = sizeof(*this);
    strncpy(sensorName, sensorId.keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
}

void HydroponicsCalibrationData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsData::toJSONObject(objectOut);

    if (sensorName[0]) { objectOut[F("sensorName")] = stringFromChars(sensorName, HYDRUINO_NAME_MAXSIZE); }
    if (calibUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("calibUnits")] = calibUnits; }
    objectOut[F("multiplier")] = multiplier;
    objectOut[F("offset")] = offset;
}

void HydroponicsCalibrationData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsData::fromJSONObject(objectIn);

    const char *sensorNameStr = objectIn[F("sensorName")];
    if (sensorNameStr && sensorNameStr[0]) { strncpy(sensorName, sensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    calibUnits = objectIn[F("calibUnits")] | calibUnits;
    multiplier = objectIn[F("multiplier")] | multiplier;
    offset = objectIn[F("offset")] | offset;
}

void HydroponicsCalibrationData::setFromTwoPoints(float point1MeasuredAt, float point1CalibratedTo,
                                                  float point2MeasuredAt, float point2CalibratedTo)
{
    float aTerm = point2CalibratedTo - point1CalibratedTo;
    float bTerm = point2MeasuredAt - point1MeasuredAt;
    HYDRUINO_SOFT_ASSERT(!isFPEqual(bTerm, 0.0f), F("Invalid parameters"));
    if (!isFPEqual(bTerm, 0.0f)) {
        multiplier = aTerm / bTerm;
        offset = ((aTerm * point2MeasuredAt) + (bTerm * point1CalibratedTo)) / bTerm;
    }
}


HydroponicsCropsLibData::HydroponicsCropsLibData()
    : HydroponicsData("HCLD", 1),
      cropType(Hydroponics_CropType_Undefined), cropName{0},
      totalGrowWeeks(14), lifeCycleWeeks(0),
      dailyLightHours{20,18,12}, phaseDurationWeeks{2,4,8},
      phRange{6,6}, ecRange{1,1}, waterTempRange{25,25}, airTempRange{25,25},
      isInvasiveOrViner(false), isLargePlant(false), isPerennial(false),
      isPruningRequired(false), isToxicToPets(false)
{
    _size = sizeof(*this);
}

HydroponicsCropsLibData::HydroponicsCropsLibData(const Hydroponics_CropType cropTypeIn)
    : HydroponicsData("HCLD", 1),
      cropType(cropTypeIn), cropName{0},
      totalGrowWeeks(0), lifeCycleWeeks(0),
      dailyLightHours{20,18,12}, phaseDurationWeeks{2,4,8},
      phRange{6,6}, ecRange{1,1}, waterTempRange{25,25}, airTempRange{25,25},
      isInvasiveOrViner(false), isLargePlant(false), isPerennial(false),
      isPruningRequired(false), isToxicToPets(false)
{
    _size = sizeof(*this);

    auto cropsLibrary = getCropsLibraryInstance();
    if (cropsLibrary) {
        auto cropsLibData = cropsLibrary->checkoutCropData(cropType);
        if (cropsLibData && this != cropsLibData) {
            *this = *cropsLibData;
        }
        cropsLibrary->returnCropData(cropsLibData);
    }
}

void HydroponicsCropsLibData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsData::toJSONObject(objectOut);

    objectOut[F("cropType")] = cropTypeToString(cropType);
    if (cropName[0]) { objectOut[F("cropName")] = stringFromChars(cropName, HYDRUINO_NAME_MAXSIZE); }

    int mainPhaseTotalWeeks = phaseDurationWeeks[0] + phaseDurationWeeks[1] + phaseDurationWeeks[2];
    HYDRUINO_SOFT_ASSERT(totalGrowWeeks == 0 || mainPhaseTotalWeeks == totalGrowWeeks, F("Total grow weeks mismatch, failure exporting totalGrowWeeks"));
    if (totalGrowWeeks > 0 && totalGrowWeeks != 14) {
        objectOut[F("totalGrowWeeks")] = totalGrowWeeks;
    } else if (!totalGrowWeeks && mainPhaseTotalWeeks > 0 && mainPhaseTotalWeeks != 14) {
        objectOut[F("totalGrowWeeks")] = mainPhaseTotalWeeks;
    }
    if (lifeCycleWeeks > 0) {
        objectOut[F("lifeCycleWeeks")] = lifeCycleWeeks;
    }

    if (dailyLightHours[0] != 20 && dailyLightHours[1] != 18 && dailyLightHours[2] != 12) {
        HYDRUINO_SOFT_ASSERT(Hydroponics_CropPhase_MainCount == 3, F("Main phase count mismatch, failure exporting dailyLightHours"));
        if (dailyLightHours[1] != 0 && dailyLightHours[1] != dailyLightHours[0] &&
            dailyLightHours[2] != 0 && dailyLightHours[2] != dailyLightHours[0]) {
            JsonObject dailyLightHoursObj = objectOut.createNestedObject(F("dailyLightHours"));
            dailyLightHoursObj[F("seed")] = dailyLightHours[0];
            dailyLightHoursObj[F("grow")] = dailyLightHours[1];
            dailyLightHoursObj[F("bloom")] = dailyLightHours[2];
        } else {
            objectOut[F("dailyLightHours")] = dailyLightHours[0];
        }
    }

    if (phaseDurationWeeks[0] != 2 && phaseDurationWeeks[1] != 4 && phaseDurationWeeks[2] != 8) {
        HYDRUINO_SOFT_ASSERT(Hydroponics_CropPhase_MainCount == 3, F("Main phase count mismatch, failure exporting phaseDurationWeeks"));
        JsonObject phaseDurationWeeksObj = objectOut.createNestedObject(F("phaseDurationWeeks"));
        phaseDurationWeeksObj[F("seed")] = phaseDurationWeeks[0];
        phaseDurationWeeksObj[F("grow")] = phaseDurationWeeks[1];
        phaseDurationWeeksObj[F("bloom")] = phaseDurationWeeks[2];
    }

    if (!isFPEqual(phRange[0], 6) && !isFPEqual(phRange[1], 6)) {
        if (!isFPEqual(phRange[0], phRange[1])) {
            JsonObject phRangeObj = objectOut.createNestedObject(F("phRange"));
            phRangeObj[F("min")] = phRange[0];
            phRangeObj[F("max")] = phRange[1];
        } else {
            objectOut[F("phRange")] = phRange[0];
        }
    }

    if (!isFPEqual(ecRange[0], 1) && !isFPEqual(ecRange[1], 1)) {
        if (!isFPEqual(ecRange[0], ecRange[1])) {
            JsonObject ecRangeObj = objectOut.createNestedObject(F("ecRange"));
            ecRangeObj[F("min")] = ecRange[0];
            ecRangeObj[F("max")] = ecRange[1];
        } else {
            objectOut[F("ecRange")] = ecRange[0];
        }
    }

    if (!isFPEqual(waterTempRange[0], 25) && !isFPEqual(waterTempRange[1], 25)) {
        if (!isFPEqual(waterTempRange[0], waterTempRange[1])) {
            JsonObject waterTempRangeObj = objectOut.createNestedObject(F("waterTempRange"));
            waterTempRangeObj[F("min")] = waterTempRange[0];
            waterTempRangeObj[F("max")] = waterTempRange[1];
        } else {
            objectOut[F("waterTempRange")] = waterTempRange[0];
        }
    }

    if (!isFPEqual(airTempRange[0], 25) && !isFPEqual(airTempRange[1], 25)) {
        if (!isFPEqual(airTempRange[0], airTempRange[1])) {
            JsonObject airTempRangeObj = objectOut.createNestedObject(F("airTempRange"));
            airTempRangeObj[F("min")] = airTempRange[0];
            airTempRangeObj[F("max")] = airTempRange[1];
        } else {
            objectOut[F("airTempRange")] = airTempRange[0];
        }
    }

    if (isInvasiveOrViner || isLargePlant || isPerennial || isPruningRequired || isToxicToPets) {
        auto flagsArray = objectOut.createNestedArray(F("flags"));
        if (isInvasiveOrViner) { flagsArray.add(F("invasive")); }
        if (isLargePlant) { flagsArray.add(F("large")); }
        if (isPerennial) { flagsArray.add(F("perennial")); }
        if (isPruningRequired) { flagsArray.add(F("pruning")); }
        if (isToxicToPets) { flagsArray.add(F("toxic")); }
    }
}

void HydroponicsCropsLibData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsData::fromJSONObject(objectIn);

    cropType = cropTypeFromString(objectIn[F("cropType")]);
    const char *cropNameStr = objectIn[F("cropName")];
    if (cropNameStr && cropNameStr[0]) { strncpy(cropName, cropNameStr, HYDRUINO_NAME_MAXSIZE); }

    totalGrowWeeks = objectIn[F("totalGrowWeeks")] | totalGrowWeeks;
    lifeCycleWeeks = objectIn[F("lifeCycleWeeks")] | lifeCycleWeeks;

    {   HYDRUINO_SOFT_ASSERT(Hydroponics_CropPhase_MainCount == 3, F("Main phase count mismatch, failure importing dailyLightHours"));
        JsonVariantConst dailyLightHoursVar = objectIn[F("dailyLightHours")];
        dailyLightHours[0] = dailyLightHoursVar[F("seed")] | dailyLightHoursVar[0] | dailyLightHoursVar | dailyLightHours[0];
        dailyLightHours[1] = dailyLightHoursVar[F("grow")] | dailyLightHoursVar[1] | dailyLightHoursVar | dailyLightHours[1];
        dailyLightHours[2] = dailyLightHoursVar[F("bloom")] | dailyLightHoursVar[2] | dailyLightHoursVar | dailyLightHours[2];
    }

    {   HYDRUINO_SOFT_ASSERT(Hydroponics_CropPhase_MainCount == 3, F("Main phase count mismatch, failure importing phaseDurationWeeks"));
        JsonVariantConst phaseDurationWeeksVar = objectIn[F("phaseDurationWeeks")];
        phaseDurationWeeks[0] = phaseDurationWeeksVar[F("seed")] | phaseDurationWeeksVar[0] | phaseDurationWeeks[0];
        phaseDurationWeeks[1] = phaseDurationWeeksVar[F("grow")] | phaseDurationWeeksVar[1] | phaseDurationWeeks[1];
        phaseDurationWeeks[2] = phaseDurationWeeksVar[F("bloom")] | phaseDurationWeeksVar[2] | phaseDurationWeeks[2];
    }

    {   JsonVariantConst phRangeVar = objectIn[F("phRange")];
        phRange[0] = phRangeVar[F("min")] | phRangeVar[0] | phRangeVar | phRange[0];
        phRange[1] = phRangeVar[F("max")] | phRangeVar[1] | phRangeVar | phRange[1];
    }
    {   JsonVariantConst ecRangeVar = objectIn[F("ecRange")];
        ecRange[0] = ecRangeVar[F("min")] | ecRangeVar[0] | ecRangeVar | ecRange[0];
        ecRange[1] = ecRangeVar[F("max")] | ecRangeVar[1] | ecRangeVar | ecRange[1];
    }
    {   JsonVariantConst waterTempRangeVar = objectIn[F("waterTempRange")];
        waterTempRange[0] = waterTempRangeVar[F("min")] | waterTempRangeVar[0] | waterTempRangeVar | waterTempRange[0];
        waterTempRange[1] = waterTempRangeVar[F("max")] | waterTempRangeVar[1] | waterTempRangeVar | waterTempRange[1];
    }
    {   JsonVariantConst airTempRangeVar = objectIn[F("airTempRange")];
        airTempRange[0] = airTempRangeVar[F("min")] | airTempRangeVar[0] | airTempRangeVar | airTempRange[0];
        airTempRange[1] = airTempRangeVar[F("max")] | airTempRangeVar[1] | airTempRangeVar | airTempRange[1];
    }

    {   JsonArrayConst flagsArray = objectIn[F("flags")];
        for (String flagStr : flagsArray) {
            if (flagStr.equalsIgnoreCase(F("invasive"))) { isInvasiveOrViner = true; }
            if (flagStr.equalsIgnoreCase(F("large"))) { isLargePlant = true; }
            if (flagStr.equalsIgnoreCase(F("perennial"))) { isPerennial = true; }
            if (flagStr.equalsIgnoreCase(F("pruning"))) { isPruningRequired = true; }
            if (flagStr.equalsIgnoreCase(F("toxic"))) { isToxicToPets = true; }
        }
    }
}
