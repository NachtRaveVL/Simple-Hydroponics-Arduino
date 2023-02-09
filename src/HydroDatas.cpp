/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Datas
*/

#include "Hydruino.h"

HydroData *_allocateDataFromBaseDecode(const HydroData &baseDecode)
{
    HydroData *retVal = nullptr;

    if (baseDecode.isStandardData()) {
        if (baseDecode.isSystemData()) {
            retVal = new HydroSystemData();
        } else if (baseDecode.isCalibrationData()) {
            retVal = new HydroCalibrationData();
        } else if (baseDecode.isCropsLibData()) {
            retVal = new HydroCropsLibData();
        } else if (baseDecode.isAdditiveData()) {
            retVal = new HydroCustomAdditiveData();
        }
    } else if (baseDecode.isObjectData()) {
        retVal = _allocateDataForObjType(baseDecode.id.object.idType, baseDecode.id.object.classType);
    }

    HYDRO_SOFT_ASSERT(retVal, F("Unknown data decode"));
    if (retVal) {
        retVal->id = baseDecode.id;
        HYDRO_SOFT_ASSERT(retVal->_version == baseDecode._version, F("Data version mismatch")); // TODO: Version updaters
        retVal->_revision = baseDecode._revision;
        return retVal;
    }
    return new HydroData(baseDecode);
}

HydroData *_allocateDataForObjType(int8_t idType, int8_t classType)
{
    switch (idType) {
        case (int8_t)HydroIdentity::Actuator:
            switch (classType) {
                case (int8_t)HydroActuator::Relay:
                    return new HydroActuatorData();
                case (int8_t)HydroActuator::RelayPump:
                    return new HydroPumpActuatorData();
                case (int8_t)HydroActuator::Variable:
                    return new HydroActuatorData();
                case (int8_t)HydroActuator::VariablePump:
                    return new HydroPumpActuatorData();
                default: break;
            }
            break;

        case (int8_t)HydroIdentity::Sensor:
            switch (classType) {
                case (int8_t)HydroSensor::Binary:
                    return new HydroBinarySensorData();
                case (int8_t)HydroSensor::Analog:
                    return new HydroAnalogSensorData();
                //case 2: // Digital (not instance-able)
                case (int8_t)HydroSensor::DHT1W:
                    return new HydroDHTTempHumiditySensorData();
                case (int8_t)HydroSensor::DS1W:
                    return new HydroDSTemperatureSensorData();
                default: break;
            }
            break;

        case (int8_t)HydroIdentity::Crop:
            switch (classType) {
                case (int8_t)HydroCrop::Timed:
                    return new HydroTimedCropData();
                case (int8_t)HydroCrop::Adaptive:
                    return new HydroAdaptiveCropData();
                default: break;
            }
            break;

        case (int8_t)HydroIdentity::Reservoir:
            switch (classType) {
                case (int8_t)HydroReservoir::Fluid:
                    return new HydroFluidReservoirData();
                case (int8_t)HydroReservoir::Feed:
                    return new HydroFeedReservoirData();
                case (int8_t)HydroReservoir::Pipe:
                    return new HydroInfiniteReservoirData();
                default: break;
            }
            break;

        case (int8_t)HydroIdentity::Rail:
            switch (classType) {
                case (int8_t)HydroRail::Simple:
                    return new HydroSimpleRailData();
                case (int8_t)HydroRail::Regulated:
                    return new HydroRegulatedRailData();
                default: break;
            }
            break;

        default: break;
    }

    return nullptr;
}

