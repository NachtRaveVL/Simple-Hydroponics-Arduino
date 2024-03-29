/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Scheduler
*/

#include "Hydruino.h"

HydroScheduler::HydroScheduler()
    : _needsScheduling(false), _inDaytimeMode(false), _lastDay{0}
{ ; }

HydroScheduler::~HydroScheduler()
{
    while (_feedings.size()) {
        auto feedingIter = _feedings.begin();
        delete feedingIter->second;
        _feedings.erase(feedingIter);
    }
    while (_lightings.size()) {
        auto lightingIter = _lightings.begin();
        delete lightingIter->second;
        _lightings.erase(lightingIter);
    }
}

void HydroScheduler::update()
{
    if (hasSchedulerData()) {
        #ifdef HYDRO_USE_VERBOSE_OUTPUT
            Serial.println(F("Scheduler::update")); flushYield();
        #endif

        {   time_t time = unixNow();
            DateTime currTime = localTime(time);
            bool daytimeMode = _dailyTwilight.isDaytime(time);

            if (_inDaytimeMode != daytimeMode) {
                _inDaytimeMode = daytimeMode;
                setNeedsScheduling();
                Hydruino::_activeInstance->setNeedsRedraw();
            }

            if (!(_lastDay[0] == currTime.year()-2000 &&
                  _lastDay[1] == currTime.month() &&
                  _lastDay[2] == currTime.day())) {
                // only log uptime upon actual day change and if uptime has been at least 1d
                if (getLogger()->getSystemUptime() >= SECS_PER_DAY) {
                    getLogger()->logSystemUptime();
                }
                broadcastDayChange();
            }
        }

        if (needsScheduling()) { performScheduling(); }

        for (auto feedingIter = _feedings.begin(); feedingIter != _feedings.end(); ++feedingIter) {
            feedingIter->second->update();
        }
        for (auto lightingIter = _lightings.begin(); lightingIter != _lightings.end(); ++lightingIter) {
            lightingIter->second->update();
        }

        #ifdef HYDRO_USE_VERBOSE_OUTPUT
            Serial.println(F("Scheduler::~update")); flushYield();
        #endif
    }
}

