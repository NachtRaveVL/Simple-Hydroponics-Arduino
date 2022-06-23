/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Data Objects
*/

#include "Hydroponics.h"

HydroponicsData *dataFromBinaryStream(Stream *streamIn)
{
    HydroponicsData baseDecode;
    baseDecode.fromBinaryStream(streamIn);

    if (baseDecode.isSystemData()) {
        auto data = new HydroponicsSystemData();
        data->fromBinaryStream(streamIn);
        return data;
    } else if (baseDecode.isCalibrationData()) {
        auto data = new HydroponicsCalibrationData();
        data->fromBinaryStream(streamIn);
        return data;
    } else if (baseDecode.isCropsLibData()) {
        auto data = new HydroponicsCropsLibData();
        data->fromBinaryStream(streamIn);
        return data;
    } else {
        return new HydroponicsData(baseDecode);
    }
}

HydroponicsData *dataFromJSONElement(JsonVariantConst &elementIn)
{
    HydroponicsData baseDecode;
    baseDecode.fromJSONElement(elementIn);

    if (baseDecode.isSystemData()) {
        auto data = new HydroponicsSystemData();
        data->fromJSONElement(elementIn);
        return data;
    } else if (baseDecode.isCalibrationData()) {
        auto data = new HydroponicsCalibrationData();
        data->fromJSONElement(elementIn);
        return data;
    } else if (baseDecode.isCropsLibData()) {
        auto data = new HydroponicsCropsLibData();
        data->fromJSONElement(elementIn);
        return data;
    } else {
        return new HydroponicsData(baseDecode);
    }
}


HydroponicsData::HydroponicsData()
    : _ident{.chars={'\0','\0','\0','\0'}}, _version(-1), _revision(-1), _modified(false)
{
    _size = sizeof(*this);
}

HydroponicsData::HydroponicsData(const char *ident, uint16_t version, uint16_t revision)
    : _ident{.chars={'\0','\0','\0','\0'}}, _version(version), _revision(revision), _modified(false)
{
    _size = sizeof(*this);
    HYDRUINO_SOFT_ASSERT(ident, "Invalid id");
    strncpy(_ident.chars, ident, 4);
}

HydroponicsData::HydroponicsData(int16_t idType, int16_t classType, uint16_t version, uint16_t revision)
    : _ident{.object={idType,classType}}, _version(version), _revision(revision), _modified(false)
{
    _size = sizeof(*this);
}

void HydroponicsData::toBinaryStream(Print *streamOut) const
{
    // TODO: Endianness handling?
    streamOut->write((const uint8_t *)((uintptr_t)this + (uintptr_t)sizeof(void*)), _size - sizeof(void*)); // Plus void* for vptr
}

void HydroponicsData::fromBinaryStream(Stream *streamIn)
{
    streamIn->readBytes((uint8_t *)((uintptr_t)this + (uintptr_t)sizeof(void*)), _size - sizeof(void*)); // Plus void* for vptr
    // TODO: Endianness handling?
}

void HydroponicsData::toJSONElement(JsonVariant &elementOut) const
{
    if (_ident.chars[0] > '\0' && _ident.chars[1] > '\0' && _ident.chars[2] > '\0' && _ident.chars[3] > '\0') {
        elementOut[F("_ident")] = stringFromChars(_ident.chars, 4);
    } else {
        auto object = elementOut.createNestedObject(F("_ident"));
        object[F("type")] = _ident.object.idType;
        object[F("class")] = _ident.object.classType;
    }
    elementOut[F("_version")] = _version;
    elementOut[F("_revision")] = _revision;
}

void HydroponicsData::fromJSONElement(JsonVariantConst &elementIn)
{
    auto identObj = elementIn[F("_ident")];
    const char *identStr = identObj.as<const char*>();
    if (strlen(identStr) == 4) {
        strncpy(_ident.chars, identStr, 4);
    } else {
        _ident.object.idType = identObj[F("type")];
        _ident.object.classType = identObj[F("class")];
    }
    _version = elementIn[F("_version")];
    _revision = elementIn[F("_revision")];
}


HydroponicsSystemData::HydroponicsSystemData()
    : HydroponicsData("HSYS", 1),
      systemMode(Hydroponics_SystemMode_Undefined), measureMode(Hydroponics_MeasurementMode_Undefined),
      dispOutMode(Hydroponics_DisplayOutputMode_Undefined), ctrlInMode(Hydroponics_ControlInputMode_Undefined),
      timeZoneOffset(0),
      pollingIntMs(0)
{
    _size = sizeof(*this);
    auto defaultSysName = String(F("Hydruino"));
    strncpy(systemName, defaultSysName.c_str(), HYDRUINO_NAME_MAXSIZE);
}

void HydroponicsSystemData::toBinaryStream(Print *streamOut) const
{
    // TODO: Endianness handling?
    HydroponicsData::toBinaryStream(streamOut);
}

