/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Attachment Points
*/

#include "Hydruino.h"

inline Hydro_UnitsType HydroDilutionUnitsInterfaceStorage::getVolumeUnits() const
{
    return baseUnits(getDilutionUnits());
}

inline Hydro_UnitsType HydroFlowRateUnitsInterfaceStorage::getVolumeUnits() const
{
    return baseUnits(getFlowRateUnits());
}

inline Hydro_UnitsType HydroMeasurementUnitsInterface::getRateUnits(uint8_t measurementRow) const
{
    return rateUnits(getMeasurementUnits(measurementRow));
}

inline Hydro_UnitsType HydroMeasurementUnitsInterface::getBaseUnits(uint8_t measurementRow) const
{
    return baseUnits(getMeasurementUnits(measurementRow));
}

inline Hydro_UnitsType HydroVolumeUnitsInterfaceStorage::getFlowRateUnits() const
{
    return rateUnits(_volumeUnits);
}

inline Hydro_UnitsType HydroVolumeUnitsInterfaceStorage::getDilutionUnits() const
{
    return dilutionUnits(_volumeUnits);
}


inline void HydroActuatorObjectInterface::setContinuousPowerUsage(float contPowerUsage, Hydro_UnitsType contPowerUsageUnits)
{
    setContinuousPowerUsage(HydroSingleMeasurement(contPowerUsage, contPowerUsageUnits));
}


template <class U>
inline void HydroPumpObjectInterface::setSourceReservoir(U reservoir)
{
    getSourceReservoirAttachment().setObject(reservoir);
}