void HydroScheduler::setupWaterPHBalancer(HydroReservoir *reservoir, SharedPtr<HydroBalancer> waterPHBalancer)
{
    if (reservoir && waterPHBalancer) {
        {   Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> incActuators;
            auto phUpPumps = linksFilterPumpActuatorsByOutputReservoirAndSourceReservoirType<HYDRO_BAL_ACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydro_ReservoirType_PhUpSolution);
            float dosingRate = getCombinedDosingRate(reservoir, Hydro_ReservoirType_PhUpSolution);

            linksResolveActuatorsToAttachmentsByRateAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(phUpPumps, waterPHBalancer.get(), dosingRate, incActuators, Hydro_ActuatorType_PeristalticPump);
            if (!incActuators.size()) { // prefer peristaltic, else use full pump
                linksResolveActuatorsToAttachmentsByRateAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(phUpPumps, waterPHBalancer.get(), dosingRate, incActuators, Hydro_ActuatorType_WaterPump);
            }

            waterPHBalancer->setIncrementActuators(incActuators);
        }

        {   Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> decActuators;
            auto phDownPumps = linksFilterPumpActuatorsByOutputReservoirAndSourceReservoirType<HYDRO_BAL_ACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydro_ReservoirType_PhDownSolution);
            float dosingRate = getCombinedDosingRate(reservoir, Hydro_ReservoirType_PhDownSolution);

            linksResolveActuatorsToAttachmentsByRateAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(phDownPumps, waterPHBalancer.get(), dosingRate, decActuators, Hydro_ActuatorType_PeristalticPump);
            if (!decActuators.size()) { // prefer peristaltic, else use full pump
                linksResolveActuatorsToAttachmentsByRateAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(phDownPumps, waterPHBalancer.get(), dosingRate, decActuators, Hydro_ActuatorType_WaterPump);
            }

            waterPHBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroScheduler::setupWaterTDSBalancer(HydroReservoir *reservoir, SharedPtr<HydroBalancer> waterTDSBalancer)
{
    if (reservoir && waterTDSBalancer) {
        {   Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> incActuators;
            float dosingRate = getCombinedDosingRate(reservoir, Hydro_ReservoirType_NutrientPremix);

            if (dosingRate > FLT_EPSILON) {
                auto nutrientPumps = linksFilterPumpActuatorsByOutputReservoirAndSourceReservoirType<HYDRO_BAL_ACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydro_ReservoirType_NutrientPremix);

                linksResolveActuatorsToAttachmentsByRateAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(nutrientPumps, waterTDSBalancer.get(), dosingRate, incActuators, Hydro_ActuatorType_PeristalticPump);
                if (!incActuators.size()) { // prefer peristaltic, else use full pump
                    linksResolveActuatorsToAttachmentsByRateAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(nutrientPumps, waterTDSBalancer.get(), dosingRate, incActuators, Hydro_ActuatorType_WaterPump);
                }
            }

            if (Hydruino::_activeInstance->hasCustomAdditives()) {
                int prevIncSize = incActuators.size();

                for (int reservoirType = Hydro_ReservoirType_CustomAdditive1; reservoirType < Hydro_ReservoirType_CustomAdditive1 + Hydro_ReservoirType_CustomAdditiveCount; ++reservoirType) {
                    if (Hydruino::_activeInstance->getCustomAdditiveData((Hydro_ReservoirType)reservoirType)) {
                        dosingRate = getCombinedDosingRate(reservoir, (Hydro_ReservoirType)reservoirType);

                        if (dosingRate > FLT_EPSILON) {
                            auto nutrientPumps = linksFilterPumpActuatorsByOutputReservoirAndSourceReservoirType<HYDRO_BAL_ACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, (Hydro_ReservoirType)reservoirType);

                            linksResolveActuatorsToAttachmentsByRateAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(nutrientPumps, waterTDSBalancer.get(), dosingRate, incActuators, Hydro_ActuatorType_PeristalticPump);
                            if (incActuators.size() == prevIncSize) { // prefer peristaltic, else use full pump
                                linksResolveActuatorsToAttachmentsByRateAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(nutrientPumps, waterTDSBalancer.get(), dosingRate, incActuators, Hydro_ActuatorType_WaterPump);
                            }
                        }

                        prevIncSize = incActuators.size();
                    }
                }
            }

            waterTDSBalancer->setIncrementActuators(incActuators);
        }

        {   Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> decActuators;
            float dosingRate = getCombinedDosingRate(reservoir, Hydro_ReservoirType_FreshWater);

            if (dosingRate > FLT_EPSILON) {
                auto dilutionPumps = linksFilterPumpActuatorsByOutputReservoirAndSourceReservoirType<HYDRO_BAL_ACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydro_ReservoirType_NutrientPremix);

                linksResolveActuatorsToAttachmentsByRateAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(dilutionPumps, waterTDSBalancer.get(), dosingRate, decActuators, Hydro_ActuatorType_PeristalticPump);
                if (!decActuators.size()) { // prefer peristaltic, else use full pump
                    linksResolveActuatorsToAttachmentsByRateAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(dilutionPumps, waterTDSBalancer.get(), dosingRate, decActuators, Hydro_ActuatorType_WaterPump);
                }
            }

            waterTDSBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroScheduler::setupWaterTemperatureBalancer(HydroReservoir *reservoir, SharedPtr<HydroBalancer> waterTempBalancer)
{
    if (reservoir && waterTempBalancer) {
        {   Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> incActuators;
            auto heaters = linksFilterActuatorsByReservoirAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydro_ActuatorType_WaterHeater);

            linksResolveActuatorsToAttachmentsByRateAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(heaters, waterTempBalancer.get(), 1.0f, incActuators, Hydro_ActuatorType_WaterHeater);

            waterTempBalancer->setIncrementActuators(incActuators);
        }

        {   Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> decActuators;
            waterTempBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroScheduler::setupAirTemperatureBalancer(HydroReservoir *reservoir, SharedPtr<HydroBalancer> airTempBalancer)
{
    if (reservoir && airTempBalancer) {
        {   Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> incActuators;
            airTempBalancer->setIncrementActuators(incActuators);
        }

        {   Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> decActuators;
            auto fans = linksFilterActuatorsByReservoirAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydro_ActuatorType_FanExhaust);

            linksResolveActuatorsToAttachmentsByRateAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(fans, airTempBalancer.get(), 1.0f, decActuators, Hydro_ActuatorType_FanExhaust);

            airTempBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroScheduler::setupAirCO2Balancer(HydroReservoir *reservoir, SharedPtr<HydroBalancer> airCO2Balancer)
{
    if (reservoir && airCO2Balancer) {
        {   Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> incActuators;
            auto fans = linksFilterActuatorsByReservoirAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydro_ActuatorType_FanExhaust);

            linksResolveActuatorsToAttachmentsByRateAndType<HYDRO_BAL_ACTUATORS_MAXSIZE>(fans, airCO2Balancer.get(), 1.0f, incActuators, Hydro_ActuatorType_FanExhaust);

            airCO2Balancer->setIncrementActuators(incActuators);
        }

        {   Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> decActuators;
            airCO2Balancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroScheduler::setBaseFeedMultiplier(float baseFeedMultiplier)
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    if (hasSchedulerData()) {
        schedulerData()->baseFeedMultiplier = baseFeedMultiplier;

        setNeedsScheduling();
        Hydruino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

void HydroScheduler::setWeeklyDosingRate(int weekIndex, float dosingRate, Hydro_ReservoirType reservoirType)
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRO_SOFT_ASSERT(!hasSchedulerData() || (weekIndex >= 0 && weekIndex < HYDRO_CROPS_GROWWEEKS_MAX), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && weekIndex >= 0 && weekIndex < HYDRO_CROPS_GROWWEEKS_MAX) {
        if (reservoirType == Hydro_ReservoirType_NutrientPremix) {
            schedulerData()->weeklyDosingRates[weekIndex] = dosingRate;
            
            setNeedsScheduling();
            Hydruino::_activeInstance->_systemData->bumpRevisionIfNeeded();
        } else if (reservoirType >= Hydro_ReservoirType_CustomAdditive1 && reservoirType < Hydro_ReservoirType_CustomAdditive1 + Hydro_ReservoirType_CustomAdditiveCount) {
            HydroCustomAdditiveData newAdditiveData(reservoirType);
            newAdditiveData.weeklyDosingRates[weekIndex] = dosingRate;
            newAdditiveData.bumpRevisionIfNeeded();
            Hydruino::_activeInstance->setCustomAdditiveData(&newAdditiveData);

            setNeedsScheduling();
        } else {
            HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_UnsupportedOperation));
        }
    }
}

void HydroScheduler::setStandardDosingRate(float dosingRate, Hydro_ReservoirType reservoirType)
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRO_SOFT_ASSERT(!hasSchedulerData() || (reservoirType >= Hydro_ReservoirType_FreshWater && reservoirType < Hydro_ReservoirType_CustomAdditive1), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && (reservoirType >= Hydro_ReservoirType_FreshWater && reservoirType < Hydro_ReservoirType_CustomAdditive1)) {
        schedulerData()->stdDosingRates[reservoirType - Hydro_ReservoirType_FreshWater] = dosingRate;

        setNeedsScheduling();
        Hydruino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

void HydroScheduler::setLastWeekAsFlush(Hydro_CropType cropType)
{
    auto cropLibData = hydroCropsLib.checkoutCropsData(cropType);
    if (cropLibData) {
        setFlushWeek(cropLibData->totalGrowWeeks-1);
        hydroCropsLib.returnCropsData(cropLibData);
    }
}

void HydroScheduler::setFlushWeek(int weekIndex)
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRO_SOFT_ASSERT(!hasSchedulerData() || (weekIndex >= 0 && weekIndex < HYDRO_CROPS_GROWWEEKS_MAX), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && weekIndex >= 0 && weekIndex < HYDRO_CROPS_GROWWEEKS_MAX) {
        schedulerData()->weeklyDosingRates[weekIndex] = 0;

        for (Hydro_ReservoirType reservoirType = Hydro_ReservoirType_CustomAdditive1;
             reservoirType < Hydro_ReservoirType_CustomAdditive1 + Hydro_ReservoirType_CustomAdditiveCount;
             reservoirType = (Hydro_ReservoirType)((int)reservoirType + 1)) {
            auto additiveData = Hydruino::_activeInstance->getCustomAdditiveData(reservoirType);
            if (additiveData) {
                HydroCustomAdditiveData newAdditiveData = *additiveData;
                newAdditiveData.weeklyDosingRates[weekIndex] = 0;
                newAdditiveData.bumpRevisionIfNeeded();
                Hydruino::_activeInstance->setCustomAdditiveData(&newAdditiveData);
            }
        }

        setNeedsScheduling();
    }
}

void HydroScheduler::setTotalFeedingsPerDay(unsigned int feedingsDay)
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasSchedulerData() && schedulerData()->totalFeedingsPerDay != feedingsDay) {
        schedulerData()->totalFeedingsPerDay = feedingsDay;

        setNeedsScheduling();
        Hydruino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

void HydroScheduler::setPreFeedAeratorMins(unsigned int aeratorMins)
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasSchedulerData() && schedulerData()->preFeedAeratorMins != aeratorMins) {
        schedulerData()->preFeedAeratorMins = aeratorMins;

        setNeedsScheduling();
        Hydruino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

void HydroScheduler::setPreDawnSprayMins(unsigned int sprayMins)
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasSchedulerData() && schedulerData()->preDawnSprayMins != sprayMins) {
        schedulerData()->preDawnSprayMins = sprayMins;

        setNeedsScheduling();
        Hydruino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

void HydroScheduler::setAirReportInterval(TimeSpan reportInterval)
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasSchedulerData() && schedulerData()->airReportInterval != reportInterval.totalseconds()) {
        schedulerData()->airReportInterval = reportInterval.totalseconds();
        Hydruino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

void HydroScheduler::setUseNaturalLight(bool useNaturalLight, unsigned int twilightOffsetMins)
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasSchedulerData() && ((useNaturalLight && schedulerData()->natLightOffsetMins != twilightOffsetMins) ||
                               (!useNaturalLight && schedulerData()->natLightOffsetMins != -1))) {
        schedulerData()->natLightOffsetMins = useNaturalLight ? twilightOffsetMins : -1;

        setNeedsScheduling();
        Hydruino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

float HydroScheduler::getCombinedDosingRate(HydroReservoir *reservoir, Hydro_ReservoirType reservoirType)
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRO_SOFT_ASSERT(!hasSchedulerData() || reservoir, SFP(HStr_Err_InvalidParameter));
    HYDRO_SOFT_ASSERT(!hasSchedulerData() || !reservoir || (reservoirType >= Hydro_ReservoirType_NutrientPremix &&
                                                            reservoirType < Hydro_ReservoirType_CustomAdditive1 + Hydro_ReservoirType_CustomAdditiveCount), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && reservoir &&
        (reservoirType >= Hydro_ReservoirType_NutrientPremix &&
         reservoirType < Hydro_ReservoirType_CustomAdditive1 + Hydro_ReservoirType_CustomAdditiveCount)) {
        auto crops = linksFilterCrops(reservoir->getLinkages());
        float totalWeights = 0;
        float totalDosing = 0;

        for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
            auto crop = (HydroCrop *)(*cropIter);
            if (crop) {
                if (reservoirType <= Hydro_ReservoirType_NutrientPremix) {
                    totalWeights += crop->getFeedingWeight();
                    totalDosing += schedulerData()->weeklyDosingRates[constrain(crop->getGrowWeek(), 0, crop->getTotalGrowWeeks() - 1)];
                } else if (reservoirType < Hydro_ReservoirType_CustomAdditive1) {
                    totalWeights += crop->getFeedingWeight();
                    totalDosing += schedulerData()->stdDosingRates[reservoirType - Hydro_ReservoirType_FreshWater];
                } else {
                    auto additiveData = Hydruino::_activeInstance->getCustomAdditiveData(reservoirType);
                    if (additiveData) {
                        totalWeights += crop->getFeedingWeight();
                        totalDosing += additiveData->weeklyDosingRates[constrain(crop->getGrowWeek(), 0, crop->getTotalGrowWeeks() - 1)];
                    }
                }
            }
        }

        if (totalWeights <= FLT_EPSILON) {
            totalWeights = 1.0f;
        }

        return totalDosing / totalWeights;
    }

    return 0.0f;
}

float HydroScheduler::getBaseFeedMultiplier() const
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    return hasSchedulerData() ? schedulerData()->baseFeedMultiplier : 1.0f;
}

float HydroScheduler::getWeeklyDosingRate(int weekIndex, Hydro_ReservoirType reservoirType) const
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRO_SOFT_ASSERT(!hasSchedulerData() || (weekIndex >= 0 && weekIndex < HYDRO_CROPS_GROWWEEKS_MAX), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && weekIndex >= 0 && weekIndex < HYDRO_CROPS_GROWWEEKS_MAX) {
        if (reservoirType == Hydro_ReservoirType_NutrientPremix) {
            return schedulerData()->weeklyDosingRates[weekIndex];
        } else if (reservoirType >= Hydro_ReservoirType_CustomAdditive1 && reservoirType < Hydro_ReservoirType_CustomAdditive1 + Hydro_ReservoirType_CustomAdditiveCount) {
            auto additiveDate = Hydruino::_activeInstance->getCustomAdditiveData(reservoirType);
            return additiveDate ? additiveDate->weeklyDosingRates[weekIndex] : 0.0f;
        } else {
            HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_UnsupportedOperation));
        }
    }

    return 0.0f;
}

