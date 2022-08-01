/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Scheduler
*/

#include "Hydroponics.h"

HydroponicsScheduler::HydroponicsScheduler()
    : _inDaytimeMode(false), _needsScheduling(false), _lastDayNum(-1)
{ ; }

HydroponicsScheduler::~HydroponicsScheduler()
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

void HydroponicsScheduler::update()
{
    if (hasSchedulerData()) {
        {   DateTime currTime = getCurrentTime();
            bool daytimeMode = currTime.hour() >= HYDRUINO_CROP_NIGHT_ENDHR && currTime.hour() < HYDRUINO_CROP_NIGHT_BEGINHR;

            if (_inDaytimeMode != daytimeMode) {
                _inDaytimeMode = daytimeMode;
                setNeedsScheduling();
                // TODO: UI update notify on day/night transition
            }

            if (_lastDayNum != currTime.day()) {
                // only log uptime upon actual day change and if uptime has been at least 1d
                if (getLoggerInstance()->getSystemUptime() >= SECS_PER_DAY) {
                    getLoggerInstance()->logSystemUptime();
                }
                broadcastDayChange();
            }
        }

        if (_needsScheduling) { performScheduling(); }

        for (auto feedingIter = _feedings.begin(); feedingIter != _feedings.end(); ++feedingIter) {
            feedingIter->second->update();
        }
        for (auto lightingIter = _lightings.begin(); lightingIter != _lightings.end(); ++lightingIter) {
            lightingIter->second->update();
        }
    }
}

void HydroponicsScheduler::handleLowMemory()
{
    #ifdef HYDRUINO_USE_STDCPP_CONTAINERS
        _feedings.shrink_to_fit();
        _lightings.shrink_to_fit();
        for (auto feedingIter = _feedings.begin(); feedingIter != _feedings.end(); ++feedingIter) {
            if (feedingIter->second) { feedingIter->second->actuatorReqs.shrink_to_fit(); }
        }
        for (auto lightingIter = _lightings.begin(); lightingIter != _lightings.end(); ++lightingIter) {
            if (lightingIter->second) { lightingIter->second->actuatorReqs.shrink_to_fit(); }
        }
    #endif
}

void HydroponicsScheduler::setupWaterPHBalancer(HydroponicsReservoir *reservoir, shared_ptr<HydroponicsBalancer> waterPHBalancer)
{
    if (reservoir && waterPHBalancer) {
        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type incActuators;
            auto phUpPumps = linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydroponics_ReservoirType_PhUpSolution);
            float dosingRate = getCombinedDosingRate(reservoir, Hydroponics_ReservoirType_PhUpSolution);

            linksResolveActuatorsPairRateByType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(phUpPumps, dosingRate, incActuators, Hydroponics_ActuatorType_PeristalticPump);
            if (!incActuators.size()) { // prefer peristaltic, else use full pump
                linksResolveActuatorsPairRateByType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(phUpPumps, dosingRate, incActuators, Hydroponics_ActuatorType_WaterPump);
            }

            waterPHBalancer->setIncrementActuators(incActuators);
        }

        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type decActuators;
            auto phDownPumps = linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType<HYDRUINO_BAL_DECACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydroponics_ReservoirType_PhDownSolution);
            float dosingRate = getCombinedDosingRate(reservoir, Hydroponics_ReservoirType_PhDownSolution);

            linksResolveActuatorsPairRateByType<HYDRUINO_BAL_DECACTUATORS_MAXSIZE>(phDownPumps, dosingRate, decActuators, Hydroponics_ActuatorType_PeristalticPump);
            if (!decActuators.size()) { // prefer peristaltic, else use full pump
                linksResolveActuatorsPairRateByType<HYDRUINO_BAL_DECACTUATORS_MAXSIZE>(phDownPumps, dosingRate, decActuators, Hydroponics_ActuatorType_WaterPump);
            }

            waterPHBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setupWaterTDSBalancer(HydroponicsReservoir *reservoir, shared_ptr<HydroponicsBalancer> waterTDSBalancer)
{
    if (reservoir && waterTDSBalancer) {
        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type incActuators;
            float dosingRate = getCombinedDosingRate(reservoir, Hydroponics_ReservoirType_NutrientPremix);

            if (dosingRate > FLT_EPSILON) {
                auto nutrientPumps = linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydroponics_ReservoirType_NutrientPremix);

                linksResolveActuatorsPairRateByType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(nutrientPumps, dosingRate, incActuators, Hydroponics_ActuatorType_PeristalticPump);
                if (!incActuators.size()) { // prefer peristaltic, else use full pump
                    linksResolveActuatorsPairRateByType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(nutrientPumps, dosingRate, incActuators, Hydroponics_ActuatorType_WaterPump);
                }
            }

            if (Hydroponics::_activeInstance->_additives.size()) {
                int prevIncSize = incActuators.size();

                for (int reservoirType = Hydroponics_ReservoirType_CustomAdditive1; reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount; ++reservoirType) {
                    if (Hydroponics::_activeInstance->getCustomAdditiveData((Hydroponics_ReservoirType)reservoirType)) {
                        dosingRate = getCombinedDosingRate(reservoir, (Hydroponics_ReservoirType)reservoirType);

                        if (dosingRate > FLT_EPSILON) {
                            auto nutrientPumps = linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, (Hydroponics_ReservoirType)reservoirType);

                            linksResolveActuatorsPairRateByType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(nutrientPumps, dosingRate, incActuators, Hydroponics_ActuatorType_PeristalticPump);
                            if (incActuators.size() == prevIncSize) { // prefer peristaltic, else use full pump
                                linksResolveActuatorsPairRateByType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(nutrientPumps, dosingRate, incActuators, Hydroponics_ActuatorType_WaterPump);
                            }
                        }

                        prevIncSize = incActuators.size();
                    }
                }
            }

            waterTDSBalancer->setIncrementActuators(incActuators);
        }

        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type decActuators;
            float dosingRate = getCombinedDosingRate(reservoir, Hydroponics_ReservoirType_FreshWater);

            if (dosingRate > FLT_EPSILON) {
                auto dilutionPumps = linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType<HYDRUINO_BAL_DECACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydroponics_ReservoirType_NutrientPremix);

                linksResolveActuatorsPairRateByType<HYDRUINO_BAL_DECACTUATORS_MAXSIZE>(dilutionPumps, dosingRate, decActuators, Hydroponics_ActuatorType_PeristalticPump);
                if (!decActuators.size()) { // prefer peristaltic, else use full pump
                    linksResolveActuatorsPairRateByType<HYDRUINO_BAL_DECACTUATORS_MAXSIZE>(dilutionPumps, dosingRate, decActuators, Hydroponics_ActuatorType_WaterPump);
                }
            }

            waterTDSBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setupWaterTemperatureBalancer(HydroponicsReservoir *reservoir, shared_ptr<HydroponicsBalancer> waterTempBalancer)
{
    if (reservoir && waterTempBalancer) {
        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type incActuators;
            auto heaters = linksFilterActuatorsByReservoirAndType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydroponics_ActuatorType_WaterHeater);

            linksResolveActuatorsPairRateByType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(heaters, 1.0f, incActuators, Hydroponics_ActuatorType_WaterHeater);

            waterTempBalancer->setIncrementActuators(incActuators);
        }

        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type decActuators;
            waterTempBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setupAirTemperatureBalancer(HydroponicsReservoir *reservoir, shared_ptr<HydroponicsBalancer> airTempBalancer)
{
    if (reservoir && airTempBalancer) {
        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type incActuators;
            airTempBalancer->setIncrementActuators(incActuators);
        }

        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type decActuators;
            auto fans = linksFilterActuatorsByReservoirAndType<HYDRUINO_BAL_DECACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydroponics_ActuatorType_FanExhaust);

            linksResolveActuatorsPairRateByType<HYDRUINO_BAL_DECACTUATORS_MAXSIZE>(fans, 1.0f, decActuators, Hydroponics_ActuatorType_FanExhaust);
        }
    }
}

