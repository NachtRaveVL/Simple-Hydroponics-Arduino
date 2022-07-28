/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Factory
*/

#include "Hydroponics.h"

shared_ptr<HydroponicsRelayActuator> HydroponicsFactory::addGrowLightsRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_GrowLights));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, SFP(HS_Err_InvalidPinOrType));

    if (outputPinIsDigital && positionIndex != -1) {
        auto actuator = make_shared<HydroponicsRelayActuator>(
            Hydroponics_ActuatorType_GrowLights,
            positionIndex,
            outputPin
        );
        if (getHydroponicsInstance()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

shared_ptr<HydroponicsPumpRelayActuator> HydroponicsFactory::addWaterPumpRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_WaterPump));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (outputPinIsDigital && positionIndex != -1) {
        auto actuator = make_shared<HydroponicsPumpRelayActuator>(
            Hydroponics_ActuatorType_WaterPump,
            positionIndex,
            outputPin
        );
        if (getHydroponicsInstance()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

shared_ptr<HydroponicsRelayActuator> HydroponicsFactory::addWaterHeaterRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_WaterHeater));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (outputPinIsDigital && positionIndex != -1) {
        auto actuator = make_shared<HydroponicsRelayActuator>(
            Hydroponics_ActuatorType_WaterHeater,
            positionIndex,
            outputPin
        );
        if (getHydroponicsInstance()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

shared_ptr<HydroponicsRelayActuator> HydroponicsFactory::addWaterSprayerRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_WaterSprayer));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (outputPinIsDigital && positionIndex != -1) {
        auto actuator = make_shared<HydroponicsRelayActuator>(
            Hydroponics_ActuatorType_WaterSprayer,
            positionIndex,
            outputPin
        );
        if (getHydroponicsInstance()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

shared_ptr<HydroponicsRelayActuator> HydroponicsFactory::addWaterAeratorRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_WaterAerator));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (outputPinIsDigital && positionIndex != -1) {
        auto actuator = make_shared<HydroponicsRelayActuator>(
            Hydroponics_ActuatorType_WaterAerator,
            positionIndex,
            outputPin
        );
        if (getHydroponicsInstance()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

shared_ptr<HydroponicsRelayActuator> HydroponicsFactory::addFanExhaustRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_FanExhaust));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (outputPinIsDigital && positionIndex != -1) {
        auto actuator = make_shared<HydroponicsRelayActuator>(
            Hydroponics_ActuatorType_FanExhaust,
            positionIndex,
            outputPin
        );
        if (getHydroponicsInstance()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

shared_ptr<HydroponicsPWMActuator> HydroponicsFactory::addAnalogPWMFanExhaust(byte outputPin, byte outputBitRes)
{
    bool outputPinIsPWM = checkPinIsPWMOutput(outputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_FanExhaust));
    HYDRUINO_HARD_ASSERT(outputPinIsPWM, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (outputPinIsPWM && positionIndex != -1) {
        auto actuator = make_shared<HydroponicsPWMActuator>(
            Hydroponics_ActuatorType_FanExhaust,
            positionIndex,
            outputPin, outputBitRes
        );
        if (getHydroponicsInstance()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

shared_ptr<HydroponicsPumpRelayActuator> HydroponicsFactory::addPeristalticPumpRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_PeristalticPump));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (outputPinIsDigital && positionIndex != -1) {
        auto actuator = make_shared<HydroponicsPumpRelayActuator>(
            Hydroponics_ActuatorType_PeristalticPump,
            positionIndex,
            outputPin
        );
        if (getHydroponicsInstance()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

shared_ptr<HydroponicsBinarySensor> HydroponicsFactory::addLevelIndicator(byte inputPin)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterLevelIndicator));
    HYDRUINO_HARD_ASSERT(inputPinIsDigital, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (inputPinIsDigital && positionIndex != -1) {
        auto sensor = make_shared<HydroponicsBinarySensor>(
            Hydroponics_SensorType_WaterLevelIndicator,
            positionIndex,
            inputPin
        );
        if (getHydroponicsInstance()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> HydroponicsFactory::addAnalogPhMeter(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_PotentialHydrogen));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && positionIndex != -1) {
        auto sensor = make_shared<HydroponicsAnalogSensor>(
            Hydroponics_SensorType_PotentialHydrogen,
            positionIndex,
            inputPin, inputBitRes
        );
        if (getHydroponicsInstance()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> HydroponicsFactory::addAnalogTDSElectrode(byte inputPin, int ppmScale, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_TotalDissolvedSolids));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && positionIndex != -1) {
        auto sensor = make_shared<HydroponicsAnalogSensor>(
            Hydroponics_SensorType_TotalDissolvedSolids,
            positionIndex,
            inputPin, inputBitRes
        );
        if (getHydroponicsInstance()->registerObject(sensor)) {
            if (ppmScale != 500) {
                HydroponicsCalibrationData userCalibData(sensor->getId());
                userCalibData.setFromScale(ppmScale / 500.0f);
                userCalibData.calibUnits = Hydroponics_UnitsType_Concentration_EC;
                sensor->setUserCalibrationData(&userCalibData);
            }

            return sensor;
        }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> HydroponicsFactory::addAnalogTemperatureSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterTemperature));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && positionIndex != -1) {
        auto sensor = make_shared<HydroponicsAnalogSensor>(
            Hydroponics_SensorType_WaterTemperature,
            positionIndex,
            inputPin, inputBitRes
        );
        if (getHydroponicsInstance()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> HydroponicsFactory::addAnalogCO2Sensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_AirCarbonDioxide));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && positionIndex != -1) {
        auto sensor = make_shared<HydroponicsAnalogSensor>(
            Hydroponics_SensorType_AirCarbonDioxide,
            positionIndex,
            inputPin, inputBitRes, true
        );
        if (getHydroponicsInstance()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> HydroponicsFactory::addAnalogMoistureSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_SoilMoisture));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && positionIndex != -1) {
        auto sensor = make_shared<HydroponicsAnalogSensor>(
            Hydroponics_SensorType_SoilMoisture,
            positionIndex,
            inputPin, inputBitRes, true
        );
        if (getHydroponicsInstance()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> HydroponicsFactory::addAnalogPWMPumpFlowSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterPumpFlowSensor));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && positionIndex != -1) {
        auto sensor = make_shared<HydroponicsAnalogSensor>(
            Hydroponics_SensorType_WaterPumpFlowSensor,
            positionIndex,
            inputPin, inputBitRes
        );
        if (getHydroponicsInstance()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> HydroponicsFactory::addAnalogWaterHeightMeter(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterHeightMeter));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && positionIndex != -1) {
        auto sensor = make_shared<HydroponicsAnalogSensor>(
            Hydroponics_SensorType_WaterHeightMeter,
            positionIndex,
            inputPin, inputBitRes
        );
        if (getHydroponicsInstance()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> HydroponicsFactory::addUltrasonicDistanceSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterHeightMeter));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && positionIndex != -1) {
        auto sensor = make_shared<HydroponicsAnalogSensor>(
            Hydroponics_SensorType_WaterHeightMeter,
            positionIndex,
            inputPin, inputBitRes, true
        );
        if (getHydroponicsInstance()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> HydroponicsFactory::addPowerUsageMeter(byte inputPin, bool isWattageBased, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_PowerUsageMeter));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (inputPinIsAnalog && positionIndex != -1) {
        auto sensor = make_shared<HydroponicsAnalogSensor>(
            Hydroponics_SensorType_PowerUsageMeter,
            positionIndex,
            inputPin, inputBitRes
        );
        if (getHydroponicsInstance()->registerObject(sensor)) {
            if (!isWattageBased) { sensor->setMeasurementUnits(Hydroponics_UnitsType_Power_Amperage); }
            return sensor;
        }
    }

    return nullptr;
}