float HydroScheduler::getStandardDosingRate(Hydro_ReservoirType reservoirType) const
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRO_SOFT_ASSERT(!hasSchedulerData() || (reservoirType >= Hydro_ReservoirType_FreshWater && reservoirType < Hydro_ReservoirType_CustomAdditive1), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && reservoirType >= Hydro_ReservoirType_FreshWater && reservoirType < Hydro_ReservoirType_CustomAdditive1) {
        return schedulerData()->stdDosingRates[reservoirType - Hydro_ReservoirType_FreshWater];
    }

    return 0.0f;
}

bool HydroScheduler::isFlushWeek(int weekIndex)
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRO_SOFT_ASSERT(!hasSchedulerData() || (weekIndex >= 0 && weekIndex < HYDRO_CROPS_GROWWEEKS_MAX), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && weekIndex >= 0 && weekIndex < HYDRO_CROPS_GROWWEEKS_MAX) {
        return isFPEqual(schedulerData()->weeklyDosingRates[weekIndex], 0.0f);
    }

    return false;
}

unsigned int HydroScheduler::getTotalFeedingsPerDay() const
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    return hasSchedulerData() ? schedulerData()->totalFeedingsPerDay : 0;
}

unsigned int HydroScheduler::getPreFeedAeratorMins() const
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    return hasSchedulerData() ? schedulerData()->preFeedAeratorMins : 0;
}

unsigned int HydroScheduler::getPreDawnSprayMins() const
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    return hasSchedulerData() ? schedulerData()->preDawnSprayMins : 0;
}

TimeSpan HydroScheduler::getAirReportInterval() const
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    return TimeSpan(hasSchedulerData() ? schedulerData()->airReportInterval : 0);
}

int HydroScheduler::getNaturalLightOffsetMins() const
{
    HYDRO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    return hasSchedulerData() && schedulerData()->natLightOffsetMins != -1 ? schedulerData()->natLightOffsetMins : -1;
}

void HydroScheduler::updateDayTracking()
{
    time_t time = unixNow();
    DateTime currTime = localTime(time);
    _lastDay[0] = currTime.year()-2000;
    _lastDay[1] = currTime.month();
    _lastDay[2] = currTime.day();

    Location loc = getController()->getSystemLocation();
    if (loc.hasPosition()) {
        double transit;
        calcSunriseSunset((unsigned long)time, loc.latitude, loc.longitude, transit, _dailyTwilight.sunrise, _dailyTwilight.sunset,
                          loc.resolveSunAlt(), HYDRO_SYS_SUNRISESET_CALCITERS);
        _dailyTwilight.isUTC = true;
    } else if (_dailyTwilight.isUTC) {
        _dailyTwilight = Twilight();
    }
    _inDaytimeMode = _dailyTwilight.isDaytime(time);

    setNeedsScheduling();
    Hydruino::_activeInstance->setNeedsRedraw();
}

void HydroScheduler::performScheduling()
{
    HYDRO_HARD_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));

    for (auto iter = Hydruino::_activeInstance->_objects.begin(); iter != Hydruino::_activeInstance->_objects.end(); ++iter) {
        if (iter->second->isReservoirType() && ((HydroReservoir *)(iter->second.get()))->isFeedClass()) {
            auto feedReservoir = static_pointer_cast<HydroFeedReservoir>(iter->second);

            {   auto feedingIter = _feedings.find(feedReservoir->getKey());

                if (linksCountSowableCrops(feedReservoir->getLinkages())) {
                    if (feedingIter != _feedings.end()) {
                        if (feedingIter->second) {
                            feedingIter->second->recalcFeeding();
                        }
                    } else {
                        #ifdef HYDRO_USE_VERBOSE_OUTPUT
                            Serial.print(F("Scheduler::performScheduling Sowable crop linkages found for: ")); Serial.print(iter->second->getId().getDisplayString());
                            Serial.print(':'); Serial.print(' '); Serial.println(linksCountSowableCrops(feedReservoir->getLinkages())); flushYield();
                        #endif

                        HydroFeeding *feeding = new HydroFeeding(feedReservoir);
                        HYDRO_SOFT_ASSERT(feeding, SFP(HStr_Err_AllocationFailure));
                        if (feeding) { _feedings[feedReservoir->getKey()] = feeding; }
                    }
                } else if (feedingIter != _feedings.end()) { // No sowable crops to warrant process -> delete if exists
                    #ifdef HYDRO_USE_VERBOSE_OUTPUT
                        Serial.print(F("Scheduler::performScheduling NO sowable crop linkages found for: ")); Serial.println(iter->second->getId().getDisplayString()); flushYield();
                    #endif
                    if (feedingIter->second) { delete feedingIter->second; }
                    _feedings.erase(feedingIter);
                }
            }

            {   auto lightingIter = _lightings.find(feedReservoir->getKey());

                if (linksCountActuatorsByReservoirAndType(feedReservoir->getLinkages(), feedReservoir.get(), Hydro_ActuatorType_WaterSprayer) ||
                    linksCountActuatorsByReservoirAndType(feedReservoir->getLinkages(), feedReservoir.get(), Hydro_ActuatorType_GrowLights)) {
                    if (lightingIter != _lightings.end()) {
                        if (lightingIter->second) {
                            lightingIter->second->recalcLighting();
                        }
                    } else {
                        #ifdef HYDRO_USE_VERBOSE_OUTPUT
                            Serial.print(F("Scheduler::performScheduling Light linkages found for: ")); Serial.print(iter->second->getId().getDisplayString()); Serial.print(':'); Serial.print(' ');
                            Serial.println(linksCountActuatorsByReservoirAndType(feedReservoir->getLinkages(), feedReservoir.get(), Hydro_ActuatorType_WaterSprayer) +
                                           linksCountActuatorsByReservoirAndType(feedReservoir->getLinkages(), feedReservoir.get(), Hydro_ActuatorType_GrowLights)); flushYield();
                        #endif

                        HydroLighting *lighting = new HydroLighting(feedReservoir);
                        HYDRO_SOFT_ASSERT(lighting, SFP(HStr_Err_AllocationFailure));
                        if (lighting) { _lightings[feedReservoir->getKey()] = lighting; }
                    }
                } else if (lightingIter != _lightings.end()) { // No lights or sprayers to warrant process -> delete if exists
                    #ifdef HYDRO_USE_VERBOSE_OUTPUT
                        Serial.print(F("Scheduler::performScheduling NO more light linkages found for: ")); Serial.println(iter->second->getId().getDisplayString()); flushYield();
                    #endif
                    if (lightingIter->second) { delete lightingIter->second; }
                    _lightings.erase(lightingIter);
                }
            }
        }
    }

    _needsScheduling = false;
}

void HydroScheduler::broadcastDayChange()
{
    updateDayTracking();

    #ifdef HYDRO_USE_MULTITASKING
        // these can take a while to complete
        taskManager.scheduleOnce(0, []{
            if (getController()) {
                getController()->notifyDayChanged();
            }
            yield();
            if (getLogger()) {
                getLogger()->notifyDayChanged();
            }
            yield();
            if (getPublisher()) {
                getPublisher()->notifyDayChanged();
            }
            yield();
        });
    #else
        if (getController()) {
            getController()->notifyDayChanged();
        }
        if (getLogger()) {
            getLogger()->notifyDayChanged();
        }
        if (getPublisher()) {
            getPublisher()->notifyDayChanged();
        }
    #endif
}