HydroSystemData::HydroSystemData()
    : HydroData('H','S','Y','S', 1),
      systemMode(Hydro_SystemMode_Undefined), measureMode(Hydro_MeasurementMode_Undefined),
      dispOutMode(Hydro_DisplayOutputMode_Undefined), ctrlInMode(Hydro_ControlInputMode_Undefined),
      systemName{0}, timeZoneOffset(0), pollingInterval(HYDRO_DATA_LOOP_INTERVAL),
      autosaveEnabled(Hydro_Autosave_Disabled), autosaveFallback(Hydro_Autosave_Disabled), autosaveInterval(HYDRO_SYS_AUTOSAVE_INTERVAL),
      wifiSSID{0}, wifiPassword{0}, wifiPasswordSeed(0),
      macAddress{0},
      latitude(DBL_UNDEF), longitude(DBL_UNDEF), altitude(DBL_UNDEF)
{
    _size = sizeof(*this);
    HYDRO_HARD_ASSERT(isSystemData(), SFP(HStr_Err_OperationFailure));
    strncpy(systemName, SFP(HStr_Default_SystemName).c_str(), HYDRO_NAME_MAXSIZE);
}

void HydroSystemData::toJSONObject(JsonObject &objectOut) const
{
    HydroData::toJSONObject(objectOut);

    objectOut[SFP(HStr_Key_SystemMode)] = systemModeToString(systemMode);
    objectOut[SFP(HStr_Key_MeasureMode)] = measurementModeToString(measureMode);
    #ifdef HYDRO_USE_GUI
        objectOut[SFP(HStr_Key_DispOutMode)] = displayOutputModeToString(dispOutMode);
        objectOut[SFP(HStr_Key_CtrlInMode)] = controlInputModeToString(ctrlInMode);
    #else
        objectOut[SFP(HStr_Key_DispOutMode)] = displayOutputModeToString(Hydro_DisplayOutputMode_Disabled);
        objectOut[SFP(HStr_Key_CtrlInMode)] = controlInputModeToString(Hydro_ControlInputMode_Disabled);
    #endif
    if (systemName[0]) { objectOut[SFP(HStr_Key_SystemName)] = charsToString(systemName, HYDRO_NAME_MAXSIZE); }
    if (timeZoneOffset != 0) { objectOut[SFP(HStr_Key_TimeZoneOffset)] = timeZoneOffset; }
    if (pollingInterval && pollingInterval != HYDRO_DATA_LOOP_INTERVAL) { objectOut[SFP(HStr_Key_PollingInterval)] = pollingInterval; }
    if (autosaveEnabled != Hydro_Autosave_Disabled) { objectOut[SFP(HStr_Key_AutosaveEnabled)] = autosaveEnabled; }
    if (autosaveFallback != Hydro_Autosave_Disabled) { objectOut[SFP(HStr_Key_AutosaveFallback)] = autosaveFallback; }
    if (autosaveInterval && autosaveInterval != HYDRO_SYS_AUTOSAVE_INTERVAL) { objectOut[SFP(HStr_Key_AutosaveInterval)] = autosaveInterval; }
    if (wifiSSID[0]) { objectOut[SFP(HStr_Key_WiFiSSID)] = charsToString(wifiSSID, HYDRO_NAME_MAXSIZE); }
    if (wifiPasswordSeed) {
        objectOut[SFP(HStr_Key_WiFiPassword)] = hexStringFromBytes(wifiPassword, HYDRO_NAME_MAXSIZE);
        objectOut[SFP(HStr_Key_WiFiPasswordSeed)] = wifiPasswordSeed;
    } else if (wifiPassword[0]) {
        objectOut[SFP(HStr_Key_WiFiPassword)] = charsToString((const char *)wifiPassword, HYDRO_NAME_MAXSIZE);
    }
    if (!arrayElementsEqual<uint8_t>(macAddress, 6, 0)) {
        objectOut[SFP(HStr_Key_MACAddress)] = commaStringFromArray(macAddress, 6);
    }
    if (!isFPEqual(latitude, DBL_UNDEF)) { objectOut[SFP(HStr_Key_Latitude)] = latitude; }
    if (!isFPEqual(longitude, DBL_UNDEF)) { objectOut[SFP(HStr_Key_Longitude)] = longitude; }
    if (!isFPEqual(altitude, DBL_UNDEF)) { objectOut[SFP(HStr_Key_Altitude)] = altitude; }

    JsonObject schedulerObj = objectOut.createNestedObject(SFP(HStr_Key_Scheduler));
    scheduler.toJSONObject(schedulerObj); if (!schedulerObj.size()) { objectOut.remove(SFP(HStr_Key_Scheduler)); }
    JsonObject loggerObj = objectOut.createNestedObject(SFP(HStr_Key_Logger));
    logger.toJSONObject(loggerObj); if (!loggerObj.size()) { objectOut.remove(SFP(HStr_Key_Logger)); }
    JsonObject publisherObj = objectOut.createNestedObject(SFP(HStr_Key_Publisher));
    publisher.toJSONObject(publisherObj); if (!publisherObj.size()) { objectOut.remove(SFP(HStr_Key_Publisher)); }
}