void HydroponicsScheduler::setupAirCO2Balancer(HydroponicsReservoir *reservoir, shared_ptr<HydroponicsBalancer> airCO2Balancer)
{
    if (reservoir && airCO2Balancer) {
        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type incActuators;
            auto fans = linksFilterActuatorsByReservoirAndType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydroponics_ActuatorType_FanExhaust);

            linksResolveActuatorsPairRateByType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(fans, 1.0f, incActuators, Hydroponics_ActuatorType_FanExhaust);

            airCO2Balancer->setIncrementActuators(incActuators);
        }

        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type decActuators;
            airCO2Balancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setBaseFeedMultiplier(float baseFeedMultiplier)
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    if (hasSchedulerData()) {
        Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
        schedulerData()->baseFeedMultiplier = baseFeedMultiplier;

        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setWeeklyDosingRate(int weekIndex, float dosingRate, Hydroponics_ReservoirType reservoirType)
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!hasSchedulerData() || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX) {
        if (reservoirType == Hydroponics_ReservoirType_NutrientPremix) {
            Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
            schedulerData()->weeklyDosingRates[weekIndex] = dosingRate;

            setNeedsScheduling();
        } else if (reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 && reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
            HydroponicsCustomAdditiveData newAdditiveData(reservoirType);
            newAdditiveData._bumpRevIfNotAlreadyModded();
            newAdditiveData.weeklyDosingRates[weekIndex] = dosingRate;
            Hydroponics::_activeInstance->setCustomAdditiveData(&newAdditiveData);

            setNeedsScheduling();
        } else {
            HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_UnsupportedOperation));
        }
    }
}

void HydroponicsScheduler::setStandardDosingRate(float dosingRate, Hydroponics_ReservoirType reservoirType)
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!hasSchedulerData() || (reservoirType >= Hydroponics_ReservoirType_FreshWater && reservoirType < Hydroponics_ReservoirType_CustomAdditive1), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && (reservoirType >= Hydroponics_ReservoirType_FreshWater && reservoirType < Hydroponics_ReservoirType_CustomAdditive1)) {
        Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
        schedulerData()->stdDosingRates[reservoirType - Hydroponics_ReservoirType_FreshWater] = dosingRate;

        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setLastWeekAsFlush(Hydroponics_CropType cropType)
{
    auto cropLibData = getCropsLibraryInstance()->checkoutCropsData(cropType);
    if (cropLibData) {
        setFlushWeek(cropLibData->totalGrowWeeks-1);
        getCropsLibraryInstance()->returnCropsData(cropLibData);
    }
}

void HydroponicsScheduler::setFlushWeek(int weekIndex)
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!hasSchedulerData() || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX) {
        schedulerData()->weeklyDosingRates[weekIndex] = 0;

        for (Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_CustomAdditive1;
             reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount;
             reservoirType = (Hydroponics_ReservoirType)((int)reservoirType + 1)) {
            auto additiveData = Hydroponics::_activeInstance->getCustomAdditiveData(reservoirType);
            if (additiveData) {
                HydroponicsCustomAdditiveData newAdditiveData = *additiveData;
                newAdditiveData._bumpRevIfNotAlreadyModded();
                newAdditiveData.weeklyDosingRates[weekIndex] = 0;
                Hydroponics::_activeInstance->setCustomAdditiveData(&newAdditiveData);
            }
        }

        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setTotalFeedingsDay(unsigned int feedingsDay)
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasSchedulerData() && schedulerData()->totalFeedingsDay != feedingsDay) {
        Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
        schedulerData()->totalFeedingsDay = feedingsDay;

        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setPreFeedAeratorMins(unsigned int aeratorMins)
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasSchedulerData() && schedulerData()->preFeedAeratorMins != aeratorMins) {
        Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
        schedulerData()->preFeedAeratorMins = aeratorMins;

        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setPreLightSprayMins(unsigned int sprayMins)
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasSchedulerData() && schedulerData()->preLightSprayMins != sprayMins) {
        Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
        schedulerData()->preLightSprayMins = sprayMins;

        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setAirReportInterval(TimeSpan interval)
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasSchedulerData() && schedulerData()->airReportInterval != interval.totalseconds()) {
        Hydroponics::_activeInstance->_systemData->_bumpRevIfNotAlreadyModded();
        schedulerData()->airReportInterval = interval.totalseconds();
    }
}

float HydroponicsScheduler::getCombinedDosingRate(HydroponicsReservoir *reservoir, Hydroponics_ReservoirType reservoirType)
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!hasSchedulerData() || reservoir, SFP(HStr_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(!hasSchedulerData() || !reservoir || (reservoirType >= Hydroponics_ReservoirType_NutrientPremix &&
                                                               reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && reservoir &&
        (reservoirType >= Hydroponics_ReservoirType_NutrientPremix &&
         reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount)) {
        auto crops = linksFilterCrops(reservoir->getLinkages());
        float totalWeights = 0;
        float totalDosing = 0;

        for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
            auto crop = (HydroponicsCrop *)(*cropIter);
            if (crop) {
                if (reservoirType <= Hydroponics_ReservoirType_NutrientPremix) {
                    totalWeights += crop->getFeedingWeight();
                    totalDosing += schedulerData()->weeklyDosingRates[constrain(crop->getGrowWeek(), 0, crop->getTotalGrowWeeks() - 1)];
                } else if (reservoirType < Hydroponics_ReservoirType_CustomAdditive1) {
                    totalWeights += crop->getFeedingWeight();
                    totalDosing += schedulerData()->stdDosingRates[reservoirType - Hydroponics_ReservoirType_FreshWater];
                } else {
                    auto additiveData = Hydroponics::_activeInstance->getCustomAdditiveData(reservoirType);
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

float HydroponicsScheduler::getBaseFeedMultiplier() const
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    return hasSchedulerData() ? schedulerData()->baseFeedMultiplier : 1.0f;
}

float HydroponicsScheduler::getWeeklyDosingRate(int weekIndex, Hydroponics_ReservoirType reservoirType) const
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!hasSchedulerData() || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX) {
        if (reservoirType == Hydroponics_ReservoirType_NutrientPremix) {
            return schedulerData()->weeklyDosingRates[weekIndex];
        } else if (reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 && reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
            auto additiveDate = Hydroponics::_activeInstance->getCustomAdditiveData(reservoirType);
            return additiveDate ? additiveDate->weeklyDosingRates[weekIndex] : 0.0f;
        } else {
            HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_UnsupportedOperation));
        }
    }

    return 0.0f;
}

