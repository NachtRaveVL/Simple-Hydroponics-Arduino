/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics System
*/

#include "Hydroponics.h"

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

inline Hydroponics_LogLevel HydroponicsLogger::getLogLevel() const
{
    return hasLoggerData() ? loggerData()->logLevel : Hydroponics_LogLevel_None;
}

inline bool HydroponicsLogger::isLoggingEnabled() const
{
    return hasLoggerData() && loggerData()->logLevel != Hydroponics_LogLevel_None && (loggerData()->logToSDCard);
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
    return hasPublisherData() && publisherData()->publishToSDCard;
}

inline bool HydroponicsPublisher::isPublishingEnabled() const
{
    return hasPublisherData() && (publisherData()->publishToSDCard);
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