HydroProcess::HydroProcess(SharedPtr<HydroFeedReservoir> feedResIn)
    : feedRes(feedResIn), stageStart(unixNow())
{ ; }

void HydroProcess::clearActuatorReqs()
{
    while (actuatorReqs.size()) {
        actuatorReqs.begin()->disableActivation();
        actuatorReqs.erase(actuatorReqs.begin());
    }
}

void HydroProcess::setActuatorReqs(const Vector<HydroActuatorAttachment, HYDRO_SCH_REQACTS_MAXSIZE> &actuatorReqsIn)
{
    for (auto attachIter = actuatorReqs.begin(); attachIter != actuatorReqs.end(); ++attachIter) {
        bool found = false;
        auto key = attachIter->getKey();

        for (auto attachInIter = actuatorReqsIn.begin(); attachInIter != actuatorReqsIn.end(); ++attachInIter) {
            if (key == attachInIter->getKey()) {
                found = true;
                break;
            }
        }

        if (!found) { // disables actuators not found in new list
            attachIter->disableActivation();
        }
    }

    {   actuatorReqs.clear();
        for (auto attachInIter = actuatorReqsIn.begin(); attachInIter != actuatorReqsIn.end(); ++attachInIter) {
            actuatorReqs.push_back(*attachInIter);
            actuatorReqs.back().setParent(nullptr);
        }
    }  
}


HydroFeeding::HydroFeeding(SharedPtr<HydroFeedReservoir> feedRes)
    : HydroProcess(feedRes), stage(Init), canProcessAfter(0), lastAirReport(0),
      phSetpoint(0), tdsSetpoint(0), waterTempSetpoint(0), airTempSetpoint(0), co2Setpoint(0)
{
    recalcFeeding();
}

HydroFeeding::~HydroFeeding()
{
    clearActuatorReqs();
}

void HydroFeeding::recalcFeeding()
{
    float totalWeights = 0;
    float totalSetpoints[5] = {0,0,0,0,0};

    {   auto crops = linksFilterCrops(feedRes->getLinkages());
        for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
            auto crop = (HydroCrop *)(*cropIter);
            auto cropsLibData = hydroCropsLib.checkoutCropsData(crop->getCropType());

            if (cropsLibData) {
                float weight = crop->getFeedingWeight();
                totalWeights += weight;

                float feedRate = ((cropsLibData->tdsRange[0] + cropsLibData->tdsRange[1]) * 0.5);
                if (!getScheduler()->inDaytimeMode()) {
                    feedRate *= cropsLibData->nightlyFeedRate;
                }
                feedRate *= getScheduler()->schedulerData()->baseFeedMultiplier;

                totalSetpoints[0] += feedRate * weight;
                totalSetpoints[1] += ((cropsLibData->phRange[0] + cropsLibData->phRange[1]) * 0.5) * weight;
                totalSetpoints[2] += ((cropsLibData->waterTempRange[0] + cropsLibData->waterTempRange[1]) * 0.5) * weight;
                totalSetpoints[3] += ((cropsLibData->airTempRange[0] + cropsLibData->airTempRange[1]) * 0.5) * weight;
                totalSetpoints[4] += cropsLibData->co2Levels[(crop->getCropPhase() <= Hydro_CropPhase_Vegetative ? 0 : 1)] * weight;

                hydroCropsLib.returnCropsData(cropsLibData);
            }
        }
    }

    if (totalWeights < FLT_EPSILON) {
        totalWeights = 1.0f;
        totalSetpoints[0] = 1;
        totalSetpoints[1] = 6;
    }

    tdsSetpoint = totalSetpoints[0] / totalWeights;
    phSetpoint = tdsSetpoint > FLT_EPSILON ? (totalSetpoints[1] / totalWeights) : 7.0f; // handle flushing
    waterTempSetpoint = totalSetpoints[2] / totalWeights;
    airTempSetpoint = totalSetpoints[3] / totalWeights;
    co2Setpoint = totalSetpoints[4] / totalWeights;

    #ifdef HYDRO_USE_VERBOSE_OUTPUT // only works for singular feed res in system, otherwise output will be erratic
    {   static float _totalSetpoints[5] = {0,0,0,0,0};
        if (!isFPEqual(_totalSetpoints[0], totalSetpoints[0]) || !isFPEqual(_totalSetpoints[1], totalSetpoints[1]) || !isFPEqual(_totalSetpoints[2], totalSetpoints[2]) || !isFPEqual(_totalSetpoints[3], totalSetpoints[3]) || !isFPEqual(_totalSetpoints[4], totalSetpoints[4])) {
            _totalSetpoints[0] = totalSetpoints[0]; _totalSetpoints[1] = totalSetpoints[1]; _totalSetpoints[2] = totalSetpoints[2]; _totalSetpoints[3] = totalSetpoints[3]; _totalSetpoints[4] = totalSetpoints[4];
            Serial.print(F("Feeding::recalcFeeding setpoints: {tds,pH,wTmp,aTmp,aCO2} = [")); Serial.print(_totalSetpoints[0]); Serial.print(' '); Serial.print(_totalSetpoints[1]); Serial.print(' '); Serial.print(_totalSetpoints[2]); Serial.print(' '); Serial.print(_totalSetpoints[3]); Serial.print(' '); Serial.print(_totalSetpoints[4]); Serial.println(']'); flushYield(); } }
    #endif

    setupStaging();
}

