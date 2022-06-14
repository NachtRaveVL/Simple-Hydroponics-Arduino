/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Data Objects
*/

#include "Hydroponics.h"

HydroponicsData::HydroponicsData(const char *ident, uint16_t version, uint16_t revision)
    : _ident{'\0'}, _version(version), _revision(revision)
{
    //assert(ident && "Invalid identity");
    strncpy(_ident, ident, 4);
}

void HydroponicsData::toBinaryStream(Print *streamOut) const
{
    // TODO: Endianness handling.
    streamOut->write(_ident, sizeof(_ident));
    streamOut->write((byte *)&_version, sizeof(_version));
    streamOut->write((byte *)&_revision, sizeof(_revision));
}

void HydroponicsData::fromBinaryStream(Stream *streamIn)
{
    // TODO: Endianness handling.
    streamIn->readBytes(_ident, sizeof(_ident));
    streamIn->readBytes((byte *)&_version, sizeof(_version));
    streamIn->readBytes((byte *)&_revision, sizeof(_revision));
}

void HydroponicsData::toJSONDocument(JsonDocument *docOut) const
{
    (*docOut)[F("_ident")] = _ident;
    (*docOut)[F("_version")] = _version;
    (*docOut)[F("_revision")] = _revision;
}

void HydroponicsData::fromJSONDocument(JsonDocument *docIn)
{
    String identStr = (*docIn)[F("_ident")];
    strncpy(_ident, identStr.c_str(), 4);
    _version = (*docIn)[F("_version")];
    _revision = (*docIn)[F("_revision")];
}


HydroponicsSystemData::HydroponicsSystemData()
    : HydroponicsData("HSYS", 1),
      systemMode(Hydroponics_SystemMode_Undefined), measureMode(Hydroponics_MeasurementMode_Undefined),
      dispOutMode(Hydroponics_DisplayOutputMode_Undefined), ctrlInMode(Hydroponics_ControlInputMode_Undefined),
      systemName{'\0'}, timeZoneOffset(0),
      pollingIntMs(0), maxActiveRelayCount{2},
      reservoirVol{0}, pumpFlowRate{0},
      reservoirVolUnits(Hydroponics_UnitsType_Undefined),
      pumpFlowRateUnits(Hydroponics_UnitsType_Undefined)
{
    auto defaultSysName = String(F("Hydruino"));
    strncpy(systemName, defaultSysName.c_str(), HYDRUINO_NAME_MAXSIZE);
}

void HydroponicsSystemData::toBinaryStream(Print *streamOut) const
{
    HydroponicsData::toBinaryStream(streamOut);

    // TODO
}

void HydroponicsSystemData::fromBinaryStream(Stream *streamIn)
{
    HydroponicsData::fromBinaryStream(streamIn);

    // TODO
}

void HydroponicsSystemData::toJSONDocument(JsonDocument *docOut) const
{
    HydroponicsData::toJSONDocument(docOut);

    // TODO
}

void HydroponicsSystemData::fromJSONDocument(JsonDocument *docIn)
{
    HydroponicsData::fromJSONDocument(docIn);

    // TODO
}


HydroponicsCalibrationData::HydroponicsCalibrationData()
    : HydroponicsData("HCAL", 1),
      sensorId(), calibUnits(Hydroponics_UnitsType_Undefined),
      multiplier(1.0f), offset(0.0f)
{ ; }

HydroponicsCalibrationData::HydroponicsCalibrationData(HydroponicsIdentity sensorIdIn, Hydroponics_UnitsType calibUnitsIn)
    : HydroponicsData("HCAL", 1),
      sensorId(sensorIdIn), calibUnits(calibUnitsIn),
      multiplier(1.0f), offset(0.0f)
{ ; }

void HydroponicsCalibrationData::toBinaryStream(Print *streamOut) const
{
    HydroponicsData::toBinaryStream(streamOut);

    // TODO
}

void HydroponicsCalibrationData::fromBinaryStream(Stream *streamIn)
{
    HydroponicsData::fromBinaryStream(streamIn);

    // TODO
}

void HydroponicsCalibrationData::toJSONDocument(JsonDocument *docOut) const
{
    HydroponicsData::toJSONDocument(docOut);

    // TODO
}

void HydroponicsCalibrationData::fromJSONDocument(JsonDocument *docIn)
{
    HydroponicsData::fromJSONDocument(docIn);

    // TODO
}

void HydroponicsCalibrationData::setFromTwoPoints(float point1MeasuredAt, float point1CalibratedTo,
                                                  float point2MeasuredAt, float point2CalibratedTo)
{
    float aTerm = point2CalibratedTo - point1CalibratedTo;
    float bTerm = point2MeasuredAt - point1MeasuredAt;
    //assert(!isFPEqual(bTerm, 0.0f) && "Invalid parameters");
    if (!isFPEqual(bTerm, 0.0f)) {
        multiplier = aTerm / bTerm;
        offset = ((aTerm * point2MeasuredAt) + (bTerm * point1CalibratedTo)) / bTerm;
    }
}


