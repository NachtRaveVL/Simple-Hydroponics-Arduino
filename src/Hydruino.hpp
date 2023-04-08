/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino System
*/

#include "Hydruino.h"

inline bool Twilight::isDaytime(time_t unixTime) const {
    DateTime time = isUTC ? DateTime((uint32_t)unixTime) : localTime(unixTime);
    double hour = time.hour() + (time.minute() / 60.0) + (time.second() / 3600.0);
    return sunrise <= sunset ? hour >= sunrise && hour <= sunset
                             : hour >= sunrise || hour <= sunset;
}

inline bool Twilight::isDaytime(DateTime localTime) const
{
    DateTime time = isUTC ? DateTime((uint32_t)unixTime(localTime)) : localTime;
    double hour = time.hour() + (time.minute() / 60.0) + (time.second() / 3600.0);
    return sunrise <= sunset ? hour >= sunrise && hour <= sunset
                             : hour >= sunrise || hour <= sunset;
}

inline time_t Twilight::hourToUnixTime(double hour, bool isUTC)
{
    return isUTC ? unixDayStart() + (time_t)(hour * SECS_PER_HOUR)
                 : unixTime(localDayStart() + TimeSpan(hour * SECS_PER_HOUR));
}

inline DateTime Twilight::hourToLocalTime(double hour, bool isUTC)
{
    return isUTC ? localTime(unixDayStart() + (time_t)(hour * SECS_PER_HOUR))
                 : localDayStart() + TimeSpan(hour * SECS_PER_HOUR);
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

inline void Hydruino::performAutosave()
{
    for (int autosave = 0; autosave < 2; ++autosave) {
        switch (autosave == 0 ? _systemData->autosaveEnabled : _systemData->autosaveFallback) {
            case Hydro_Autosave_EnabledToSDCardJson:
                saveToSDCard(JSON);
                break;
            case Hydro_Autosave_EnabledToSDCardRaw:
                saveToSDCard(RAW);
                break;
            case Hydro_Autosave_EnabledToEEPROMJson:
                saveToEEPROM(JSON);
                break;
            case Hydro_Autosave_EnabledToEEPROMRaw:
                saveToEEPROM(RAW);
                break;
            case Hydro_Autosave_EnabledToWiFiStorageJson:
                #ifdef HYDRO_USE_WIFI_STORAGE
                    saveToWiFiStorage(JSON);
                #endif
                break;
            case Hydro_Autosave_EnabledToWiFiStorageRaw:
                #ifdef HYDRO_USE_WIFI_STORAGE
                    saveToWiFiStorage(RAW);
                #endif
            case Hydro_Autosave_Disabled:
                break;
        }
    }

    _lastAutosave = unixNow();
}

inline void Hydruino::broadcastLowMemory()
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        iter->second->handleLowMemory();
    }
}

inline void Hydruino::notifyRTCTimeUpdated()
{
    _rtcBattFail = false;
}

inline void Hydruino::broadcastDateChanged()
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second->isReservoirType()) {
            auto reservoir = static_pointer_cast<HydroReservoir>(iter->second);

            if (reservoir && reservoir->isFeedClass()) {
                auto feedReservoir = static_pointer_cast<HydroFeedReservoir>(iter->second);
                if (feedReservoir) {feedReservoir->notifyDateChanged(); }
            }
        } else if (iter->second->isCropType()) {
            auto crop = static_pointer_cast<HydroCrop>(iter->second);

            if (crop) { crop->notifyDateChanged(); }
        }
    }
}

inline void Hydruino::notifySignificantTime(time_t time)
{
    logger.updateInitTracking(time);
    _lastAutosave = isAutosaveEnabled() ? time : 0;
}

inline void Hydruino::notifySignificantLocation(Location loc)
{
    if (_systemData) { _systemData->bumpRevisionIfNeeded(); }
}


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
    if (actuator) { logMessage(actuator->getId().getDisplayString(), SFP(HStr_Log_HasEnabled)); }
}

inline void HydroLogger::logDeactivation(const HydroActuator *actuator)
{
    if (actuator) { logMessage(actuator->getId().getDisplayString(), SFP(HStr_Log_HasDisabled)); }
}

inline void HydroLogger::logProcess(const HydroObjInterface *obj, const String &processString, const String &statusString)
{
    if (obj) { logMessage(obj->getId().getDisplayString(), processString, statusString); }
}

inline void HydroLogger::logStatus(const HydroObjInterface *obj, const String &statusString)
{
    if (obj) { logMessage(obj->getId().getDisplayString(), statusString); }
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