void HydroponicsSystemData::fromBinaryStream(Stream *streamIn)
{
    HydroponicsData::fromBinaryStream(streamIn);
    // TODO: Endianness handling?
}

void HydroponicsSystemData::toJSONElement(JsonVariant &elementOut) const
{
    HydroponicsData::toJSONElement(elementOut);

    // TODO
}

void HydroponicsSystemData::fromJSONElement(JsonVariantConst &elementIn)
{
    HydroponicsData::fromJSONElement(elementIn);

    // TODO
}


HydroponicsCalibrationData::HydroponicsCalibrationData()
    : HydroponicsData("HCAL", 1),
      sensorId(), calibUnits(Hydroponics_UnitsType_Undefined),
      multiplier(1.0f), offset(0.0f)
{
    _size = sizeof(*this);
}

HydroponicsCalibrationData::HydroponicsCalibrationData(HydroponicsIdentity sensorIdIn, Hydroponics_UnitsType calibUnitsIn)
    : HydroponicsData("HCAL", 1),
      sensorId(sensorIdIn), calibUnits(calibUnitsIn),
      multiplier(1.0f), offset(0.0f)
{
    _size = sizeof(*this);
}

void HydroponicsCalibrationData::toBinaryStream(Print *streamOut) const
{
    // TODO: Endianness handling?
    HydroponicsData::toBinaryStream(streamOut);
}

void HydroponicsCalibrationData::fromBinaryStream(Stream *streamIn)
{
    HydroponicsData::fromBinaryStream(streamIn);
    // TODO: Endianness handling?
}

void HydroponicsCalibrationData::toJSONElement(JsonVariant &elementOut) const
{
    HydroponicsData::toJSONElement(elementOut);

    // TODO
}

void HydroponicsCalibrationData::fromJSONElement(JsonVariantConst &elementIn)
{
    HydroponicsData::fromJSONElement(elementIn);

    // TODO
}

void HydroponicsCalibrationData::setFromTwoPoints(float point1MeasuredAt, float point1CalibratedTo,
                                                  float point2MeasuredAt, float point2CalibratedTo)
{
    float aTerm = point2CalibratedTo - point1CalibratedTo;
    float bTerm = point2MeasuredAt - point1MeasuredAt;
    HYDRUINO_SOFT_ASSERT(!isFPEqual(bTerm, 0.0f), "Invalid parameters");
    if (!isFPEqual(bTerm, 0.0f)) {
        multiplier = aTerm / bTerm;
        offset = ((aTerm * point2MeasuredAt) + (bTerm * point1CalibratedTo)) / bTerm;
    }
}


HydroponicsCropsLibData::HydroponicsCropsLibData()
    : HydroponicsData("HCLD", 1),
      cropType(Hydroponics_CropType_Undefined),
      growWeeksToHarvest(0), weeksBetweenHarvest(0),
      isInvasiveOrViner(false), isLargePlant(false), isPerennial(false),
      isPruningRequired(false), isToxicToPets(false)
{
    _size = sizeof(*this);
    memset(plantName, '\0', sizeof(plantName));
    memset(phaseBeginWeek, 0, sizeof(phaseBeginWeek));
    memset(lightHoursPerDay, 0, sizeof(lightHoursPerDay));
    memset(feedIntervalMins, 0, sizeof(feedIntervalMins));
    memset(phRange, 0, sizeof(phRange));
    memset(ecRange, 0, sizeof(ecRange));
    memset(waterTempRange, 0, sizeof(waterTempRange));
    memset(airTempRange, 0, sizeof(airTempRange));
}

HydroponicsCropsLibData::HydroponicsCropsLibData(const Hydroponics_CropType cropTypeIn)
    : HydroponicsData("HCLD", 1),
      cropType(cropTypeIn),
      growWeeksToHarvest(0), weeksBetweenHarvest(0),
      isInvasiveOrViner(false), isLargePlant(false), isPerennial(false),
      isPruningRequired(false), isToxicToPets(false)
{
    _size = sizeof(*this);
    memset(plantName, '\0', sizeof(plantName));
    memset(phaseBeginWeek, 0, sizeof(phaseBeginWeek));
    memset(lightHoursPerDay, 0, sizeof(lightHoursPerDay));
    memset(feedIntervalMins, 0, sizeof(feedIntervalMins));
    memset(phRange, 0, sizeof(phRange));
    memset(ecRange, 0, sizeof(ecRange));
    memset(waterTempRange, 0, sizeof(waterTempRange));
    memset(airTempRange, 0, sizeof(airTempRange));

    if (HydroponicsCropsLibrary::_libraryBuilt) {
        auto *cropLibData = HydroponicsCropsLibrary::getInstance()->checkoutCropData(cropType);
        if (cropLibData && this != cropLibData) {
            memcpy(this, cropLibData, sizeof(HydroponicsCropsLibData));
        }
        HydroponicsCropsLibrary::getInstance()->returnCropData(cropLibData);
    }
}

