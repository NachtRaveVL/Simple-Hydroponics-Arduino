/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Scheduler
*/

#include "Hydroponics.h"

HydroponicsScheduler::HydroponicsScheduler()
    : _schedulerData(nullptr), _inDaytimeMode(false), _needsScheduling(false), _lastDayNum(-1)
{ ; }

HydroponicsScheduler::~HydroponicsScheduler()
{
    while (_feedings.size()) {
        auto feedingIter = _feedings.begin();
        if (feedingIter->second) { delete feedingIter->second; }
        _feedings.erase(feedingIter);
    }
    while (_lightings.size()) {
        auto lightingIter = _lightings.begin();
        if (lightingIter->second) { delete lightingIter->second; }
        _lightings.erase(lightingIter);
    }
}

void HydroponicsScheduler::initFromData(HydroponicsSchedulerSubData *dataIn)
{
    _schedulerData = dataIn;
    setNeedsScheduling();
}

void HydroponicsScheduler::update()
{
    if (_schedulerData) {
        {   DateTime currTime = getCurrentTime();
            bool inDaytimeMode = currTime.hour() >= HYDRUINO_CROP_NIGHT_ENDHR && currTime.hour() < HYDRUINO_CROP_NIGHT_BEGINHR;

            if (_inDaytimeMode != inDaytimeMode) {
                _inDaytimeMode = inDaytimeMode;
                setNeedsScheduling();
                // TODO: GUI update notify on day/night transition
            }

            if (_lastDayNum != currTime.day()) {
                // only log uptime upon actual day change and if uptime has been at least 1d
                if (getLoggerInstance()->getSystemUptime() >= SECS_PER_DAY) {
                    getLoggerInstance()->logSystemUptime();
                }
                broadcastDayChange();
            }
        }

        if (_needsScheduling) {
            performScheduling();
        }

        for (auto feedingIter = _feedings.begin(); feedingIter != _feedings.end(); ++feedingIter) {
            feedingIter->second->update();
        }
        for (auto lightingIter = _lightings.begin(); lightingIter != _lightings.end(); ++lightingIter) {
            lightingIter->second->update();
        }
    }
}

