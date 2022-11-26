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
            switch (classType) {
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
            switch (classType) {
                case 0: // Binary
                    return new HydroponicsBinarySensorData();
                case 1: // Analog
                    return new HydroponicsAnalogSensorData();
                case 3: // DHT
                    return new HydroponicsDHTTempHumiditySensorData();
                case 4: // DS
                    return new HydroponicsDSTemperatureSensorData();
                default: break;
            }
            break;

        case 2: // Crop
            switch (classType) {
                case 0: // Timed
                    return new HydroponicsTimedCropData();
                case 1: // Adaptive
                    return new HydroponicsAdaptiveCropData();
                default: break;
            }
            break;

        case 3: // Reservoir
            switch (classType) {
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
            switch (classType) {
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
    : HydroponicsData('H','S','Y','S', 1),
      systemMode(Hydroponics_SystemMode_Undefined), measureMode(Hydroponics_MeasurementMode_Undefined),
      dispOutMode(Hydroponics_DisplayOutputMode_Undefined), ctrlInMode(Hydroponics_ControlInputMode_Undefined),
      systemName{0}, timeZoneOffset(0), pollingInterval(HYDRUINO_DATA_LOOP_INTERVAL),
      autosaveEnabled(Hydroponics_Autosave_Disabled), autosaveFallback(Hydroponics_Autosave_Disabled), autosaveInterval(HYDRUINO_SYS_AUTOSAVE_INTERVAL),
      wifiSSID{0}, wifiPassword{0}, wifiPasswordSeed(0)
{
    _size = sizeof(*this);
    HYDRUINO_HARD_ASSERT(isSystemData(), SFP(HStr_Err_OperationFailure));
    strncpy(systemName, SFP(HStr_Default_SystemName).c_str(), HYDRUINO_NAME_MAXSIZE);
}

void HydroponicsSystemData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsData::toJSONObject(objectOut);

    objectOut[SFP(HStr_Key_SystemMode)] = systemModeToString(systemMode);
    objectOut[SFP(HStr_Key_MeasureMode)] = measurementModeToString(measureMode);
    #ifndef HYDRUINO_DISABLE_GUI
        objectOut[SFP(HStr_Key_DispOutMode)] = displayOutputModeToString(dispOutMode);
        objectOut[SFP(HStr_Key_CtrlInMode)] = controlInputModeToString(ctrlInMode);
    #else
        objectOut[SFP(HStr_Key_DispOutMode)] = displayOutputModeToString(Hydroponics_DisplayOutputMode_Disabled);
        objectOut[SFP(HStr_Key_CtrlInMode)] = controlInputModeToString(Hydroponics_ControlInputMode_Disabled);
    #endif
    if (systemName[0]) { objectOut[SFP(HStr_Key_SystemName)] = charsToString(systemName, HYDRUINO_NAME_MAXSIZE); }
    if (timeZoneOffset != 0) { objectOut[SFP(HStr_Key_TimeZoneOffset)] = timeZoneOffset; }
    if (pollingInterval && pollingInterval != HYDRUINO_DATA_LOOP_INTERVAL) { objectOut[SFP(HStr_Key_PollingInterval)] = pollingInterval; }
    if (autosaveEnabled != Hydroponics_Autosave_Disabled) { objectOut[SFP(HStr_Key_AutosaveEnabled)] = autosaveEnabled; }
    if (autosaveFallback != Hydroponics_Autosave_Disabled) { objectOut[SFP(HStr_Key_AutosaveFallback)] = autosaveFallback; }
    if (autosaveInterval && autosaveInterval != HYDRUINO_SYS_AUTOSAVE_INTERVAL) { objectOut[SFP(HStr_Key_AutosaveInterval)] = autosaveInterval; }
    if (wifiSSID[0]) { objectOut[SFP(HStr_Key_WiFiSSID)] = charsToString(wifiSSID, HYDRUINO_NAME_MAXSIZE); }
    if (wifiPasswordSeed) {
        objectOut[SFP(HStr_Key_WiFiPassword)] = hexStringFromBytes(wifiPassword, HYDRUINO_NAME_MAXSIZE);
        objectOut[SFP(HStr_Key_WiFiPasswordSeed)] = wifiPasswordSeed;
    } else if (wifiPassword[0]) {
        objectOut[SFP(HStr_Key_WiFiPassword)] = charsToString((const char *)wifiPassword, HYDRUINO_NAME_MAXSIZE);
    }

    JsonObject schedulerObj = objectOut.createNestedObject(SFP(HStr_Key_Scheduler));
    scheduler.toJSONObject(schedulerObj); if (!schedulerObj.size()) { objectOut.remove(SFP(HStr_Key_Scheduler)); }
    JsonObject loggerObj = objectOut.createNestedObject(SFP(HStr_Key_Logger));
    logger.toJSONObject(loggerObj); if (!loggerObj.size()) { objectOut.remove(SFP(HStr_Key_Logger)); }
    JsonObject publisherObj = objectOut.createNestedObject(SFP(HStr_Key_Publisher));
    publisher.toJSONObject(publisherObj); if (!publisherObj.size()) { objectOut.remove(SFP(HStr_Key_Publisher)); }
}

void HydroponicsSystemData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsData::fromJSONObject(objectIn);

    systemMode = systemModeFromString(objectIn[SFP(HStr_Key_SystemMode)]);
    measureMode = measurementModeFromString(objectIn[SFP(HStr_Key_MeasureMode)]);
    #ifndef HYDRUINO_DISABLE_GUI
        dispOutMode = displayOutputModeFromString(objectIn[SFP(HStr_Key_DispOutMode)]);
        ctrlInMode = controlInputModeFromString(objectIn[SFP(HStr_Key_CtrlInMode)]);
    #else
        dispOutMode = Hydroponics_DisplayOutputMode_Disabled;
        ctrlInMode = Hydroponics_ControlInputMode_Disabled;
    #endif
    const char *systemNameStr = objectIn[SFP(HStr_Key_SystemName)];
    if (systemNameStr && systemNameStr[0]) { strncpy(systemName, systemNameStr, HYDRUINO_NAME_MAXSIZE); }
    timeZoneOffset = objectIn[SFP(HStr_Key_TimeZoneOffset)] | timeZoneOffset;
    pollingInterval = objectIn[SFP(HStr_Key_PollingInterval)] | pollingInterval;
    autosaveEnabled = objectIn[SFP(HStr_Key_AutosaveEnabled)] | autosaveEnabled;
    autosaveFallback = objectIn[SFP(HStr_Key_AutosaveFallback)] | autosaveFallback;
    autosaveInterval = objectIn[SFP(HStr_Key_AutosaveInterval)] | autosaveInterval;
    const char *wifiSSIDStr = objectIn[SFP(HStr_Key_WiFiSSID)];
    if (wifiSSIDStr && wifiSSIDStr[0]) { strncpy(wifiSSID, wifiSSIDStr, HYDRUINO_NAME_MAXSIZE); }
    const char *wifiPasswordStr = objectIn[SFP(HStr_Key_WiFiPassword)];
    wifiPasswordSeed = objectIn[SFP(HStr_Key_WiFiPasswordSeed)] | wifiPasswordSeed;
    if (wifiPasswordStr && wifiPasswordSeed) { hexStringToBytes(String(wifiPasswordStr), wifiPassword, HYDRUINO_NAME_MAXSIZE); }
    else if (wifiPasswordStr && wifiPasswordStr[0]) { strncpy((char *)wifiPassword, wifiPasswordStr, HYDRUINO_NAME_MAXSIZE); wifiPasswordSeed = 0; }

    JsonObjectConst schedulerObj = objectIn[SFP(HStr_Key_Scheduler)];
    if (!schedulerObj.isNull()) { scheduler.fromJSONObject(schedulerObj); }
    JsonObjectConst loggerObj = objectIn[SFP(HStr_Key_Logger)];
    if (!loggerObj.isNull()) { logger.fromJSONObject(loggerObj); }
    JsonObjectConst publisherObj = objectIn[SFP(HStr_Key_Publisher)];
    if (!publisherObj.isNull()) { publisher.fromJSONObject(publisherObj); }
}


HydroponicsCalibrationData::HydroponicsCalibrationData()
    : HydroponicsData('H','C','A','L', 1),
      sensorName{0}, calibUnits(Hydroponics_UnitsType_Undefined),
      multiplier(1.0f), offset(0.0f)
{
    _size = sizeof(*this);
    HYDRUINO_HARD_ASSERT(isCalibrationData(), SFP(HStr_Err_OperationFailure));
}

HydroponicsCalibrationData::HydroponicsCalibrationData(HydroponicsIdentity sensorId, Hydroponics_UnitsType calibUnitsIn)
    : HydroponicsData('H','C','A','L', 1),
      sensorName{0}, calibUnits(calibUnitsIn),
      multiplier(1.0f), offset(0.0f)
{
    _size = sizeof(*this);
    HYDRUINO_HARD_ASSERT(isCalibrationData(), SFP(HStr_Err_OperationFailure));
    if (sensorId) {
        strncpy(sensorName, sensorId.keyString.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}

void HydroponicsCalibrationData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsData::toJSONObject(objectOut);

    if (sensorName[0]) { objectOut[SFP(HStr_Key_SensorName)] = charsToString(sensorName, HYDRUINO_NAME_MAXSIZE); }
    if (calibUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HStr_Key_CalibUnits)] = unitsTypeToSymbol(calibUnits); }
    objectOut[SFP(HStr_Key_Multiplier)] = multiplier;
    objectOut[SFP(HStr_Key_Offset)] = offset;
}

