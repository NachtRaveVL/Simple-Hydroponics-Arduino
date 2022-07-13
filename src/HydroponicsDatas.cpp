/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Datas
*/

#include "Hydroponics.h"

HydroponicsData *_allocateDataFromBaseDecode(const HydroponicsData &baseDecode)
{
    HydroponicsData *retVal = nullptr;

    if (baseDecode.isStandardData()) {
        if (baseDecode.isSystemData()) {
            retVal = new HydroponicsSystemData();
        } else if (baseDecode.isCalibrationData()) {
            retVal = new HydroponicsCalibrationData();
        } else if (baseDecode.isCropsLibData()) {
            retVal = new HydroponicsCropsLibData();
        } else if (baseDecode.isAdditiveData()) {
            retVal = new HydroponicsCustomAdditiveData();
        }
    } else if (baseDecode.isObjectData()) {
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
                case 0: // Relay
                    return new HydroponicsRelayActuatorData();
                case 1: // Pump Relay
                    return new HydroponicsPumpRelayActuatorData();
                case 2: // PWM
                    return new HydroponicsPWMActuatorData();
                default: break;
            }
            break;

        case 1: // Sensor
            switch(classType) {
                case 0: // Binary
                    return new HydroponicsBinarySensorData();
                case 1: // Analog
                    return new HydroponicsAnalogSensorData();
                case 3: // DHT
                    return new HydroponicsDHTTempHumiditySensorData();
                case 4: // DS
                    return new HydroponicsDSTemperatureSensorData();
                case 5: // TMP
                    return new HydroponicsTMPMoistureSensorData();
                default: break;
            }
            break;

        case 2: // Crop
            switch(classType) {
                case 0: // Timed
                    return new HydroponicsTimedCropData();
                case 1: // Adaptive
                    return new HydroponicsAdaptiveCropData();
                default: break;
            }
            break;

        case 3: // Reservoir
            switch(classType) {
                case 0: // Fluid
                    return new HydroponicsFluidReservoirData();
                case 1: // Feed
                    return new HydroponicsFeedReservoirData();
                case 2: // Pipe
                    return new HydroponicsInfiniteReservoirData();
                default: break;
            }
            break;

        case 4: // Rail
            switch(classType) {
                case 0: // Simple
                    return new HydroponicsSimpleRailData();
                case 1: // Regulated
                    return new HydroponicsRegulatedRailData();
                default: break;
            }
            break;

        default: break;
    }

    return nullptr;
}