float HydroponicsScheduler::getStandardDosingRate(Hydroponics_ReservoirType reservoirType) const
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!hasSchedulerData() || (reservoirType >= Hydroponics_ReservoirType_FreshWater && reservoirType < Hydroponics_ReservoirType_CustomAdditive1), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && reservoirType >= Hydroponics_ReservoirType_FreshWater && reservoirType < Hydroponics_ReservoirType_CustomAdditive1) {
        return schedulerData()->stdDosingRates[reservoirType - Hydroponics_ReservoirType_FreshWater];
    }

    return 0.0f;
}

bool HydroponicsScheduler::isFlushWeek(int weekIndex)
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!hasSchedulerData() || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX), SFP(HStr_Err_InvalidParameter));

    if (hasSchedulerData() && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX) {
        return isFPEqual(schedulerData()->weeklyDosingRates[weekIndex], 0.0f);
    }

    return false;
}

unsigned int HydroponicsScheduler::getTotalFeedingsDay() const
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    return hasSchedulerData() ? schedulerData()->totalFeedingsDay : 0;
}

unsigned int HydroponicsScheduler::getPreFeedAeratorMins() const
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    return hasSchedulerData() ? schedulerData()->preFeedAeratorMins : 0;
}

unsigned int HydroponicsScheduler::getPreLightSprayMins() const
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    return hasSchedulerData() ? schedulerData()->preLightSprayMins : 0;
}

TimeSpan HydroponicsScheduler::getAirReportInterval() const
{
    HYDRUINO_SOFT_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));
    return TimeSpan(hasSchedulerData() ? schedulerData()->airReportInterval : 0);
}

void HydroponicsScheduler::updateDayTracking()
{
    auto currTime = getCurrentTime();
    _lastDayNum = currTime.day();
    _inDaytimeMode = currTime.hour() >= HYDRUINO_CROP_NIGHT_ENDHR && currTime.hour() < HYDRUINO_CROP_NIGHT_BEGINHR;
    setNeedsScheduling();
    // TODO: UI update notify on major time event
}

void HydroponicsScheduler::performScheduling()
{
    HYDRUINO_HARD_ASSERT(hasSchedulerData(), SFP(HStr_Err_NotYetInitialized));

    for (auto iter = Hydroponics::_activeInstance->_objects.begin(); iter != Hydroponics::_activeInstance->_objects.end(); ++iter) {
        if (iter->second && iter->second->isReservoirType()) {
            if (((HydroponicsReservoir *)(iter->second.get()))->isFeedClass()) {
                auto feedReservoir = static_pointer_cast<HydroponicsFeedReservoir>(iter->second);

                {   auto feedingIter = _feedings.find(feedReservoir->getKey());

                    if (linksCountCrops(feedReservoir->getLinkages())) {
                        if (feedingIter != _feedings.end()) {
                            if (feedingIter->second) {
                                feedingIter->second->recalcFeeding();
                            }
                        } else {
                            #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
                                Serial.print(F("Scheduler::performScheduling Crop linkages found for: ")); Serial.print(iter->second->getKeyString());
                                Serial.print(':'); Serial.print(' '); Serial.println(linksCountCrops(feedReservoir->getLinkages())); flushYield();
                            #endif

                            HydroponicsFeeding *feeding = new HydroponicsFeeding(feedReservoir);
                            HYDRUINO_SOFT_ASSERT(feeding, SFP(HStr_Err_AllocationFailure));
                            if (feeding) { _feedings[feedReservoir->getKey()] = feeding; }
                        }
                    } else if (feedingIter != _feedings.end()) { // No crops to warrant process -> delete if exists
                        #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
                            Serial.print(F("Scheduler::performScheduling NO more crop linkages found for: ")); Serial.println(iter->second->getKeyString()); flushYield();
                        #endif
                        if (feedingIter->second) { delete feedingIter->second; }
                        _feedings.erase(feedingIter);
                    }
                }

                {   auto lightingIter = _lightings.find(feedReservoir->getKey());

                    if (linksCountActuatorsByReservoirAndType(feedReservoir->getLinkages(), feedReservoir.get(), Hydroponics_ActuatorType_WaterSprayer) ||
                        linksCountActuatorsByReservoirAndType(feedReservoir->getLinkages(), feedReservoir.get(), Hydroponics_ActuatorType_GrowLights)) {
                        if (lightingIter != _lightings.end()) {
                            if (lightingIter->second) {
                                lightingIter->second->recalcLighting();
                            }
                        } else {
                            #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
                                Serial.print(F("Scheduler::performScheduling Light linkages found for: ")); Serial.print(iter->second->getKeyString()); Serial.print(':'); Serial.print(' ');
                                Serial.println(linksCountActuatorsByReservoirAndType(feedReservoir->getLinkages(), feedReservoir.get(), Hydroponics_ActuatorType_WaterSprayer) + linksCountActuatorsByReservoirAndType(feedReservoir->getLinkages(), feedReservoir.get(), Hydroponics_ActuatorType_GrowLights)); flushYield();
                            #endif

                            HydroponicsLighting *lighting = new HydroponicsLighting(feedReservoir);
                            HYDRUINO_SOFT_ASSERT(lighting, SFP(HStr_Err_AllocationFailure));
                            if (lighting) { _lightings[feedReservoir->getKey()] = lighting; }
                        }
                    } else if (lightingIter != _lightings.end()) { // No lights or sprayers to warrant process -> delete if exists
                        #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
                            Serial.print(F("Scheduler::performScheduling NO more light linkages found for: ")); Serial.println(iter->second->getKeyString()); flushYield();
                        #endif
                        if (lightingIter->second) { delete lightingIter->second; }
                        _lightings.erase(lightingIter);
                    }
                }
            }
        }
    }

    _needsScheduling = false;
}

void HydroponicsScheduler::broadcastDayChange()
{
    updateDayTracking();

    #ifndef HYDRUINO_DISABLE_MULTITASKING
        // these can take a while to complete
        taskManager.scheduleOnce(0, []{
            if (getHydroponicsInstance()) {
                Hydroponics::_activeInstance->notifyDayChanged();
            }
            yield();
            if (getLoggerInstance()) {
                getLoggerInstance()->notifyDayChanged();
            }
            yield();
            if (getPublisherInstance()) {
                getPublisherInstance()->notifyDayChanged();
            }
            yield();
        });
    #else
        if (getHydroponicsInstance()) {
            Hydroponics::_activeInstance->notifyDayChanged();
        }
        if (getLoggerInstance()) {
            getLoggerInstance()->notifyDayChanged();
        }
        if (getPublisherInstance()) {
            getPublisherInstance()->notifyDayChanged();
        }
    #endif
}