void HydroponicsCalibrationData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsData::fromJSONObject(objectIn);

    const char *sensorNameStr = objectIn[SFP(HStr_Key_SensorName)];
    if (sensorNameStr && sensorNameStr[0]) { strncpy(sensorName, sensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    calibUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_CalibUnits)]);
    multiplier = objectIn[SFP(HStr_Key_Multiplier)] | multiplier;
    offset = objectIn[SFP(HStr_Key_Offset)] | offset;
}

void HydroponicsCalibrationData::setFromTwoPoints(float point1MeasuredAt, float point1CalibratedTo,
                                                  float point2MeasuredAt, float point2CalibratedTo)
{
    float aTerm = point2CalibratedTo - point1CalibratedTo;
    float bTerm = point2MeasuredAt - point1MeasuredAt;
    HYDRUINO_SOFT_ASSERT(!isFPEqual(bTerm, 0.0f), SFP(HStr_Err_InvalidParameter));
    if (!isFPEqual(bTerm, 0.0f)) {
        _bumpRevIfNotAlreadyModded();
        multiplier = aTerm / bTerm;
        offset = ((aTerm * point2MeasuredAt) + (bTerm * point1CalibratedTo)) / bTerm;
    }
}


HydroponicsCropsLibData::HydroponicsCropsLibData()
    : HydroponicsData('H','C','L','D', 1),
      cropType(Hydroponics_CropType_Undefined), cropName{0},
      totalGrowWeeks(14), lifeCycleWeeks(0),
      dailyLightHours{20,18,12}, phaseDurationWeeks{2,4,8},
      phRange{6,6}, tdsRange{1.8f,2.4f}, nightlyFeedRate(1),
      waterTempRange{25,25}, airTempRange{25,25}, co2Levels{700,1400},
      flags(0)
{
    HYDRUINO_HARD_ASSERT(isCropsLibData(), SFP(HStr_Err_OperationFailure));
    _size = sizeof(*this);
}