void HydroponicsScheduler::resolveLinks()
{
    setNeedsScheduling();
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

void HydroponicsScheduler::setupWaterPHBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *waterPHBalancer)
{
    if (reservoir && waterPHBalancer) {
        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type incActuators;
            auto phUpPumps = linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydroponics_ReservoirType_PhUpSolution);
            float dosingRate = getCombinedDosingRate(reservoir, Hydroponics_ReservoirType_PhUpSolution);

            for (auto pumpIter = phUpPumps.begin(); pumpIter != phUpPumps.end(); ++pumpIter) {
                auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_PeristalticPump) {
                    incActuators.push_back(make_pair(pump, dosingRate));
                }
            }

            if (!incActuators.size()) { // prefer peristaltic, else use full pump
                for (auto pumpIter = phUpPumps.begin(); pumpIter != phUpPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                    if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_WaterPump) {
                        incActuators.push_back(make_pair(pump, dosingRate));
                    }
                }
            }

            waterPHBalancer->setIncrementActuators(incActuators);
        }

        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type decActuators;
            auto phDownPumps = linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType<HYDRUINO_BAL_DECACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydroponics_ReservoirType_PhDownSolution);
            float dosingRate = getCombinedDosingRate(reservoir, Hydroponics_ReservoirType_PhDownSolution);

            for (auto pumpIter = phDownPumps.begin(); pumpIter != phDownPumps.end(); ++pumpIter) {
                auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_PeristalticPump) {
                    decActuators.push_back(make_pair(pump, dosingRate));
                }
            }

            if (!decActuators.size()) { // prefer peristaltic, else use full pump
                for (auto pumpIter = phDownPumps.begin(); pumpIter != phDownPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                    if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_WaterPump) {
                        decActuators.push_back(make_pair(pump, dosingRate));
                    }
                }
            }

            waterPHBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setupWaterTDSBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *waterTDSBalancer)
{
    if (reservoir && waterTDSBalancer) {
        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type incActuators;
            float dosingRate = getCombinedDosingRate(reservoir, Hydroponics_ReservoirType_NutrientPremix);

            if (dosingRate > FLT_EPSILON) {
                auto nutrientPumps = linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydroponics_ReservoirType_NutrientPremix);

                for (auto pumpIter = nutrientPumps.begin(); pumpIter != nutrientPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                    if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_PeristalticPump) {
                        incActuators.push_back(make_pair(pump, dosingRate));
                    }
                }

                if (!incActuators.size()) { // prefer peristaltic, else use full pump
                    for (auto pumpIter = nutrientPumps.begin(); pumpIter != nutrientPumps.end(); ++pumpIter) {
                        auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                        if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_WaterPump) {
                            incActuators.push_back(make_pair(pump, dosingRate));
                        }
                    }
                }
            }

            if (getHydroponicsInstance()->_additives.size()) {
                int prevIncSize = incActuators.size();

                for (Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_CustomAdditive1;
                    reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount;
                    reservoirType = (Hydroponics_ReservoirType)((int)reservoirType + 1)) {
                    if (getHydroponicsInstance()->getCustomAdditiveData(reservoirType)) {
                        dosingRate = getCombinedDosingRate(reservoir, reservoirType);

                        if (dosingRate > FLT_EPSILON) {
                            auto nutrientPumps = linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, reservoirType);

                            for (auto pumpIter = nutrientPumps.begin(); pumpIter != nutrientPumps.end(); ++pumpIter) {
                                auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                                if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_PeristalticPump) {
                                    incActuators.push_back(make_pair(pump, dosingRate));
                                }
                            }

                            if (incActuators.size() == prevIncSize) { // prefer peristaltic, else use full pump
                                for (auto pumpIter = nutrientPumps.begin(); pumpIter != nutrientPumps.end(); ++pumpIter) {
                                    auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                                    if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_WaterPump) {
                                        incActuators.push_back(make_pair(pump, dosingRate));
                                    }
                                }
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

                for (auto pumpIter = dilutionPumps.begin(); pumpIter != dilutionPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                    if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_PeristalticPump) {
                        decActuators.push_back(make_pair(pump, dosingRate));
                    }
                }

                if (!decActuators.size()) { // prefer peristaltic, else use full pump
                    for (auto pumpIter = dilutionPumps.begin(); pumpIter != dilutionPumps.end(); ++pumpIter) {
                        auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                        if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_WaterPump) {
                            decActuators.push_back(make_pair(pump, dosingRate));
                        }
                    }
                }
            }

            waterTDSBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setupWaterTempBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *waterTempBalancer)
{
    if (reservoir && waterTempBalancer) {
        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type incActuators;
            auto heaters = linksFilterActuatorsByReservoirAndType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydroponics_ActuatorType_WaterHeater);

            for (auto heaterIter = heaters.begin(); heaterIter != heaters.end(); ++heaterIter) {
                auto heater = getSharedPtr<HydroponicsActuator>(*heaterIter);
                if (heater) { incActuators.push_back(make_pair(heater, 1.0f)); }
            }

            waterTempBalancer->setIncrementActuators(incActuators);
        }

        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type decActuators;
            waterTempBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setupAirTempBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *airTempBalancer)
{
    if (reservoir && airTempBalancer) {
        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type incActuators;
            airTempBalancer->setIncrementActuators(incActuators);
        }

        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type decActuators;
            auto fans = linksFilterActuatorsByReservoirAndType<HYDRUINO_BAL_DECACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydroponics_ActuatorType_FanExhaust);

            for (auto fanIter = fans.begin(); fanIter != fans.end(); ++fanIter) {
                auto fan = getSharedPtr<HydroponicsActuator>(*fanIter);
                if (fan) { decActuators.push_back(make_pair(fan, 1.0f)); }
            }

            airTempBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setupAirCO2Balancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *airCO2Balancer)
{
    if (reservoir && airCO2Balancer) {
        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type incActuators;
            auto fans = linksFilterActuatorsByReservoirAndType<HYDRUINO_BAL_INCACTUATORS_MAXSIZE>(reservoir->getLinkages(), reservoir, Hydroponics_ActuatorType_FanExhaust);

            for (auto fanIter = fans.begin(); fanIter != fans.end(); ++fanIter) {
                auto fan = getSharedPtr<HydroponicsActuator>(*fanIter);
                if (fan) { incActuators.push_back(make_pair(fan, 1.0f)); }
            }

            airCO2Balancer->setIncrementActuators(incActuators);
        }

        {   Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type decActuators;
            airCO2Balancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setBaseFeedMultiplier(float baseFeedMultiplier)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));
    if (_schedulerData) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _schedulerData->baseFeedMultiplier = baseFeedMultiplier;
        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setWeeklyDosingRate(int weekIndex, float dosingRate, Hydroponics_ReservoirType reservoirType)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX), SFP(HS_Err_InvalidParameter));

    if (_schedulerData && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX) {
        if (reservoirType == Hydroponics_ReservoirType_NutrientPremix) {
            getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
            _schedulerData->weeklyDosingRates[weekIndex] = dosingRate;

            setNeedsScheduling();
        } else if (reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 && reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
            HydroponicsCustomAdditiveData newAdditiveData(reservoirType);
            newAdditiveData._bumpRevIfNotAlreadyModded();
            newAdditiveData.weeklyDosingRates[weekIndex] = dosingRate;
            getHydroponicsInstance()->setCustomAdditiveData(&newAdditiveData);

            setNeedsScheduling();
        } else {
            HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_UnsupportedOperation));
        }
    }
}

void HydroponicsScheduler::setStandardDosingRate(float dosingRate, Hydroponics_ReservoirType reservoirType)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (reservoirType >= Hydroponics_ReservoirType_FreshWater && reservoirType < Hydroponics_ReservoirType_CustomAdditive1), SFP(HS_Err_InvalidParameter));

    if (_schedulerData && (reservoirType >= Hydroponics_ReservoirType_FreshWater && reservoirType < Hydroponics_ReservoirType_CustomAdditive1)) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _schedulerData->stdDosingRates[reservoirType - Hydroponics_ReservoirType_FreshWater] = dosingRate;

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

void HydroponicsScheduler::setLastWeekAsFlush(HydroponicsCrop *crop)
{
    if (crop) { setFlushWeek(crop->getTotalGrowWeeks() - 1); }
}