HydroponicsProcess::HydroponicsProcess(shared_ptr<HydroponicsFeedReservoir> feedResIn)
    : feedRes(feedResIn), stageStart(0)
{ ; }

void HydroponicsProcess::clearActuatorReqs()
{
    while (actuatorReqs.size()) {
        actuatorReqs.begin()->get()->disableActuator();
        actuatorReqs.erase(actuatorReqs.begin());
    }
}

void HydroponicsProcess::setActuatorReqs(const Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type &actuatorReqsIn)
{
    for (auto actuatorIter = actuatorReqs.begin(); actuatorIter != actuatorReqs.end(); ++actuatorIter) {
        bool found = false;
        auto key = (*actuatorIter)->getKey();
    
        for (auto actuatorInIter = actuatorReqsIn.begin(); actuatorInIter != actuatorReqsIn.end(); ++actuatorInIter) {
            if (key == (*actuatorInIter)->getKey()) {
                found = true;
                break;
            }
        }
    
        if (!found && (*actuatorIter)->isEnabled()) { // disables actuators not found in new list
            (*actuatorIter)->disableActuator();
        }
    }

    {   actuatorReqs.clear();
        for (auto actuatorInIter = actuatorReqsIn.begin(); actuatorInIter != actuatorReqsIn.end(); ++actuatorInIter) {
            auto actuator = (*actuatorInIter);
            actuatorReqs.push_back(actuator);
        }
    }  
}


HydroponicsFeeding::HydroponicsFeeding(shared_ptr<HydroponicsFeedReservoir> feedRes)
    : HydroponicsProcess(feedRes), stage(Unknown), canFeedAfter(0), lastAirReport(0),
      phSetpoint(0), tdsSetpoint(0), waterTempSetpoint(0), airTempSetpoint(0), co2Setpoint(0)
{
    reset();
}

HydroponicsFeeding::~HydroponicsFeeding()
{
    clearActuatorReqs();
}

void HydroponicsFeeding::reset()
{
    clearActuatorReqs();
    stage = Init; stageStart = unixNow();
    recalcFeeding();
}