void HydroSystemData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroData::fromJSONObject(objectIn);

    systemMode = systemModeFromString(objectIn[SFP(HStr_Key_SystemMode)]);
    measureMode = measurementModeFromString(objectIn[SFP(HStr_Key_MeasureMode)]);
    #ifdef HYDRO_USE_GUI
        dispOutMode = displayOutputModeFromString(objectIn[SFP(HStr_Key_DispOutMode)]);
        ctrlInMode = controlInputModeFromString(objectIn[SFP(HStr_Key_CtrlInMode)]);
    #else
        dispOutMode = Hydro_DisplayOutputMode_Disabled;
        ctrlInMode = Hydro_ControlInputMode_Disabled;
    #endif
    const char *systemNameStr = objectIn[SFP(HStr_Key_SystemName)];
    if (systemNameStr && systemNameStr[0]) { strncpy(systemName, systemNameStr, HYDRO_NAME_MAXSIZE); }
    timeZoneOffset = objectIn[SFP(HStr_Key_TimeZoneOffset)] | timeZoneOffset;
    pollingInterval = objectIn[SFP(HStr_Key_PollingInterval)] | pollingInterval;
    autosaveEnabled = objectIn[SFP(HStr_Key_AutosaveEnabled)] | autosaveEnabled;
    autosaveFallback = objectIn[SFP(HStr_Key_AutosaveFallback)] | autosaveFallback;
    autosaveInterval = objectIn[SFP(HStr_Key_AutosaveInterval)] | autosaveInterval;
    const char *wifiSSIDStr = objectIn[SFP(HStr_Key_WiFiSSID)];
    if (wifiSSIDStr && wifiSSIDStr[0]) { strncpy(wifiSSID, wifiSSIDStr, HYDRO_NAME_MAXSIZE); }
    const char *wifiPasswordStr = objectIn[SFP(HStr_Key_WiFiPassword)];
    wifiPasswordSeed = objectIn[SFP(HStr_Key_WiFiPasswordSeed)] | wifiPasswordSeed;
    if (wifiPasswordStr && wifiPasswordSeed) { hexStringToBytes(String(wifiPasswordStr), wifiPassword, HYDRO_NAME_MAXSIZE); }
    else if (wifiPasswordStr && wifiPasswordStr[0]) { strncpy((char *)wifiPassword, wifiPasswordStr, HYDRO_NAME_MAXSIZE); wifiPasswordSeed = 0; }
    JsonVariantConst macAddressVar = objectIn[SFP(HStr_Key_MACAddress)];
    commaStringToArray(macAddressVar, macAddress, 6);
    latitude = objectIn[SFP(HStr_Key_Latitude)] | latitude;
    longitude = objectIn[SFP(HStr_Key_Longitude)] | longitude;
    altitude = objectIn[SFP(HStr_Key_Altitude)] | altitude;

    JsonObjectConst schedulerObj = objectIn[SFP(HStr_Key_Scheduler)];
    if (!schedulerObj.isNull()) { scheduler.fromJSONObject(schedulerObj); }
    JsonObjectConst loggerObj = objectIn[SFP(HStr_Key_Logger)];
    if (!loggerObj.isNull()) { logger.fromJSONObject(loggerObj); }
    JsonObjectConst publisherObj = objectIn[SFP(HStr_Key_Publisher)];
    if (!publisherObj.isNull()) { publisher.fromJSONObject(publisherObj); }
}