HydroponicsCropsLibData::HydroponicsCropsLibData(const Hydroponics_CropType cropTypeIn)
    : HydroponicsData('H','C','L','D', 1),
      cropType(cropTypeIn), cropName{0},
      totalGrowWeeks(14), lifeCycleWeeks(0),
      dailyLightHours{20,18,12}, phaseDurationWeeks{2,4,8},
      phRange{6,6}, tdsRange{1.8f,2.4f}, nightlyFeedRate(1),
      waterTempRange{25,25}, airTempRange{25,25}, co2Levels{700,1400},
      flags(0)
{
    _size = sizeof(*this);
    HYDRUINO_HARD_ASSERT(isCropsLibData(), SFP(HStr_Err_OperationFailure));

    {   auto cropsLibData = hydroCropsLib.checkoutCropsData(cropType);
        if (cropsLibData && this != cropsLibData) {
            *this = *cropsLibData;
        }
        hydroCropsLib.returnCropsData(cropsLibData);
    }
}

void HydroponicsCropsLibData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsData::toJSONObject(objectOut);

    objectOut[SFP(HStr_Key_Id)] = cropTypeToString(cropType);
    if (cropName[0]) { objectOut[SFP(HStr_Key_CropName)] = charsToString(cropName, HYDRUINO_NAME_MAXSIZE); }

    int mainPhaseTotalWeeks = phaseDurationWeeks[0] + phaseDurationWeeks[1] + phaseDurationWeeks[2];
    HYDRUINO_SOFT_ASSERT(!totalGrowWeeks || !mainPhaseTotalWeeks || mainPhaseTotalWeeks == totalGrowWeeks, SFP(HStr_Err_ExportFailure));
    if (totalGrowWeeks && totalGrowWeeks != 14) {
        objectOut[SFP(HStr_Key_TotalGrowWeeks)] = totalGrowWeeks;
    } else if (!totalGrowWeeks && mainPhaseTotalWeeks && mainPhaseTotalWeeks != 14) {
        objectOut[SFP(HStr_Key_TotalGrowWeeks)] = mainPhaseTotalWeeks;
    }
    if (lifeCycleWeeks) {
        objectOut[SFP(HStr_Key_LifeCycleWeeks)] = lifeCycleWeeks;
    }

    if (!(dailyLightHours[0] == 20 && dailyLightHours[1] == 18 && dailyLightHours[2] == 12)) {
        HYDRUINO_SOFT_ASSERT(Hydroponics_CropPhase_MainCount == 3, SFP(HStr_Err_ExportFailure));
        if (dailyLightHours[1] != 0 && dailyLightHours[1] != dailyLightHours[0] &&
            dailyLightHours[2] != 0 && dailyLightHours[2] != dailyLightHours[0]) {
            objectOut[SFP(HStr_Key_DailyLightHours)] = commaStringFromArray(dailyLightHours, 3);
        } else {
            objectOut[SFP(HStr_Key_DailyLightHours)] = dailyLightHours[0];
        }
    }

    if (!(phaseDurationWeeks[0] == 2 && phaseDurationWeeks[1] == 4 && phaseDurationWeeks[2] == 8)) {
        HYDRUINO_SOFT_ASSERT(Hydroponics_CropPhase_MainCount == 3, SFP(HStr_Err_ExportFailure));
        objectOut[SFP(HStr_Key_PhaseDurationWeeks)] = commaStringFromArray(phaseDurationWeeks, 3);
    }

    if (!(isFPEqual(phRange[0], 6.0f) && isFPEqual(phRange[1], 6.0f))) {
        if (!isFPEqual(phRange[0], phRange[1])) {
            objectOut[SFP(HStr_Key_PHRange)] = commaStringFromArray(phRange, 2);
        } else {
            objectOut[SFP(HStr_Key_PHRange)] = phRange[0];
        }
    }

    if (!(isFPEqual(tdsRange[0], 1.8f) && isFPEqual(tdsRange[1], 2.4f))) {
        if (!isFPEqual(tdsRange[0], tdsRange[1])) {
            objectOut[SFP(HStr_Key_TDSRange)] = commaStringFromArray(tdsRange, 2);
        } else {
            objectOut[SFP(HStr_Key_TDSRange)] = tdsRange[0];
        }
    }

    if (!isFPEqual(nightlyFeedRate, 1.0f)) { objectOut[SFP(HStr_Key_NightlyFeedRate)] = nightlyFeedRate; }

    if (!(isFPEqual(waterTempRange[0], 25.0f) && isFPEqual(waterTempRange[1], 25.0f))) {
        if (!isFPEqual(waterTempRange[0], waterTempRange[1])) {
            objectOut[SFP(HStr_Key_WaterTemperatureRange)] = commaStringFromArray(waterTempRange, 2);
        } else {
            objectOut[SFP(HStr_Key_WaterTemperatureRange)] = waterTempRange[0];
        }
    }

    if (!(isFPEqual(airTempRange[0], 25.0f) && isFPEqual(airTempRange[1], 25.0f))) {
        if (!isFPEqual(airTempRange[0], airTempRange[1])) {
            objectOut[SFP(HStr_Key_AirTempRange)] = commaStringFromArray(airTempRange, 2);
        } else {
            objectOut[SFP(HStr_Key_AirTempRange)] = airTempRange[0];
        }
    }

    if (!(isFPEqual(co2Levels[0], 700.0f) && isFPEqual(co2Levels[1], 1400.0f))) {
        if (!isFPEqual(co2Levels[0], co2Levels[1])) {
            objectOut[SFP(HStr_Key_CO2Levels)] = commaStringFromArray(co2Levels, 2);
        } else {
            objectOut[SFP(HStr_Key_CO2Levels)] = co2Levels[0];
        }
    }

    if (flags) {
        String flagsString;
        if (isInvasive()) { if (flagsString.length()) { flagsString.concat(','); } flagsString.concat(SFP(HStr_Key_Invasive)); }
        if (isViner()) { if (flagsString.length()) { flagsString.concat(','); } flagsString.concat(SFP(HStr_Key_Viner)); }
        if (isLarge()) { if (flagsString.length()) { flagsString.concat(','); } flagsString.concat(SFP(HStr_Key_Large)); }
        if (isPerennial()) { if (flagsString.length()) { flagsString.concat(','); } flagsString.concat(SFP(HStr_Key_Perennial)); }
        if (isToxicToPets()) { if (flagsString.length()) { flagsString.concat(','); } flagsString.concat(SFP(HStr_Key_Toxic)); }
        if (needsPrunning()) { if (flagsString.length()) { flagsString.concat(','); } flagsString.concat(SFP(HStr_Key_Pruning)); }
        if (needsSpraying()) { if (flagsString.length()) { flagsString.concat(','); } flagsString.concat(SFP(HStr_Key_Spraying)); }
        objectOut[SFP(HStr_Key_Flags)] = flagsString;
    }
}

