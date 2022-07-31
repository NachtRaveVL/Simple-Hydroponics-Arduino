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

inline HydroponicsPublisherSubData *HydroponicsPublisher::publisherData() const
{
    return &Hydroponics::_activeInstance->_systemData->publisher;
}

inline bool HydroponicsPublisher::hasPublisherData() const
{
    return Hydroponics::_activeInstance && Hydroponics::_activeInstance->_systemData;
}

inline void HydroponicsPublisher::setNeedsTabulation()
{
    _needsTabulation = hasPublisherData();
}

inline void HydroponicsScheduler::setNeedsScheduling() {
    _needsScheduling = hasSchedulerData();
}

inline HydroponicsSchedulerSubData *HydroponicsScheduler::schedulerData() const
{
    return &Hydroponics::_activeInstance->_systemData->scheduler;
}

inline bool HydroponicsScheduler::hasSchedulerData() const
{
    return Hydroponics::_activeInstance && Hydroponics::_activeInstance->_systemData;
}
