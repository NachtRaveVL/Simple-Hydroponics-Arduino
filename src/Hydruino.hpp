/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino System
*/

#include "Hydruino.h"

inline void Hydruino::returnPinLock(pintype_t pin)
{
    _pinLocks.erase(pin);
}

inline SharedPtr<HydroPinMuxer> Hydruino::getPinMuxer(pintype_t pin)
{
    return _pinMuxers[pin];
}

inline void Hydruino::setPinMuxer(pintype_t pin, SharedPtr<HydroPinMuxer> pinMuxer)
{
    _pinMuxers[pin] = pinMuxer;
}

#ifdef HYDRO_USE_WIFI

inline WiFiClass *Hydruino::getWiFi(bool begin)
{
    return getWiFi(getWiFiSSID(), getWiFiPassword(), begin);
}

#endif
#ifdef HYDRO_USE_ETHERNET

inline EthernetClass *Hydruino::getEthernet(bool begin)
{
    return getEthernet(getMACAddress(), begin);
}

#endif


inline HydroLoggerSubData *HydroLogger::loggerData() const
{
    return &Hydruino::_activeInstance->_systemData->logger;
}

inline bool HydroLogger::hasLoggerData() const
{
    return Hydruino::_activeInstance && Hydruino::_activeInstance->_systemData;
}

inline bool HydroLogger::isLoggingToSDCard() const
{
    return hasLoggerData() && loggerData()->logLevel != Hydro_LogLevel_None && loggerData()->logToSDCard;
}

#ifdef HYDRO_USE_WIFI_STORAGE

inline bool HydroLogger::isLoggingToWiFiStorage() const
{
    return hasLoggerData() && loggerData()->logLevel != Hydro_LogLevel_None && loggerData()->logToWiFiStorage;
}

#endif

inline void HydroLogger::logActivation(const HydroActuator *actuator)
{
    if (actuator) { logMessage(actuator->getKeyString(), SFP(HStr_Log_HasEnabled)); }
}

inline void HydroLogger::logDeactivation(const HydroActuator *actuator)
{
    if (actuator) { logMessage(actuator->getKeyString(), SFP(HStr_Log_HasDisabled)); }
}

inline void HydroLogger::logProcess(const HydroObject *obj, const String &processString, const String &statusString)
{
    if (obj) { logMessage(obj->getKeyString(), processString, statusString); }
}

inline void HydroLogger::logStatus(const HydroObject *obj, const String &statusString)
{
    if (obj) { logMessage(obj->getKeyString(), statusString); }
}

inline Hydro_LogLevel HydroLogger::getLogLevel() const
{
    return hasLoggerData() ? loggerData()->logLevel : Hydro_LogLevel_None;
}

inline bool HydroLogger::isLoggingEnabled() const
{
    return hasLoggerData() && loggerData()->logLevel != Hydro_LogLevel_None && (loggerData()->logToSDCard || loggerData()->logToWiFiStorage);
}


inline HydroPublisherSubData *HydroPublisher::publisherData() const
{
    return &Hydruino::_activeInstance->_systemData->publisher;
}

inline bool HydroPublisher::hasPublisherData() const
{
    return Hydruino::_activeInstance && Hydruino::_activeInstance->_systemData;
}

inline bool HydroPublisher::isPublishingToSDCard() const
{
    return hasPublisherData() && publisherData()->pubToSDCard;
}

#ifdef HYDRO_USE_WIFI_STORAGE

inline bool HydroPublisher::isPublishingToWiFiStorage() const
{
    return hasPublisherData() && publisherData()->pubToWiFiStorage;
}

#endif
#ifdef HYDRO_USE_MQTT

inline bool HydroPublisher::isPublishingToMQTTClient() const
{
    return hasPublisherData() && _mqttClient;
}

#endif

inline bool HydroPublisher::isPublishingEnabled() const
{
    return hasPublisherData() && (publisherData()->pubToSDCard || publisherData()->pubToWiFiStorage
        #ifdef HYDRO_USE_MQTT
            || _mqttClient
        #endif
        );
}

inline void HydroPublisher::setNeedsTabulation()
{
    _needsTabulation = hasPublisherData();
}


inline HydroSchedulerSubData *HydroScheduler::schedulerData() const
{
    return &Hydruino::_activeInstance->_systemData->scheduler;
}

inline bool HydroScheduler::hasSchedulerData() const
{
    return Hydruino::_activeInstance && Hydruino::_activeInstance->_systemData;
}

inline void HydroScheduler::setLastWeekAsFlush(HydroCrop *crop)
{
    if (crop) { setFlushWeek(crop->getTotalGrowWeeks() - 1); }
}

inline void HydroScheduler::setNeedsScheduling() {
    _needsScheduling = hasSchedulerData();
}