void HydroponicsScheduler::setFlushWeek(int weekIndex)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX), SFP(HS_Err_InvalidParameter));

    if (_schedulerData && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX) {
        _schedulerData->weeklyDosingRates[weekIndex] = 0;

        for (Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_CustomAdditive1;
             reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount;
             reservoirType = (Hydroponics_ReservoirType)((int)reservoirType + 1)) {
            auto additiveData = getHydroponicsInstance()->getCustomAdditiveData(reservoirType);
            if (additiveData) {
                HydroponicsCustomAdditiveData newAdditiveData = *additiveData;
                newAdditiveData._bumpRevIfNotAlreadyModded();
                newAdditiveData.weeklyDosingRates[weekIndex] = 0;
                getHydroponicsInstance()->setCustomAdditiveData(&newAdditiveData);
            }
        }

        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setTotalFeedingsDay(unsigned int feedingsDay)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));

    if (_schedulerData) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _schedulerData->totalFeedingsDay = feedingsDay;

        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setPreFeedAeratorMins(unsigned int aeratorMins)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));

    if (_schedulerData) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _schedulerData->preFeedAeratorMins = aeratorMins;

        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setPreLightSprayMins(unsigned int sprayMins)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));

    if (_schedulerData) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _schedulerData->preLightSprayMins = sprayMins;

        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setNeedsScheduling()
{
    _needsScheduling = (bool)_schedulerData;
}

float HydroponicsScheduler::getCombinedDosingRate(HydroponicsReservoir *reservoir, Hydroponics_ReservoirType reservoirType)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || reservoir, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || !reservoir || (reservoirType >= Hydroponics_ReservoirType_NutrientPremix &&
                                                           reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount), SFP(HS_Err_InvalidParameter));

    if (_schedulerData && reservoir &&
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
                    totalDosing += _schedulerData->weeklyDosingRates[constrain(crop->getGrowWeek(), 0, crop->getTotalGrowWeeks() - 1)];
                } else if (reservoirType < Hydroponics_ReservoirType_CustomAdditive1) {
                    totalWeights += crop->getFeedingWeight();
                    totalDosing += _schedulerData->stdDosingRates[reservoirType - Hydroponics_ReservoirType_FreshWater];
                } else {
                    auto additiveData = getHydroponicsInstance()->getCustomAdditiveData(reservoirType);
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
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));
    return _schedulerData ? _schedulerData->baseFeedMultiplier : 1.0f;
}

float HydroponicsScheduler::getWeeklyDosingRate(int weekIndex, Hydroponics_ReservoirType reservoirType) const
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX), SFP(HS_Err_InvalidParameter));

    if (_schedulerData && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX) {
        if (reservoirType == Hydroponics_ReservoirType_NutrientPremix) {
            return _schedulerData->weeklyDosingRates[weekIndex];
        } else if (reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 && reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
            auto additiveDate = getHydroponicsInstance()->getCustomAdditiveData(reservoirType);
            return additiveDate ? additiveDate->weeklyDosingRates[weekIndex] : 0.0f;
        } else {
            HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_UnsupportedOperation));
        }
    }

    return 0.0f;
}

float HydroponicsScheduler::getStandardDosingRate(Hydroponics_ReservoirType reservoirType) const
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (reservoirType >= Hydroponics_ReservoirType_FreshWater && reservoirType < Hydroponics_ReservoirType_CustomAdditive1), SFP(HS_Err_InvalidParameter));

    if (_schedulerData && reservoirType >= Hydroponics_ReservoirType_FreshWater && reservoirType < Hydroponics_ReservoirType_CustomAdditive1) {
        return _schedulerData->stdDosingRates[reservoirType - Hydroponics_ReservoirType_FreshWater];
    }

    return 0.0f;
}

bool HydroponicsScheduler::getIsFlushWeek(int weekIndex)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX), SFP(HS_Err_InvalidParameter));

    if (_schedulerData && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX) {
        return isFPEqual(_schedulerData->weeklyDosingRates[weekIndex], 0.0f);
    }

    return false;
}

unsigned int HydroponicsScheduler::getTotalFeedingsDay() const
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));
    return _schedulerData ? _schedulerData->totalFeedingsDay : 0;
}

unsigned int HydroponicsScheduler::getPreFeedAeratorMins() const
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));
    return _schedulerData ? _schedulerData->preFeedAeratorMins : 0;
}

unsigned int HydroponicsScheduler::getPreLightSprayMins() const
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, SFP(HS_Err_NotYetInitialized));
    return _schedulerData ? _schedulerData->preLightSprayMins : 0;
}

bool HydroponicsScheduler::getInDaytimeMode() const
{
    return _inDaytimeMode;
}

void HydroponicsScheduler::updateDayTracking()
{
    auto currTime = getCurrentTime();
    _lastDayNum = currTime.day();
    _inDaytimeMode = currTime.hour() >= HYDRUINO_CROP_NIGHT_ENDHR && currTime.hour() < HYDRUINO_CROP_NIGHT_BEGINHR;
    setNeedsScheduling();
    // TODO: GUI update notify on major time event
}