void HydroponicsFeeding::recalcFeeding()
{
    float totalWeights = 0;
    float totalSetpoints[5] = {0,0,0,0,0};

    {   auto crops = linksFilterCrops(feedRes->getLinkages());
        for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
            auto crop = (HydroponicsCrop *)(*cropIter);
            auto cropsLibData = getCropsLibraryInstance()->checkoutCropsData(crop->getCropType());

            if (cropsLibData) {
                float weight = crop->getFeedingWeight();
                totalWeights += weight;

                float feedRate = ((cropsLibData->tdsRange[0] + cropsLibData->tdsRange[1]) * 0.5);
                if (!getSchedulerInstance()->inDaytimeMode()) {
                    feedRate *= cropsLibData->nightlyFeedRate;
                }
                feedRate *= getSchedulerInstance()->getBaseFeedMultiplier();

                totalSetpoints[0] += feedRate * weight;
                totalSetpoints[1] += ((cropsLibData->phRange[0] + cropsLibData->phRange[1]) * 0.5) * weight;
                totalSetpoints[2] += ((cropsLibData->waterTempRange[0] + cropsLibData->waterTempRange[1]) * 0.5) * weight;
                totalSetpoints[3] += ((cropsLibData->airTempRange[0] + cropsLibData->airTempRange[1]) * 0.5) * weight;
                totalSetpoints[4] += cropsLibData->co2Levels[(crop->getCropPhase() <= Hydroponics_CropPhase_Vegetative ? 0 : 1)] * weight;

                getCropsLibraryInstance()->returnCropsData(cropsLibData);
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

    #ifdef HYDRUINO_USE_VERBOSE_OUTPUT // only works for singular feed res in system, otherwise output will be erratic
    {   static float _totalSetpoints[5] = {0,0,0,0,0};
        if (!isFPEqual(_totalSetpoints[0], totalSetpoints[0]) || !isFPEqual(_totalSetpoints[1], totalSetpoints[1]) || !isFPEqual(_totalSetpoints[2], totalSetpoints[2]) || !isFPEqual(_totalSetpoints[3], totalSetpoints[3]) || !isFPEqual(_totalSetpoints[4], totalSetpoints[4])) {
            _totalSetpoints[0] = totalSetpoints[0]; _totalSetpoints[1] = totalSetpoints[1]; _totalSetpoints[2] = totalSetpoints[2]; _totalSetpoints[3] = totalSetpoints[3]; _totalSetpoints[4] = totalSetpoints[4];
            Serial.print(F("Feeding::recalcFeeding setpoints: {tds,pH,wTmp,aTmp,aCO2} = [")); Serial.print(_totalSetpoints[0]); Serial.print(' '); Serial.print(_totalSetpoints[1]); Serial.print(' '); Serial.print(_totalSetpoints[2]); Serial.print(' '); Serial.print(_totalSetpoints[3]); Serial.print(' '); Serial.print(_totalSetpoints[4]); Serial.println(']'); flushYield(); } }
    #endif

    setupStaging();
}

void HydroponicsFeeding::setupStaging()
{
    #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
    {   static int8_t _stage = (int8_t)stage; if (_stage != (int8_t)stage) {
        Serial.print(F("Feeding::setupStaging stage: ")); Serial.println((_stage = (int8_t)stage)); flushYield(); } }
    #endif

    if (stage == PreFeed) {
        if (feedRes->getWaterPHSensor()) {
            auto phBalancer = feedRes->getWaterPHBalancer();
            if (!phBalancer) {
                phBalancer = make_shared<HydroponicsTimedDosingBalancer>(feedRes->getWaterPHSensor(), phSetpoint, HYDRUINO_RANGE_PH_HALF, feedRes->getMaxVolume(), feedRes->getVolumeUnits());
                HYDRUINO_SOFT_ASSERT(phBalancer, SFP(HStr_Err_AllocationFailure));
                getSchedulerInstance()->setupWaterPHBalancer(feedRes.get(), phBalancer);
                feedRes->setWaterPHBalancer(phBalancer);
            }
            if (phBalancer) {
                phBalancer->setTargetSetpoint(phSetpoint);
                phBalancer->setTargetUnits(Hydroponics_UnitsType_Alkalinity_pH_0_14);
                phBalancer->setEnabled(true);
            }
        }
        if (feedRes->getWaterTDSSensor()) {
            auto tdsBalancer = feedRes->getWaterTDSBalancer();
            if (!tdsBalancer) {
                tdsBalancer = make_shared<HydroponicsTimedDosingBalancer>(feedRes->getWaterTDSSensor(), tdsSetpoint, HYDRUINO_RANGE_EC_HALF, feedRes->getMaxVolume(), feedRes->getVolumeUnits());
                HYDRUINO_SOFT_ASSERT(tdsBalancer, SFP(HStr_Err_AllocationFailure));
                getSchedulerInstance()->setupWaterTDSBalancer(feedRes.get(), tdsBalancer);
                feedRes->setWaterTDSBalancer(tdsBalancer);
            }
            if (tdsBalancer) {
                tdsBalancer->setTargetSetpoint(tdsSetpoint);
                tdsBalancer->setTargetUnits(Hydroponics_UnitsType_Concentration_EC);
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
            waterTempBalancer = make_shared<HydroponicsLinearEdgeBalancer>(feedRes->getWaterTemperatureSensor(), waterTempSetpoint, HYDRUINO_RANGE_TEMP_HALF, -HYDRUINO_RANGE_TEMP_HALF * 0.25f, HYDRUINO_RANGE_TEMP_HALF * 0.5f);
            HYDRUINO_SOFT_ASSERT(waterTempBalancer, SFP(HStr_Err_AllocationFailure));
            getSchedulerInstance()->setupWaterTemperatureBalancer(feedRes.get(), waterTempBalancer);
            feedRes->setWaterTemperatureBalancer(waterTempBalancer);
        }
        if (waterTempBalancer) {
            waterTempBalancer->setTargetSetpoint(waterTempSetpoint);
            waterTempBalancer->setTargetUnits(Hydroponics_UnitsType_Temperature_Celsius);
            waterTempBalancer->setEnabled(true);
        }
    } else {
        auto waterTempBalancer = feedRes->getWaterTemperatureBalancer();
        if (waterTempBalancer) { waterTempBalancer->setEnabled(false); }
    }

    if (feedRes->getAirTemperatureSensor()) {
        auto airTempBalancer = feedRes->getAirTemperatureBalancer();
        if (!airTempBalancer) {
            airTempBalancer = make_shared<HydroponicsLinearEdgeBalancer>(feedRes->getAirTemperatureSensor(), airTempSetpoint, HYDRUINO_RANGE_TEMP_HALF, -HYDRUINO_RANGE_TEMP_HALF * 0.25f, HYDRUINO_RANGE_TEMP_HALF * 0.5f);
            HYDRUINO_SOFT_ASSERT(airTempBalancer, SFP(HStr_Err_AllocationFailure));
            getSchedulerInstance()->setupAirTemperatureBalancer(feedRes.get(), airTempBalancer);
            feedRes->setAirTemperatureBalancer(airTempBalancer);
        }
        if (airTempBalancer) {
            airTempBalancer->setTargetSetpoint(airTempSetpoint);
            airTempBalancer->setTargetUnits(Hydroponics_UnitsType_Temperature_Celsius);
            airTempBalancer->setEnabled(true);
        }
    } else {
        auto airTempBalancer = feedRes->getAirTemperatureBalancer();
        if (airTempBalancer) { airTempBalancer->setEnabled(false); }
    }

    if (feedRes->getAirCO2Sensor()) {
        auto co2Balancer = feedRes->getAirTemperatureBalancer();
        if (!co2Balancer) {
            co2Balancer = make_shared<HydroponicsLinearEdgeBalancer>(feedRes->getAirCO2Sensor(), co2Setpoint, HYDRUINO_RANGE_CO2_HALF, -HYDRUINO_RANGE_CO2_HALF * 0.25f, HYDRUINO_RANGE_CO2_HALF * 0.5f);
            HYDRUINO_SOFT_ASSERT(co2Balancer, SFP(HStr_Err_AllocationFailure));
            getSchedulerInstance()->setupAirCO2Balancer(feedRes.get(), co2Balancer);
            feedRes->setAirCO2Balancer(co2Balancer);
        }
        if (co2Balancer) {
            co2Balancer->setTargetSetpoint(co2Setpoint);
            co2Balancer->setTargetUnits(Hydroponics_UnitsType_Concentration_PPM);
            co2Balancer->setEnabled(true);
        }
    } else {
        auto co2Balancer = feedRes->getAirCO2Balancer();
        if (co2Balancer) { co2Balancer->setEnabled(false); }
    }

    switch (stage) {
        case Init: {
            auto maxFeedingsDay = getSchedulerInstance()->getTotalFeedingsDay();
            auto feedingsToday = feedRes->getFeedingsToday();

            if (!maxFeedingsDay) {
                canFeedAfter = (time_t)0;
            } else if (feedingsToday < maxFeedingsDay) {
                // this will force feedings to be spread out during the entire day
                canFeedAfter = getCurrentDayStartTime() + (time_t)(((float)SECS_PER_DAY / (maxFeedingsDay + 1)) * feedingsToday);
            } else {
                canFeedAfter = (time_t)UINT32_MAX; // no more feedings today
            }

            if (canFeedAfter > unixNow()) { clearActuatorReqs(); } // clear on wait
        } break;

        case TopOff: {
            if (!feedRes->isFilled()) {
                Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type newActuatorReqs;
                auto topOffPumps = linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ReservoirType_FreshWater);

                linksResolveActuatorsByType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(topOffPumps, newActuatorReqs, Hydroponics_ActuatorType_WaterPump); // fresh water pumps
                if (!newActuatorReqs.size()) {
                    linksResolveActuatorsByType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(topOffPumps, newActuatorReqs, Hydroponics_ActuatorType_PeristalticPump); // fresh water peristaltic pumps
                }

                HYDRUINO_SOFT_ASSERT(newActuatorReqs.size(), SFP(HStr_Err_MissingLinkage)); // no fresh water pumps
                setActuatorReqs(newActuatorReqs);
            } else {
                clearActuatorReqs();
            }
        } break;

        case PreFeed: {
            Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type newActuatorReqs;
            auto aerators = linksFilterActuatorsByReservoirAndType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ActuatorType_WaterAerator);

            linksResolveActuatorsByType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(aerators, newActuatorReqs, Hydroponics_ActuatorType_WaterAerator);

            setActuatorReqs(newActuatorReqs);
        } break;

        case Feed: {
            Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type newActuatorReqs;

            {   auto feedPumps = linksFilterPumpActuatorsByInputReservoirAndOutputReservoirType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ReservoirType_FeedWater);

                linksResolveActuatorsByType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedPumps, newActuatorReqs, Hydroponics_ActuatorType_WaterPump); // feed water pump
            }

            if (!newActuatorReqs.size() && getHydroponicsInstance()->getSystemMode() == Hydroponics_SystemMode_DrainToWaste) { // prefers feed water pumps, else direct to waste is feed
                auto feedPumps = linksFilterPumpActuatorsByInputReservoirAndOutputReservoirType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ReservoirType_DrainageWater);

                linksResolveActuatorsByType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedPumps, newActuatorReqs, Hydroponics_ActuatorType_WaterPump); // DTW feed water pump
            }

            HYDRUINO_SOFT_ASSERT(newActuatorReqs.size(), SFP(HStr_Err_MissingLinkage)); // no feed water pumps

            #if HYDRUINO_SCH_AERATORS_FEEDRUN
                {   auto aerators = linksFilterActuatorsByReservoirAndType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ActuatorType_WaterAerator);

                    linksResolveActuatorsByType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(aerators, newActuatorReqs, Hydroponics_ActuatorType_WaterAerator);
                }
            #endif

            setActuatorReqs(newActuatorReqs);
        } break;

        case Drain: {
            Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type newActuatorReqs;
            auto drainPumps = linksFilterPumpActuatorsByInputReservoirAndOutputReservoirType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ReservoirType_DrainageWater);

            linksResolveActuatorsByType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(drainPumps, newActuatorReqs, Hydroponics_ActuatorType_WaterPump); // drainage water pump

            HYDRUINO_SOFT_ASSERT(newActuatorReqs.size(), SFP(HStr_Err_MissingLinkage)); // no drainage water pumps
            setActuatorReqs(newActuatorReqs);
        } break;

        case Done: {
            clearActuatorReqs();
        } break;

        default:
            break;
    }

    #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
    {   static int8_t _stage = (int8_t)stage; if (_stage != (int8_t)stage) {
        Serial.print(F("Feeding::~setupStaging stage: ")); Serial.println((_stage = (int8_t)stage)); flushYield(); } }
    #endif
}

