/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Attachment Points
*/

#include "Hydruino.h"

inline Hydro_UnitsType HydroDilutionUnitsInterface::getVolumeUnits() const
{
    return baseUnits(getDilutionUnits());
}

inline Hydro_UnitsType HydroFlowRateUnitsInterface::getVolumeUnits() const
{
    return baseUnits(getFlowRateUnits());
}

inline Hydro_UnitsType HydroMeasureUnitsInterface::getRateUnits(uint8_t measureRow) const
{
    return rateUnits(getMeasureUnits(measureRow));
}

inline Hydro_UnitsType HydroMeasureUnitsInterface::getBaseUnits(uint8_t measureRow) const
{
    return baseUnits(getMeasureUnits(measureRow));
}

inline Hydro_UnitsType HydroVolumeUnitsInterface::getFlowRateUnits() const
{
    return rateUnits(_volumeUnits);
}

inline Hydro_UnitsType HydroVolumeUnitsInterface::getDilutionUnits() const
{
    return dilutionUnits(_volumeUnits);
}


inline void HydroActuatorObjectInterface::setContinuousPowerUsage(float contPowerUsage, Hydro_UnitsType contPowerUsageUnits)
{
    setContinuousPowerUsage(HydroSingleMeasurement(contPowerUsage, contPowerUsageUnits));
}


template <class U>
inline void HydroPumpObjectInterface::setInputReservoir(U reservoir)
{
    getParentReservoir().setObject(reservoir);
}