void HydroponicsScheduler::performScheduling()
{
    auto hydroponics = getHydroponicsInstance();

    for (auto iter = hydroponics->_objects.begin(); iter != hydroponics->_objects.end(); ++iter) {
        if (iter->second && iter->second->isReservoirType()) {
            if (((HydroponicsReservoir *)(iter->second.get()))->isFeedClass()) {
                auto feedReservoir = static_pointer_cast<HydroponicsFeedReservoir>(iter->second);

                if (feedReservoir) {
                    {   auto feedingIter = _feedings.find(feedReservoir->getKey());
                        auto cropsCount = linksFilterCrops(feedReservoir->getLinkages()).size();

                        if (cropsCount) {
                            if (feedingIter != _feedings.end()) {
                                if (feedingIter->second) {
                                    feedingIter->second->recalcFeeding();
                                }
                            } else {
                                HydroponicsFeeding *feeding = new HydroponicsFeeding(feedReservoir);
                                HYDRUINO_SOFT_ASSERT(feeding, SFP(HS_Err_AllocationFailure));
                                if (feeding) { _feedings[feedReservoir->getKey()] = feeding; }
                            }
                        } else if (feedingIter != _feedings.end()) { // No crops to warrant process -> delete
                            if (feedingIter->second) { delete feedingIter->second; }
                            _feedings.erase(feedingIter);
                        }
                    }

                    {   auto lightingIter = _lightings.find(feedReservoir->getKey());
                        auto sprayersCount = linksFilterActuatorsByReservoirAndType(feedReservoir->getLinkages(), feedReservoir.get(), Hydroponics_ActuatorType_WaterSprayer).size();
                        auto lightsCount = linksFilterActuatorsByReservoirAndType(feedReservoir->getLinkages(), feedReservoir.get(), Hydroponics_ActuatorType_GrowLights).size();

                        if (sprayersCount || lightsCount) {
                            if (lightingIter != _lightings.end()) {
                                if (lightingIter->second) {
                                    lightingIter->second->recalcLighting();
                                }
                            } else {
                                HydroponicsLighting *lighting = new HydroponicsLighting(feedReservoir);
                                HYDRUINO_SOFT_ASSERT(lighting, SFP(HS_Err_AllocationFailure));
                                if (lighting) { _lightings[feedReservoir->getKey()] = lighting; }
                            }
                        } else if (lightingIter != _lightings.end()) { // No lights or sprayers to warrant process -> delete
                            if (lightingIter->second) { delete lightingIter->second; }
                            _lightings.erase(lightingIter);
                        }
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
                getHydroponicsInstance()->notifyDayChanged();
            }
            yield();
            if (getLoggerInstance()) {
                getLoggerInstance()->notifyDayChanged();
            }
            yield();
            if (getPublisherInstance()) {
                getPublisherInstance()->notifyDayChanged();
            }
        });
    #else
        if (getHydroponicsInstance()) {
            getHydroponicsInstance()->notifyDayChanged();
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
    while(actuatorReqs.size()) {
        actuatorReqs.begin()->get()->disableActuator();
        actuatorReqs.erase(actuatorReqs.begin());
    }
}

void HydroponicsProcess::setActuatorReqs(const Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type &actuatorReqsIn)
{
    for (auto actuatorIter = actuatorReqs.begin(); actuatorIter != actuatorReqs.end(); ++actuatorIter) {
        if (*actuatorIter) {
            bool found = false;
            auto key = (*actuatorIter)->getKey();
            for (auto actuatorInIter = actuatorReqsIn.begin(); actuatorInIter != actuatorReqsIn.end(); ++actuatorInIter) {
                if (key == (*actuatorInIter)->getKey()) {
                    found = true;
                    break;
                }
            }
            if (!found && (*actuatorIter)->getIsEnabled()) {
                (*actuatorIter)->disableActuator();
            }
        }
    }

    actuatorReqs = actuatorReqsIn;
}


HydroponicsFeeding::HydroponicsFeeding(shared_ptr<HydroponicsFeedReservoir> feedRes)
    : HydroponicsProcess(feedRes), stage(Unknown), canFeedAfter(0),
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
    float totalSetpoints[5] = {0};
    auto crops = linksFilterCrops(feedRes->getLinkages());

    for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
        auto crop = (HydroponicsCrop *)(*cropIter);
        auto cropsLibData = crop ? getCropsLibraryInstance()->checkoutCropsData(crop->getCropType()) : nullptr;

        if (cropsLibData) {
            float weight = crop->getFeedingWeight();
            totalWeights += weight;

            float feedRate = ((cropsLibData->tdsRange[0] + cropsLibData->tdsRange[1]) * 0.5);
            if (!getSchedulerInstance()->getInDaytimeMode()) {
                feedRate *= cropsLibData->nightlyFeedMultiplier;
            }
            feedRate *= getSchedulerInstance()->getBaseFeedMultiplier();

            totalSetpoints[0] += ((cropsLibData->phRange[0] + cropsLibData->phRange[1]) * 0.5) * weight;
            totalSetpoints[1] += feedRate * weight;
            totalSetpoints[2] += ((cropsLibData->waterTempRange[0] + cropsLibData->waterTempRange[1]) * 0.5) * weight;
            totalSetpoints[3] += ((cropsLibData->airTempRange[0] + cropsLibData->airTempRange[1]) * 0.5) * weight;
            totalSetpoints[4] += cropsLibData->co2Levels[(crop->getCropPhase() <= Hydroponics_CropPhase_Vegetative ? 0 : 1)] * weight;

            getCropsLibraryInstance()->returnCropsData(cropsLibData);
        }
    }

    if (totalWeights < FLT_EPSILON) {
        totalWeights = 1.0f;
        totalSetpoints[0] = 6;
    }

    tdsSetpoint = totalSetpoints[1] / totalWeights;
    if (tdsSetpoint < FLT_EPSILON) { // flushing
        phSetpoint = 6;
    } else {
        phSetpoint = totalSetpoints[0] / totalWeights;
    }
    waterTempSetpoint = totalSetpoints[2] / totalWeights;
    airTempSetpoint = totalSetpoints[3] / totalWeights;
    co2Setpoint = totalSetpoints[4] / totalWeights;

    setupStaging();
}

void HydroponicsFeeding::setupBalancers()
{
    if (stage == PreFeed) {
        if (feedRes->getWaterPHSensor()) {
            auto phBalancer = feedRes->setWaterPHBalancer(phSetpoint, Hydroponics_UnitsType_Alkalinity_pH_0_14);
            if (phBalancer) {
                getSchedulerInstance()->setupWaterPHBalancer(feedRes.get(), phBalancer);
                phBalancer->setEnabled(true);
            }
        }
        if (feedRes->getWaterTDSSensor()) {
            auto tdsBalancer = feedRes->setWaterTDSBalancer(tdsSetpoint, Hydroponics_UnitsType_Concentration_EC);
            if (tdsBalancer) {
                getSchedulerInstance()->setupWaterTDSBalancer(feedRes.get(), tdsBalancer);
                tdsBalancer->setEnabled(true);
            }
        }
    } else {
        auto phBalancer = feedRes->getWaterPHBalancer();
        if (phBalancer) { phBalancer->setEnabled(false); }
        auto tdsBalancer = feedRes->getWaterTDSBalancer();
        if (tdsBalancer) { tdsBalancer->setEnabled(false); }
    }

    if ((stage == PreFeed || stage == Feed) && feedRes->getWaterTempSensor()) {
        auto waterTempBalancer = feedRes->setWaterTempBalancer(waterTempSetpoint, Hydroponics_UnitsType_Temperature_Celsius);
        if (waterTempBalancer) {
            getSchedulerInstance()->setupWaterTempBalancer(feedRes.get(), waterTempBalancer);
            waterTempBalancer->setEnabled(true);
        }
    } else {
        auto waterTempBalancer = feedRes->getWaterTempBalancer();
        if (waterTempBalancer) { waterTempBalancer->setEnabled(false); }
    }

    if (feedRes->getAirTempSensor()) {
        auto airTempBalancer = feedRes->setAirTempBalancer(airTempSetpoint, Hydroponics_UnitsType_Temperature_Celsius);
        if (airTempBalancer) {
            getSchedulerInstance()->setupAirTempBalancer(feedRes.get(), airTempBalancer);
            airTempBalancer->setEnabled(true);
        }
    } else {
        auto airTempBalancer = feedRes->getAirTempBalancer();
        if (airTempBalancer) { airTempBalancer->setEnabled(false); }
    }

    if (feedRes->getAirCO2Sensor()) {
        auto co2Balancer = feedRes->setAirCO2Balancer(co2Setpoint, Hydroponics_UnitsType_Concentration_PPM);
        if (co2Balancer) {
            getSchedulerInstance()->setupAirCO2Balancer(feedRes.get(), co2Balancer);
            co2Balancer->setEnabled(true);
        }
    } else {
        auto co2Balancer = feedRes->getAirCO2Balancer();
        if (co2Balancer) { co2Balancer->setEnabled(false); }
    }
}

void HydroponicsFeeding::setupStaging()
{
    setupBalancers();

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

            clearActuatorReqs();
        } break;

        case TopOff: {
            if (!feedRes->getIsFilled()) {
                Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type newActuatorReqs;
                auto topOffPumps = linksFilterPumpActuatorsByOutputReservoirAndInputReservoirType(feedRes->getLinkages(), feedRes.get(), Hydroponics_ReservoirType_FreshWater);

                for (auto pumpIter = topOffPumps.begin(); pumpIter != topOffPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                    if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_WaterPump) {
                        newActuatorReqs.push_back(pump); // fresh water pumps
                    }
                }

                if (!newActuatorReqs.size()) {
                    for (auto pumpIter = topOffPumps.begin(); pumpIter != topOffPumps.end(); ++pumpIter) {
                        auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                        if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_PeristalticPump) {
                            newActuatorReqs.push_back(pump); // fresh water peristaltic pumps
                        }
                    }
                }

                HYDRUINO_SOFT_ASSERT(newActuatorReqs.size(), SFP(HS_Err_MissingLinkage)); // no fresh water pumps
                setActuatorReqs(newActuatorReqs);
            } else {
                clearActuatorReqs();
            }
        } break;

        case PreFeed: {
            Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type newActuatorReqs;
            auto aerators = linksFilterActuatorsByReservoirAndType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ActuatorType_WaterAerator);

            for (auto aeratorIter = aerators.begin(); aeratorIter != aerators.end(); ++aeratorIter) {
                auto aerator = getSharedPtr<HydroponicsActuator>(*aeratorIter);
                if (aerator) { newActuatorReqs.push_back(aerator); }
            }

            setActuatorReqs(newActuatorReqs);
        } break;

        case Feed: {
            Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type newActuatorReqs;

            {   auto feedPumps = linksFilterPumpActuatorsByInputReservoirAndOutputReservoirType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ReservoirType_FeedWater);

                for (auto pumpIter = feedPumps.begin(); pumpIter != feedPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                    if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_WaterPump) {
                        newActuatorReqs.push_back(pump); // feed water pump
                    }
                }

                if (!newActuatorReqs.size() && getHydroponicsInstance()->getSystemMode() == Hydroponics_SystemMode_DrainToWaste) { // prefers feed water pumps, else direct to waste is feed
                    feedPumps = linksFilterPumpActuatorsByInputReservoirAndOutputReservoirType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ReservoirType_DrainageWater);

                    for (auto pumpIter = feedPumps.begin(); pumpIter != feedPumps.end(); ++pumpIter) {
                        auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                        if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_WaterPump) {
                            newActuatorReqs.push_back(pump); // DTW feed water pump
                        }
                    }
                }
            }

            HYDRUINO_SOFT_ASSERT(newActuatorReqs.size(), SFP(HS_Err_MissingLinkage)); // no feed water pumps

            #if HYDRUINO_SCH_AERATORS_FEEDRUN
                {   auto aerators = linksFilterActuatorsByReservoirAndType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ActuatorType_WaterAerator);

                    for (auto aeratorIter = aerators.begin(); aeratorIter != aerators.end(); ++aeratorIter) {
                        auto aerator = getSharedPtr<HydroponicsActuator>(*aeratorIter);
                        if (aerator) { newActuatorReqs.push_back(aerator); }
                    }
                }
            #endif

            setActuatorReqs(newActuatorReqs);
        } break;

        case Drain: {
            Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type newActuatorReqs;
            auto drainPumps = linksFilterPumpActuatorsByInputReservoirAndOutputReservoirType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ReservoirType_DrainageWater);

            for (auto pumpIter = drainPumps.begin(); pumpIter != drainPumps.end(); ++pumpIter) {
                auto pump = getSharedPtr<HydroponicsActuator>(*pumpIter);
                if (pump && pump->getActuatorType() == Hydroponics_ActuatorType_WaterPump) {
                    newActuatorReqs.push_back(pump); // drainage water pump
                }
            }

            HYDRUINO_SOFT_ASSERT(newActuatorReqs.size(), SFP(HS_Err_MissingLinkage)); // no drainage water pumps
            setActuatorReqs(newActuatorReqs);
        } break;

        case Done: {
            clearActuatorReqs();
        } break;

        default:
            break;
    }
}