void HydroponicsFeeding::update()
{
    #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
    {   static int8_t _stage = (int8_t)stage; if (_stage != (int8_t)stage) {
        Serial.print(F("Feeding::update stage: ")); Serial.println((_stage = (int8_t)stage)); flushYield(); } }
    #endif

    if ((!lastAirReport || unixNow() >= lastAirReport + getSchedulerInstance()->getAirReportInterval().totalseconds()) &&
        (getSchedulerInstance()->getAirReportInterval().totalseconds() > 0) && // 0 disables
        (feedRes->getAirTemperatureSensor() || feedRes->getAirCO2Sensor())) {
        getLoggerInstance()->logProcess(feedRes.get(), SFP(HStr_Log_AirReport));
        logFeeding(HydroponicsFeedingLogType_AirSetpoints);
        logFeeding(HydroponicsFeedingLogType_AirMeasures);
        lastAirReport = unixNow();
    }

    switch (stage) {
        case Init: {
            if (!canFeedAfter || unixNow() >= canFeedAfter) {
                int cropsCount = 0;
                int cropsHungry = 0;

                {   auto crops = linksFilterCrops(feedRes->getLinkages());
                    cropsCount = crops.size();
                    for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
                        if (((HydroponicsCrop *)(*cropIter))->needsFeeding()) { cropsHungry++; }
                    }
                }

                if (!cropsCount || cropsHungry / (float)cropsCount >= HYDRUINO_SCH_FEED_FRACTION - FLT_EPSILON) {
                    stage = TopOff; stageStart = unixNow();
                    setupStaging();

                    if (actuatorReqs.size()) {
                        getLoggerInstance()->logProcess(feedRes.get(), SFP(HStr_Log_PreFeedTopOff), SFP(HStr_Log_HasBegan));
                    }
                }
            }
        } break;

        case TopOff: {
            if (feedRes->isFilled() || !actuatorReqs.size()) {
                stage = PreFeed; stageStart = unixNow();
                canFeedAfter = 0; // will be used to track how long balancers stay balanced
                setupStaging();

                getLoggerInstance()->logProcess(feedRes.get(), SFP(HStr_Log_PreFeedBalancing), SFP(HStr_Log_HasBegan));
                if (actuatorReqs.size()) {
                    getLoggerInstance()->logMessage(SFP(HSTR_Log_Field_Aerator_Duration), String(getSchedulerInstance()->getPreFeedAeratorMins()), String('m'));
                }
                logFeeding(HydroponicsFeedingLogType_WaterSetpoints);
                logFeeding(HydroponicsFeedingLogType_WaterMeasures);
            }
        } break;

        case PreFeed: {
            if (!actuatorReqs.size() || unixNow() >= stageStart + (getSchedulerInstance()->getPreFeedAeratorMins() * SECS_PER_MIN)) {
                auto phBalancer = feedRes->getWaterPHBalancer();
                auto tdsBalancer = feedRes->getWaterTDSBalancer();
                auto waterTempBalancer = feedRes->getWaterTemperatureBalancer();

                if ((!phBalancer || (phBalancer->isEnabled() && phBalancer->isBalanced())) &&
                    (!tdsBalancer || (tdsBalancer->isEnabled() && tdsBalancer->isBalanced())) &&
                    (!waterTempBalancer || (waterTempBalancer->isEnabled() && waterTempBalancer->isBalanced()))) {
                    // Can proceed after above are marked balanced for min time
                    if (!canFeedAfter) { canFeedAfter = unixNow() + HYDRUINO_SCH_BALANCE_MINTIME; }
                    else if (unixNow() >= canFeedAfter) {
                        stage = Feed; stageStart = unixNow();
                        setupStaging();

                        broadcastFeeding(HydroponicsFeedingBroadcastType_Began);
                    }
                } else {
                    canFeedAfter = 0;
                }
            }
        } break;

        case Feed: {
            int cropsCount = 0;
            int cropsFed = 0;

            {   auto crops = linksFilterCrops(feedRes->getLinkages());
                cropsCount = crops.size();
                for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
                    if (!((HydroponicsCrop *)(*cropIter))->needsFeeding()) { cropsFed++; }
                }
            }

            if (!cropsCount || cropsFed / (float)cropsCount >= HYDRUINO_SCH_FEED_FRACTION - FLT_EPSILON ||
                feedRes->isEmpty()) {
                stage = (getHydroponicsInstance()->getSystemMode() == Hydroponics_SystemMode_DrainToWaste ? Drain : Done);
                stageStart = unixNow();
                setupStaging();

                broadcastFeeding(HydroponicsFeedingBroadcastType_Ended);
            }
        } break;

        case Drain: {
            if (getHydroponicsInstance()->getSystemMode() != Hydroponics_SystemMode_DrainToWaste ||
                feedRes->isEmpty()) {
                stage = Done; stageStart = unixNow();
                setupStaging();
            }
        } break;

        case Done: {
            stage = Init; stageStart = unixNow();
            setupStaging();
        } break;

        default:
            break;
    }

    if (actuatorReqs.size()) {
        for (auto actuatorIter = actuatorReqs.begin(); actuatorIter != actuatorReqs.end(); ++actuatorIter) {
            auto actuator = (*actuatorIter);
            if (!actuator->isEnabled() && !actuator->enableActuator()) {
                // TODO: Something clever to track stalled actuators
            }
        }
    }

    #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
    {   static int8_t _stage = (int8_t)stage; if (_stage != (int8_t)stage) {
        Serial.print(F("Feeding::~update stage: ")); Serial.println((_stage = (int8_t)stage)); flushYield(); } }
    #endif
}

