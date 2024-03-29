/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Factory
*/

#include "Hydruino.h"

SharedPtr<HydroRelayActuator> HydroFactory::addGrowLightsRelay(pintype_t outputPin, int8_t pinChannel)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_ActuatorType_GrowLights));
    HYDRO_HARD_ASSERT(outputPinIsDigital, SFP(HStr_Err_InvalidPinOrType));

    if (outputPinIsDigital && isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<HydroRelayActuator>(new HydroRelayActuator(
            Hydro_ActuatorType_GrowLights,
            positionIndex,
            HydroDigitalPin(outputPin, OUTPUT, pinChannel)
        ));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<HydroRelayPumpActuator> HydroFactory::addWaterPumpRelay(pintype_t outputPin, int8_t pinChannel)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_ActuatorType_WaterPump));
    HYDRO_HARD_ASSERT(outputPinIsDigital, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (outputPinIsDigital && isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<HydroRelayPumpActuator>(new HydroRelayPumpActuator(
            Hydro_ActuatorType_WaterPump,
            positionIndex,
            HydroDigitalPin(outputPin, OUTPUT, pinChannel)
        ));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<HydroRelayActuator> HydroFactory::addWaterHeaterRelay(pintype_t outputPin, int8_t pinChannel)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_ActuatorType_WaterHeater));
    HYDRO_HARD_ASSERT(outputPinIsDigital, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (outputPinIsDigital && isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<HydroRelayActuator>(new HydroRelayActuator(
            Hydro_ActuatorType_WaterHeater,
            positionIndex,
            HydroDigitalPin(outputPin, OUTPUT, pinChannel)
        ));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<HydroRelayActuator> HydroFactory::addWaterSprayerRelay(pintype_t outputPin, int8_t pinChannel)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_ActuatorType_WaterSprayer));
    HYDRO_HARD_ASSERT(outputPinIsDigital, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (outputPinIsDigital && isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<HydroRelayActuator>(new HydroRelayActuator(
            Hydro_ActuatorType_WaterSprayer,
            positionIndex,
            HydroDigitalPin(outputPin, OUTPUT, pinChannel)
        ));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<HydroRelayActuator> HydroFactory::addWaterAeratorRelay(pintype_t outputPin, int8_t pinChannel)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_ActuatorType_WaterAerator));
    HYDRO_HARD_ASSERT(outputPinIsDigital, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (outputPinIsDigital && isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<HydroRelayActuator>(new HydroRelayActuator(
            Hydro_ActuatorType_WaterAerator,
            positionIndex,
            HydroDigitalPin(outputPin, OUTPUT, pinChannel)
        ));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<HydroRelayActuator> HydroFactory::addFanExhaustRelay(pintype_t outputPin, int8_t pinChannel)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_ActuatorType_FanExhaust));
    HYDRO_HARD_ASSERT(outputPinIsDigital, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (outputPinIsDigital && isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<HydroRelayActuator>(new HydroRelayActuator(
            Hydro_ActuatorType_FanExhaust,
            positionIndex,
            HydroDigitalPin(outputPin, OUTPUT, pinChannel)
        ));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<HydroVariableActuator> HydroFactory::addAnalogFanExhaust(pintype_t outputPin, uint8_t outputBitRes,
#ifdef ESP32
                                                                   uint8_t pwmChannel,
#endif
#ifdef ESP_PLATFORM
                                                                   float pwmFrequency,
#endif
                                                                   int8_t pinChannel)
{
    bool outputPinIsPWM = checkPinIsPWMOutput(outputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_ActuatorType_FanExhaust));
    HYDRO_HARD_ASSERT(outputPinIsPWM, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (outputPinIsPWM && isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<HydroVariableActuator>(new HydroVariableActuator(
            Hydro_ActuatorType_FanExhaust,
            positionIndex,
            HydroAnalogPin(outputPin, OUTPUT, outputBitRes,
#ifdef ESP32
                           pwmChannel,
#endif
#ifdef ESP_PLATFORM
                           pwmFrequency,
#endif
                           pinChannel)));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<HydroRelayPumpActuator> HydroFactory::addPeristalticPumpRelay(pintype_t outputPin, int8_t pinChannel)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_ActuatorType_PeristalticPump));
    HYDRO_HARD_ASSERT(outputPinIsDigital, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (outputPinIsDigital && isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<HydroRelayPumpActuator>(new HydroRelayPumpActuator(
            Hydro_ActuatorType_PeristalticPump,
            positionIndex,
            HydroDigitalPin(outputPin, OUTPUT, pinChannel)
        ));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<HydroBinarySensor> HydroFactory::addLevelIndicator(pintype_t inputPin, bool isActiveLow, int8_t pinChannel)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_SensorType_WaterLevel));
    HYDRO_HARD_ASSERT(inputPinIsDigital, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (inputPinIsDigital && isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<HydroBinarySensor>(new HydroBinarySensor(
            Hydro_SensorType_WaterLevel,
            positionIndex,
            HydroDigitalPin(inputPin, INPUT, isActiveLow, pinChannel)
        ));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<HydroAnalogSensor> HydroFactory::addAnalogPhMeter(pintype_t inputPin, uint8_t inputBitRes, int8_t pinChannel)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_SensorType_PotentialHydrogen));
    HYDRO_HARD_ASSERT(inputPinIsAnalog, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<HydroAnalogSensor>(new HydroAnalogSensor(
            Hydro_SensorType_PotentialHydrogen,
            positionIndex,
            HydroAnalogPin(inputPin, INPUT, inputBitRes, pinChannel)
        ));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<HydroAnalogSensor> HydroFactory::addAnalogTDSElectrode(pintype_t inputPin, uint8_t inputBitRes, int8_t pinChannel)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_SensorType_TotalDissolvedSolids));
    HYDRO_HARD_ASSERT(inputPinIsAnalog, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<HydroAnalogSensor>(new HydroAnalogSensor(
            Hydro_SensorType_TotalDissolvedSolids,
            positionIndex,
            HydroAnalogPin(inputPin, INPUT, inputBitRes, pinChannel)
        ));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<HydroAnalogSensor> HydroFactory::addAnalogTemperatureSensor(pintype_t inputPin, uint8_t inputBitRes, int8_t pinChannel)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_SensorType_WaterTemperature));
    HYDRO_HARD_ASSERT(inputPinIsAnalog, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<HydroAnalogSensor>(new HydroAnalogSensor(
            Hydro_SensorType_WaterTemperature,
            positionIndex,
            HydroAnalogPin(inputPin, INPUT, inputBitRes, pinChannel)
        ));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<HydroAnalogSensor> HydroFactory::addAnalogCO2Sensor(pintype_t inputPin, int ppmScale, uint8_t inputBitRes, int8_t pinChannel)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_SensorType_AirCarbonDioxide));
    HYDRO_HARD_ASSERT(inputPinIsAnalog, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<HydroAnalogSensor>(new HydroAnalogSensor(
            Hydro_SensorType_AirCarbonDioxide,
            positionIndex,
            HydroAnalogPin(inputPin, INPUT, inputBitRes, pinChannel),
            true
        ));
        if (getController()->registerObject(sensor)) {
            if (ppmScale == 500) {
                sensor->setMeasurementUnits(Hydro_UnitsType_Concentration_PPM_500);
            } else if (ppmScale == 640) {
                sensor->setMeasurementUnits(Hydro_UnitsType_Concentration_PPM_640);
            } else if (ppmScale == 700) {
                sensor->setMeasurementUnits(Hydro_UnitsType_Concentration_PPM_700);
            } else {
                HydroCalibrationData userCalibData(sensor->getId(), Hydro_UnitsType_Concentration_EC_5);
                userCalibData.setFromScale(ppmScale / 500.0f);
                sensor->setUserCalibrationData(&userCalibData);
            }

            return sensor;
        }
    }

    return nullptr;
}

