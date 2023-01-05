/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydroponics Attachment Points
*/

#include "Hydroponics.h"

template <class U>
inline void HydroponicsActuatorAttachmentInterface::setActuator(U actuator)
{
    getParentActuator(false).setObject(actuator);
}

template <class U>
inline SharedPtr<U> HydroponicsActuatorAttachmentInterface::getActuator(bool resolve)
{
    return getParentActuator(resolve).template getObject<U>();
}

template <class U>
inline void HydroponicsSensorAttachmentInterface::setSensor(U sensor)
{
    getParentSensor(false).setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroponicsSensorAttachmentInterface::getSensor(bool resolve)
{
    return getParentSensor(resolve).template getObject<U>();
}

template <class U>
inline void HydroponicsCropAttachmentInterface::setCrop(U crop)
{
    getParentCrop(false).setObject(crop);
}

template <class U>
inline SharedPtr<U> HydroponicsCropAttachmentInterface::getCrop(bool resolve)
{
    return getParentCrop(resolve).template getObject<U>();
}

template <class U>
inline void HydroponicsReservoirAttachmentInterface::setReservoir(U reservoir)
{
    getParentReservoir(false).setObject(reservoir);
}

template <class U>
inline SharedPtr<U> HydroponicsReservoirAttachmentInterface::getReservoir(bool resolve)
{
    return getParentReservoir(resolve).template getObject<U>();
}

template <class U>
inline void HydroponicsRailAttachmentInterface::setRail(U rail)
{
    getParentRail(false).setObject(rail);
}

template <class U>
inline SharedPtr<U> HydroponicsRailAttachmentInterface::getRail(bool resolve)
{
    return getParentRail(resolve).template getObject<U>();
}

inline bool HydroponicsBalancerObjectInterface::isBalanced() const
{
    return getBalancerState() == Hydroponics_BalancerState_Balanced;
}

template <class U>
inline void HydroponicsPumpObjectInterface::setInputReservoir(U reservoir)
{
    getParentReservoir(false).setObject(reservoir);
}

template <class U>
inline SharedPtr<U> HydroponicsPumpObjectInterface::getInputReservoir(bool resolve)
{
    return getParentReservoir(resolve).template getObject<U>();
}

template <class U>
inline void HydroponicsPumpObjectInterface::setOutputReservoir(U reservoir)
{
    getDestinationReservoir(false).setObject(reservoir);
}

template <class U>
inline SharedPtr<U> HydroponicsPumpObjectInterface::getOutputReservoir(bool resolve)
{
    return getDestinationReservoir(resolve).template getObject<U>();
}

template <class U>
inline void HydroponicsFeedReservoirAttachmentInterface::setFeedReservoir(U reservoir)
{
    getFeedingReservoir(false).setObject(reservoir);
}

template <class U>
inline SharedPtr<U> HydroponicsFeedReservoirAttachmentInterface::getFeedReservoir(bool resolve)
{
    return getFeedingReservoir(resolve).template getObject<U>();
}

template <class U>
inline void HydroponicsFlowSensorAttachmentInterface::setFlowRateSensor(U sensor)
{
    getFlowRate(false).setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroponicsFlowSensorAttachmentInterface::getFlowRateSensor(bool poll)
{
    return hy_static_ptr_cast<U>(getFlowRate(poll).getObject());
}

template <class U>
inline void HydroponicsVolumeSensorAttachmentInterface::setWaterVolumeSensor(U sensor)
{
    getWaterVolume(false).setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroponicsVolumeSensorAttachmentInterface::getWaterVolumeSensor(bool poll)
{
    return hy_static_ptr_cast<U>(getWaterVolume(poll).getObject());
}

template <class U>
inline void HydroponicsPowerSensorAttachmentInterface::setPowerUsageSensor(U sensor)
{
    getPowerUsage(false).setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroponicsPowerSensorAttachmentInterface::getPowerUsageSensor(bool poll)
{
    return hy_static_ptr_cast<U>(getPowerUsage(poll).getObject());
}

template <class U>
inline void HydroponicsWaterTemperatureSensorAttachmentInterface::setWaterTemperatureSensor(U sensor)
{
    getWaterTemperature(false).setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroponicsWaterTemperatureSensorAttachmentInterface::getWaterTemperatureSensor(bool poll)
{
    return hy_static_ptr_cast<U>(getWaterTemperature(poll).getObject());
}

template <class U>
inline void HydroponicsWaterPHSensorAttachmentInterface::setWaterPHSensor(U sensor)
{
    getWaterPH(false).setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroponicsWaterPHSensorAttachmentInterface::getWaterPHSensor(bool poll)
{
    return hy_static_ptr_cast<U>(getWaterPH(poll).getObject());
}

template <class U>
inline void HydroponicsWaterTDSSensorAttachmentInterface::setWaterTDSSensor(U sensor)
{
    getWaterTDS(false).setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroponicsWaterTDSSensorAttachmentInterface::getWaterTDSSensor(bool poll)
{
    return hy_static_ptr_cast<U>(getWaterTDS(poll).getObject());
}

template <class U>
inline void HydroponicsSoilMoistureSensorAttachmentInterface::setSoilMoistureSensor(U sensor)
{
    getSoilMoisture(false).setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroponicsSoilMoistureSensorAttachmentInterface::getSoilMoistureSensor(bool poll)
{
    return hy_static_ptr_cast<U>(getSoilMoisture(poll).getObject());
}

template <class U>
inline void HydroponicsAirTemperatureSensorAttachmentInterface::setAirTemperatureSensor(U sensor)
{
    getAirTemperature(false).setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroponicsAirTemperatureSensorAttachmentInterface::getAirTemperatureSensor(bool poll)
{
    return hy_static_ptr_cast<U>(getAirTemperature(poll).getObject());
}

template <class U>
inline void HydroponicsAirHumiditySensorAttachmentInterface::setAirHumiditySensor(U sensor)
{
    getAirHumidity(false).setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroponicsAirHumiditySensorAttachmentInterface::getAirHumiditySensor(bool poll)
{
    return hy_static_ptr_cast<U>(getAirHumidity(poll).getObject());
}

template <class U>
inline void HydroponicsAirCO2SensorAttachmentInterface::setAirCO2Sensor(U sensor)
{
    getAirCO2(false).setObject(sensor);
}

template <class U>
inline SharedPtr<U> HydroponicsAirCO2SensorAttachmentInterface::getAirCO2Sensor(bool poll)
{
    return hy_static_ptr_cast<U>(getAirCO2(poll).getObject());
}