void HydroponicsFeeding::logFeeding(HydroponicsFeedingLogType logType)
{
    switch (logType) {
        case HydroponicsFeedingLogType_WaterSetpoints:
            {   auto ph = HydroponicsSingleMeasurement(phSetpoint, Hydroponics_UnitsType_Alkalinity_pH_0_14);
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_pH_Setpoint), measurementToString(ph));
            }
            {   auto tds = HydroponicsSingleMeasurement(tdsSetpoint, Hydroponics_UnitsType_Concentration_TDS);
                convertUnits(&tds, feedRes->getTDSUnits());
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_TDS_Setpoint), measurementToString(tds, 1));
            }
            {   auto temp = HydroponicsSingleMeasurement(waterTempSetpoint, Hydroponics_UnitsType_Temperature_Celsius);
                convertUnits(&temp, feedRes->getTemperatureUnits());
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Temp_Setpoint), measurementToString(temp));
            }
            break;

        case HydroponicsFeedingLogType_WaterMeasures:
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                // Yield will allow measurements to complete, ensures first log out doesn't contain zero'ed values
                if ((feedRes->getWaterPHSensor() && !feedRes->getWaterPH().getMeasurementFrame()) ||
                    (feedRes->getWaterTDSSensor() && !feedRes->getWaterTDS().getMeasurementFrame()) ||
                    (feedRes->getWaterTemperatureSensor() && !feedRes->getWaterTemperature().getMeasurementFrame())) {
                    yield();
                }
            #endif
             if (feedRes->getWaterPHSensor()) {
                auto ph = feedRes->getWaterPH().getMeasurement(true);
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_pH_Measured), measurementToString(ph));
            }
            if (feedRes->getWaterTDSSensor()) {
                auto tds = feedRes->getWaterTDS().getMeasurement(true);
                convertUnits(&tds, feedRes->getTDSUnits());
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_TDS_Measured), measurementToString(tds, 1));
            }
            if (feedRes->getWaterTemperatureSensor()) {
                auto temp = feedRes->getWaterTemperature().getMeasurement(true);
                convertUnits(&temp, feedRes->getTemperatureUnits());
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Temp_Measured), measurementToString(temp));
            }
            break;

        case HydroponicsFeedingLogType_AirSetpoints:
            {   auto temp = HydroponicsSingleMeasurement(airTempSetpoint, Hydroponics_UnitsType_Temperature_Celsius);
                convertUnits(&temp, feedRes->getTemperatureUnits());
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Temp_Setpoint), measurementToString(temp));
            }
            {   auto co2 = HydroponicsSingleMeasurement(co2Setpoint, Hydroponics_UnitsType_Concentration_PPM);
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_CO2_Setpoint), measurementToString(co2));
            }
            break;

        case HydroponicsFeedingLogType_AirMeasures:
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                // Yield will allow measurements to complete, ensures first log out doesn't contain zero'ed values
                if ((feedRes->getAirTemperatureSensor() && !feedRes->getAirTemperature().getMeasurementFrame()) ||
                    (feedRes->getAirCO2Sensor() && !feedRes->getAirCO2().getMeasurementFrame())) {
                    yield();
                }
            #endif
            if (feedRes->getAirTemperatureSensor()) {
                auto temp = feedRes->getAirTemperature().getMeasurement(true);
                convertUnits(&temp, feedRes->getTemperatureUnits());
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Temp_Measured), measurementToString(temp));
            }
            if (feedRes->getAirCO2Sensor()) {
                auto co2 = feedRes->getAirCO2().getMeasurement(true);
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_CO2_Measured), measurementToString(co2));
            }
            break;
    }
}

void HydroponicsFeeding::broadcastFeeding(HydroponicsFeedingBroadcastType broadcastType)
{
    getLoggerInstance()->logProcess(feedRes.get(), SFP(HStr_Log_FeedingSequence),
                                    SFP(broadcastType == HydroponicsFeedingBroadcastType_Began ? HStr_Log_HasBegan : HStr_Log_HasEnded));
    logFeeding(HydroponicsFeedingLogType_WaterMeasures);

    broadcastType == HydroponicsFeedingBroadcastType_Began ? feedRes->notifyFeedingBegan() : feedRes->notifyFeedingEnded();

    {   auto crops = linksFilterCrops(feedRes->getLinkages());
        for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
            broadcastType == HydroponicsFeedingBroadcastType_Began ? ((HydroponicsCrop *)(*cropIter))->notifyFeedingBegan()
                                                                   : ((HydroponicsCrop *)(*cropIter))->notifyFeedingEnded();
        }
    }
}


HydroponicsLighting::HydroponicsLighting(shared_ptr<HydroponicsFeedReservoir> feedRes)
    : HydroponicsProcess(feedRes), stage(Init), sprayStart(0), lightStart(0), lightEnd(0), lightHours(0.0f)
{
    stageStart = unixNow();
    recalcLighting();
}

HydroponicsLighting::~HydroponicsLighting()
{
    clearActuatorReqs();
}

void HydroponicsLighting::recalcLighting()
{
    float totalWeights = 0;
    float totalLightHours = 0;
    bool sprayingNeeded = false;

    {   auto crops = linksFilterCrops(feedRes->getLinkages());

        for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
            auto crop = (HydroponicsCrop *)(*cropIter);
            auto cropPhase = (Hydroponics_CropPhase)constrain((int)(crop->getCropPhase()), 0, (int)Hydroponics_CropPhase_MainCount - 1);

            if ((int)cropPhase >= 0) {
                auto cropsLibData = getCropsLibraryInstance()->checkoutCropsData(crop->getCropType());

                if (cropsLibData) {
                    auto weight = crop->getFeedingWeight();
                    totalWeights += weight;
                    totalLightHours += (cropsLibData->dailyLightHours[cropPhase] * weight);
                    sprayingNeeded = sprayingNeeded || cropsLibData->needsSpraying();

                    getCropsLibraryInstance()->returnCropsData(cropsLibData);
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
        if (sprayingNeeded && linksCountActuatorsByReservoirAndType(feedRes->getLinkages(), feedRes.get(), Hydroponics_ActuatorType_WaterSprayer)) {
            daySprayerSecs = getSchedulerInstance()->getPreLightSprayMins() * SECS_PER_MIN;
        }

        time_t dayStart = getCurrentDayStartTime();
        lightStart = dayStart + ((SECS_PER_DAY - dayLightSecs) >> 1);
        sprayStart = max(dayStart, lightStart - daySprayerSecs);
        lightStart = sprayStart + daySprayerSecs;
        lightEnd = lightStart + dayLightSecs;
    }

    setupStaging();
}

void HydroponicsLighting::setupStaging()
{
    #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
    {   static int8_t _stage = (int8_t)stage; if (_stage != (int8_t)stage) {
        Serial.print(F("Lighting::setupStaging stage: ")); Serial.println((_stage = (int8_t)stage)); flushYield(); } }
    #endif

    switch (stage) {
        case Init: {
            clearActuatorReqs();
        } break;

        case Spray: {
            Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type newActuatorReqs;
            auto sprayers = linksFilterActuatorsByReservoirAndType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ActuatorType_WaterSprayer);

            linksResolveActuatorsByType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(sprayers, newActuatorReqs, Hydroponics_ActuatorType_WaterSprayer);

            setActuatorReqs(newActuatorReqs);
        } break;

        case Light: {
            Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type newActuatorReqs;
            auto lights = linksFilterActuatorsByReservoirAndType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ActuatorType_GrowLights);

            linksResolveActuatorsByType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(lights, newActuatorReqs, Hydroponics_ActuatorType_GrowLights);

            setActuatorReqs(newActuatorReqs);
        } break;

        case Done: {
            clearActuatorReqs();
        } break;

        default:
            break;
    }

    #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
    {   static int8_t _stage = (int8_t)stage; if (_stage != (int8_t)stage) {
        Serial.print(F("Lighting::~setupStaging stage: ")); Serial.println((_stage = (int8_t)stage)); flushYield(); } }
    #endif
}