void HydroponicsCropsLibData::toBinaryStream(Print *streamOut) const
{
    // TODO: Endianness handling?
    HydroponicsData::toBinaryStream(streamOut);
}

void HydroponicsCropsLibData::fromBinaryStream(Stream *streamIn)
{
    HydroponicsData::fromBinaryStream(streamIn);
    // TODO: Endianness handling?
}

void HydroponicsCropsLibData::toJSONElement(JsonVariant &elementOut) const
{
    HydroponicsData::toJSONElement(elementOut);

    elementOut[F("cropType")] = cropTypeToString(cropType);
    elementOut[F("plantName")] = stringFromChars(plantName, HYDRUINO_NAME_MAXSIZE);

    if (growWeeksToHarvest > 0) {
        elementOut[F("growWeeksToHarvest")] = growWeeksToHarvest;
    }
    if (weeksBetweenHarvest > 0) {
        elementOut[F("weeksBetweenHarvest")] = weeksBetweenHarvest;
    }
    if (lightHoursPerDay[0] > 0) {
        elementOut[F("lightHoursPerDay")] = lightHoursPerDay[0];
    }

    if (phaseBeginWeek[(int)Hydroponics_CropPhase_Count-1] > (int)Hydroponics_CropPhase_Count-1) {
        auto phaseBegArray = elementOut.createNestedArray(F("phaseBeginWeek"));
        for (int phaseIndex = 0; phaseIndex < (int)Hydroponics_CropPhase_Count; ++phaseIndex) {
            phaseBegArray.add(phaseBeginWeek[phaseIndex]);
        }
    }

    if (feedIntervalMins[0][0] > 0 || feedIntervalMins[0][1] > 0) {
        if (!isFPEqual(feedIntervalMins[0][0], feedIntervalMins[0][1])) {
            auto feedIntrvlObj = elementOut.createNestedObject(F("feedIntervalMins"));
            feedIntrvlObj[F("on")] = feedIntervalMins[0][0];
            feedIntrvlObj[F("off")] = feedIntervalMins[0][1];
        } else {
            elementOut[F("feedIntervalMins")] = feedIntervalMins[0][0];
        }
    }

    if (phRange[0][0] > 0 || phRange[0][1] > 0) {
        if (!isFPEqual(phRange[0][0], phRange[0][1])) {
            auto phRangeObj = elementOut.createNestedObject(F("phRange"));
            phRangeObj[F("min")] = phRange[0][0];
            phRangeObj[F("max")] = phRange[0][1];
        } else {
            elementOut[F("phRange")] = phRange[0][0];
        }
    }

    if (ecRange[0][0] > 0 || ecRange[0][1] > 0) {
        if (!isFPEqual(ecRange[0][0], ecRange[0][1])) {
            auto ecRangeObj = elementOut.createNestedObject(F("ecRange"));
            ecRangeObj[F("min")] = ecRange[0][0];
            ecRangeObj[F("max")] = ecRange[0][1];
        } else {
            elementOut[F("ecRange")] = ecRange[0][0];
        }
    }

    if (waterTempRange[0][0] > 0 || waterTempRange[0][1] > 0) {
        if (!isFPEqual(waterTempRange[0][0], waterTempRange[0][1])) {
            auto waterTempRangeObj = elementOut.createNestedObject(F("waterTempRange"));
            waterTempRangeObj[F("min")] = waterTempRange[0][0];
            waterTempRangeObj[F("max")] = waterTempRange[0][1];
        } else {
            elementOut[F("waterTempRange")] = waterTempRange[0][0];
        }
    }

    if (airTempRange[0][0] > 0 || airTempRange[0][1] > 0) {
        if (!isFPEqual(airTempRange[0][0], airTempRange[0][1])) {
            auto airTempRangeObj = elementOut.createNestedObject(F("airTempRange"));
            airTempRangeObj[F("min")] = airTempRange[0][0];
            airTempRangeObj[F("max")] = airTempRange[0][1];
        } else {
            elementOut[F("airTempRange")] = airTempRange[0][0];
        }
    }

    if (isInvasiveOrViner || isLargePlant || isPerennial || isPruningRequired || isToxicToPets) {
        auto flagsArray = elementOut.createNestedArray(F("flags"));
        if (isInvasiveOrViner) { flagsArray.add(F("invasive")); }
        if (isLargePlant) { flagsArray.add(F("large")); }
        if (isPerennial) { flagsArray.add(F("perennial")); }
        if (isPruningRequired) { flagsArray.add(F("pruning")); }
        if (isToxicToPets) { flagsArray.add(F("toxic")); }
    }
}

void HydroponicsCropsLibData::fromJSONElement(JsonVariantConst &elementIn)
{
    HydroponicsData::fromJSONElement(elementIn);

    cropType = cropTypeFromString(elementIn[F("cropType")]);
    String plantNameStr = elementIn[F("plantName")];
    strncpy(plantName, plantNameStr.c_str(), HYDRUINO_NAME_MAXSIZE);

    // TODO
}
