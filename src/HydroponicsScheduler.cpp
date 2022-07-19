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
            }
            if (_lastDayNum != currTime.day()) {
                _lastDayNum = currTime.day();
                setNeedsScheduling();
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
{ ; }

void HydroponicsScheduler::setupWaterPHBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *waterPHBalancer)
{
    if (reservoir && waterPHBalancer) {
        {   Map<Hydroponics_KeyType, Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_ACTUATORS_MAXSIZE>::type incActuators;
            auto waterPumps = linksFilterPumpActuatorsByInputReservoirType(reservoir->getLinkages(), Hydroponics_ReservoirType_PhUpSolution);
            auto phUpPumps = linksFilterActuatorsByType(waterPumps, Hydroponics_ActuatorType_PeristalticPump);
            float dosingRate = getCombinedDosingRate(reservoir, Hydroponics_ReservoirType_PhUpSolution);

            if (phUpPumps.size()) {
                for (auto pumpIter = phUpPumps.begin(); pumpIter != phUpPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                    if (pump) { incActuators.insert(pumpIter->first, arx::make_pair(pump, dosingRate)); }
                }
            } else if (waterPumps.size()) {
                phUpPumps = linksFilterActuatorsByType(waterPumps, Hydroponics_ActuatorType_WaterPump);

                for (auto pumpIter = phUpPumps.begin(); pumpIter != phUpPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                    if (pump) { incActuators.insert(pumpIter->first, arx::make_pair(pump, dosingRate)); }
                }
            }

            waterPHBalancer->setIncrementActuators(incActuators);
        }

        {   Map<Hydroponics_KeyType, Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_ACTUATORS_MAXSIZE>::type decActuators;
            auto waterPumps = linksFilterPumpActuatorsByInputReservoirType(reservoir->getLinkages(), Hydroponics_ReservoirType_PhDownSolution);
            auto phDownPumps = linksFilterActuatorsByType(waterPumps, Hydroponics_ActuatorType_PeristalticPump);
            float dosingRate = getCombinedDosingRate(reservoir, Hydroponics_ReservoirType_PhDownSolution);

            if (phDownPumps.size()) {
                for (auto pumpIter = phDownPumps.begin(); pumpIter != phDownPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                    if (pump) { decActuators.insert(pumpIter->first, arx::make_pair(pump, dosingRate)); }
                }
            } else if (waterPumps.size()) {
                phDownPumps = linksFilterActuatorsByType(waterPumps, Hydroponics_ActuatorType_WaterPump);

                for (auto pumpIter = phDownPumps.begin(); pumpIter != phDownPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                    if (pump) { decActuators.insert(pumpIter->first, arx::make_pair(pump, dosingRate)); }
                }
            }

            waterPHBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setupWaterTDSBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *waterTDSBalancer)
{
    if (reservoir && waterTDSBalancer) {
        {   Map<Hydroponics_KeyType, Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_ACTUATORS_MAXSIZE>::type incActuators;
            auto pumps = linksFilterPumpActuatorsByOutputReservoir(reservoir->getLinkages(), reservoir);
            auto nutrientPumps = linksFilterPumpActuatorsByInputReservoirType(pumps, Hydroponics_ReservoirType_NutrientPremix);

            for (auto pumpIter = nutrientPumps.begin(); pumpIter != nutrientPumps.end(); ++pumpIter) {
                auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                float dosingRate = getCombinedDosingRate(reservoir, Hydroponics_ReservoirType_NutrientPremix);
                if (pump && dosingRate > FLT_EPSILON) { incActuators.insert(pumpIter->first, arx::make_pair(pump, dosingRate)); }
            }

            for (Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_CustomAdditive1;
                 reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount;
                 reservoirType = (Hydroponics_ReservoirType)((int)reservoirType + 1)) {
                if (getHydroponicsInstance()->getCustomAdditiveData(reservoirType)) {
                    nutrientPumps = linksFilterPumpActuatorsByInputReservoirType(pumps, reservoirType);

                    for (auto pumpIter = nutrientPumps.begin(); pumpIter != nutrientPumps.end(); ++pumpIter) {
                        auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                        float dosingRate = getCombinedDosingRate(reservoir, reservoirType);
                        if (pump && dosingRate > FLT_EPSILON) { incActuators.insert(pumpIter->first, arx::make_pair(pump, dosingRate)); }
                    }
                }
            }

            waterTDSBalancer->setIncrementActuators(incActuators);
        }

        {   Map<Hydroponics_KeyType, Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_ACTUATORS_MAXSIZE>::type decActuators;
            auto pumps = linksFilterPumpActuatorsByInputReservoirType(reservoir->getLinkages(), Hydroponics_ReservoirType_FreshWater);
            auto dilutionPumps = linksFilterActuatorsByType(pumps, Hydroponics_ActuatorType_PeristalticPump);
            float dosingRate = getCombinedDosingRate(reservoir, Hydroponics_ReservoirType_FreshWater);

            if (dilutionPumps.size()) {
                for (auto pumpIter = dilutionPumps.begin(); pumpIter != dilutionPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                    if (pump) { decActuators.insert(pumpIter->first, arx::make_pair(pump, dosingRate)); }
                }
            } else if (pumps.size()) {
                dilutionPumps = linksFilterActuatorsByType(pumps, Hydroponics_ActuatorType_WaterPump);

                for (auto pumpIter = dilutionPumps.begin(); pumpIter != dilutionPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                    if (pump) { decActuators.insert(pumpIter->first, arx::make_pair(pump, dosingRate)); }
                }
            }

            waterTDSBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setupWaterTempBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *waterTempBalancer)
{
    if (reservoir && waterTempBalancer) {
        {   Map<Hydroponics_KeyType, Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_ACTUATORS_MAXSIZE>::type incActuators;
            auto heaters = linksFilterActuatorsByType(reservoir->getLinkages(), Hydroponics_ActuatorType_WaterHeater);

            for (auto heaterIter = heaters.begin(); heaterIter != heaters.end(); ++heaterIter) {
                auto heater = getSharedPtr<HydroponicsActuator>(heaterIter->second);
                if (heater) { incActuators.insert(heaterIter->first, arx::make_pair(heater, 1.0f)); }
            }

            waterTempBalancer->setIncrementActuators(incActuators);
        }

        {   Map<Hydroponics_KeyType, Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_ACTUATORS_MAXSIZE>::type decActuators;
            waterTempBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setupAirTempBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *airTempBalancer)
{
    if (reservoir && airTempBalancer) {
        {   Map<Hydroponics_KeyType, Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_ACTUATORS_MAXSIZE>::type incActuators;
            airTempBalancer->setIncrementActuators(incActuators);
        }

        {   Map<Hydroponics_KeyType, Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_ACTUATORS_MAXSIZE>::type decActuators;
            auto fans = linksFilterActuatorsByType(reservoir->getLinkages(), Hydroponics_ActuatorType_FanExhaust);

            for (auto fanIter = fans.begin(); fanIter != fans.end(); ++fanIter) {
                auto fan = getSharedPtr<HydroponicsActuator>(fanIter->second);
                if (fan) { decActuators.insert(fanIter->first, arx::make_pair(fan, 1.0f)); }
            }

            airTempBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setupAirCO2Balancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *airCO2Balancer)
{
    if (reservoir && airCO2Balancer) {
        {   Map<Hydroponics_KeyType, Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_ACTUATORS_MAXSIZE>::type incActuators;
            auto fans = linksFilterActuatorsByType(reservoir->getLinkages(), Hydroponics_ActuatorType_FanExhaust);

            for (auto fanIter = fans.begin(); fanIter != fans.end(); ++fanIter) {
                auto fan = getSharedPtr<HydroponicsActuator>(fanIter->second);
                if (fan) { incActuators.insert(fanIter->first, arx::make_pair(fan, 1.0f)); }
            }

            airCO2Balancer->setIncrementActuators(incActuators);
        }

        {   Map<Hydroponics_KeyType, Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_ACTUATORS_MAXSIZE>::type decActuators;
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
        auto crops = reservoir->getCrops();
        float totalWeights = 0;
        float totalDosing = 0;

        for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
            auto crop = (HydroponicsCrop *)(cropIter->second);
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

void HydroponicsScheduler::performScheduling()
{
    for (auto iter = getHydroponicsInstance()->_objects.begin(); iter != getHydroponicsInstance()->_objects.end(); ++iter) {
        if (iter->second && iter->second->isReservoirType()) {
            auto reservoir = static_pointer_cast<HydroponicsReservoir>(iter->second);
            if (reservoir && reservoir->isFeedClass()) {
                auto feedReservoir = static_pointer_cast<HydroponicsFeedReservoir>(iter->second);

                if (feedReservoir) {
                    {   auto feedingIter = _feedings.find(feedReservoir->getId().key);
                        auto crops = feedReservoir->getCrops();
                        if (crops.size()) {
                            if (feedingIter != _feedings.end()) {
                                if (feedingIter->second) { feedingIter->second->recalcFeeding(); }
                            } else {
                                HydroponicsFeeding *feeding = new HydroponicsFeeding(feedReservoir);
                                HYDRUINO_SOFT_ASSERT(feeding, SFP(HS_Err_AllocationFailure));
                                if (feeding) { _feedings.insert(feedReservoir->getId().key, feeding); }
                            }
                        } else if (feedingIter != _feedings.end()) { // No crops, but found in feedings
                            if (feedingIter->second) { delete feedingIter->second; }
                            _feedings.erase(feedingIter);
                        }
                    }

                    {   auto lightingIter = _lightings.find(feedReservoir->getId().key);
                        auto actuators = feedReservoir->getActuators();
                        if (actuators.size() &&
                            (linksFilterActuatorsByType(actuators, Hydroponics_ActuatorType_GrowLights).size() ||
                             linksFilterActuatorsByType(actuators, Hydroponics_ActuatorType_WaterSprayer).size())) {
                            if (lightingIter != _lightings.end()) {
                                if (lightingIter->second) { lightingIter->second->recalcLighting(); }
                            } else {
                                HydroponicsLighting *lighting = new HydroponicsLighting(feedReservoir);
                                HYDRUINO_SOFT_ASSERT(lighting, SFP(HS_Err_AllocationFailure));
                                if (lighting) { _lightings.insert(feedReservoir->getId().key, lighting); }
                            }
                        } else if (lightingIter != _lightings.end()) { // No lights/sprayers, but found in lightings
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

void HydroponicsScheduler::broadcastDayChange() const
{
    for (auto iter = getHydroponicsInstance()->_objects.begin(); iter != getHydroponicsInstance()->_objects.end(); ++iter) {
        if (iter->second) {
            if (iter->second->isReservoirType()) {
                auto reservoir = static_pointer_cast<HydroponicsReservoir>(iter->second);
                if (reservoir && reservoir->isFeedClass()) {
                    auto feedReservoir = static_pointer_cast<HydroponicsFeedReservoir>(iter->second);
                    if (feedReservoir) { feedReservoir->notifyDayChanged(); }
                }
            } else if (iter->second->isCropType()) {
                auto crop = static_pointer_cast<HydroponicsCrop>(iter->second);
                if (crop) { crop->notifyDayChanged(); }
            }
        }
    }
}


HydroponicsFeeding::HydroponicsFeeding(shared_ptr<HydroponicsFeedReservoir> feedResIn)
    : feedRes(feedResIn), stage(Unknown), stageStart(0), canFeedAfter(0), phSetpoint(0), tdsSetpoint(0), waterTempSetpoint(0), airTempSetpoint(0), co2Setpoint(0)
{
    reset();
}

HydroponicsFeeding::~HydroponicsFeeding()
{
    clearActReqs();
}

void HydroponicsFeeding::reset()
{
    clearActReqs();
    stage = Init; stageStart = now();
    canFeedAfter = 0;
    recalcFeeding();
    setupStaging();
}

void HydroponicsFeeding::clearActReqs()
{
    while(actuatorReqs.size()) {
        (*(actuatorReqs.begin()))->disableActuator();
        actuatorReqs.erase(actuatorReqs.begin());
    }
}

void HydroponicsFeeding::recalcFeeding()
{
    float totalWeights = 0;
    float totalSetpoints[5] = {0};
    auto crops = feedRes->getCrops();

    for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
        auto crop = (HydroponicsCrop *)(cropIter->second);
        auto cropsLibData = crop ? getCropsLibraryInstance()->checkoutCropsData(crop->getCropType()) : nullptr;

        if (cropsLibData) {
            totalWeights += crop->getFeedingWeight();

            float feedRate = ((cropsLibData->tdsRange[0] + cropsLibData->tdsRange[1]) * 0.5);
            if (!getSchedulerInstance()->getInDaytimeMode()) {
                feedRate *= cropsLibData->nightlyFeedMultiplier;
            }
            feedRate *= getSchedulerInstance()->getBaseFeedMultiplier();

            totalSetpoints[0] += ((cropsLibData->phRange[0] + cropsLibData->phRange[1]) * 0.5);
            totalSetpoints[1] += feedRate;
            totalSetpoints[2] += ((cropsLibData->waterTempRange[0] + cropsLibData->waterTempRange[1]) * 0.5);
            totalSetpoints[3] += ((cropsLibData->airTempRange[0] + cropsLibData->airTempRange[1]) * 0.5);
            totalSetpoints[4] += cropsLibData->co2Levels[(crop->getCropPhase() <= Hydroponics_CropPhase_Vegetative ? 0 : 1)];

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

    if (feedRes->getWaterPHBalancer()) { feedRes->setWaterPHBalancer(phSetpoint, Hydroponics_UnitsType_pHScale_0_14); }
    if (feedRes->getWaterTDSBalancer()) { feedRes->setWaterTDSBalancer(tdsSetpoint, Hydroponics_UnitsType_Concentration_EC); }
    if (feedRes->getWaterTempBalancer()) { feedRes->setWaterTempBalancer(waterTempSetpoint, Hydroponics_UnitsType_Temperature_Celsius); }
    if (feedRes->getAirTempBalancer()) { feedRes->setAirTempBalancer(airTempSetpoint, Hydroponics_UnitsType_Temperature_Celsius); }
    if (feedRes->getAirCO2Balancer()) { feedRes->setAirCO2Balancer(co2Setpoint, Hydroponics_UnitsType_Concentration_PPM); }
}

void HydroponicsFeeding::setupStaging()
{
    clearActReqs();

    if (stage == PreFeed) {
        if (feedRes->getWaterPHSensor()) {
            auto phBalancer = feedRes->setWaterPHBalancer(phSetpoint, Hydroponics_UnitsType_pHScale_0_14);
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
                canFeedAfter = (time_t)UINT32_MAX;
            }
        } break;

        case TopOff: {
            auto pumps = linksFilterPumpActuatorsByOutputReservoir(feedRes->getLinkages(), feedRes.get());
            pumps = linksFilterPumpActuatorsByInputReservoirType(pumps, Hydroponics_ReservoirType_FreshWater);
            auto fillPumps = linksFilterActuatorsByType(pumps, Hydroponics_ActuatorType_WaterPump);

            if (fillPumps.size()) { // Fresh water pumps
                for (auto pumpIter = fillPumps.begin(); pumpIter != fillPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                    if (pump) { actuatorReqs.push_back(pump); }
                }
            } else { // Fresh water peristaltic pumps
                fillPumps = linksFilterActuatorsByType(pumps, Hydroponics_ActuatorType_PeristalticPump);

                for (auto pumpIter = fillPumps.begin(); pumpIter != fillPumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                    if (pump) { actuatorReqs.push_back(pump); }
                }
            }

            HYDRUINO_SOFT_ASSERT(actuatorReqs.size(), SFP(HS_Err_MissingLinkage)); // no fresh water pumps
        } break;

        case PreFeed: {
            canFeedAfter = 0; // will be used to track time balancers stay balanced
            auto aerators = linksFilterActuatorsByType(feedRes->getLinkages(), Hydroponics_ActuatorType_WaterAerator);

            for (auto aeratorIter = aerators.begin(); aeratorIter != aerators.end(); ++aeratorIter) {
                auto aerator = getSharedPtr<HydroponicsActuator>(aeratorIter->second);
                if (aerator) { actuatorReqs.push_back(aerator); }
            }
        } break;

        case Feed: {
            {   auto pumps = linksFilterPumpActuatorsByInputReservoir(feedRes->getLinkages(), feedRes.get());
                pumps = linksFilterActuatorsByType(pumps, Hydroponics_ActuatorType_WaterPump);
                auto feedPumps = linksFilterPumpActuatorsByOutputReservoir(pumps, feedRes.get());

                if (feedPumps.size()) { // Recycling feed pumps
                    for (auto pumpIter = feedPumps.begin(); pumpIter != feedPumps.end(); ++pumpIter) {
                        auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                        if (pump) { actuatorReqs.push_back(pump); }
                    }
                } else if (pumps.size() && getHydroponicsInstance()->getSystemMode() == Hydroponics_SystemMode_DrainToWaste) { // DTW feed pumps
                    feedPumps = linksFilterPumpActuatorsByOutputReservoirType(pumps, Hydroponics_ReservoirType_DrainageWater);

                    for (auto pumpIter = feedPumps.begin(); pumpIter != feedPumps.end(); ++pumpIter) {
                        auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                        if (pump) { actuatorReqs.push_back(pump); }
                    }
                }
            }

            HYDRUINO_SOFT_ASSERT(actuatorReqs.size(), SFP(HS_Err_MissingLinkage)); // no feed water pumps

            {   auto aerators = linksFilterActuatorsByType(feedRes->getLinkages(), Hydroponics_ActuatorType_WaterAerator);

                for (auto aeratorIter = aerators.begin(); aeratorIter != aerators.end(); ++aeratorIter) {
                    auto aerator = getSharedPtr<HydroponicsActuator>(aeratorIter->second);
                    if (aerator) { actuatorReqs.push_back(aerator); }
                }
            }
        } break;

        case Drain: {
            auto pumps = linksFilterPumpActuatorsByInputReservoir(feedRes->getLinkages(), feedRes.get());
            pumps = linksFilterPumpActuatorsByOutputReservoirType(pumps, Hydroponics_ReservoirType_DrainageWater);
            auto drainagePumps = linksFilterActuatorsByType(pumps, Hydroponics_ActuatorType_WaterPump);

            if (drainagePumps.size()) { // Drainage pumps
                for (auto pumpIter = drainagePumps.begin(); pumpIter != drainagePumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                    if (pump) { actuatorReqs.push_back(pump); }
                }
            } else if (pumps.size()) { // Drainage peristaltic pumps (why? why not)
                drainagePumps = linksFilterActuatorsByType(pumps, Hydroponics_ActuatorType_PeristalticPump);

                for (auto pumpIter = drainagePumps.begin(); pumpIter != drainagePumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                    if (pump) { actuatorReqs.push_back(pump); }
                }
            }

            HYDRUINO_SOFT_ASSERT(actuatorReqs.size(), SFP(HS_Err_MissingLinkage)); // no drainage water pumps
        } break;
    }
}

void HydroponicsFeeding::update()
{
    if (isDone()) { reset(); return; }

    switch (stage) {
        case Init: {
            if (!canFeedAfter || now() >= canFeedAfter) {
                int cropsHungry = 0;
                auto crops = feedRes->getCrops();
                for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
                    auto crop = (HydroponicsCrop *)(cropIter->second);
                    if (crop->getNeedsFeeding()) { cropsHungry++; }
                }

                if (cropsHungry / (float)crops.size() >= HYDRUINO_SCHEDULER_FEED_FRACTION - FLT_EPSILON) {
                    stage = TopOff; stageStart = now();
                    setupStaging();
                }
            }
        } break;

        case TopOff: {
            if (feedRes->getIsFilled()) {
                stage = PreFeed; stageStart = now();
                setupStaging();
            }
        } break;

        case PreFeed: {
            if (!actuatorReqs.size() || now() >= stageStart + getSchedulerInstance()->getPreFeedAeratorMins()) {
                auto phBalancer = feedRes->getWaterPHBalancer();
                auto tdsBalancer = feedRes->getWaterTDSBalancer();
                auto waterTempBalancer = feedRes->getWaterTempBalancer();

                if ((!phBalancer || (phBalancer->getIsEnabled() && phBalancer->getIsBalanced())) &&
                    (!tdsBalancer || (tdsBalancer->getIsEnabled() && tdsBalancer->getIsBalanced())) &&
                    (!waterTempBalancer || (waterTempBalancer->getIsEnabled() && waterTempBalancer->getIsBalanced()))) {
                    // Can proceed after all tanks are marked balanced for min time
                    if (!canFeedAfter) { canFeedAfter = now() + HYDRUINO_SCHEDULER_BALANCE_MINTIME; }
                    else if (now() >= canFeedAfter) {
                        stage = Feed; stageStart = now();
                        setupStaging();
                        broadcastFeedingBegan();
                    }
                } else {
                    canFeedAfter = 0;
                }
            }
        } break;

        case Feed: {
            int cropsFed = 0;
            auto crops = feedRes->getCrops();
            for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
                auto crop = (HydroponicsCrop *)(cropIter->second);
                if (!crop->getNeedsFeeding()) { cropsFed++; }
            }

            if (cropsFed / (float)crops.size() >= HYDRUINO_SCHEDULER_FEED_FRACTION - FLT_EPSILON) {
                stage = (getHydroponicsInstance()->getSystemMode() == Hydroponics_SystemMode_DrainToWaste ? Drain : Done);
                stageStart = now();
                setupStaging();
                broadcastFeedingEnded();
            }
        } break;

        case Drain: {
            if (getHydroponicsInstance()->getSystemMode() != Hydroponics_SystemMode_DrainToWaste ||
                feedRes->getIsEmpty()) {
                stage = Done; stageStart = now();
                setupStaging();
            }
        } break;

        default:
            break;
    }

    for (auto actuatorIter = actuatorReqs.begin(); actuatorIter != actuatorReqs.end(); ++actuatorIter) {
        if (!(*actuatorIter)->getIsEnabled()) {
            (*actuatorIter)->enableActuator();
        }
    }
}

void HydroponicsFeeding::broadcastFeedingBegan()
{
    auto crops = feedRes->getCrops();
    feedRes->notifyFeedingBegan();
    for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
        auto crop = (HydroponicsCrop *)(cropIter->second);
        crop->notifyFeedingBegan();
    }
}

void HydroponicsFeeding::broadcastFeedingEnded()
{
    auto crops = feedRes->getCrops();
    feedRes->notifyFeedingEnded();
    for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
        auto crop = (HydroponicsCrop *)(cropIter->second);
        crop->notifyFeedingEnded();
    }
}


HydroponicsLighting::HydroponicsLighting(shared_ptr<HydroponicsFeedReservoir> feedResIn)
    : feedRes(feedResIn), stage(Init), sprayStart(0), lightStart(0), lightEnd(0)
{
    recalcLighting();
}

HydroponicsLighting::~HydroponicsLighting()
{
    clearActReqs();
}

void HydroponicsLighting::clearActReqs()
{
    while(actuatorReqs.size()) {
        (*(actuatorReqs.begin()))->disableActuator();
        actuatorReqs.erase(actuatorReqs.begin());
    }
}

void HydroponicsLighting::recalcLighting()
{
    float totalWeights = 0;
    int totalLightHours = 0;
    auto crops = feedRes->getCrops();
    bool sprayingNeeded = false;

    for (auto cropIter = crops.begin(); cropIter != crops.end(); ++cropIter) {
        auto crop = (HydroponicsCrop *)(cropIter->second);
        Hydroponics_CropPhase cropPhase = crop ? crop->getCropPhase() : Hydroponics_CropPhase_Undefined;

        if ((int)cropPhase >= 0 && cropPhase < Hydroponics_CropPhase_MainCount) {
            auto cropsLibData = getCropsLibraryInstance()->checkoutCropsData(crop->getCropType());

            if (cropsLibData) {
                totalWeights += crop->getFeedingWeight();
                totalLightHours += cropsLibData->dailyLightHours[cropPhase];
                sprayingNeeded = sprayingNeeded || cropsLibData->getNeedsSpraying();

                getCropsLibraryInstance()->returnCropsData(cropsLibData);
            }
        }
    }

    if (totalWeights < FLT_EPSILON) {
        totalWeights = 1.0f;
        totalLightHours = 12;
    }

    time_t dayLightSecs = (totalLightHours / totalWeights) * SECS_PER_HOUR;
    dayLightSecs = constrain(dayLightSecs, 0, SECS_PER_DAY);

    time_t daySprayerSecs = 0;
    if (sprayingNeeded && linksFilterActuatorsByType(feedRes->getLinkages(), Hydroponics_ActuatorType_WaterSprayer).size()) {
        daySprayerSecs = getSchedulerInstance()->getPreLightSprayMins() * SECS_PER_MIN;
    }

    time_t dayStart = getCurrentDayStartTime();
    lightStart = dayStart + ((SECS_PER_DAY - dayLightSecs) >> 1);
    sprayStart = max(dayStart, lightStart - daySprayerSecs);
    lightStart = sprayStart + daySprayerSecs;
    lightEnd = lightStart + dayLightSecs;

    setupStaging();
}

void HydroponicsLighting::setupStaging()
{
    clearActReqs();

    time_t time = now();
    stage = Init;
    if (time >= sprayStart) { stage = (typeof(stage))((int)stage + 1); }
    if (time >= lightStart) { stage = (typeof(stage))((int)stage + 1); }
    if (time >= lightEnd) { stage = (typeof(stage))((int)stage + 1); }

    switch(stage) {
        case Spray: {
            auto sprayers = linksFilterActuatorsByType(feedRes->getLinkages(), Hydroponics_ActuatorType_WaterSprayer);
            for (auto sprayerIter = sprayers.begin(); sprayerIter != sprayers.end(); ++sprayerIter) {
                auto sprayer = getSharedPtr<HydroponicsActuator>(sprayerIter->second);
                if (sprayer) { actuatorReqs.push_back(sprayer); }
            }
        } break;

        case Light: {
            auto lights = linksFilterActuatorsByType(feedRes->getLinkages(), Hydroponics_ActuatorType_GrowLights);
            for (auto lightIter = lights.begin(); lightIter != lights.end(); ++lightIter) {
                auto light = getSharedPtr<HydroponicsActuator>(lightIter->second);
                if (light) { actuatorReqs.push_back(light); }
            }
        } break;

        default:
            break;
    }
}

void HydroponicsLighting::update()
{
    if (isDone()) { return; }
    time_t time = now();

    switch (stage) {
        case Init:
            if ((sprayStart && time >= sprayStart) || (lightStart && time >= lightStart)) { setupStaging(); }
            break;

        case Spray:
            if ((lightStart && time >= lightStart) || (lightEnd && time >= lightEnd)) { setupStaging(); }
            break;

        case Light:
            if (lightEnd && time >= lightEnd) { setupStaging(); }
            break;

        default:
            break;
    }

    for (auto actuatorIter = actuatorReqs.begin(); actuatorIter != actuatorReqs.end(); ++actuatorIter) {
        if (!(*actuatorIter)->getIsEnabled()) {
            (*actuatorIter)->enableActuator();
        }
    }
}


HydroponicsSchedulerSubData::HydroponicsSchedulerSubData()
    : HydroponicsSubData(), baseFeedMultiplier(1), weeklyDosingRates{1}, stdDosingRates{1.0f,0.5f,0.5f}, totalFeedingsDay(0), preFeedAeratorMins(30), preLightSprayMins(60)
{ ; }

void HydroponicsSchedulerSubData::toJSONObject(JsonObject &objectOut) const
{
    // purposeful no call to base method (ignores type)

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
    // purposeful no call to base method (ignores type)

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