void HydroponicsCropsLibData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsData::fromJSONObject(objectIn);

    cropType = cropTypeFromString(objectIn[SFP(HStr_Key_Id)]);
    const char *cropStr = objectIn[SFP(HStr_Key_CropName)];
    if (cropStr && cropStr[0]) { strncpy(cropName, cropStr, HYDRUINO_NAME_MAXSIZE); }

    totalGrowWeeks = objectIn[SFP(HStr_Key_TotalGrowWeeks)] | totalGrowWeeks;
    lifeCycleWeeks = objectIn[SFP(HStr_Key_LifeCycleWeeks)] | lifeCycleWeeks;

    {   HYDRUINO_SOFT_ASSERT(Hydroponics_CropPhase_MainCount == 3, SFP(HStr_Err_ImportFailure));
        JsonVariantConst dailyLightHoursVar = objectIn[SFP(HStr_Key_DailyLightHours)];
        commaStringToArray(dailyLightHoursVar, dailyLightHours, 3);
    }

    {   HYDRUINO_SOFT_ASSERT(Hydroponics_CropPhase_MainCount == 3, SFP(HStr_Err_ImportFailure));
        JsonVariantConst phaseDurationWeeksVar = objectIn[SFP(HStr_Key_PhaseDurationWeeks)];
        commaStringToArray(phaseDurationWeeksVar, phaseDurationWeeks, 3);
    }

    {   JsonVariantConst phRangeVar = objectIn[SFP(HStr_Key_PHRange)];
        commaStringToArray(phRangeVar, phRange, 2);
    }
    {   JsonVariantConst tdsRangeVar = objectIn[SFP(HStr_Key_TDSRange)];
        commaStringToArray(tdsRangeVar, tdsRange, 2);
    }

    nightlyFeedRate = objectIn[SFP(HStr_Key_NightlyFeedRate)] | nightlyFeedRate;

    {   JsonVariantConst waterTempRangeVar = objectIn[SFP(HStr_Key_WaterTemperatureRange)];
        commaStringToArray(waterTempRangeVar, waterTempRange, 2);
    }
    {   JsonVariantConst airTempRangeVar = objectIn[SFP(HStr_Key_AirTempRange)];
        commaStringToArray(airTempRangeVar, airTempRange, 2);
    }

    {   JsonVariantConst flagsVar = objectIn[SFP(HStr_Key_Flags)];
        if (flagsVar.is<JsonArrayConst>()) {
            JsonArrayConst flagsArray = flagsVar;
            for (String flagStr : flagsArray) {
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Invasive))) { flags |= Hydroponics_CropsDataFlag_Invasive; }
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Viner))) { flags |= Hydroponics_CropsDataFlag_Viner; }
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Large))) { flags |= Hydroponics_CropsDataFlag_Large; }
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Perennial))) { flags |= Hydroponics_CropsDataFlag_Perennial; }
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Toxic))) { flags |= Hydroponics_CropsDataFlag_Toxic; }
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Pruning))) { flags |= Hydroponics_CropsDataFlag_Pruning; }
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Spraying))) { flags |= Hydroponics_CropsDataFlag_Spraying; }
            }
        } else if (!flagsVar.isNull()) {
            String flagsString = String(',') + objectIn[SFP(HStr_Key_Flags)].as<String>() + String(',');
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Invasive) + String(','))) { flags |= Hydroponics_CropsDataFlag_Invasive; }
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Viner) + String(','))) { flags |= Hydroponics_CropsDataFlag_Viner; }
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Large) + String(','))) { flags |= Hydroponics_CropsDataFlag_Large; }
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Perennial) + String(','))) { flags |= Hydroponics_CropsDataFlag_Perennial; }
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Toxic) + String(','))) { flags |= Hydroponics_CropsDataFlag_Toxic; }
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Pruning) + String(','))) { flags |= Hydroponics_CropsDataFlag_Pruning; }
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Spraying) + String(','))) { flags |= Hydroponics_CropsDataFlag_Spraying; }
        }
    }
}