void HydroFeeding::setupStaging()
{
    #ifdef HYDRO_USE_VERBOSE_OUTPUT
    {   static int8_t _stageFS1 = (int8_t)-1; if (_stageFS1 != (int8_t)stage) {
        Serial.print(F("Feeding::setupStaging stage: ")); Serial.println((_stageFS1 = (int8_t)stage)); flushYield(); } }
    #endif

    if (stage == PreFeed) {
        if (feedRes->getWaterPHSensor()) {
            auto phBalancer = feedRes->getWaterPHBalancer();
            if (!phBalancer) {
                phBalancer = SharedPtr<HydroTimedDosingBalancer>(new HydroTimedDosingBalancer(feedRes->getWaterPHSensor(), phSetpoint, HYDRO_RANGE_PH_HALF, feedRes->getMaxVolume(), feedRes->getVolumeUnits()));
                HYDRO_SOFT_ASSERT(phBalancer, SFP(HStr_Err_AllocationFailure));
                getScheduler()->setupWaterPHBalancer(feedRes.get(), phBalancer);
                feedRes->setWaterPHBalancer(phBalancer);
            }
            if (phBalancer) {
                phBalancer->setTargetSetpoint(phSetpoint);
                phBalancer->setMeasurementUnits(Hydro_UnitsType_Alkalinity_pH_14);
                phBalancer->setEnabled(true);
            }
        }
        if (feedRes->getWaterTDSSensor()) {
            auto tdsBalancer = feedRes->getWaterTDSBalancer();
            if (!tdsBalancer) {
                tdsBalancer = SharedPtr<HydroTimedDosingBalancer>(new HydroTimedDosingBalancer(feedRes->getWaterTDSSensor(), tdsSetpoint, HYDRO_RANGE_EC_HALF, feedRes->getMaxVolume(), feedRes->getVolumeUnits()));
                HYDRO_SOFT_ASSERT(tdsBalancer, SFP(HStr_Err_AllocationFailure));
                getScheduler()->setupWaterTDSBalancer(feedRes.get(), tdsBalancer);
                feedRes->setWaterTDSBalancer(tdsBalancer);
            }
            if (tdsBalancer) {
                tdsBalancer->setTargetSetpoint(tdsSetpoint);
                tdsBalancer->setMeasurementUnits(Hydro_UnitsType_Concentration_EC_5);
                tdsBalancer->setEnabled(true);
            }
        }
    } else {
        auto phBalancer = feedRes->getWaterPHBalancer();
        if (phBalancer) { phBalancer->setEnabled(false); }
        auto tdsBalancer = feedRes->getWaterTDSBalancer();
        if (tdsBalancer) { tdsBalancer->setEnabled(false); }
    }

    if ((stage == PreFeed || stage == Feed) && feedRes->getWaterTemperatureSensor()) {
        auto waterTempBalancer = feedRes->getWaterTemperatureBalancer();
        if (!waterTempBalancer) {
            waterTempBalancer = SharedPtr<HydroLinearEdgeBalancer>(new HydroLinearEdgeBalancer(feedRes->getWaterTemperatureSensor(), waterTempSetpoint, HYDRO_RANGE_TEMP_HALF, -HYDRO_RANGE_TEMP_HALF * 0.25f, HYDRO_RANGE_TEMP_HALF * 0.5f));
            HYDRO_SOFT_ASSERT(waterTempBalancer, SFP(HStr_Err_AllocationFailure));
            getScheduler()->setupWaterTemperatureBalancer(feedRes.get(), waterTempBalancer);
            feedRes->setWaterTemperatureBalancer(waterTempBalancer);
        }
        if (waterTempBalancer) {
            waterTempBalancer->setTargetSetpoint(waterTempSetpoint);
            waterTempBalancer->setMeasurementUnits(Hydro_UnitsType_Temperature_Celsius);
            waterTempBalancer->setEnabled(true);
        }
    } else {
        auto waterTempBalancer = feedRes->getWaterTemperatureBalancer();
        if (waterTempBalancer) { waterTempBalancer->setEnabled(false); }
    }

    if (feedRes->getAirTemperatureSensor()) {
        auto airTempBalancer = feedRes->getAirTemperatureBalancer();
        if (!airTempBalancer) {
            airTempBalancer = SharedPtr<HydroLinearEdgeBalancer>(new HydroLinearEdgeBalancer(feedRes->getAirTemperatureSensor(), airTempSetpoint, HYDRO_RANGE_TEMP_HALF, -HYDRO_RANGE_TEMP_HALF * 0.25f, HYDRO_RANGE_TEMP_HALF * 0.5f));
            HYDRO_SOFT_ASSERT(airTempBalancer, SFP(HStr_Err_AllocationFailure));
            getScheduler()->setupAirTemperatureBalancer(feedRes.get(), airTempBalancer);
            feedRes->setAirTemperatureBalancer(airTempBalancer);
        }
        if (airTempBalancer) {
            airTempBalancer->setTargetSetpoint(airTempSetpoint);
            airTempBalancer->setMeasurementUnits(Hydro_UnitsType_Temperature_Celsius);
            airTempBalancer->setEnabled(true);
        }
    } else {
        auto airTempBalancer = feedRes->getAirTemperatureBalancer();
        if (airTempBalancer) { airTempBalancer->setEnabled(false); }
    }

    if (feedRes->getAirCO2Sensor()) {
        auto co2Balancer = feedRes->getAirTemperatureBalancer();
        if (!co2Balancer) {
            co2Balancer = SharedPtr<HydroLinearEdgeBalancer>(new HydroLinearEdgeBalancer(feedRes->getAirCO2Sensor(), co2Setpoint, HYDRO_RANGE_CO2_HALF, -HYDRO_RANGE_CO2_HALF * 0.25f, HYDRO_RANGE_CO2_HALF * 0.5f));
            HYDRO_SOFT_ASSERT(co2Balancer, SFP(HStr_Err_AllocationFailure));
            getScheduler()->setupAirCO2Balancer(feedRes.get(), co2Balancer);
            feedRes->setAirCO2Balancer(co2Balancer);
        }
        if (co2Balancer) {
            co2Balancer->setTargetSetpoint(co2Setpoint);
            co2Balancer->setMeasurementUnits(Hydro_UnitsType_Concentration_PPM);
            co2Balancer->setEnabled(true);
        }
    } else {
        auto co2Balancer = feedRes->getAirCO2Balancer();
        if (co2Balancer) { co2Balancer->setEnabled(false); }
    }

    switch (stage) {
        case Init: {
            auto maxFeedingsDay = getScheduler()->schedulerData()->totalFeedingsPerDay;
            auto feedingsToday = feedRes->getFeedingsToday();

            if (!maxFeedingsDay) {
                canProcessAfter = (time_t)0;
            } else if (feedingsToday < maxFeedingsDay) {
                // this will force feedings to be spread out during the entire day
                canProcessAfter = unixTime(localDayStart()) + (time_t)(((float)SECS_PER_DAY / (maxFeedingsDay + 1)) * feedingsToday);
            } else {
                canProcessAfter = unixTime(localDayStart()) + SECS_PER_DAY; // no more feedings today
            }

            if (canProcessAfter > unixNow()) { clearActuatorReqs(); } // clear on wait
        } break;

        case TopOff: {
            if (!feedRes->isFilled()) {
                Vector<HydroActuatorAttachment, HYDRO_SCH_REQACTS_MAXSIZE> newActuatorReqs;
                auto topOffPumps = linksFilterPumpActuatorsByOutputReservoirAndSourceReservoirType<HYDRO_SCH_REQACTS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydro_ReservoirType_FreshWater);

                linksResolveActuatorsToAttachmentsByType<HYDRO_SCH_REQACTS_MAXSIZE>(topOffPumps, newActuatorReqs, Hydro_ActuatorType_WaterPump); // fresh water pumps
                if (!newActuatorReqs.size()) {
                    linksResolveActuatorsToAttachmentsByType<HYDRO_SCH_REQACTS_MAXSIZE>(topOffPumps, newActuatorReqs, Hydro_ActuatorType_PeristalticPump); // fresh water peristaltic pumps
                }

                HYDRO_SOFT_ASSERT(newActuatorReqs.size(), SFP(HStr_Err_MissingLinkage)); // no fresh water pumps
                setActuatorReqs(newActuatorReqs);
            } else {
                clearActuatorReqs();
            }
        } break;

        case PreFeed: {
            Vector<HydroActuatorAttachment, HYDRO_SCH_REQACTS_MAXSIZE> newActuatorReqs;
            auto aerators = linksFilterActuatorsByReservoirAndType<HYDRO_SCH_REQACTS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydro_ActuatorType_WaterAerator);

            linksResolveActuatorsToAttachmentsByType<HYDRO_SCH_REQACTS_MAXSIZE>(aerators, newActuatorReqs, Hydro_ActuatorType_WaterAerator);

            setActuatorReqs(newActuatorReqs);
        } break;

        case Feed: {
            Vector<HydroActuatorAttachment, HYDRO_SCH_REQACTS_MAXSIZE> newActuatorReqs;

            {   auto feedPumps = linksFilterPumpActuatorsBySourceReservoirAndOutputReservoirType<HYDRO_SCH_REQACTS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydro_ReservoirType_FeedWater);

                linksResolveActuatorsToAttachmentsByType<HYDRO_SCH_REQACTS_MAXSIZE>(feedPumps, newActuatorReqs, Hydro_ActuatorType_WaterPump); // feed water pump
            }

            if (!newActuatorReqs.size() && getController()->getSystemMode() == Hydro_SystemMode_DrainToWaste) { // prefers feed water pumps, else direct to waste is feed
                auto feedPumps = linksFilterPumpActuatorsBySourceReservoirAndOutputReservoirType<HYDRO_SCH_REQACTS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydro_ReservoirType_DrainageWater);

                linksResolveActuatorsToAttachmentsByType<HYDRO_SCH_REQACTS_MAXSIZE>(feedPumps, newActuatorReqs, Hydro_ActuatorType_WaterPump); // DTW feed water pump
            }

            HYDRO_SOFT_ASSERT(newActuatorReqs.size(), SFP(HStr_Err_MissingLinkage)); // no feed water pumps

            #if HYDRO_SCH_AERATORS_FEEDRUN
                {   auto aerators = linksFilterActuatorsByReservoirAndType<HYDRO_SCH_REQACTS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydro_ActuatorType_WaterAerator);

                    linksResolveActuatorsToAttachmentsByType<HYDRO_SCH_REQACTS_MAXSIZE>(aerators, newActuatorReqs, Hydro_ActuatorType_WaterAerator);
                }
            #endif

            setActuatorReqs(newActuatorReqs);
        } break;

        case Drain: {
            Vector<HydroActuatorAttachment, HYDRO_SCH_REQACTS_MAXSIZE> newActuatorReqs;
            auto drainPumps = linksFilterPumpActuatorsBySourceReservoirAndOutputReservoirType<HYDRO_SCH_REQACTS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydro_ReservoirType_DrainageWater);

            linksResolveActuatorsToAttachmentsByType<HYDRO_SCH_REQACTS_MAXSIZE>(drainPumps, newActuatorReqs, Hydro_ActuatorType_WaterPump); // drainage water pump

            HYDRO_SOFT_ASSERT(newActuatorReqs.size(), SFP(HStr_Err_MissingLinkage)); // no drainage water pumps
            setActuatorReqs(newActuatorReqs);
        } break;

        case Done: {
            clearActuatorReqs();
        } break;

        default:
            break;
    }

    #ifdef HYDRO_USE_VERBOSE_OUTPUT
    {   static int8_t _stageFS2 = (int8_t)-1; if (_stageFS2 != (int8_t)stage) {
        Serial.print(F("Feeding::~setupStaging stage: ")); Serial.println((_stageFS2 = (int8_t)stage)); flushYield(); } }
    #endif
}