void HydroponicsLighting::update()
{
    #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
    {   static int8_t _stage = (int8_t)stage; if (_stage != (int8_t)stage) {
        Serial.print(F("Lighting::update stage: ")); Serial.println((_stage = (int8_t)stage)); flushYield(); } }
    #endif

    switch (stage) {
        case Init: {
            time_t currTime = getCurrentTime().unixtime();

            if (currTime >= sprayStart && currTime < lightEnd) {
                stage = Spray; stageStart = unixNow();
                setupStaging();

                if (lightStart > sprayStart) {
                    getLoggerInstance()->logProcess(feedRes.get(), SFP(HStr_Log_PreLightSpraying), SFP(HStr_Log_HasBegan));
                    getLoggerInstance()->logMessage(SFP(HSTR_Log_Field_Sprayer_Duration), String(getSchedulerInstance()->getPreLightSprayMins()), String('m'));
                    getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Time_Start), DateTime((uint32_t)sprayStart).timestamp(DateTime::TIMESTAMP_TIME));
                    getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Time_Finish), DateTime((uint32_t)lightStart).timestamp(DateTime::TIMESTAMP_TIME));
                }
            }
        } break;

        case Spray: {
            if (getCurrentTime().unixtime() >= lightStart) {
                stage = Light; stageStart = unixNow();
                setupStaging();

                getLoggerInstance()->logProcess(feedRes.get(), SFP(HStr_Log_LightingSequence), SFP(HStr_Log_HasBegan));
                getLoggerInstance()->logMessage(SFP(HSTR_Log_Field_Light_Duration), roundToString(lightHours), String('h'));
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Time_Start), DateTime((uint32_t)lightStart).timestamp(DateTime::TIMESTAMP_TIME));
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Time_Finish), DateTime((uint32_t)lightEnd).timestamp(DateTime::TIMESTAMP_TIME));
            } else {
                stage = Done; stageStart = unixNow();
                setupStaging();
            }
        } break;

        case Light: {
            if (getCurrentTime().unixtime() >= lightEnd) {
                TimeSpan elapsedTime(unixNow() - stageStart);
                stage = Done; stageStart = unixNow();
                setupStaging();

                getLoggerInstance()->logProcess(feedRes.get(), SFP(HStr_Log_LightingSequence), SFP(HStr_Log_HasEnded));
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Time_Measured), timeSpanToString(elapsedTime));
            }
        } break;

        case Done: {
            stage = Init; stageStart = unixNow();
            setupStaging();
        } break;

        default:
            break;
    }

    if (actuatorReqs.size()) {
        for (auto actuatorIter = actuatorReqs.begin(); actuatorIter != actuatorReqs.end(); ++actuatorIter) {
            auto actuator = (*actuatorIter);
            if (!actuator->isEnabled() && !actuator->enableActuator()) {
                // TODO: Something clever to track stalled actuators
            }
        }
    }

    #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
    {   static int8_t _stage = (int8_t)stage; if (_stage != (int8_t)stage) {
        Serial.print(F("Lighting::~update stage: ")); Serial.println((_stage = (int8_t)stage)); flushYield(); } }
    #endif
}


HydroponicsSchedulerSubData::HydroponicsSchedulerSubData()
    : HydroponicsSubData(), baseFeedMultiplier(1), weeklyDosingRates{1}, stdDosingRates{1,0.5,0.5},
      totalFeedingsDay(0), preFeedAeratorMins(30), preLightSprayMins(60), airReportInterval(8 * SECS_PER_HOUR)
{
    type = 0; // no type differentiation
}

void HydroponicsSchedulerSubData::toJSONObject(JsonObject &objectOut) const
{
    //HydroponicsSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (!isFPEqual(baseFeedMultiplier, 1.0f)) { objectOut[SFP(HStr_Key_BaseFeedMultiplier)] = baseFeedMultiplier; }
    bool hasWeeklyDosings = arrayElementsEqual(weeklyDosingRates, HYDRUINO_CROP_GROWWEEKS_MAX, 1.0f);
    if (hasWeeklyDosings) { objectOut[SFP(HStr_Key_WeeklyDosingRates)] = commaStringFromArray(weeklyDosingRates, HYDRUINO_CROP_GROWWEEKS_MAX); }
    bool hasStandardDosings = !isFPEqual(stdDosingRates[0], 1.0f) || !isFPEqual(stdDosingRates[1], 0.5f) || !isFPEqual(stdDosingRates[2], 0.5f);
    if (hasStandardDosings) { objectOut[SFP(HStr_Key_StdDosingRates)] = commaStringFromArray(stdDosingRates, 3); }
    if (totalFeedingsDay > 0) { objectOut[SFP(HStr_Key_TotalFeedingsDay)] = totalFeedingsDay; }
    if (preFeedAeratorMins != 30) { objectOut[SFP(HStr_Key_PreFeedAeratorMins)] = preFeedAeratorMins; }
    if (preLightSprayMins != 60) { objectOut[SFP(HStr_Key_PreLightSprayMins)] = preLightSprayMins; }
    if (airReportInterval != (8 * SECS_PER_HOUR)) { objectOut[SFP(HStr_Key_AirReportInterval)] = airReportInterval; }
}

void HydroponicsSchedulerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroponicsSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    baseFeedMultiplier = objectIn[SFP(HStr_Key_BaseFeedMultiplier)] | baseFeedMultiplier;
    JsonVariantConst weeklyDosingRatesVar = objectIn[SFP(HStr_Key_WeeklyDosingRates)];
    commaStringToArray(weeklyDosingRatesVar, weeklyDosingRates, HYDRUINO_CROP_GROWWEEKS_MAX);
    JsonVariantConst stdDosingRatesVar = objectIn[SFP(HStr_Key_StdDosingRates)];
    commaStringToArray(stdDosingRatesVar, stdDosingRates, 3);
    totalFeedingsDay = objectIn[SFP(HStr_Key_TotalFeedingsDay)] | totalFeedingsDay;
    preFeedAeratorMins = objectIn[SFP(HStr_Key_PreFeedAeratorMins)] | preFeedAeratorMins;
    preLightSprayMins = objectIn[SFP(HStr_Key_PreLightSprayMins)] | preLightSprayMins;
    airReportInterval = objectIn[SFP(HStr_Key_AirReportInterval)] | airReportInterval;
}