HydroponicsCustomAdditiveData::HydroponicsCustomAdditiveData()
    : HydroponicsData('H','A','D','D', 1), additiveName{0}, weeklyDosingRates{1}
{
    HYDRUINO_HARD_ASSERT(isCalibrationData(), SFP(HStr_Err_OperationFailure));
    _size = sizeof(*this);
}

HydroponicsCustomAdditiveData::HydroponicsCustomAdditiveData(Hydroponics_ReservoirType reservoirType)
    : HydroponicsData('H','A','D','D', 1), additiveName{0}, weeklyDosingRates{1}
{
    _size = sizeof(*this);
    HYDRUINO_HARD_ASSERT(isCalibrationData(), SFP(HStr_Err_OperationFailure));

    {   auto additiveData = hydroAdditives.getCustomAdditiveData(reservoirType);
        if (additiveData && this != additiveData) {
            *this = *additiveData;
        }
    }
}

void HydroponicsCustomAdditiveData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsData::toJSONObject(objectOut);

    objectOut[SFP(HStr_Key_Id)] = reservoirTypeToString(reservoirType);
    if (additiveName[0]) { objectOut[SFP(HStr_Key_AdditiveName)] = charsToString(additiveName, HYDRUINO_NAME_MAXSIZE); }
    bool hasWeeklyDosings = arrayElementsEqual(weeklyDosingRates, HYDRUINO_CROP_GROWWEEKS_MAX, 0.0f);
    if (hasWeeklyDosings) { objectOut[SFP(HStr_Key_WeeklyDosingRates)] = commaStringFromArray(weeklyDosingRates, HYDRUINO_CROP_GROWWEEKS_MAX); }
}

void HydroponicsCustomAdditiveData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsData::fromJSONObject(objectIn);

    reservoirType = reservoirTypeFromString(objectIn[SFP(HStr_Key_Id)]);
    const char *additiveStr = objectIn[SFP(HStr_Key_AdditiveName)];
    if (additiveStr && additiveStr[0]) { strncpy(additiveName, additiveStr, HYDRUINO_NAME_MAXSIZE); }
    JsonVariantConst weeklyDosingRatesVar = objectIn[SFP(HStr_Key_WeeklyDosingRates)];
    commaStringToArray(weeklyDosingRatesVar, weeklyDosingRates, HYDRUINO_CROP_GROWWEEKS_MAX);
}