shared_ptr<HydroponicsDHTTempHumiditySensor> HydroponicsFactory::addDHTTempHumiditySensor(byte inputPin, byte dhtType)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_AirTempHumidity));
    HYDRUINO_HARD_ASSERT(inputPinIsDigital, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (inputPinIsDigital && positionIndex != -1) {
        auto sensor = make_shared<HydroponicsDHTTempHumiditySensor>(
            positionIndex,
            inputPin,
            dhtType
        );
        if (getHydroponicsInstance()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

shared_ptr<HydroponicsDSTemperatureSensor> HydroponicsFactory::addDSTemperatureSensor(byte inputPin, byte inputBitRes, byte pullupPin)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterTemperature));
    HYDRUINO_HARD_ASSERT(inputPinIsDigital, SFP(HS_Err_InvalidPinOrType));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (inputPinIsDigital && positionIndex != -1) {
        auto sensor = make_shared<HydroponicsDSTemperatureSensor>(
            positionIndex,
            inputPin, inputBitRes,
            pullupPin
        );
        if (getHydroponicsInstance()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

shared_ptr<HydroponicsTimedCrop> HydroponicsFactory::addTimerFedCrop(Hydroponics_CropType cropType, Hydroponics_SubstrateType substrateType, DateTime sowDate, byte minsOn, byte minsOff)
{
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(cropType));
    HYDRUINO_SOFT_ASSERT((int)cropType >= 0 && cropType <= Hydroponics_CropType_Count, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT((int)substrateType >= 0 && substrateType <= Hydroponics_SubstrateType_Count, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(sowDate.unixtime(), SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if ((int)cropType >= 0 && cropType < Hydroponics_CropType_Count && sowDate.unixtime() && positionIndex != -1) {
        auto crop = make_shared<HydroponicsTimedCrop>(
            cropType,
            positionIndex,
            substrateType,
            sowDate,
            TimeSpan(0,0,minsOn,0),
            TimeSpan(0,0,minsOff,0)
        );
        if (getHydroponicsInstance()->registerObject(crop)) { return crop; }
    }

    return nullptr;
}

shared_ptr<HydroponicsTimedCrop> HydroponicsFactory::addTimerFedPerennialCrop(Hydroponics_CropType cropType, Hydroponics_SubstrateType substrateType, DateTime lastHarvestDate, byte minsOn, byte minsOff)
{
    auto cropData = getCropsLibraryInstance()->checkoutCropsData(cropType);
    time_t sowDate = lastHarvestDate.unixtime() - (cropData->totalGrowWeeks * SECS_PER_WEEK);
    auto crop = addTimerFedCrop(cropType, substrateType, DateTime((uint32_t)sowDate), minsOn, minsOff);
    getCropsLibraryInstance()->returnCropsData(cropData);
    return crop;
}

shared_ptr<HydroponicsAdaptiveCrop> HydroponicsFactory::addAdaptiveFedCrop(Hydroponics_CropType cropType, Hydroponics_SubstrateType substrateType, DateTime sowDate)
{
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(cropType));
    HYDRUINO_SOFT_ASSERT((int)cropType >= 0 && cropType <= Hydroponics_CropType_Count, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT((int)substrateType >= 0 && substrateType <= Hydroponics_SubstrateType_Count, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(sowDate.unixtime(), SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if ((int)cropType >= 0 && cropType < Hydroponics_CropType_Count && sowDate.unixtime() && positionIndex != -1) {
        auto crop = make_shared<HydroponicsAdaptiveCrop>(
            cropType,
            positionIndex,
            substrateType,
            sowDate
        );
        if (getHydroponicsInstance()->registerObject(crop)) { return crop; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAdaptiveCrop> HydroponicsFactory::addAdaptiveFedPerennialCrop(Hydroponics_CropType cropType, Hydroponics_SubstrateType substrateType, DateTime lastHarvestDate)
{
    auto cropData = getCropsLibraryInstance()->checkoutCropsData(cropType);
    time_t sowDate = lastHarvestDate.unixtime() - (cropData->totalGrowWeeks * SECS_PER_WEEK);
    auto crop = addAdaptiveFedCrop(cropType, substrateType, DateTime((uint32_t)sowDate));
    getCropsLibraryInstance()->returnCropsData(cropData);
    return crop;
}

shared_ptr<HydroponicsFluidReservoir> HydroponicsFactory::addFluidReservoir(Hydroponics_ReservoirType reservoirType, float maxVolume, bool beginFilled)
{
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(reservoirType));
    HYDRUINO_SOFT_ASSERT((int)reservoirType >= 0 && reservoirType <= Hydroponics_ReservoirType_Count, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(maxVolume > FLT_EPSILON, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if ((int)reservoirType >= 0 && reservoirType < Hydroponics_ReservoirType_Count && maxVolume > FLT_EPSILON && positionIndex != -1) {
        auto reservoir = make_shared<HydroponicsFluidReservoir>(
            reservoirType,
            positionIndex,
            maxVolume
        );
        if (getHydroponicsInstance()->registerObject(reservoir)) {
            if (beginFilled) { reservoir->getWaterVolume().setMeasurement(reservoir->getMaxVolume()); }
            return reservoir;
        }
    }

    return nullptr;
}

shared_ptr<HydroponicsFeedReservoir> HydroponicsFactory::addFeedWaterReservoir(float maxVolume, bool beginFilled, DateTime lastChangeDate, DateTime lastPruningDate)
{
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_ReservoirType_FeedWater));
    HYDRUINO_SOFT_ASSERT(maxVolume > FLT_EPSILON, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(lastChangeDate.unixtime(), SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (maxVolume > FLT_EPSILON && positionIndex != -1) {
        auto reservoir = make_shared<HydroponicsFeedReservoir>(
            positionIndex,
            maxVolume,
            lastChangeDate,
            lastPruningDate
        );
        if (getHydroponicsInstance()->registerObject(reservoir)) {
            if (beginFilled) { reservoir->getWaterVolume().setMeasurement(reservoir->getMaxVolume() * HYDRUINO_FEEDRES_FRACTION_FILLED); }
            return reservoir;
        }
    }

    return nullptr;
}

shared_ptr<HydroponicsInfiniteReservoir> HydroponicsFactory::addDrainagePipe()
{
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_ReservoirType_DrainageWater));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (positionIndex != -1) {
        auto reservoir = make_shared<HydroponicsInfiniteReservoir>(
            Hydroponics_ReservoirType_DrainageWater,
            positionIndex,
            false
        );
        if (getHydroponicsInstance()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

shared_ptr<HydroponicsInfiniteReservoir> HydroponicsFactory::addFreshWaterMain()
{
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(Hydroponics_ReservoirType_FreshWater));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if (positionIndex != -1) {
        auto reservoir = make_shared<HydroponicsInfiniteReservoir>(
            Hydroponics_ReservoirType_FreshWater,
            positionIndex,
            true
        );
        if (getHydroponicsInstance()->registerObject(reservoir)) { return reservoir; }
    }

    return nullptr;
}

shared_ptr<HydroponicsSimpleRail> HydroponicsFactory::addSimplePowerRail(Hydroponics_RailType railType, int maxActiveAtOnce)
{
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(railType));
    HYDRUINO_SOFT_ASSERT((int)railType >= 0 && railType <= Hydroponics_RailType_Count, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(maxActiveAtOnce > 0, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if ((int)railType >= 0 && railType < Hydroponics_RailType_Count && maxActiveAtOnce > 0 && positionIndex != -1) {
        auto rail = make_shared<HydroponicsSimpleRail>(
            railType,
            positionIndex,
            maxActiveAtOnce
        );
        if (getHydroponicsInstance()->registerObject(rail)) { return rail; }
    }

    return nullptr;
}

shared_ptr<HydroponicsRegulatedRail> HydroponicsFactory::addRegulatedPowerRail(Hydroponics_RailType railType, float maxPower)
{
    Hydroponics_PositionIndex positionIndex = getHydroponicsInstance()->firstPositionOpen(HydroponicsIdentity(railType));
    HYDRUINO_SOFT_ASSERT((int)railType >= 0 && railType <= Hydroponics_RailType_Count, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(maxPower > FLT_EPSILON, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, SFP(HS_Err_NoPositionsAvailable));

    if ((int)railType >= 0 && railType < Hydroponics_RailType_Count && maxPower > FLT_EPSILON && positionIndex != -1) {
        auto rail = make_shared<HydroponicsRegulatedRail>(
            railType,
            positionIndex,
            maxPower
        );
        if (getHydroponicsInstance()->registerObject(rail)) { return rail; }
    }

    return nullptr;
}