HydroCalibrationData::HydroCalibrationData()
    : HydroData('H','C','A','L', 1),
      ownerName{0}, calibUnits(Hydro_UnitsType_Undefined),
      multiplier(1.0f), offset(0.0f)
{
    _size = sizeof(*this);
    HYDRO_HARD_ASSERT(isCalibrationData(), SFP(HStr_Err_OperationFailure));
}

HydroCalibrationData::HydroCalibrationData(HydroIdentity ownerId, Hydro_UnitsType calibUnitsIn)
    : HydroData('H','C','A','L', 1),
      ownerName{0}, calibUnits(calibUnitsIn),
      multiplier(1.0f), offset(0.0f)
{
    _size = sizeof(*this);
    HYDRO_HARD_ASSERT(isCalibrationData(), SFP(HStr_Err_OperationFailure));
    if (ownerId) {
        strncpy(ownerName, ownerId.keyString.c_str(), HYDRO_NAME_MAXSIZE);
    }
}

void HydroCalibrationData::toJSONObject(JsonObject &objectOut) const
{
    HydroData::toJSONObject(objectOut);

    if (ownerName[0]) { objectOut[SFP(HStr_Key_SensorName)] = charsToString(ownerName, HYDRO_NAME_MAXSIZE); }
    if (calibUnits != Hydro_UnitsType_Undefined) { objectOut[SFP(HStr_Key_CalibUnits)] = unitsTypeToSymbol(calibUnits); }
    objectOut[SFP(HStr_Key_Multiplier)] = multiplier;
    objectOut[SFP(HStr_Key_Offset)] = offset;
}

void HydroCalibrationData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroData::fromJSONObject(objectIn);

    const char *ownerNameStr = objectIn[SFP(HStr_Key_SensorName)];
    if (ownerNameStr && ownerNameStr[0]) { strncpy(ownerName, ownerNameStr, HYDRO_NAME_MAXSIZE); }
    calibUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_CalibUnits)]);
    multiplier = objectIn[SFP(HStr_Key_Multiplier)] | multiplier;
    offset = objectIn[SFP(HStr_Key_Offset)] | offset;
}

void HydroCalibrationData::setFromTwoPoints(float point1MeasuredAt, float point1CalibratedTo,
                                            float point2MeasuredAt, float point2CalibratedTo)
{
    float aTerm = point2CalibratedTo - point1CalibratedTo;
    float bTerm = point2MeasuredAt - point1MeasuredAt;
    HYDRO_SOFT_ASSERT(!isFPEqual(bTerm, 0.0f), SFP(HStr_Err_InvalidParameter));
    if (!isFPEqual(bTerm, 0.0f)) {
        _bumpRevIfNotAlreadyModded();
        multiplier = aTerm / bTerm;
        offset = ((aTerm * point2MeasuredAt) + (bTerm * point1CalibratedTo)) / bTerm;
    }
}


HydroCropsLibData::HydroCropsLibData()
    : HydroData('H','C','L','D', 1),
      cropType(Hydro_CropType_Undefined), cropName{0},
      totalGrowWeeks(14), lifeCycleWeeks(0),
      dailyLightHours{20,18,12}, phaseDurationWeeks{2,4,8},
      phRange{6,6}, tdsRange{1.8f,2.4f}, nightlyFeedRate(1),
      waterTempRange{25,25}, airTempRange{25,25}, co2Levels{700,1400},
      flags(Hydro_CropsDataFlag_None)
{
    HYDRO_HARD_ASSERT(isCropsLibData(), SFP(HStr_Err_OperationFailure));
    _size = sizeof(*this);
}