HydroponicsCropLibData::HydroponicsCropLibData()
    : HydroponicsData("HCLD", 1),
      cropType(Hydroponics_CropType_Undefined), plantName{'\0'},
      growWeeksToHarvest(0), weeksBetweenHarvest(0),
      phaseBeginWeek{0}, lightHoursPerDay{0},
      isInvasiveOrViner(false), isLargePlant(false), isPerennial(false),
      isPruningRequired(false), isToxicToPets(false)
{
    memset(feedIntervalMins, 0, sizeof(feedIntervalMins));
    memset(phRange, 0, sizeof(phRange));
    memset(ecRange, 0, sizeof(ecRange));
    memset(waterTempRange, 0, sizeof(waterTempRange));
    memset(airTempRange, 0, sizeof(airTempRange));
}

HydroponicsCropLibData::HydroponicsCropLibData(const Hydroponics_CropType cropTypeIn)
    : HydroponicsData("HCLD", 1),
      cropType(cropTypeIn), plantName{'\0'},
      growWeeksToHarvest(0), weeksBetweenHarvest(0),
      phaseBeginWeek{0}, lightHoursPerDay{0},
      isInvasiveOrViner(false), isLargePlant(false), isPerennial(false),
      isPruningRequired(false), isToxicToPets(false)
{
    memset(feedIntervalMins, 0, sizeof(feedIntervalMins));
    memset(phRange, 0, sizeof(phRange));
    memset(ecRange, 0, sizeof(ecRange));
    memset(waterTempRange, 0, sizeof(waterTempRange));
    memset(airTempRange, 0, sizeof(airTempRange));

    if (HydroponicsCropsLibrary::_libraryBuilt) {
        auto *cropLibData = HydroponicsCropsLibrary::getInstance()->checkoutCropData(cropType);
        if (cropLibData && this != cropLibData) {
            memcpy(this, cropLibData, sizeof(HydroponicsCropLibData));
        }
        HydroponicsCropsLibrary::getInstance()->returnCropData(cropLibData);
    }
}

void HydroponicsCropLibData::toBinaryStream(Print *streamOut) const
{
    HydroponicsData::toBinaryStream(streamOut);

    // TODO
}

void HydroponicsCropLibData::fromBinaryStream(Stream *streamIn)
{
    HydroponicsData::fromBinaryStream(streamIn);

    // TODO
}

void HydroponicsCropLibData::toJSONDocument(JsonDocument *docOut) const
{
    HydroponicsData::toJSONDocument(docOut);

    (*docOut)[F("cropType")] = cropTypeToString(cropType);
    (*docOut)[F("plantName")] = plantName;

    if (growWeeksToHarvest > 0) {
        (*docOut)[F("growWeeksToHarvest")] = growWeeksToHarvest;
    }
    if (weeksBetweenHarvest > 0) {
        (*docOut)[F("weeksBetweenHarvest")] = weeksBetweenHarvest;
    }
    if (lightHoursPerDay[0] > 0) {
        (*docOut)[F("lightHoursPerDay")] = lightHoursPerDay[0];
    }

    if (phaseBeginWeek[(int)Hydroponics_CropPhase_Count-1] > (int)Hydroponics_CropPhase_Count-1) {
        auto phaseBegArray = docOut->createNestedArray(F("phaseBeginWeek"));
        for (int phaseIndex = 0; phaseIndex < (int)Hydroponics_CropPhase_Count; ++phaseIndex) {
            phaseBegArray.add(phaseBeginWeek[phaseIndex]);
        }
    }

    if (feedIntervalMins[0][0] > 0 || feedIntervalMins[0][1] > 0) {
        if (!isFPEqual(feedIntervalMins[0][0], feedIntervalMins[0][1])) {
            auto feedIntrvlObj = docOut->createNestedObject(F("feedIntervalMins"));
            feedIntrvlObj[F("on")] = feedIntervalMins[0][0];
            feedIntrvlObj[F("off")] = feedIntervalMins[0][1];
        } else {
            (*docOut)[F("feedIntervalMins")] = feedIntervalMins[0][0];
        }
    }

    if (phRange[0][0] > 0 || phRange[0][1] > 0) {
        if (!isFPEqual(phRange[0][0], phRange[0][1])) {
            auto phRangeObj = docOut->createNestedObject(F("phRange"));
            phRangeObj[F("min")] = phRange[0][0];
            phRangeObj[F("max")] = phRange[0][1];
        } else {
            (*docOut)[F("phRange")] = phRange[0][0];
        }
    }

    if (ecRange[0][0] > 0 || ecRange[0][1] > 0) {
        if (!isFPEqual(ecRange[0][0], ecRange[0][1])) {
            auto ecRangeObj = docOut->createNestedObject(F("ecRange"));
            ecRangeObj[F("min")] = ecRange[0][0];
            ecRangeObj[F("max")] = ecRange[0][1];
        } else {
            (*docOut)[F("ecRange")] = ecRange[0][0];
        }
    }

    if (waterTempRange[0][0] > 0 || waterTempRange[0][1] > 0) {
        if (!isFPEqual(waterTempRange[0][0], waterTempRange[0][1])) {
            auto waterTempRangeObj = docOut->createNestedObject(F("waterTempRange"));
            waterTempRangeObj[F("min")] = waterTempRange[0][0];
            waterTempRangeObj[F("max")] = waterTempRange[0][1];
        } else {
            (*docOut)[F("waterTempRange")] = waterTempRange[0][0];
        }
    }

    if (airTempRange[0][0] > 0 || airTempRange[0][1] > 0) {
        if (!isFPEqual(airTempRange[0][0], airTempRange[0][1])) {
            auto airTempRangeObj = docOut->createNestedObject(F("airTempRange"));
            airTempRangeObj[F("min")] = airTempRange[0][0];
            airTempRangeObj[F("max")] = airTempRange[0][1];
        } else {
            (*docOut)[F("airTempRange")] = airTempRange[0][0];
        }
    }

    if (isInvasiveOrViner || isLargePlant || isPerennial || isPruningRequired || isToxicToPets) {
        auto flagsArray = docOut->createNestedArray(F("flags"));
        if (isInvasiveOrViner) { flagsArray.add(F("invasive")); }
        if (isLargePlant) { flagsArray.add(F("large")); }
        if (isPerennial) { flagsArray.add(F("perennial")); }
        if (isPruningRequired) { flagsArray.add(F("prunning")); }
        if (isToxicToPets) { flagsArray.add(F("toxic")); }
    }
}