void HydroFeeding::update()
{
    #ifdef HYDRO_USE_VERBOSE_OUTPUT
    {   static int8_t _stageFU1 = (int8_t)-1; if (_stageFU1 != (int8_t)stage) {
        Serial.print(F("Feeding::update stage: ")); Serial.println((_stageFU1 = (int8_t)stage)); flushYield(); } }
    #endif

    time_t time = unixNow();

    if ((!lastAirReport || time >= lastAirReport + getScheduler()->schedulerData()->airReportInterval) &&
        (getScheduler()->schedulerData()->airReportInterval > 0) && // 0 disables
        (feedRes->getAirTemperatureSensor() ||
         feedRes->getAirCO2Sensor())) {
        getLogger()->logProcess(feedRes.get(), SFP(HStr_Log_AirReport));
        logFeeding(HydroFeedingLogType_AirReport);
        lastAirReport = time;
    }

    switch (stage) {
        case Init: {
            if (!canProcessAfter || time >= canProcessAfter) {
                int cropsCount = 0;
                int cropsHungry = 0;

                {   auto crops = linksFilterCrops(feedRes->getLinkages());
                    cropsCount = crops.size();
                    for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
                        if (((HydroCrop *)(*cropIter))->needsFeeding()) { cropsHungry++; }
                    }
                }

                if (!cropsCount || cropsHungry / (float)cropsCount >= HYDRO_SCH_FEED_FRACTION - FLT_EPSILON) {
                    stage = TopOff; stageStart = time;
                    setupStaging();

                    if (actuatorReqs.size()) {
                        getLogger()->logProcess(feedRes.get(), SFP(HStr_Log_PreFeedTopOff), SFP(HStr_Log_HasBegan));
                    }
                }
            }
        } break;

        case TopOff: {
            if (feedRes->isFilled() || !actuatorReqs.size()) {
                stage = PreFeed; stageStart = time;
                canProcessAfter = 0; // will be used to track how long balancers stay balanced
                setupStaging();

                getLogger()->logProcess(feedRes.get(), SFP(HStr_Log_PreFeedBalancing), SFP(HStr_Log_HasBegan));
                if (actuatorReqs.size()) {
                    getLogger()->logMessage(SFP(HStr_Log_Field_Aerator_Duration), String(getScheduler()->schedulerData()->preFeedAeratorMins), String('m'));
                }
                if (feedRes->getWaterPHBalancer() || feedRes->getWaterTDSBalancer()) {
                    auto balancer = static_pointer_cast<HydroTimedDosingBalancer>(feedRes->getWaterPHBalancer() ? feedRes->getWaterPHBalancer() : feedRes->getWaterTDSBalancer());
                    if (balancer) {
                        getLogger()->logMessage(SFP(HStr_Log_Field_MixTime_Duration), timeSpanToString(TimeSpan(balancer->getMixTime())));
                    }
                }
                logFeeding(HydroFeedingLogType_WaterReport);
            }
        } break;

        case PreFeed: {
            if (!actuatorReqs.size() || time >= stageStart + (getScheduler()->schedulerData()->preFeedAeratorMins * SECS_PER_MIN)) {
                auto phBalancer = feedRes->getWaterPHBalancer();
                auto tdsBalancer = feedRes->getWaterTDSBalancer();
                auto waterTempBalancer = feedRes->getWaterTemperatureBalancer();

                if ((!phBalancer || (phBalancer->isEnabled() && phBalancer->isBalanced())) &&
                    (!tdsBalancer || (tdsBalancer->isEnabled() && tdsBalancer->isBalanced())) &&
                    (!waterTempBalancer || (waterTempBalancer->isEnabled() && waterTempBalancer->isBalanced()))) {
                    // Can proceed after above are marked balanced for min time
                    if (!canProcessAfter) { canProcessAfter = time + HYDRO_SCH_BALANCE_MINTIME; }
                    else if (time >= canProcessAfter) {
                        stage = Feed; stageStart = time;
                        setupStaging();

                        broadcastFeeding(HydroFeedingBroadcastType_Began);
                    }
                } else {
                    canProcessAfter = 0;
                }
            }
        } break;

        case Feed: {
            int cropsCount = 0;
            int cropsFed = 0;

            {   auto crops = linksFilterCrops(feedRes->getLinkages());
                cropsCount = crops.size();
                for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
                    if (!((HydroCrop *)(*cropIter))->needsFeeding()) { cropsFed++; }
                }
            }

            if (!cropsCount || cropsFed / (float)cropsCount >= HYDRO_SCH_FEED_FRACTION - FLT_EPSILON ||
                feedRes->isEmpty()) {
                stage = (getController()->getSystemMode() == Hydro_SystemMode_DrainToWaste ? Drain : Done);
                stageStart = time;
                setupStaging();

                broadcastFeeding(HydroFeedingBroadcastType_Ended);
            }
        } break;

        case Drain: {
            if (getController()->getSystemMode() != Hydro_SystemMode_DrainToWaste ||
                feedRes->isEmpty()) {
                stage = Done; stageStart = time;
                setupStaging();
            }
        } break;

        case Done: {
            stage = Init; stageStart = time;
            setupStaging();
        } break;

        default:
            break;
    }

    if (actuatorReqs.size()) {
        for (auto attachIter = actuatorReqs.begin(); attachIter != actuatorReqs.end(); ++attachIter) {
            attachIter->setupActivation();
            attachIter->enableActivation();
        }
    }

    #ifdef HYDRO_USE_VERBOSE_OUTPUT
    {   static int8_t _stageFU2 = (int8_t)-1; if (_stageFU2 != (int8_t)stage) {
        Serial.print(F("Feeding::~update stage: ")); Serial.println((_stageFU2 = (int8_t)stage)); flushYield(); } }
    #endif
}