HydroCropsLibData::HydroCropsLibData(const Hydro_CropType cropTypeIn)
    : HydroData('H','C','L','D', 1),
      cropType(cropTypeIn), cropName{0},
      totalGrowWeeks(14), lifeCycleWeeks(0),
      dailyLightHours{20,18,12}, phaseDurationWeeks{2,4,8},
      phRange{6,6}, tdsRange{1.8f,2.4f}, nightlyFeedRate(1),
      waterTempRange{25,25}, airTempRange{25,25}, co2Levels{700,1400},
      flags(Hydro_CropsDataFlag_None)
{
    _size = sizeof(*this);
    HYDRO_HARD_ASSERT(isCropsLibData(), SFP(HStr_Err_OperationFailure));

    {   auto cropsLibData = hydroCropsLib.checkoutCropsData(cropType);
        if (cropsLibData && this != cropsLibData) {
            *this = *cropsLibData;
        }
        hydroCropsLib.returnCropsData(cropsLibData);
    }
}

void HydroCropsLibData::toJSONObject(JsonObject &objectOut) const
{
    HydroData::toJSONObject(objectOut);

    objectOut[SFP(HStr_Key_Id)] = cropTypeToString(cropType);
    if (cropName[0]) { objectOut[SFP(HStr_Key_CropName)] = charsToString(cropName, HYDRO_NAME_MAXSIZE); }

    int mainPhaseTotalWeeks = phaseDurationWeeks[0] + phaseDurationWeeks[1] + phaseDurationWeeks[2];
    HYDRO_SOFT_ASSERT(!totalGrowWeeks || !mainPhaseTotalWeeks || mainPhaseTotalWeeks == totalGrowWeeks, SFP(HStr_Err_ExportFailure));
    if (totalGrowWeeks && totalGrowWeeks != 14) {
        objectOut[SFP(HStr_Key_TotalGrowWeeks)] = totalGrowWeeks;
    } else if (!totalGrowWeeks && mainPhaseTotalWeeks && mainPhaseTotalWeeks != 14) {
        objectOut[SFP(HStr_Key_TotalGrowWeeks)] = mainPhaseTotalWeeks;
    }
    if (lifeCycleWeeks) {
        objectOut[SFP(HStr_Key_LifeCycleWeeks)] = lifeCycleWeeks;
    }

    if (!(dailyLightHours[0] == 20 && dailyLightHours[1] == 18 && dailyLightHours[2] == 12)) {
        HYDRO_SOFT_ASSERT(Hydro_CropPhase_MainCount == 3, SFP(HStr_Err_ExportFailure));
        if (dailyLightHours[1] != 0 && dailyLightHours[1] != dailyLightHours[0] &&
            dailyLightHours[2] != 0 && dailyLightHours[2] != dailyLightHours[0]) {
            objectOut[SFP(HStr_Key_DailyLightHours)] = commaStringFromArray(dailyLightHours, 3);
        } else {
            objectOut[SFP(HStr_Key_DailyLightHours)] = dailyLightHours[0];
        }
    }

    if (!(phaseDurationWeeks[0] == 2 && phaseDurationWeeks[1] == 4 && phaseDurationWeeks[2] == 8)) {
        HYDRO_SOFT_ASSERT(Hydro_CropPhase_MainCount == 3, SFP(HStr_Err_ExportFailure));
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

void HydroCropsLibData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroData::fromJSONObject(objectIn);

    cropType = cropTypeFromString(objectIn[SFP(HStr_Key_Id)]);
    const char *cropStr = objectIn[SFP(HStr_Key_CropName)];
    if (cropStr && cropStr[0]) { strncpy(cropName, cropStr, HYDRO_NAME_MAXSIZE); }

    totalGrowWeeks = objectIn[SFP(HStr_Key_TotalGrowWeeks)] | totalGrowWeeks;
    lifeCycleWeeks = objectIn[SFP(HStr_Key_LifeCycleWeeks)] | lifeCycleWeeks;

    {   HYDRO_SOFT_ASSERT(Hydro_CropPhase_MainCount == 3, SFP(HStr_Err_ImportFailure));
        JsonVariantConst dailyLightHoursVar = objectIn[SFP(HStr_Key_DailyLightHours)];
        commaStringToArray(dailyLightHoursVar, dailyLightHours, 3);
    }

    {   HYDRO_SOFT_ASSERT(Hydro_CropPhase_MainCount == 3, SFP(HStr_Err_ImportFailure));
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
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Invasive))) { setIsInvasive(); }
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Viner))) { setIsViner(); }
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Large))) { setIsLarge(); }
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Perennial))) { setIsPerennial(); }
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Toxic))) { setIsToxicToPets(); }
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Pruning))) { setNeedsPrunning(); }
                if (flagStr.equalsIgnoreCase(SFP(HStr_Key_Spraying))) { setNeedsSpraying(); }
            }
        } else if (!flagsVar.isNull()) {
            String flagsString = String(',') + objectIn[SFP(HStr_Key_Flags)].as<String>() + String(',');
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Invasive) + String(','))) { setIsInvasive(); }
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Viner) + String(','))) { setIsViner(); }
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Large) + String(','))) { setIsLarge(); }
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Perennial) + String(','))) { setIsPerennial(); }
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Toxic) + String(','))) { setIsToxicToPets(); }
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Pruning) + String(','))) { setNeedsPrunning(); }
            if (occurrencesInStringIgnoreCase(flagsString, String(',') + SFP(HStr_Key_Spraying) + String(','))) { setNeedsSpraying(); }
        }
    }
}