HydroponicsSystemData::HydroponicsSystemData()
    : HydroponicsData("HSYS", 1),
      systemMode(Hydroponics_SystemMode_Undefined), measureMode(Hydroponics_MeasurementMode_Undefined),
      dispOutMode(Hydroponics_DisplayOutputMode_Undefined), ctrlInMode(Hydroponics_ControlInputMode_Undefined),
      systemName{0}, timeZoneOffset(0), pollingInterval(HYDRUINO_DATA_LOOP_INTERVAL),
      wifiSSID{0}, wifiPassword{0}, wifiPasswordSeed(0)
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
    if (wifiSSID[0]) { objectOut[F("wifiSSID")] = stringFromChars(wifiSSID, HYDRUINO_NAME_MAXSIZE); }
    if (wifiPasswordSeed) {
        objectOut[F("wifiPassword")] = hexStringFromBytes(wifiPassword, HYDRUINO_NAME_MAXSIZE);
        objectOut[F("wifiPasswordSeed")] = wifiPasswordSeed;
    } else if (wifiPassword[0]) {
        objectOut[F("wifiPassword")] = stringFromChars((const char *)wifiPassword, HYDRUINO_NAME_MAXSIZE);
    }
    JsonObject schedulerObj = objectOut.createNestedObject(F("scheduler"));
    scheduler.toJSONObject(schedulerObj); if (!schedulerObj.size()) { objectOut.remove(F("scheduler")); }
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
    const char *wifiSSIDStr = objectIn[F("wifiSSID")];
    if (wifiSSIDStr && wifiSSIDStr[0]) { strncpy(wifiSSID, wifiSSIDStr, HYDRUINO_NAME_MAXSIZE); }
    const char *wifiPasswordStr = objectIn[F("wifiPassword")];
    wifiPasswordSeed = objectIn[F("wifiPasswordSeed")] | wifiPasswordSeed;
    if (wifiPasswordStr && wifiPasswordSeed) { hexStringToBytes(String(wifiPasswordStr), wifiPassword, HYDRUINO_NAME_MAXSIZE); }
    else if (wifiPasswordStr && wifiPasswordStr[0]) { strncpy((char *)wifiPassword, wifiPasswordStr, HYDRUINO_NAME_MAXSIZE); wifiPasswordSeed = 0; }
    JsonObjectConst schedulerObj = objectIn[F("scheduler")];
    if (!schedulerObj.isNull()) { scheduler.fromJSONObject(schedulerObj); }
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
    if (sensorId) {
        strncpy(sensorName, sensorId.keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
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
      phRange{6,6}, tdsRange{1.8,2.4}, nightlyFeedMultiplier(1),
      waterTempRange{25,25}, airTempRange{25,25},
      isInvasiveOrViner(false), isLargePlant(false), isPerennial(false), isToxicToPets(false),
      isPruningNeeded(false), isSprayingNeeded(false)
{
    _size = sizeof(*this);
}

HydroponicsCropsLibData::HydroponicsCropsLibData(const Hydroponics_CropType cropTypeIn)
    : HydroponicsData("HCLD", 1),
      cropType(cropTypeIn), cropName{0},
      totalGrowWeeks(14), lifeCycleWeeks(0),
      dailyLightHours{20,18,12}, phaseDurationWeeks{2,4,8},
      phRange{6,6}, tdsRange{1.8,2.4}, nightlyFeedMultiplier(1),
      waterTempRange{25,25}, airTempRange{25,25},
      isInvasiveOrViner(false), isLargePlant(false), isPerennial(false), isToxicToPets(false),
      isPruningNeeded(false), isSprayingNeeded(false)
{
    _size = sizeof(*this);

    auto cropsLibrary = getCropsLibraryInstance();
    if (cropsLibrary) {
        auto cropsLibData = cropsLibrary->checkoutCropsData(cropType);
        if (cropsLibData && this != cropsLibData) {
            *this = *cropsLibData;
        }
        cropsLibrary->returnCropsData(cropsLibData);
    }
}

void HydroponicsCropsLibData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsData::toJSONObject(objectOut);

    objectOut[F("id")] = cropTypeToString(cropType);
    if (cropName[0]) { objectOut[F("cropName")] = stringFromChars(cropName, HYDRUINO_NAME_MAXSIZE); }

    int mainPhaseTotalWeeks = phaseDurationWeeks[0] + phaseDurationWeeks[1] + phaseDurationWeeks[2];
    HYDRUINO_SOFT_ASSERT(!totalGrowWeeks || !mainPhaseTotalWeeks || mainPhaseTotalWeeks == totalGrowWeeks, F("Total grow weeks mismatch, failure exporting totalGrowWeeks"));
    if (totalGrowWeeks && totalGrowWeeks != 14) {
        objectOut[F("totalGrowWeeks")] = totalGrowWeeks;
    } else if (!totalGrowWeeks && mainPhaseTotalWeeks && mainPhaseTotalWeeks != 14) {
        objectOut[F("totalGrowWeeks")] = mainPhaseTotalWeeks;
    }
    if (lifeCycleWeeks) {
        objectOut[F("lifeCycleWeeks")] = lifeCycleWeeks;
    }

    if (!(dailyLightHours[0] == 20 && dailyLightHours[1] == 18 && dailyLightHours[2] == 12)) {
        HYDRUINO_SOFT_ASSERT(Hydroponics_CropPhase_MainCount == 3, F("Main phase count mismatch, failure exporting dailyLightHours"));
        if (dailyLightHours[1] != 0 && dailyLightHours[1] != dailyLightHours[0] &&
            dailyLightHours[2] != 0 && dailyLightHours[2] != dailyLightHours[0]) {
            objectOut[F("dailyLightHours")] = commaStringFromArray(dailyLightHours, 3);
        } else {
            objectOut[F("dailyLightHours")] = dailyLightHours[0];
        }
    }

    if (!(phaseDurationWeeks[0] == 2 && phaseDurationWeeks[1] == 4 && phaseDurationWeeks[2] == 8)) {
        HYDRUINO_SOFT_ASSERT(Hydroponics_CropPhase_MainCount == 3, F("Main phase count mismatch, failure exporting phaseDurationWeeks"));
        objectOut[F("phaseDurationWeeks")] = commaStringFromArray(phaseDurationWeeks, 3);
    }

    if (!(isFPEqual(phRange[0], 6.0f) && isFPEqual(phRange[1], 6.0f))) {
        if (!isFPEqual(phRange[0], phRange[1])) {
            objectOut[F("phRange")] = commaStringFromArray(phRange, 2);
        } else {
            objectOut[F("phRange")] = phRange[0];
        }
    }

    if (!(isFPEqual(tdsRange[0], 1.8f) && isFPEqual(tdsRange[1], 2.4f))) {
        if (!isFPEqual(tdsRange[0], tdsRange[1])) {
            objectOut[F("tdsRange")] = commaStringFromArray(tdsRange, 2);
        } else {
            objectOut[F("tdsRange")] = tdsRange[0];
        }
    }

    if (!isFPEqual(nightlyFeedMultiplier, 1.0f)) { objectOut[F("nightlyFeedMultiplier")] = nightlyFeedMultiplier; }

    if (!(isFPEqual(waterTempRange[0], 25.0f) && isFPEqual(waterTempRange[1], 25.0f))) {
        if (!isFPEqual(waterTempRange[0], waterTempRange[1])) {
            objectOut[F("waterTempRange")] = commaStringFromArray(waterTempRange, 2);
        } else {
            objectOut[F("waterTempRange")] = waterTempRange[0];
        }
    }

    if (!(isFPEqual(airTempRange[0], 25.0f) && isFPEqual(airTempRange[1], 25.0f))) {
        if (!isFPEqual(airTempRange[0], airTempRange[1])) {
            objectOut[F("airTempRange")] = commaStringFromArray(airTempRange, 2);
        } else {
            objectOut[F("airTempRange")] = airTempRange[0];
        }
    }

    if (isInvasiveOrViner || isLargePlant || isPerennial || isToxicToPets || isPruningNeeded || isSprayingNeeded) {
        String flagsString = "";
        if (isInvasiveOrViner) { if (flagsString.length()) { flagsString.concat(','); } flagsString.concat(F("invasive")); }
        if (isLargePlant) { if (flagsString.length()) { flagsString.concat(','); } flagsString.concat(F("large")); }
        if (isPerennial) { if (flagsString.length()) { flagsString.concat(','); } flagsString.concat(F("perennial")); }
        if (isToxicToPets) { if (flagsString.length()) { flagsString.concat(','); } flagsString.concat(F("toxic")); }
        if (isPruningNeeded) { if (flagsString.length()) { flagsString.concat(','); } flagsString.concat(F("pruning")); }
        if (isSprayingNeeded) { if (flagsString.length()) { flagsString.concat(','); } flagsString.concat(F("spraying")); }
        objectOut[F("flags")] = flagsString;
    }
}

void HydroponicsCropsLibData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsData::fromJSONObject(objectIn);

    cropType = cropTypeFromString(objectIn[F("id")] | objectIn[F("cropType")]);
    const char *cropNameStr = objectIn[F("cropName")];
    if (cropNameStr && cropNameStr[0]) { strncpy(cropName, cropNameStr, HYDRUINO_NAME_MAXSIZE); }

    totalGrowWeeks = objectIn[F("totalGrowWeeks")] | totalGrowWeeks;
    lifeCycleWeeks = objectIn[F("lifeCycleWeeks")] | lifeCycleWeeks;

    {   HYDRUINO_SOFT_ASSERT(Hydroponics_CropPhase_MainCount == 3, F("Main phase count mismatch, failure importing dailyLightHours"));
        JsonVariantConst dailyLightHoursVar = objectIn[F("dailyLightHours")];
        commaStringToArray(dailyLightHoursVar, dailyLightHours, 3);
        dailyLightHours[0] = dailyLightHoursVar[F("seed")] | dailyLightHoursVar[0] | dailyLightHours[0];
        dailyLightHours[1] = dailyLightHoursVar[F("grow")] | dailyLightHoursVar[1] | dailyLightHours[1];
        dailyLightHours[2] = dailyLightHoursVar[F("bloom")] | dailyLightHoursVar[2] | dailyLightHours[2];
    }

    {   HYDRUINO_SOFT_ASSERT(Hydroponics_CropPhase_MainCount == 3, F("Main phase count mismatch, failure importing phaseDurationWeeks"));
        JsonVariantConst phaseDurationWeeksVar = objectIn[F("phaseDurationWeeks")];
        commaStringToArray(phaseDurationWeeksVar, phaseDurationWeeks, 3);
        phaseDurationWeeks[0] = phaseDurationWeeksVar[F("seed")] | phaseDurationWeeksVar[0] | phaseDurationWeeks[0];
        phaseDurationWeeks[1] = phaseDurationWeeksVar[F("grow")] | phaseDurationWeeksVar[1] | phaseDurationWeeks[1];
        phaseDurationWeeks[2] = phaseDurationWeeksVar[F("bloom")] | phaseDurationWeeksVar[2] | phaseDurationWeeks[2];
    }

    {   JsonVariantConst phRangeVar = objectIn[F("phRange")];
        commaStringToArray(phRangeVar, phRange, 2);
        phRange[0] = phRangeVar[F("min")] | phRangeVar[0] | phRange[0];
        phRange[1] = phRangeVar[F("max")] | phRangeVar[1] | phRange[1];
    }
    {   JsonVariantConst ecRangeVar = objectIn[F("ecRange")];
        JsonVariantConst tdsRangeVar = objectIn[F("tdsRange")];
        commaStringToArray(ecRangeVar, tdsRange, 2);
        commaStringToArray(tdsRangeVar, tdsRange, 2);
        tdsRange[0] = tdsRangeVar[F("min")] | tdsRangeVar[0] | ecRangeVar[F("min")] | ecRangeVar[0] | tdsRange[0];
        tdsRange[1] = tdsRangeVar[F("max")] | tdsRangeVar[1] | ecRangeVar[F("max")] | ecRangeVar[1] | tdsRange[1];
    }

    nightlyFeedMultiplier = objectIn[F("nightlyFeedMultiplier")] | nightlyFeedMultiplier;

    {   JsonVariantConst waterTempRangeVar = objectIn[F("waterTempRange")];
        commaStringToArray(waterTempRangeVar, waterTempRange, 2);
        waterTempRange[0] = waterTempRangeVar[F("min")] | waterTempRangeVar[0] | waterTempRange[0];
        waterTempRange[1] = waterTempRangeVar[F("max")] | waterTempRangeVar[1] | waterTempRange[1];
    }
    {   JsonVariantConst airTempRangeVar = objectIn[F("airTempRange")];
        commaStringToArray(airTempRangeVar, airTempRange, 2);
        airTempRange[0] = airTempRangeVar[F("min")] | airTempRangeVar[0] | airTempRange[0];
        airTempRange[1] = airTempRangeVar[F("max")] | airTempRangeVar[1] | airTempRange[1];
    }

    {   JsonVariantConst flagsVar = objectIn[F("flags")];

        if (flagsVar.is<JsonArrayConst>()) {
            JsonArrayConst flagsArray = flagsVar;
            for (String flagStr : flagsArray) {
                if (flagStr.equalsIgnoreCase(F("invasive"))) { isInvasiveOrViner = true; }
                if (flagStr.equalsIgnoreCase(F("large"))) { isLargePlant = true; }
                if (flagStr.equalsIgnoreCase(F("perennial"))) { isPerennial = true; }
                if (flagStr.equalsIgnoreCase(F("toxic"))) { isToxicToPets = true; }
                if (flagStr.equalsIgnoreCase(F("pruning"))) { isPruningNeeded = true; }
                if (flagStr.equalsIgnoreCase(F("spraying"))) { isSprayingNeeded = true; }
            }
        } else if (!flagsVar.isNull()) {
            String flagsString = String(F(",")) + objectIn[F("flags")].as<String>() + String(F(","));
            isInvasiveOrViner = occurrencesInStringIgnoreCase(flagsString, F(",invasive,"));
            isLargePlant = occurrencesInStringIgnoreCase(flagsString, F(",large,"));
            isPerennial = occurrencesInStringIgnoreCase(flagsString, F(",perennial,"));
            isToxicToPets = occurrencesInStringIgnoreCase(flagsString, F(",toxic,"));
            isPruningNeeded = occurrencesInStringIgnoreCase(flagsString, F(",pruning,"));
            isSprayingNeeded = occurrencesInStringIgnoreCase(flagsString, F(",spraying,"));
        }
    }
}