void HydroFeeding::logFeeding(HydroFeedingLogType logType, bool withSetpoints)
{
    switch (logType) {
        case HydroFeedingLogType_WaterReport:
            if (withSetpoints) {
                {   auto ph = HydroSingleMeasurement(phSetpoint, Hydro_UnitsType_Alkalinity_pH_14);
                    getLogger()->logMessage(SFP(HStr_Log_Field_pH_Setpoint), measurementToString(ph));
                }
                {   auto tds = HydroSingleMeasurement(tdsSetpoint, Hydro_UnitsType_Concentration_TDS);
                    convertUnits(&tds, feedRes->getAirConcentrateUnits());
                    getLogger()->logMessage(SFP(HStr_Log_Field_TDS_Setpoint), measurementToString(tds, 1));
                }
                {   auto temp = HydroSingleMeasurement(waterTempSetpoint, Hydro_UnitsType_Temperature_Celsius);
                    convertUnits(&temp, feedRes->getTemperatureUnits());
                    getLogger()->logMessage(SFP(HStr_Log_Field_Temp_Setpoint), measurementToString(temp));
                }
            }
            if (feedRes->getWaterPHSensor(true)) {
                #ifdef HYDRO_USE_MULTITASKING
                    feedRes->getWaterPHSensor()->yieldForMeasurement();
                #endif
                auto ph = feedRes->getWaterPHSensorAttachment().getMeasurement();
                
                getLogger()->logMessage(SFP(HStr_Log_Field_pH_Measured), measurementToString(ph));
            }
            if (feedRes->getWaterTDSSensor(true)) {
                #ifdef HYDRO_USE_MULTITASKING
                    feedRes->getWaterTDSSensor()->yieldForMeasurement();
                #endif
                auto tds = feedRes->getWaterTDSSensorAttachment().getMeasurement();
                convertUnits(&tds, feedRes->getAirConcentrateUnits());
                getLogger()->logMessage(SFP(HStr_Log_Field_TDS_Measured), measurementToString(tds, 1));
            }
            if (feedRes->getWaterTemperatureSensor(true)) {
                #ifdef HYDRO_USE_MULTITASKING
                    feedRes->getWaterTemperatureSensor()->yieldForMeasurement();
                #endif
                auto temp = feedRes->getWaterTemperatureSensorAttachment().getMeasurement();
                convertUnits(&temp, feedRes->getTemperatureUnits());
                getLogger()->logMessage(SFP(HStr_Log_Field_Temp_Measured), measurementToString(temp));
            }
            break;

        case HydroFeedingLogType_AirReport:
            if (withSetpoints) {
                {   auto temp = HydroSingleMeasurement(airTempSetpoint, Hydro_UnitsType_Temperature_Celsius);
                    convertUnits(&temp, feedRes->getTemperatureUnits());
                    getLogger()->logMessage(SFP(HStr_Log_Field_Temp_Setpoint), measurementToString(temp));
                }
                {   auto co2 = HydroSingleMeasurement(co2Setpoint, Hydro_UnitsType_Concentration_PPM);
                    getLogger()->logMessage(SFP(HStr_Log_Field_CO2_Setpoint), measurementToString(co2));
                }
            }
            if (feedRes->getAirTemperatureSensor(true)) {
                #ifdef HYDRO_USE_MULTITASKING
                    feedRes->getAirTemperatureSensor()->yieldForMeasurement();
                #endif
                auto temp = feedRes->getAirTemperatureSensorAttachment().getMeasurement();
                convertUnits(&temp, feedRes->getTemperatureUnits());
                getLogger()->logMessage(SFP(HStr_Log_Field_Temp_Measured), measurementToString(temp));
            }
            if (feedRes->getAirCO2Sensor(true)) {
                #ifdef HYDRO_USE_MULTITASKING
                    feedRes->getAirCO2Sensor()->yieldForMeasurement();
                #endif
                auto co2 = feedRes->getAirCO2SensorAttachment().getMeasurement();
                getLogger()->logMessage(SFP(HStr_Log_Field_CO2_Measured), measurementToString(co2));
            }
            break;
    }
}

void HydroFeeding::broadcastFeeding(HydroFeedingBroadcastType broadcastType)
{
    getLogger()->logProcess(feedRes.get(), SFP(HStr_Log_FeedingSequence),
                            SFP(broadcastType == HydroFeedingBroadcastType_Began ? HStr_Log_HasBegan : HStr_Log_HasEnded));
    logFeeding(HydroFeedingLogType_WaterReport, false);

    broadcastType == HydroFeedingBroadcastType_Began ? feedRes->notifyFeedingBegan() : feedRes->notifyFeedingEnded();

    {   auto crops = linksFilterCrops(feedRes->getLinkages());
        for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
            broadcastType == HydroFeedingBroadcastType_Began ? ((HydroCrop *)(*cropIter))->notifyFeedingBegan()
                                                             : ((HydroCrop *)(*cropIter))->notifyFeedingEnded();
        }
    }
}


HydroLighting::HydroLighting(SharedPtr<HydroFeedReservoir> feedRes)
    : HydroProcess(feedRes), stage(Init), sprayStart(0), lightStart(0), lightEnd(0), augNatLightCease(0), augNatLightResume(0), lightTimeOffset(0), lightHours(0.0f)
{
    recalcLighting();
}

HydroLighting::~HydroLighting()
{
    clearActuatorReqs();
}

void HydroLighting::recalcLighting()
{
    float totalWeights = 0;
    float totalLightHours = 0;
    bool sprayingNeeded = false;

    {   auto crops = linksFilterCrops(feedRes->getLinkages());

        for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
            auto crop = (HydroCrop *)(*cropIter);
            auto cropPhase = (Hydro_CropPhase)constrain((int)(crop->getCropPhase()), 0, (int)Hydro_CropPhase_MainCount - 1);

            if ((int)cropPhase >= 0) {
                auto cropsLibData = hydroCropsLib.checkoutCropsData(crop->getCropType());

                if (cropsLibData) {
                    auto weight = crop->getFeedingWeight();
                    totalWeights += weight;
                    totalLightHours += (cropsLibData->dailyLightHours[cropPhase] * weight);
                    sprayingNeeded = sprayingNeeded || cropsLibData->needsSpraying();

                    hydroCropsLib.returnCropsData(cropsLibData);
                }
            }
        }
    }

    if (totalWeights < FLT_EPSILON) {
        totalWeights = 1.0f;
        totalLightHours = 12.0f;
    }

    {   lightHours = (totalLightHours / totalWeights);
        lightHours = constrain(lightHours, 0.0f, 24.0f);
        time_t dayLightSecs = lightHours * SECS_PER_HOUR;

        time_t daySprayerSecs = 0;
        if (sprayingNeeded && linksCountActuatorsByReservoirAndType(feedRes->getLinkages(), feedRes.get(), Hydro_ActuatorType_WaterSprayer)) {
            daySprayerSecs = getScheduler()->schedulerData()->preDawnSprayMins * SECS_PER_MIN;
        }

        time_t dayStart = localDayStart().unixtime();
        lightStart = dayStart + ((SECS_PER_DAY - dayLightSecs) >> 1);
        sprayStart = max(dayStart, lightStart - daySprayerSecs);
        lightStart = sprayStart + daySprayerSecs;
        lightEnd = lightStart + dayLightSecs;

        int natLightOffset = getScheduler()->getNaturalLightOffsetMins();
        if (natLightOffset >= 0) {
            time_t sunrise = getScheduler()->getDailyTwilight().getSunriseLocalTime().unixtime() + (natLightOffset * SECS_PER_MIN);
            time_t sunset = getScheduler()->getDailyTwilight().getSunsetLocalTime().unixtime() - (natLightOffset * SECS_PER_MIN);
            augNatLightCease = max(sunrise, lightStart);
            augNatLightResume = min(sunset, lightEnd);
        } else {
            augNatLightCease = lightEnd;
            augNatLightResume = lightEnd;
            lightTimeOffset = 0;
        }

        #ifdef HYDRO_USE_VERBOSE_OUTPUT // only works for singular feed res in system, otherwise output will be erratic
        {   static float _totalLightHours = 0;
            if (!isFPEqual(_totalLightHours, lightHours)) {
                _totalLightHours = lightHours;
                Serial.print(F("Lighting::recalcLighting lightHours: ")); Serial.println(_totalLightHours); flushYield(); } }
        #endif
    }

    setupStaging();
}

void HydroLighting::setupStaging()
{
    #ifdef HYDRO_USE_VERBOSE_OUTPUT
    {   static int8_t _stageLS1 = (int8_t)-1; if (_stageLS1 != (int8_t)stage) {
        Serial.print(F("Lighting::setupStaging stage: ")); Serial.println((_stageLS1 = (int8_t)stage)); flushYield(); } }
    #endif

    switch (stage) {
        case Init: {
            clearActuatorReqs();
        } break;

        case Spray: {
            Vector<HydroActuatorAttachment, HYDRO_SCH_REQACTS_MAXSIZE> newActuatorReqs;
            auto sprayers = linksFilterActuatorsByReservoirAndType<HYDRO_SCH_REQACTS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydro_ActuatorType_WaterSprayer);

            linksResolveActuatorsToAttachmentsByType<HYDRO_SCH_REQACTS_MAXSIZE>(sprayers, newActuatorReqs, Hydro_ActuatorType_WaterSprayer);

            setActuatorReqs(newActuatorReqs);
        } break;

        case Light: {
            Vector<HydroActuatorAttachment, HYDRO_SCH_REQACTS_MAXSIZE> newActuatorReqs;
            auto lights = linksFilterActuatorsByReservoirAndType<HYDRO_SCH_REQACTS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydro_ActuatorType_GrowLights);

            linksResolveActuatorsToAttachmentsByType<HYDRO_SCH_REQACTS_MAXSIZE>(lights, newActuatorReqs, Hydro_ActuatorType_GrowLights);

            setActuatorReqs(newActuatorReqs);
        } break;

        case NatLight: {
            clearActuatorReqs();
        } break;

        case Done: {
            clearActuatorReqs();
        } break;

        default:
            break;
    }

    #ifdef HYDRO_USE_VERBOSE_OUTPUT
    {   static int8_t _stageLS2 = (int8_t)-1; if (_stageLS2 != (int8_t)stage) {
        Serial.print(F("Lighting::~setupStaging stage: ")); Serial.println((_stageLS2 = (int8_t)stage)); flushYield(); } }
    #endif
}