HydroCustomAdditiveData::HydroCustomAdditiveData()
    : HydroData('H','A','D','D', 1), additiveName{0}, weeklyDosingRates{1}
{
    HYDRO_HARD_ASSERT(isCalibrationData(), SFP(HStr_Err_OperationFailure));
    _size = sizeof(*this);
}

HydroCustomAdditiveData::HydroCustomAdditiveData(Hydro_ReservoirType reservoirType)
    : HydroData('H','A','D','D', 1), additiveName{0}, weeklyDosingRates{1}
{
    _size = sizeof(*this);
    HYDRO_HARD_ASSERT(isCalibrationData(), SFP(HStr_Err_OperationFailure));

    {   auto additiveData = getHydroInstance() ? getHydroInstance()->getCustomAdditiveData(reservoirType) : nullptr;
        if (additiveData && this != additiveData) {
            *this = *additiveData;
        }
    }
}

void HydroCustomAdditiveData::toJSONObject(JsonObject &objectOut) const
{
    HydroData::toJSONObject(objectOut);

    objectOut[SFP(HStr_Key_Id)] = reservoirTypeToString(reservoirType);
    if (additiveName[0]) { objectOut[SFP(HStr_Key_AdditiveName)] = charsToString(additiveName, HYDRO_NAME_MAXSIZE); }
    bool hasWeeklyDosings = arrayElementsEqual(weeklyDosingRates, HYDRO_CROPS_GROWWEEKS_MAX, 0.0f);
    if (hasWeeklyDosings) { objectOut[SFP(HStr_Key_WeeklyDosingRates)] = commaStringFromArray(weeklyDosingRates, HYDRO_CROPS_GROWWEEKS_MAX); }
}

void HydroCustomAdditiveData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroData::fromJSONObject(objectIn);

    reservoirType = reservoirTypeFromString(objectIn[SFP(HStr_Key_Id)]);
    const char *additiveStr = objectIn[SFP(HStr_Key_AdditiveName)];
    if (additiveStr && additiveStr[0]) { strncpy(additiveName, additiveStr, HYDRO_NAME_MAXSIZE); }
    JsonVariantConst weeklyDosingRatesVar = objectIn[SFP(HStr_Key_WeeklyDosingRates)];
    commaStringToArray(weeklyDosingRatesVar, weeklyDosingRates, HYDRO_CROPS_GROWWEEKS_MAX);
}
