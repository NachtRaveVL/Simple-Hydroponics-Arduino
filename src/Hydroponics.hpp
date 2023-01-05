/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydroponics System
*/

#include "Hydroponics.h"

inline void Hydroponics::returnPinLock(pintype_t pin)
{
    _pinLocks.erase(pin);
}

#ifdef HYDRUINO_USE_WIFI

inline WiFiClass *Hydroponics::getWiFi(bool begin)
{
    return getWiFi(getWiFiSSID(), getWiFiPassword(), begin);
}

#endif


inline HydroponicsLoggerSubData *HydroponicsLogger::loggerData() const
{
    return &Hydroponics::_activeInstance->_systemData->logger;
}

inline bool HydroponicsLogger::hasLoggerData() const
{
    return Hydroponics::_activeInstance && Hydroponics::_activeInstance->_systemData;
}

inline bool HydroponicsLogger::isLoggingToSDCard() const
{
    return hasLoggerData() && loggerData()->logLevel != Hydroponics_LogLevel_None && loggerData()->logToSDCard;
}

#ifdef HYDRUINO_USE_WIFI_STORAGE

inline bool HydroponicsLogger::isLoggingToWiFiStorage() const
{
    return hasLoggerData() && loggerData()->logLevel != Hydroponics_LogLevel_None && loggerData()->logToWiFiStorage;
}

#endif

inline void HydroponicsLogger::logActivation(const HydroponicsActuator *actuator)
{
    if (actuator) { logMessage(actuator->getKeyString(), SFP(HStr_Log_HasEnabled)); }
}

inline void HydroponicsLogger::logDeactivation(const HydroponicsActuator *actuator)
{
    if (actuator) { logMessage(actuator->getKeyString(), SFP(HStr_Log_HasDisabled)); }
}

inline void HydroponicsLogger::logProcess(const HydroponicsObject *obj, const String &processString, const String &statusString)
{
    if (obj) { logMessage(obj->getKeyString(), processString, statusString); }
}

inline void HydroponicsLogger::logStatus(const HydroponicsObject *obj, const String &statusString)
{
    if (obj) { logMessage(obj->getKeyString(), statusString); }
}

inline Hydroponics_LogLevel HydroponicsLogger::getLogLevel() const
{
    return hasLoggerData() ? loggerData()->logLevel : Hydroponics_LogLevel_None;
}

inline bool HydroponicsLogger::isLoggingEnabled() const
{
    return hasLoggerData() && loggerData()->logLevel != Hydroponics_LogLevel_None && (loggerData()->logToSDCard || loggerData()->logToWiFiStorage);
}


inline HydroponicsPublisherSubData *HydroponicsPublisher::publisherData() const
{
    return &Hydroponics::_activeInstance->_systemData->publisher;
}

inline bool HydroponicsPublisher::hasPublisherData() const
{
    return Hydroponics::_activeInstance && Hydroponics::_activeInstance->_systemData;
}

inline bool HydroponicsPublisher::isPublishingToSDCard() const
{
    return hasPublisherData() && publisherData()->pubToSDCard;
}

#ifdef HYDRUINO_USE_WIFI_STORAGE

inline bool HydroponicsPublisher::isPublishingToWiFiStorage() const
{
    return hasPublisherData() && publisherData()->pubToWiFiStorage;
}

#endif
#ifdef HYDRUINO_ENABLE_MQTT

inline bool HydroponicsPublisher::isPublishingToMQTTClient() const
{
    return hasPublisherData() && _mqttClient;
}

#endif

inline bool HydroponicsPublisher::isPublishingEnabled() const
{
    return hasPublisherData() && (publisherData()->pubToSDCard || publisherData()->pubToWiFiStorage
        #ifdef HYDRUINO_ENABLE_MQTT
            || _mqttClient
        #endif
        );
}

inline void HydroponicsPublisher::setNeedsTabulation()
{
    _needsTabulation = hasPublisherData();
}


inline HydroponicsSchedulerSubData *HydroponicsScheduler::schedulerData() const
{
    return &Hydroponics::_activeInstance->_systemData->scheduler;
}

inline bool HydroponicsScheduler::hasSchedulerData() const
{
    return Hydroponics::_activeInstance && Hydroponics::_activeInstance->_systemData;
}

inline void HydroponicsScheduler::setLastWeekAsFlush(HydroponicsCrop *crop)
{
    if (crop) { setFlushWeek(crop->getTotalGrowWeeks() - 1); }
}

inline void HydroponicsScheduler::setNeedsScheduling() {
    _needsScheduling = hasSchedulerData();
}