void HydroLighting::update()
{
    #ifdef HYDRO_USE_VERBOSE_OUTPUT
    {   static int8_t _stageLU1 = (int8_t)-1; if (_stageLU1 != (int8_t)stage) {
        Serial.print(F("Lighting::update stage: ")); Serial.println((_stageLU1 = (int8_t)stage)); flushYield(); } }
    #endif

    time_t time = unixNow();
    time_t currTime = localTime(time).unixtime();

    switch (stage) {
        case Init: {
            if (currTime >= sprayStart && currTime < lightEnd) {
                stage = Spray; stageStart = time;
                setupStaging();

                if (lightStart > sprayStart) {
                    getLogger()->logProcess(feedRes.get(), SFP(HStr_Log_PreDawnSpraying), SFP(HStr_Log_HasBegan));
                    getLogger()->logMessage(SFP(HStr_Log_Field_Sprayer_Duration), String(getScheduler()->schedulerData()->preDawnSprayMins), String('m'));
                    getLogger()->logMessage(SFP(HStr_Log_Field_Time_Start), DateTime((uint32_t)sprayStart).timestamp(DateTime::TIMESTAMP_TIME));
                    getLogger()->logMessage(SFP(HStr_Log_Field_Time_Finish), DateTime((uint32_t)lightStart).timestamp(DateTime::TIMESTAMP_TIME));
                }
            }
        } break;

        case Spray: {
            if (currTime >= lightStart) {
                stage = Light; stageStart = time;
                setupStaging();

                getLogger()->logProcess(feedRes.get(), SFP(HStr_Log_LightingSequence), SFP(HStr_Log_HasBegan));
                getLogger()->logMessage(SFP(HStr_Log_Field_Light_Duration), roundToString(lightHours), String('h'));
                getLogger()->logMessage(SFP(HStr_Log_Field_Time_Start), DateTime((uint32_t)lightStart).timestamp(DateTime::TIMESTAMP_TIME));
                getLogger()->logMessage(SFP(HStr_Log_Field_Time_Finish), DateTime((uint32_t)lightEnd).timestamp(DateTime::TIMESTAMP_TIME));
            } else {
                stage = Done; stageStart = time;
                setupStaging();
            }
        } break;

        case Light: {
            if (currTime >= lightEnd) {
                stage = Done; stageStart = time;
                setupStaging();

                getLogger()->logProcess(feedRes.get(), SFP(HStr_Log_LightingSequence), SFP(HStr_Log_HasEnded));
                getLogger()->logMessage(SFP(HStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan((time - stageStart) + lightTimeOffset)));
                lightTimeOffset = 0;
            } else if (currTime >= augNatLightCease && currTime < augNatLightResume) {
                lightTimeOffset = time - stageStart;
                stage = NatLight; stageStart = time;
                setupStaging();

                getLogger()->logProcess(feedRes.get(), SFP(HStr_Log_NatLightingSequence), SFP(HStr_Log_HasBegan));
                getLogger()->logMessage(SFP(HStr_Log_Field_Light_Duration), roundToString((augNatLightResume - augNatLightCease) / (float)SECS_PER_HOUR), String('h'));
                getLogger()->logMessage(SFP(HStr_Log_Field_Time_Start), DateTime((uint32_t)augNatLightCease).timestamp(DateTime::TIMESTAMP_TIME));
                getLogger()->logMessage(SFP(HStr_Log_Field_Time_Finish), DateTime((uint32_t)augNatLightResume).timestamp(DateTime::TIMESTAMP_TIME));
            }
        } break;

        case NatLight: {
            if (currTime >= augNatLightResume) {
                stage = Light; stageStart = time;
                setupStaging();

                getLogger()->logProcess(feedRes.get(), SFP(HStr_Log_NatLightingSequence), SFP(HStr_Log_HasEnded));
                getLogger()->logMessage(SFP(HStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(time - stageStart)));
            }
        } break;

        case Done: {
            stage = Init; stageStart = time;
            setupStaging();
        } break;

        default:
            break;
    }

    if (actuatorReqs.size()) {
        for (auto attachIter = actuatorReqs.begin(); attachIter != actuatorReqs.end(); ++attachIter) {
            attachIter->setupActivation();
            attachIter->enableActivation();
        }
    }

    #ifdef HYDRO_USE_VERBOSE_OUTPUT
    {   static int8_t _stageLU2 = (int8_t)-1; if (_stageLU2 != (int8_t)stage) {
        Serial.print(F("Lighting::~update stage: ")); Serial.println((_stageLU2 = (int8_t)stage)); flushYield(); } }
    #endif
}


HydroSchedulerSubData::HydroSchedulerSubData()
    : HydroSubData(), baseFeedMultiplier(1), weeklyDosingRates{1}, stdDosingRates{1,0.5,0.5},
      totalFeedingsPerDay(0), preFeedAeratorMins(30), preDawnSprayMins(60), airReportInterval(8 * SECS_PER_HOUR), natLightOffsetMins(-1)
{
    type = 0; // no type differentiation
}

void HydroSchedulerSubData::toJSONObject(JsonObject &objectOut) const
{
    //HydroSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (!isFPEqual(baseFeedMultiplier, 1.0f)) { objectOut[SFP(HStr_Key_BaseFeedMultiplier)] = baseFeedMultiplier; }
    bool hasWeeklyDosings = arrayElementsEqual(weeklyDosingRates, HYDRO_CROPS_GROWWEEKS_MAX, 1.0f);
    if (hasWeeklyDosings) { objectOut[SFP(HStr_Key_WeeklyDosingRates)] = commaStringFromArray(weeklyDosingRates, HYDRO_CROPS_GROWWEEKS_MAX); }
    bool hasStandardDosings = !isFPEqual(stdDosingRates[0], 1.0f) || !isFPEqual(stdDosingRates[1], 0.5f) || !isFPEqual(stdDosingRates[2], 0.5f);
    if (hasStandardDosings) { objectOut[SFP(HStr_Key_StdDosingRates)] = commaStringFromArray(stdDosingRates, 3); }
    if (totalFeedingsPerDay > 0) { objectOut[SFP(HStr_Key_TotalFeedingsPerDay)] = totalFeedingsPerDay; }
    if (preFeedAeratorMins != 30) { objectOut[SFP(HStr_Key_PreFeedAeratorMins)] = preFeedAeratorMins; }
    if (preDawnSprayMins != 60) { objectOut[SFP(HStr_Key_PreDawnSprayMins)] = preDawnSprayMins; }
    if (airReportInterval != (8 * SECS_PER_HOUR)) { objectOut[SFP(HStr_Key_AirReportInterval)] = airReportInterval; }
    if (natLightOffsetMins != -1) { objectOut[SFP(HStr_Key_NaturalLightOffsetMins)] = natLightOffsetMins; }
}

void HydroSchedulerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    baseFeedMultiplier = objectIn[SFP(HStr_Key_BaseFeedMultiplier)] | baseFeedMultiplier;
    JsonVariantConst weeklyDosingRatesVar = objectIn[SFP(HStr_Key_WeeklyDosingRates)];
    commaStringToArray(weeklyDosingRatesVar, weeklyDosingRates, HYDRO_CROPS_GROWWEEKS_MAX);
    JsonVariantConst stdDosingRatesVar = objectIn[SFP(HStr_Key_StdDosingRates)];
    commaStringToArray(stdDosingRatesVar, stdDosingRates, 3);
    totalFeedingsPerDay = objectIn[SFP(HStr_Key_TotalFeedingsPerDay)] | totalFeedingsPerDay;
    preFeedAeratorMins = objectIn[SFP(HStr_Key_PreFeedAeratorMins)] | preFeedAeratorMins;
    preDawnSprayMins = objectIn[SFP(HStr_Key_PreDawnSprayMins)] | preDawnSprayMins;
    airReportInterval = objectIn[SFP(HStr_Key_AirReportInterval)] | airReportInterval;
    natLightOffsetMins = objectIn[SFP(HStr_Key_NaturalLightOffsetMins)] | natLightOffsetMins;
}