SharedPtr<HydroAnalogSensor> HydroFactory::addAnalogMoistureSensor(pintype_t inputPin, uint8_t inputBitRes, int8_t pinChannel)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_SensorType_SoilMoisture));
    HYDRO_HARD_ASSERT(inputPinIsAnalog, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<HydroAnalogSensor>(new HydroAnalogSensor(
            Hydro_SensorType_SoilMoisture,
            positionIndex,
            HydroAnalogPin(inputPin, INPUT, inputBitRes, pinChannel),
            true
        ));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<HydroAnalogSensor> HydroFactory::addAnalogPumpFlowSensor(pintype_t inputPin, uint8_t inputBitRes, int8_t pinChannel)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_SensorType_PumpFlow));
    HYDRO_HARD_ASSERT(inputPinIsAnalog, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<HydroAnalogSensor>(new HydroAnalogSensor(
            Hydro_SensorType_PumpFlow,
            positionIndex,
            HydroAnalogPin(inputPin, INPUT, inputBitRes, pinChannel)
        ));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<HydroAnalogSensor> HydroFactory::addAnalogWaterHeightMeter(pintype_t inputPin, uint8_t inputBitRes, int8_t pinChannel)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_SensorType_WaterHeight));
    HYDRO_HARD_ASSERT(inputPinIsAnalog, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<HydroAnalogSensor>(new HydroAnalogSensor(
            Hydro_SensorType_WaterHeight,
            positionIndex,
            HydroAnalogPin(inputPin, INPUT, inputBitRes, pinChannel)
        ));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<HydroAnalogSensor> HydroFactory::addUltrasonicDistanceSensor(pintype_t inputPin, uint8_t inputBitRes, int8_t pinChannel)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_SensorType_WaterHeight));
    HYDRO_HARD_ASSERT(inputPinIsAnalog, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<HydroAnalogSensor>(new HydroAnalogSensor(
            Hydro_SensorType_WaterHeight,
            positionIndex,
            HydroAnalogPin(inputPin, INPUT, inputBitRes, pinChannel),
            true
        ));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<HydroAnalogSensor> HydroFactory::addPowerLevelMeter(pintype_t inputPin, bool isWattageBased, uint8_t inputBitRes, int8_t pinChannel)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_SensorType_PowerLevel));
    HYDRO_HARD_ASSERT(inputPinIsAnalog, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<HydroAnalogSensor>(new HydroAnalogSensor(
            Hydro_SensorType_PowerLevel,
            positionIndex,
            HydroAnalogPin(inputPin, INPUT, inputBitRes, pinChannel)
        ));
        if (getController()->registerObject(sensor)) {
            if (!isWattageBased) { sensor->setMeasurementUnits(Hydro_UnitsType_Power_Amperage); }
            return sensor;
        }
    }

    return nullptr;
}