void HydroponicsFeeding::update()
{
    switch (stage) {
        case Init: {
            if (!canFeedAfter || unixNow() >= canFeedAfter) {
                int cropsCount = 0;
                int cropsHungry = 0;

                {   auto crops = linksFilterCrops(feedRes->getLinkages());
                    cropsCount = crops.size();
                    for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
                        if (((HydroponicsCrop *)(*cropIter))->getNeedsFeeding()) { cropsHungry++; }
                    }
                }

                if (cropsHungry / (float)cropsCount >= HYDRUINO_SCH_FEED_FRACTION - FLT_EPSILON) {
                    stage = TopOff; stageStart = unixNow();
                    setupStaging();

                    if (actuatorReqs.size()) {
                        getLoggerInstance()->logProcess(feedRes.get(), SFP(HS_Log_PreFeedTopOff), SFP(HS_Log_HasBegan));
                    }
                }
            }
        } break;

        case TopOff: {
            if (feedRes->getIsFilled() || !actuatorReqs.size()) {
                stage = PreFeed; stageStart = unixNow();
                canFeedAfter = 0; // will be used to track how long balancers stay balanced
                setupStaging();

                getLoggerInstance()->logProcess(feedRes.get(), SFP(HS_Log_PreFeedBalancing), SFP(HS_Log_HasBegan));
                if (actuatorReqs.size()) {
                    String aeratorMinsStr; aeratorMinsStr.reserve(8);
                    aeratorMinsStr.concat(SFP(HS_ColonSpace));
                    aeratorMinsStr.concat(getSchedulerInstance()->getPreFeedAeratorMins());
                    aeratorMinsStr.concat('m');

                    getLoggerInstance()->logMessage(SFP(HS_DoubleSpace), SFP(HS_Key_PreFeedAeratorMins), aeratorMinsStr);
                }
                if (feedRes->getWaterPHSensor()) {
                    auto ph = HydroponicsSingleMeasurement(phSetpoint, Hydroponics_UnitsType_Alkalinity_pH_0_14);
                    getLoggerInstance()->logMessage(SFP(HS_Log_Field_pH), measurementToString(ph));
                }
                if (feedRes->getWaterTDSSensor()) {
                    auto tds = HydroponicsSingleMeasurement(tdsSetpoint, Hydroponics_UnitsType_Concentration_TDS);
                    convertUnits(&tds, feedRes->getTDSUnits());
                    getLoggerInstance()->logMessage(SFP(HS_Log_Field_TDS), measurementToString(tds, 1));
                }
                if (feedRes->getWaterTempSensor()) {
                    auto temp = HydroponicsSingleMeasurement(waterTempSetpoint, Hydroponics_UnitsType_Temperature_Celsius);
                    convertUnits(&temp, feedRes->getTemperatureUnits());
                    getLoggerInstance()->logMessage(SFP(HS_Log_Field_Temp), measurementToString(temp));
                }
            }
        } break;

        case PreFeed: {
            if (!actuatorReqs.size() || unixNow() >= stageStart + (getSchedulerInstance()->getPreFeedAeratorMins() * SECS_PER_MIN)) {
                auto phBalancer = feedRes->getWaterPHBalancer();
                auto tdsBalancer = feedRes->getWaterTDSBalancer();
                auto waterTempBalancer = feedRes->getWaterTempBalancer();

                if ((!phBalancer || (phBalancer->getIsEnabled() && phBalancer->getIsBalanced())) &&
                    (!tdsBalancer || (tdsBalancer->getIsEnabled() && tdsBalancer->getIsBalanced())) &&
                    (!waterTempBalancer || (waterTempBalancer->getIsEnabled() && waterTempBalancer->getIsBalanced()))) {
                    // Can proceed after above are marked balanced for min time
                    if (!canFeedAfter) { canFeedAfter = unixNow() + HYDRUINO_SCH_BALANCE_MINTIME; }
                    else if (unixNow() >= canFeedAfter) {
                        stage = Feed; stageStart = unixNow();
                        setupStaging();
                        broadcastFeedingBegan();
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
                    if (!((HydroponicsCrop *)(*cropIter))->getNeedsFeeding()) { cropsFed++; }
                }
            }

            if (cropsFed / (float)cropsCount >= HYDRUINO_SCH_FEED_FRACTION - FLT_EPSILON ||
                feedRes->getIsEmpty()) {
                stage = (getHydroponicsInstance()->getSystemMode() == Hydroponics_SystemMode_DrainToWaste ? Drain : Done);
                stageStart = unixNow();
                setupStaging();
                broadcastFeedingEnded();
            }
        } break;

        case Drain: {
            if (getHydroponicsInstance()->getSystemMode() != Hydroponics_SystemMode_DrainToWaste ||
                feedRes->getIsEmpty()) {
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
            if (!(*actuatorIter)->getIsEnabled()) {
                if (!(*actuatorIter)->enableActuator()) {
                    // TODO: Something clever to track stalled actuators
                }
            }
        }
    }
}

void HydroponicsFeeding::broadcastFeedingBegan()
{
    getLoggerInstance()->logProcess(feedRes.get(), SFP(HS_Log_FeedingSequence), SFP(HS_Log_HasBegan));
    {   auto ph = feedRes->getWaterPHSensor() ? feedRes->getWaterPH() : HydroponicsSingleMeasurement(phSetpoint, Hydroponics_UnitsType_Alkalinity_pH_0_14);
        getLoggerInstance()->logMessage(SFP(HS_Log_Field_pH), measurementToString(ph));
    }
    {   auto tds = feedRes->getWaterTDSSensor() ? feedRes->getWaterTDS() : HydroponicsSingleMeasurement(tdsSetpoint, Hydroponics_UnitsType_Concentration_TDS);
        convertUnits(&tds, feedRes->getTDSUnits());
        getLoggerInstance()->logMessage(SFP(HS_Log_Field_TDS), measurementToString(tds, 1));
    }
    {   auto temp = feedRes->getWaterTempSensor() ? feedRes->getWaterTemperature() : HydroponicsSingleMeasurement(waterTempSetpoint, Hydroponics_UnitsType_Temperature_Celsius);
        convertUnits(&temp, feedRes->getTemperatureUnits());
        getLoggerInstance()->logMessage(SFP(HS_Log_Field_Temp), measurementToString(temp));
    }

    feedRes->notifyFeedingBegan();

    {   auto crops = linksFilterCrops(feedRes->getLinkages());
        for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
            if (*cropIter) {
                ((HydroponicsCrop *)(*cropIter))->notifyFeedingBegan();
            }
        }
    }
}