template <class U>
inline SharedPtr<U> HydroPumpObjectInterface::getInputReservoir()
{
    return getParentReservoir().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroPumpObjectInterface::setOutputReservoir(U reservoir)
{
    getDestinationReservoir().setObject(reservoir);
}

template <class U>
inline SharedPtr<U> HydroPumpObjectInterface::getOutputReservoir()
{
    return getDestinationReservoir().HydroAttachment::getObject<U>();
}

inline void HydroPumpObjectInterface::setContinuousFlowRate(float contFlowRate, Hydro_UnitsType contFlowRateUnits)
{
    setContinuousFlowRate(HydroSingleMeasurement(contFlowRate, contFlowRateUnits));
}


template <class U>
inline void HydroParentActuatorAttachmentInterface::setActuator(U actuator)
{
    getParentActuator().setObject(actuator);
}

template <class U>
inline SharedPtr<U> HydroParentActuatorAttachmentInterface::getActuator()
{
    return getParentActuator().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroParentSensorAttachmentInterface::setSensor(U sensor)
{
    getParentSensor().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroParentSensorAttachmentInterface::getSensor()
{
    return getParentSensor().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroParentCropAttachmentInterface::setCrop(U crop)
{
    getParentCrop().setObject(crop);
}

template <class U>
inline SharedPtr<U> HydroParentCropAttachmentInterface::getCrop()
{
    return getParentCrop().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroParentReservoirAttachmentInterface::setReservoir(U reservoir)
{
    getParentReservoir().setObject(reservoir);
}

template <class U>
inline SharedPtr<U> HydroParentReservoirAttachmentInterface::getReservoir()
{
    return getParentReservoir().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroParentRailAttachmentInterface::setRail(U rail)
{
    getParentRail().setObject(rail);
}

template <class U>
inline SharedPtr<U> HydroParentRailAttachmentInterface::getRail()
{
    return getParentRail().HydroAttachment::getObject<U>();
}


template <class U>
inline void HydroFeedReservoirAttachmentInterface::setFeedReservoir(U reservoir)
{
    getFeed().setObject(reservoir);
}

template <class U>
inline SharedPtr<U> HydroFeedReservoirAttachmentInterface::getFeedReservoir()
{
    return getFeed().HydroAttachment::getObject<U>();
}


template <class U>
inline void HydroAirCO2SensorAttachmentInterface::setAirCO2Sensor(U sensor)
{
    getAirCO2().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroAirCO2SensorAttachmentInterface::getAirCO2Sensor(bool poll)
{
    getAirCO2().updateIfNeeded(poll);
    return getAirCO2().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroAirTemperatureSensorAttachmentInterface::setAirTemperatureSensor(U sensor)
{
    getAirTemperature().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroAirTemperatureSensorAttachmentInterface::getAirTemperatureSensor(bool poll)
{
    getAirTemperature().updateIfNeeded(poll);
    return getAirTemperature().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroPowerProductionSensorAttachmentInterface::setPowerProductionSensor(U sensor)
{
    getPowerProduction().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroPowerProductionSensorAttachmentInterface::getPowerProductionSensor(bool poll)
{
    getPowerProduction().updateIfNeeded(poll);
    return getPowerProduction().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroPowerUsageSensorAttachmentInterface::setPowerUsageSensor(U sensor)
{
    getPowerUsage().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroPowerUsageSensorAttachmentInterface::getPowerUsageSensor(bool poll)
{
    getPowerUsage().updateIfNeeded(poll);
    return getPowerUsage().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroSoilMoistureSensorAttachmentInterface::setSoilMoistureSensor(U sensor)
{
    getSoilMoisture().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroSoilMoistureSensorAttachmentInterface::getSoilMoistureSensor(bool poll)
{
    getSoilMoisture().updateIfNeeded(poll);
    return getSoilMoisture().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroWaterFlowRateSensorAttachmentInterface::setFlowRateSensor(U sensor)
{
    getFlowRate().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroWaterFlowRateSensorAttachmentInterface::getFlowRateSensor(bool poll)
{
    getFlowRate().updateIfNeeded(poll);
    return getFlowRate().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroWaterPHSensorAttachmentInterface::setWaterPHSensor(U sensor)
{
    getWaterPH().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroWaterPHSensorAttachmentInterface::getWaterPHSensor(bool poll)
{
    getWaterPH().updateIfNeeded(poll);
    return getWaterPH().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroWaterTDSSensorAttachmentInterface::setWaterTDSSensor(U sensor)
{
    getWaterTDS().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroWaterTDSSensorAttachmentInterface::getWaterTDSSensor(bool poll)
{
    getWaterTDS().updateIfNeeded(poll);
    return getWaterTDS().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroWaterTemperatureSensorAttachmentInterface::setWaterTemperatureSensor(U sensor)
{
    getWaterTemperature().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroWaterTemperatureSensorAttachmentInterface::getWaterTemperatureSensor(bool poll)
{
    getWaterTemperature().updateIfNeeded(poll);
    return getWaterTemperature().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroWaterVolumeSensorAttachmentInterface::setWaterVolumeSensor(U sensor)
{
    getWaterVolume().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroWaterVolumeSensorAttachmentInterface::getWaterVolumeSensor(bool poll)
{
    getWaterVolume().updateIfNeeded(poll);
    return getWaterVolume().HydroAttachment::getObject<U>();
}


template <class U>
inline void HydroFilledTriggerAttachmentInterface::setFilledTrigger(U trigger)
{
    getFilled().setObject(trigger);
}

template <class U>
inline SharedPtr<U> HydroFilledTriggerAttachmentInterface::getFilledTrigger(bool poll)
{
    getFilled().updateIfNeeded(poll);
    return getFilled().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroEmptyTriggerAttachmentInterface::setEmptyTrigger(U trigger)
{
    getEmpty().setObject(trigger);
}

template <class U>
inline SharedPtr<U> HydroEmptyTriggerAttachmentInterface::getEmptyTrigger(bool poll)
{
    getEmpty().updateIfNeeded(poll);
    return getEmpty().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroFeedingTriggerAttachmentInterface::setFeedingTrigger(U trigger)
{
    getFeeding().setObject(trigger);
}

template <class U>
inline SharedPtr<U> HydroFeedingTriggerAttachmentInterface::getFeedingTrigger(bool poll)
{
    getFeeding().updateIfNeeded(poll);
    return getFeeding().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroLimitTriggerAttachmentInterface::setLimitTrigger(U trigger)
{
    getLimit().setObject(trigger);
}

template <class U>
inline SharedPtr<U> HydroLimitTriggerAttachmentInterface::getLimitTrigger(bool poll)
{
    getLimit().updateIfNeeded(poll);
    return getLimit().HydroAttachment::getObject<U>();
}