template <class U>
inline SharedPtr<U> HydroPumpObjectInterface::getSourceReservoir()
{
    return getSourceReservoirAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroPumpObjectInterface::setDestinationReservoir(U reservoir)
{
    getDestinationReservoirAttachment().setObject(reservoir);
}

template <class U>
inline SharedPtr<U> HydroPumpObjectInterface::getDestinationReservoir()
{
    return getDestinationReservoirAttachment().HydroAttachment::getObject<U>();
}

inline void HydroPumpObjectInterface::setContinuousFlowRate(float contFlowRate, Hydro_UnitsType contFlowRateUnits)
{
    setContinuousFlowRate(HydroSingleMeasurement(contFlowRate, contFlowRateUnits));
}

inline bool HydroPumpObjectInterface::isSourceReservoirEmpty(bool poll)
{
    return getSourceReservoir() && getSourceReservoir()->isEmpty(poll);
}

inline bool HydroPumpObjectInterface::isDestinationReservoirFilled(bool poll)
{
    return getDestinationReservoir() && getDestinationReservoir()->isFilled(poll);
}


template <class U>
inline void HydroParentActuatorAttachmentInterface::setParentActuator(U actuator)
{
    getParentActuatorAttachment().setObject(actuator);
}

template <class U>
inline SharedPtr<U> HydroParentActuatorAttachmentInterface::getParentActuator()
{
    return getParentActuatorAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroParentSensorAttachmentInterface::setParentSensor(U sensor)
{
    getParentSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroParentSensorAttachmentInterface::getParentSensor()
{
    return getParentSensorAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroParentCropAttachmentInterface::setParentCrop(U crop)
{
    getParentCropAttachment().setObject(crop);
}

template <class U>
inline SharedPtr<U> HydroParentCropAttachmentInterface::getParentCrop()
{
    return getParentCropAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroParentReservoirAttachmentInterface::setParentReservoir(U reservoir)
{
    getParentReservoirAttachment().setObject(reservoir);
}

template <class U>
inline SharedPtr<U> HydroParentReservoirAttachmentInterface::getParentReservoir()
{
    return getParentReservoirAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroParentRailAttachmentInterface::setParentRail(U rail)
{
    getParentRailAttachment().setObject(rail);
}

template <class U>
inline SharedPtr<U> HydroParentRailAttachmentInterface::getParentRail()
{
    return getParentRailAttachment().HydroAttachment::getObject<U>();
}


template <class U>
inline void HydroFeedReservoirAttachmentInterface::setFeedReservoir(U reservoir)
{
    getFeedReservoirAttachment().setObject(reservoir);
}

template <class U>
inline SharedPtr<U> HydroFeedReservoirAttachmentInterface::getFeedReservoir(bool poll)
{
    getFeedReservoirAttachment().updateIfNeeded(poll);
    return getFeedReservoirAttachment().HydroAttachment::getObject<U>();
}


template <class U>
inline void HydroSensorAttachmentInterface::setSensor(U sensor)
{
    getSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroSensorAttachmentInterface::getSensor(bool poll)
{
    getSensorAttachment().updateIfNeeded(poll);
    return getSensorAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroAirCO2SensorAttachmentInterface::setAirCO2Sensor(U sensor)
{
    getAirCO2SensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroAirCO2SensorAttachmentInterface::getAirCO2Sensor(bool poll)
{
    getAirCO2SensorAttachment().updateIfNeeded(poll);
    return getAirCO2SensorAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroAirTemperatureSensorAttachmentInterface::setAirTemperatureSensor(U sensor)
{
    getAirTemperatureSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroAirTemperatureSensorAttachmentInterface::getAirTemperatureSensor(bool poll)
{
    getAirTemperatureSensorAttachment().updateIfNeeded(poll);
    return getAirTemperatureSensorAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroPowerProductionSensorAttachmentInterface::setPowerProductionSensor(U sensor)
{
    getPowerProductionSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroPowerProductionSensorAttachmentInterface::getPowerProductionSensor(bool poll)
{
    getPowerProductionSensorAttachment().updateIfNeeded(poll);
    return getPowerProductionSensorAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroPowerUsageSensorAttachmentInterface::setPowerUsageSensor(U sensor)
{
    getPowerUsageSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroPowerUsageSensorAttachmentInterface::getPowerUsageSensor(bool poll)
{
    getPowerUsageSensorAttachment().updateIfNeeded(poll);
    return getPowerUsageSensorAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroSoilMoistureSensorAttachmentInterface::setSoilMoistureSensor(U sensor)
{
    getSoilMoistureSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroSoilMoistureSensorAttachmentInterface::getSoilMoistureSensor(bool poll)
{
    getSoilMoistureSensorAttachment().updateIfNeeded(poll);
    return getSoilMoistureSensorAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroWaterFlowRateSensorAttachmentInterface::setFlowRateSensor(U sensor)
{
    getFlowRateSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroWaterFlowRateSensorAttachmentInterface::getFlowRateSensor(bool poll)
{
    getFlowRateSensorAttachment().updateIfNeeded(poll);
    return getFlowRateSensorAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroWaterPHSensorAttachmentInterface::setWaterPHSensor(U sensor)
{
    getWaterPHSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroWaterPHSensorAttachmentInterface::getWaterPHSensor(bool poll)
{
    getWaterPHSensorAttachment().updateIfNeeded(poll);
    return getWaterPHSensorAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroWaterTDSSensorAttachmentInterface::setWaterTDSSensor(U sensor)
{
    getWaterTDSSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroWaterTDSSensorAttachmentInterface::getWaterTDSSensor(bool poll)
{
    getWaterTDSSensorAttachment().updateIfNeeded(poll);
    return getWaterTDSSensorAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroWaterTemperatureSensorAttachmentInterface::setWaterTemperatureSensor(U sensor)
{
    getWaterTemperatureSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroWaterTemperatureSensorAttachmentInterface::getWaterTemperatureSensor(bool poll)
{
    getWaterTemperatureSensorAttachment().updateIfNeeded(poll);
    return getWaterTemperatureSensorAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroWaterVolumeSensorAttachmentInterface::setWaterVolumeSensor(U sensor)
{
    getWaterVolumeSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroWaterVolumeSensorAttachmentInterface::getWaterVolumeSensor(bool poll)
{
    getWaterVolumeSensorAttachment().updateIfNeeded(poll);
    return getWaterVolumeSensorAttachment().HydroAttachment::getObject<U>();
}


template <class U>
inline void HydroTriggerAttachmentInterface::setTrigger(U trigger)
{
    getTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> HydroTriggerAttachmentInterface::getTrigger(bool poll)
{
    getTriggerAttachment().updateIfNeeded(poll);
    return getTriggerAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroFilledTriggerAttachmentInterface::setFilledTrigger(U trigger)
{
    getFilledTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> HydroFilledTriggerAttachmentInterface::getFilledTrigger(bool poll)
{
    getFilledTriggerAttachment().updateIfNeeded(poll);
    return getFilledTriggerAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroEmptyTriggerAttachmentInterface::setEmptyTrigger(U trigger)
{
    getEmptyTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> HydroEmptyTriggerAttachmentInterface::getEmptyTrigger(bool poll)
{
    getEmptyTriggerAttachment().updateIfNeeded(poll);
    return getEmptyTriggerAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroFeedingTriggerAttachmentInterface::setFeedingTrigger(U trigger)
{
    getFeedingTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> HydroFeedingTriggerAttachmentInterface::getFeedingTrigger(bool poll)
{
    getFeedingTriggerAttachment().updateIfNeeded(poll);
    return getFeedingTriggerAttachment().HydroAttachment::getObject<U>();
}

template <class U>
inline void HydroLimitTriggerAttachmentInterface::setLimitTrigger(U trigger)
{
    getLimitTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> HydroLimitTriggerAttachmentInterface::getLimitTrigger(bool poll)
{
    getLimitTriggerAttachment().updateIfNeeded(poll);
    return getLimitTriggerAttachment().HydroAttachment::getObject<U>();
}