void HydroponicsCropLibData::fromJSONDocument(JsonDocument *docIn)
{
    HydroponicsData::fromJSONDocument(docIn);

    cropType = cropTypeFromString((*docIn)[F("cropType")]);
    String plantNameStr = (*docIn)[F("plantName")];
    strncpy(plantName, plantNameStr.c_str(), HYDRUINO_NAME_MAXSIZE);

    // TODO
}


HydroponicsSensorMeasurement::HydroponicsSensorMeasurement()
    : timestamp(-1)
{ ; }

HydroponicsSensorMeasurement::HydroponicsSensorMeasurement(time_t timestampIn)
    : timestamp(timestampIn)
{ ; }

HydroponicsBinarySensorMeasurement::HydroponicsBinarySensorMeasurement()
    : HydroponicsSensorMeasurement(), state(false)
{ ; }

HydroponicsBinarySensorMeasurement::HydroponicsBinarySensorMeasurement(bool stateIn, time_t timestamp)
    : HydroponicsSensorMeasurement(timestamp), state(stateIn)
{ ; }

HydroponicsAnalogSensorMeasurement::HydroponicsAnalogSensorMeasurement()
    : HydroponicsSensorMeasurement(), value(0.0f), units(Hydroponics_UnitsType_Undefined)
{ ; }

HydroponicsAnalogSensorMeasurement::HydroponicsAnalogSensorMeasurement(float valueIn, Hydroponics_UnitsType unitsIn, time_t timestamp)
    : HydroponicsSensorMeasurement(timestamp), value(valueIn), units(unitsIn)
{ ; }

HydroponicsDHTOneWireSensorMeasurement::HydroponicsDHTOneWireSensorMeasurement()
    : HydroponicsSensorMeasurement(),
      temperature(0.0f), temperatureUnits(Hydroponics_UnitsType_Undefined),
      humidity(0.0f), humidityUnits(Hydroponics_UnitsType_Undefined),
      heatIndex(0.0f), heatIndexUnits(Hydroponics_UnitsType_Undefined)
{ ; }

HydroponicsDHTOneWireSensorMeasurement::HydroponicsDHTOneWireSensorMeasurement(float temperatureIn, Hydroponics_UnitsType temperatureUnitsIn,
                                                                               float humidityIn, Hydroponics_UnitsType humidityUnitsIn,
                                                                               float heatIndexIn, Hydroponics_UnitsType heatIndexUnitsIn,
                                                                               time_t timestamp)
    : HydroponicsSensorMeasurement(timestamp),
      temperature(temperatureIn), temperatureUnits(temperatureUnitsIn),
      humidity(humidityIn), humidityUnits(humidityUnitsIn),
      heatIndex(heatIndexIn), heatIndexUnits(heatIndexUnitsIn)
{ ; }