SharedPtr<HydroDHTTempHumiditySensor> HydroFactory::addDHTTempHumiditySensor(pintype_t inputPin, Hydro_DHTType dhtType)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_SensorType_AirTempHumidity));
    HYDRO_HARD_ASSERT(inputPinIsDigital, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (inputPinIsDigital && isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<HydroDHTTempHumiditySensor>(new HydroDHTTempHumiditySensor(
            positionIndex,
            HydroDigitalPin(inputPin, INPUT_PULLUP),
            dhtType
        ));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<HydroDSTemperatureSensor> HydroFactory::addDSTemperatureSensor(pintype_t inputPin, uint8_t inputBitRes, pintype_t pullupPin)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_SensorType_WaterTemperature));
    HYDRO_HARD_ASSERT(inputPinIsDigital, SFP(HStr_Err_InvalidPinOrType));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (inputPinIsDigital && isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<HydroDSTemperatureSensor>(new HydroDSTemperatureSensor(
            positionIndex,
            HydroDigitalPin(inputPin, INPUT_PULLUP), inputBitRes,
            HydroDigitalPin(pullupPin, INPUT_PULLUP)
        ));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<HydroTimedCrop> HydroFactory::addTimerFedCrop(Hydro_CropType cropType, Hydro_SubstrateType substrateType, DateTime sowTime, uint8_t minsOn, uint8_t minsOff)
{
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(cropType));
    HYDRO_SOFT_ASSERT((int)cropType >= 0 && cropType <= Hydro_CropType_Count, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT((int)substrateType >= 0 && substrateType <= Hydro_SubstrateType_Count, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if ((int)cropType >= 0 && cropType < Hydro_CropType_Count && (int)substrateType >= 0 && substrateType <= Hydro_SubstrateType_Count && isValidIndex(positionIndex)) {
        auto crop = SharedPtr<HydroTimedCrop>(new HydroTimedCrop(
            cropType,
            positionIndex,
            substrateType,
            sowTime,
            TimeSpan(0,0,minsOn,0),
            TimeSpan(0,0,minsOff,0)
        ));
        if (getController()->registerObject(crop)) { return crop; }
    }

    return nullptr;
}

SharedPtr<HydroTimedCrop> HydroFactory::addTimerFedCropByHarvest(Hydro_CropType cropType, Hydro_SubstrateType substrateType, DateTime harvestTime, uint8_t minsOn, uint8_t minsOff)
{
    auto cropData = hydroCropsLib.checkoutCropsData(cropType);
    DateTime sowTime = harvestTime - TimeSpan(cropData->totalGrowWeeks * SECS_PER_WEEK);
    sowTime = DateTime(localNow().year(), sowTime.month(), sowTime.day());
    auto crop = addTimerFedCrop(cropType, substrateType, sowTime, minsOn, minsOff);
    hydroCropsLib.returnCropsData(cropData);
    return crop;
}

SharedPtr<HydroAdaptiveCrop> HydroFactory::addAdaptiveFedCrop(Hydro_CropType cropType, Hydro_SubstrateType substrateType, DateTime sowTime)
{
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(cropType));
    HYDRO_SOFT_ASSERT((int)cropType >= 0 && cropType <= Hydro_CropType_Count, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT((int)substrateType >= 0 && substrateType <= Hydro_SubstrateType_Count, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if ((int)cropType >= 0 && cropType < Hydro_CropType_Count && (int)substrateType >= 0 && substrateType <= Hydro_SubstrateType_Count && isValidIndex(positionIndex)) {
        auto crop = SharedPtr<HydroAdaptiveCrop>(new HydroAdaptiveCrop(
            cropType,
            positionIndex,
            substrateType,
            sowTime
        ));
        if (getController()->registerObject(crop)) { return crop; }
    }

    return nullptr;
}

SharedPtr<HydroAdaptiveCrop> HydroFactory::addAdaptiveFedCropByHarvest(Hydro_CropType cropType, Hydro_SubstrateType substrateType, DateTime harvestTime)
{
    auto cropData = hydroCropsLib.checkoutCropsData(cropType);
    DateTime sowTime = harvestTime - TimeSpan(cropData->totalGrowWeeks * SECS_PER_WEEK);
    sowTime = DateTime(localNow().year(), sowTime.month(), sowTime.day());
    auto crop = addAdaptiveFedCrop(cropType, substrateType, sowTime);
    hydroCropsLib.returnCropsData(cropData);
    return crop;
}

SharedPtr<HydroFluidReservoir> HydroFactory::addFluidReservoir(Hydro_ReservoirType reservoirType, float maxVolume, bool beginFilled)
{
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(reservoirType));
    HYDRO_SOFT_ASSERT((int)reservoirType >= 0 && reservoirType <= Hydro_ReservoirType_Count, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT(maxVolume > FLT_EPSILON, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if ((int)reservoirType >= 0 && reservoirType < Hydro_ReservoirType_Count && maxVolume > FLT_EPSILON && isValidIndex(positionIndex)) {
        auto reservoir = SharedPtr<HydroFluidReservoir>(new HydroFluidReservoir(
            reservoirType,
            positionIndex,
            maxVolume
        ));
        if (getController()->registerObject(reservoir)) {
            if (beginFilled) { reservoir->getWaterVolumeSensorAttachment().setMeasurement(reservoir->getMaxVolume()); }
            return reservoir;
        }
    }

    return nullptr;
}

SharedPtr<HydroFeedReservoir> HydroFactory::addFeedWaterReservoir(float maxVolume, bool beginFilled, DateTime lastChangeTime, DateTime lastPruningTime)
{
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_ReservoirType_FeedWater));
    HYDRO_SOFT_ASSERT(maxVolume > FLT_EPSILON, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (maxVolume > FLT_EPSILON && isValidIndex(positionIndex)) {
        auto reservoir = SharedPtr<HydroFeedReservoir>(new HydroFeedReservoir(
            positionIndex,
            maxVolume,
            lastChangeTime,
            lastPruningTime
        ));
        if (getController()->registerObject(reservoir)) {
            if (beginFilled) { reservoir->getWaterVolumeSensorAttachment().setMeasurement(reservoir->getMaxVolume() * HYDRO_FEEDRES_FRACTION_FILLED); }
            return reservoir;
        }
    }

    return nullptr;
}

SharedPtr<HydroInfiniteReservoir> HydroFactory::addDrainagePipe()
{
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_ReservoirType_DrainageWater));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto reservoir = SharedPtr<HydroInfiniteReservoir>(new HydroInfiniteReservoir(
            Hydro_ReservoirType_DrainageWater,
            positionIndex,
            false
        ));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<HydroInfiniteReservoir> HydroFactory::addFreshWaterMain()
{
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(Hydro_ReservoirType_FreshWater));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto reservoir = SharedPtr<HydroInfiniteReservoir>(new HydroInfiniteReservoir(
            Hydro_ReservoirType_FreshWater,
            positionIndex,
            true
        ));
        if (getController()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

SharedPtr<HydroSimpleRail> HydroFactory::addSimplePowerRail(Hydro_RailType railType, int maxActiveAtOnce)
{
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(railType));
    HYDRO_SOFT_ASSERT((int)railType >= 0 && railType <= Hydro_RailType_Count, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT(maxActiveAtOnce > 0, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if ((int)railType >= 0 && railType < Hydro_RailType_Count && maxActiveAtOnce > 0 && isValidIndex(positionIndex)) {
        auto rail = SharedPtr<HydroSimpleRail>(new HydroSimpleRail(
            railType,
            positionIndex,
            maxActiveAtOnce
        ));
        if (getController()->registerObject(rail)) { return rail; }
    }

    return nullptr;
}

SharedPtr<HydroRegulatedRail> HydroFactory::addRegulatedPowerRail(Hydro_RailType railType, float maxPower)
{
    hposi_t positionIndex = getController()->firstPositionOpen(HydroIdentity(railType));
    HYDRO_SOFT_ASSERT((int)railType >= 0 && railType <= Hydro_RailType_Count, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT(maxPower > FLT_EPSILON, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(HStr_Err_NoPositionsAvailable));

    if ((int)railType >= 0 && railType < Hydro_RailType_Count && maxPower > FLT_EPSILON && isValidIndex(positionIndex)) {
        auto rail = SharedPtr<HydroRegulatedRail>(new HydroRegulatedRail(
            railType,
            positionIndex,
            maxPower
        ));
        if (getController()->registerObject(rail)) { return rail; }
    }

    return nullptr;
}