void HydroponicsFeeding::broadcastFeedingEnded()
{
    getLoggerInstance()->logProcess(feedRes.get(), SFP(HS_Log_FeedingSequence), SFP(HS_Log_HasEnded));

    feedRes->notifyFeedingEnded();

    {   auto crops = linksFilterCrops(feedRes->getLinkages());
        for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
            if (*cropIter) {
                ((HydroponicsCrop *)(*cropIter))->notifyFeedingEnded();
            }
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
            auto cropPhase = crop ? (Hydroponics_CropPhase)constrain((int)(crop->getCropPhase()), 0, (int)Hydroponics_CropPhase_MainCount - 1)
                                  : Hydroponics_CropPhase_Undefined;

            if ((int)cropPhase >= 0) {
                auto cropsLibData = getCropsLibraryInstance()->checkoutCropsData(crop->getCropType());

                if (cropsLibData) {
                    auto weight = crop->getFeedingWeight();
                    totalWeights += weight;
                    totalLightHours += (cropsLibData->dailyLightHours[cropPhase] * weight);
                    sprayingNeeded = sprayingNeeded || cropsLibData->getNeedsSpraying();

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
        if (sprayingNeeded && linksFilterActuatorsByReservoirAndType(feedRes->getLinkages(), feedRes.get(), Hydroponics_ActuatorType_WaterSprayer).size()) {
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
    switch (stage) {
        case Init: {
            clearActuatorReqs();
        } break;

        case Spray: {
            Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type newActuatorReqs;
            auto sprayers = linksFilterActuatorsByReservoirAndType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ActuatorType_WaterSprayer);

            for (auto sprayerIter = sprayers.begin(); sprayerIter != sprayers.end(); ++sprayerIter) {
                auto sprayer = getSharedPtr<HydroponicsActuator>(*sprayerIter);
                if (sprayer) { newActuatorReqs.push_back(sprayer); }
            }

            setActuatorReqs(newActuatorReqs);
        } break;

        case Light: {
            Vector<shared_ptr<HydroponicsActuator>, HYDRUINO_SCH_REQACTUATORS_MAXSIZE>::type newActuatorReqs;
            auto lights = linksFilterActuatorsByReservoirAndType<HYDRUINO_SCH_REQACTUATORS_MAXSIZE>(feedRes->getLinkages(), feedRes.get(), Hydroponics_ActuatorType_GrowLights);

            for (auto lightIter = lights.begin(); lightIter != lights.end(); ++lightIter) {
                auto light = getSharedPtr<HydroponicsActuator>(*lightIter);
                if (light) { newActuatorReqs.push_back(light); }
            }

            setActuatorReqs(newActuatorReqs);
        } break;

        case Done: {
            clearActuatorReqs();
        } break;

        default:
            break;
    }
}

void HydroponicsLighting::update()
{
    switch (stage) {
        case Init: {
            time_t currTime = getCurrentTime().unixtime();

            if (currTime >= sprayStart && currTime < lightEnd) {
                stage = Spray; stageStart = unixNow();
                setupStaging();

                if (lightStart > sprayStart) {
                    String sprayMinsStr; sprayMinsStr.reserve(8);
                    sprayMinsStr.concat(SFP(HS_ColonSpace));
                    sprayMinsStr.concat(getSchedulerInstance()->getPreLightSprayMins());
                    sprayMinsStr.concat('m');

                    getLoggerInstance()->logProcess(feedRes.get(), SFP(HS_Log_PreLightSpraying), SFP(HS_Log_HasBegan));
                    getLoggerInstance()->logMessage(SFP(HS_DoubleSpace), SFP(HS_Key_PreLightSprayMins), sprayMinsStr);
                }
            }
        } break;

        case Spray: {
            if (getCurrentTime().unixtime() >= lightStart) {
                stage = Light; stageStart = unixNow();
                setupStaging();

                {   String lightHrsStr; lightHrsStr.reserve(8);
                    lightHrsStr.concat(SFP(HS_ColonSpace));
                    lightHrsStr.concat(roundToString(lightHours));
                    lightHrsStr.concat('h');

                    getLoggerInstance()->logProcess(feedRes.get(), SFP(HS_Log_LightingSequence), SFP(HS_Log_HasBegan));
                    getLoggerInstance()->logMessage(SFP(HS_DoubleSpace), SFP(HS_Key_DailyLightHours), lightHrsStr);
                }
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

                {   String lightHrsStr; lightHrsStr.reserve(16);
                    lightHrsStr.concat(SFP(HS_ColonSpace));
                    lightHrsStr.concat(timeSpanToString(elapsedTime));

                    getLoggerInstance()->logProcess(feedRes.get(), SFP(HS_Log_LightingSequence), SFP(HS_Log_HasEnded));
                    getLoggerInstance()->logMessage(SFP(HS_DoubleSpace), SFP(HS_Key_DailyLightHours), lightHrsStr);
                }
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
            if (!(*actuatorIter)->getIsEnabled()) {
                if (!(*actuatorIter)->enableActuator()) {
                    // TODO: Something clever to track stalled actuators
                }
            }
        }
    }
}


HydroponicsSchedulerSubData::HydroponicsSchedulerSubData()
    : HydroponicsSubData(), baseFeedMultiplier(1), weeklyDosingRates{1}, stdDosingRates{1.0f,0.5f,0.5f}, totalFeedingsDay(0), preFeedAeratorMins(30), preLightSprayMins(60)
{
    type = 0; // no type differentiation
}

void HydroponicsSchedulerSubData::toJSONObject(JsonObject &objectOut) const
{
    //HydroponicsSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (!isFPEqual(baseFeedMultiplier, 1.0f)) { objectOut[SFP(HS_Key_BaseFeedMultiplier)] = baseFeedMultiplier; }
    bool hasWeeklyDosings = arrayElementsEqual(weeklyDosingRates, HYDRUINO_CROP_GROWWEEKS_MAX, 1.0f);
    if (hasWeeklyDosings) { objectOut[SFP(HS_Key_WeeklyDosingRates)] = commaStringFromArray(weeklyDosingRates, HYDRUINO_CROP_GROWWEEKS_MAX); }
    bool hasStandardDosings = !isFPEqual(stdDosingRates[0], 1.0f) || !isFPEqual(stdDosingRates[1], 0.5f) || !isFPEqual(stdDosingRates[2], 0.5f);
    if (hasStandardDosings) { objectOut[SFP(HS_Key_StdDosingRates)] = commaStringFromArray(stdDosingRates, 3); }
    if (totalFeedingsDay > 0) { objectOut[SFP(HS_Key_TotalFeedingsDay)] = totalFeedingsDay; }
    if (preFeedAeratorMins != 30) { objectOut[SFP(HS_Key_PreFeedAeratorMins)] = preFeedAeratorMins; }
    if (preLightSprayMins != 60) { objectOut[SFP(HS_Key_PreLightSprayMins)] = preLightSprayMins; }
}

void HydroponicsSchedulerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroponicsSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    baseFeedMultiplier = objectIn[SFP(HS_Key_BaseFeedMultiplier)] | baseFeedMultiplier;
    JsonVariantConst weeklyDosingRatesVar = objectIn[SFP(HS_Key_WeeklyDosingRates)];
    commaStringToArray(weeklyDosingRatesVar, weeklyDosingRates, HYDRUINO_CROP_GROWWEEKS_MAX);
    for (int weekIndex = 0; weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX; ++weekIndex) { 
        weeklyDosingRates[weekIndex] = weeklyDosingRatesVar[weekIndex] | weeklyDosingRates[weekIndex];
    }
    JsonVariantConst stdDosingRatesVar = objectIn[SFP(HS_Key_StdDosingRates)];
    commaStringToArray(stdDosingRatesVar, stdDosingRates, 3);
    for (int resIndex = 0; resIndex < 3; ++resIndex) { 
        stdDosingRates[resIndex] = stdDosingRatesVar[resIndex] | stdDosingRates[resIndex];
    }
    totalFeedingsDay = objectIn[SFP(HS_Key_TotalFeedingsDay)] | totalFeedingsDay;
    preFeedAeratorMins = objectIn[SFP(HS_Key_PreFeedAeratorMins)] | preFeedAeratorMins;
    preLightSprayMins = objectIn[SFP(HS_Key_PreLightSprayMins)] | preLightSprayMins;
}