HydroponicsCustomAdditiveData::HydroponicsCustomAdditiveData()
    : HydroponicsData("HCAL", 1), additiveName{0}, weeklyDosingRates{1}
{
    _size = sizeof(*this);
}

HydroponicsCustomAdditiveData::HydroponicsCustomAdditiveData(Hydroponics_ReservoirType reservoirType)
    : HydroponicsData("HCAL", 1), additiveName{0}, weeklyDosingRates{0}
{
    _size = sizeof(*this);

    auto hydroponics = getHydroponicsInstance();
    if (hydroponics) { 
        auto additiveData = hydroponics->getCustomAdditiveData(reservoirType);
        if (additiveData && this != additiveData) {
            *this = *additiveData;
        }
    }
}

void HydroponicsCustomAdditiveData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsData::toJSONObject(objectOut);

    objectOut[F("id")] = reservoirTypeToString(reservoirType);
    if (additiveName[0]) { objectOut[F("additiveName")] = stringFromChars(additiveName, HYDRUINO_NAME_MAXSIZE); }
    bool hasWeeklyDosings = arrayElementsEqual(weeklyDosingRates, HYDRUINO_CROP_GROWEEKS_MAX, 0.0f);
    if (hasWeeklyDosings) { objectOut[F("weeklyDosingRates")] = commaStringFromArray(weeklyDosingRates, HYDRUINO_CROP_GROWEEKS_MAX); }
}

void HydroponicsCustomAdditiveData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsData::fromJSONObject(objectIn);

    reservoirType = reservoirTypeFromString(objectIn[F("id")] | objectIn[F("reservoirType")]);
    const char *additiveNameStr = objectIn[F("additiveName")];
    if (additiveNameStr && additiveNameStr[0]) { strncpy(additiveName, additiveNameStr, HYDRUINO_NAME_MAXSIZE); }
    JsonVariantConst weeklyDosingRatesVar = objectIn[F("weeklyDosingRates")];
    commaStringToArray(weeklyDosingRatesVar, weeklyDosingRates, HYDRUINO_CROP_GROWEEKS_MAX);
    for (int weekIndex = 0; weekIndex < HYDRUINO_CROP_GROWEEKS_MAX; ++weekIndex) { 
        weeklyDosingRates[weekIndex] = weeklyDosingRatesVar[weekIndex] | weeklyDosingRates[weekIndex];
    }
}
