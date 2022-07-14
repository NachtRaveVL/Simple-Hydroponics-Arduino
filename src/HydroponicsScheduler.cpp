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
            bool nowDaytimeMode = currTime.hour() >= HYDRUINO_CROP_NIGHT_END_HR && currTime.hour() < HYDRUINO_CROP_NIGHT_BEGIN_HR;

            if (_inDaytimeMode != nowDaytimeMode) {
                _inDaytimeMode = nowDaytimeMode;
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

void HydroponicsScheduler::setupPHBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *phBalancer)
{
    if (reservoir && phBalancer) {
        {   arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> incActuators;
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

            phBalancer->setIncrementActuators(incActuators);
        }

        {   arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> decActuators;
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

            phBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setupTDSBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *tdsBalancer)
{
    if (reservoir && tdsBalancer) {
        {   arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> incActuators;
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

            tdsBalancer->setIncrementActuators(incActuators);
        }

        {   arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> decActuators;
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

            tdsBalancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setupTempBalancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *tempBalancer)
{
    if (reservoir && tempBalancer) {
        {   arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> incActuators;
            auto heaters = linksFilterActuatorsByType(reservoir->getLinkages(), Hydroponics_ActuatorType_WaterHeater);

            for (auto heaterIter = heaters.begin(); heaterIter != heaters.end(); ++heaterIter) {
                auto heater = getSharedPtr<HydroponicsActuator>(heaterIter->second);
                if (heater) { incActuators.insert(heaterIter->first, arx::make_pair(heater, 1.0f)); }
            }

            tempBalancer->setIncrementActuators(incActuators);
        }

        {   arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> decActuators;
            tempBalancer->setDecrementActuators(decActuators);
        }
    }
}

void setupCO2Balancer(HydroponicsReservoir *reservoir, HydroponicsBalancer *co2Balancer)
{
    if (reservoir && co2Balancer) {
        {   arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> incActuators;
            auto fans = linksFilterActuatorsByType(reservoir->getLinkages(), Hydroponics_ActuatorType_FanExhaust);

            for (auto fanIter = fans.begin(); fanIter != fans.end(); ++fanIter) {
                auto fan = static_pointer_cast<HydroponicsActuator>(fanIter->second->getSharedPtr());
                if (fan) { incActuators.insert(fanIter->first, arx::make_pair(fan, 1.0f)); }
            }

            co2Balancer->setIncrementActuators(incActuators);
        }

        {   arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> decActuators;
            co2Balancer->setDecrementActuators(decActuators);
        }
    }
}

void HydroponicsScheduler::setBaseFeedMultiplier(float baseFeedMultiplier)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    if (_schedulerData) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _schedulerData->baseFeedMultiplier = baseFeedMultiplier;
        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setWeeklyDosingRate(int weekIndex, float dosingRate, Hydroponics_ReservoirType reservoirType)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX), F("Invalid week index"));

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
            HYDRUINO_SOFT_ASSERT(false, F("Invalid reservoir type"));
        }
    }
}

void HydroponicsScheduler::setStandardDosingRate(float dosingRate, Hydroponics_ReservoirType reservoirType)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (reservoirType >= Hydroponics_ReservoirType_FreshWater && reservoirType < Hydroponics_ReservoirType_CustomAdditive1), F("Invalid reservoir type"));

    if (_schedulerData && (reservoirType >= Hydroponics_ReservoirType_FreshWater && reservoirType < Hydroponics_ReservoirType_CustomAdditive1)) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _schedulerData->standardDosingRates[reservoirType - Hydroponics_ReservoirType_FreshWater] = dosingRate;

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
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX), F("Invalid week index"));

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
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));

    if (_schedulerData) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _schedulerData->totalFeedingsDay = feedingsDay;

        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setPreFeedAeratorMins(unsigned int aeratorMins)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));

    if (_schedulerData) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _schedulerData->preFeedAeratorMins = aeratorMins;

        setNeedsScheduling();
    }
}

void HydroponicsScheduler::setPreLightSprayMins(unsigned int sprayMins)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));

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
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || reservoir, F("Invalid reservoir"));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || !reservoir || (reservoirType >= Hydroponics_ReservoirType_NutrientPremix &&
                                                           reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount), F("Invalid reservoir type"));

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
                    totalDosing += _schedulerData->standardDosingRates[reservoirType - Hydroponics_ReservoirType_FreshWater];
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
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    return _schedulerData ? _schedulerData->baseFeedMultiplier : 0.0f;
}

float HydroponicsScheduler::getWeeklyDosingRate(int weekIndex, Hydroponics_ReservoirType reservoirType) const
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX), F("Invalid week index"));

    if (_schedulerData && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX) {
        if (reservoirType == Hydroponics_ReservoirType_NutrientPremix) {
            return _schedulerData->weeklyDosingRates[weekIndex];
        } else if (reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 && reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
            auto additiveDate = getHydroponicsInstance()->getCustomAdditiveData(reservoirType);
            return additiveDate ? additiveDate->weeklyDosingRates[weekIndex] : 0.0f;
        } else {
            HYDRUINO_SOFT_ASSERT(false, F("Invalid reservoir type"));
        }
    }
    return 0.0f;
}

float HydroponicsScheduler::getStandardDosingRate(Hydroponics_ReservoirType reservoirType) const
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (reservoirType >= Hydroponics_ReservoirType_FreshWater && reservoirType < Hydroponics_ReservoirType_CustomAdditive1), F("Invalid reservoir type"));

    if (_schedulerData && reservoirType >= Hydroponics_ReservoirType_FreshWater && reservoirType < Hydroponics_ReservoirType_CustomAdditive1) {
        return _schedulerData->standardDosingRates[reservoirType - Hydroponics_ReservoirType_FreshWater];
    }
    return 0.0f;
}

bool HydroponicsScheduler::getIsFlushWeek(int weekIndex)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX), F("Invalid week index"));

    if (_schedulerData && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX) {
        return isFPEqual(_schedulerData->weeklyDosingRates[weekIndex], 0.0f);
    }
    return false;
}

unsigned int HydroponicsScheduler::getTotalFeedingsDay() const
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    return _schedulerData ? _schedulerData->totalFeedingsDay : 0;
}

unsigned int HydroponicsScheduler::getPreFeedAeratorMins() const
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    return _schedulerData ? _schedulerData->preFeedAeratorMins : 0;
}

unsigned int HydroponicsScheduler::getPreLightSprayMins() const
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
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
                                HYDRUINO_SOFT_ASSERT(feeding, F("Failure allocating feeding"));
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
                                HYDRUINO_SOFT_ASSERT(lighting, F("Failure allocating lighting"));
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
    : feedRes(feedResIn), stage(Unknown), stageStart(0), canFeedAfter(0), phSetpoint(0), tdsSetpoint(0), tempSetpoint(0)
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
    float totalSetpoints[3] = {0};
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

            getCropsLibraryInstance()->returnCropsData(cropsLibData);
        }
    }

    if (totalWeights < FLT_EPSILON) {
        totalWeights = 1.0f;
        totalSetpoints[0] = 6;
    }

    tdsSetpoint = totalSetpoints[1] / totalWeights;
    if (tdsSetpoint < FLT_EPSILON) { // Flushing
        phSetpoint = 6;
    } else {
        phSetpoint = totalSetpoints[0] / totalWeights;
    }
    tempSetpoint = totalSetpoints[2] / totalWeights;

    if (feedRes->getWaterPHBalancer()) { feedRes->setWaterPHBalancer(phSetpoint, Hydroponics_UnitsType_pHScale_0_14); }
    if (feedRes->getWaterTDSBalancer()) { feedRes->setWaterTDSBalancer(tdsSetpoint, Hydroponics_UnitsType_Concentration_EC); }
    if (feedRes->getWaterTempBalancer()) { feedRes->setWaterTempBalancer(tempSetpoint, Hydroponics_UnitsType_Temperature_Celsius); }
}

void HydroponicsFeeding::setupStaging()
{
    clearActReqs();

    if (stage == PreFeed) {
        if (feedRes->getWaterPHSensor()) {
            auto phBalancer = feedRes->setWaterPHBalancer(phSetpoint, Hydroponics_UnitsType_pHScale_0_14);
            getSchedulerInstance()->setupPHBalancer(feedRes.get(), phBalancer);
        }
        if (feedRes->getWaterTDSSensor()) {
            auto tdsBalancer = feedRes->setWaterTDSBalancer(tdsSetpoint, Hydroponics_UnitsType_Concentration_EC);
            getSchedulerInstance()->setupTDSBalancer(feedRes.get(), tdsBalancer);
        }
    }
    if ((stage == PreFeed || stage == Feed) && feedRes->getWaterTempSensor()) {
        auto tempBalancer = feedRes->setWaterTempBalancer(tempSetpoint, Hydroponics_UnitsType_Temperature_Celsius);
        getSchedulerInstance()->setupTempBalancer(feedRes.get(), tempBalancer);
    }
    if (feedRes->getWaterPHBalancer()) {
        feedRes->getWaterPHBalancer()->setEnabled(stage == PreFeed);
    }
    if (feedRes->getWaterTDSBalancer()) {
        feedRes->getWaterTDSBalancer()->setEnabled(stage == PreFeed);
    }
    if (feedRes->getWaterTempBalancer()) {
        feedRes->getWaterTempBalancer()->setEnabled(stage == PreFeed || stage == Feed);
    }

    switch (stage) {
        case Init: {
            auto maxFeedingsDay = getSchedulerInstance()->getTotalFeedingsDay();
            auto feedingsToday = feedRes->getFeedingsToday();

            if (!maxFeedingsDay) {
                canFeedAfter = (time_t)0;
            } else if (feedingsToday < maxFeedingsDay) {
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

            HYDRUINO_SOFT_ASSERT(actuatorReqs.size(), F("No linked fresh water pumps"));
        } break;

        case PreFeed: {
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

            HYDRUINO_SOFT_ASSERT(actuatorReqs.size(), F("No linked feed water pumps"));

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
            } else if (pumps.size()) { // Drainage peristaltic pumps
                drainagePumps = linksFilterActuatorsByType(pumps, Hydroponics_ActuatorType_PeristalticPump);

                for (auto pumpIter = drainagePumps.begin(); pumpIter != drainagePumps.end(); ++pumpIter) {
                    auto pump = getSharedPtr<HydroponicsActuator>(pumpIter->second);
                    if (pump) { actuatorReqs.push_back(pump); }
                }
            }

            HYDRUINO_SOFT_ASSERT(actuatorReqs.size(), F("No linked drainage pumps found"));
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
                auto tempBalancer = feedRes->getWaterTempBalancer();

                if ((!phBalancer || phBalancer->getIsBalanced()) &&
                    (!tdsBalancer || tdsBalancer->getIsBalanced()) &&
                    (!tempBalancer || tempBalancer->getIsBalanced())) {
                    stage = Feed; stageStart = now();
                    setupStaging();
                    broadcastFeedingBegan();
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
                sprayingNeeded = sprayingNeeded || cropsLibData->isSprayingNeeded;

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
        case Spray:
            auto sprayers = linksFilterActuatorsByType(feedRes->getLinkages(), Hydroponics_ActuatorType_WaterSprayer);
            for (auto sprayerIter = sprayers.begin(); sprayerIter != sprayers.end(); ++sprayerIter) {
                auto sprayer = getSharedPtr<HydroponicsActuator>(sprayerIter->second);
                if (sprayer) { actuatorReqs.push_back(sprayer); }
            }
            break;
        case Light:
            auto lights = linksFilterActuatorsByType(feedRes->getLinkages(), Hydroponics_ActuatorType_GrowLights);
            for (auto lightIter = lights.begin(); lightIter != lights.end(); ++lightIter) {
                auto light = getSharedPtr<HydroponicsActuator>(lightIter->second);
                if (light) { actuatorReqs.push_back(light); }
            }
            break;
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
    : HydroponicsSubData(), baseFeedMultiplier(1), weeklyDosingRates{1}, standardDosingRates{1.0f,0.5f,0.5f}, totalFeedingsDay(0), preFeedAeratorMins(30), preLightSprayMins(60)
{ ; }

void HydroponicsSchedulerSubData::toJSONObject(JsonObject &objectOut) const
{
    // purposeful no call to base method (ignores type)

    if (!isFPEqual(baseFeedMultiplier, 1.0f)) { objectOut[F("baseFeedMultiplier")] = baseFeedMultiplier; }
    bool hasWeeklyDosings = arrayElementsEqual(weeklyDosingRates, HYDRUINO_CROP_GROWWEEKS_MAX, 1.0f);
    if (hasWeeklyDosings) { objectOut[F("weeklyDosingRates")] = commaStringFromArray(weeklyDosingRates, HYDRUINO_CROP_GROWWEEKS_MAX); }
    bool hasStandardDosings = !isFPEqual(standardDosingRates[0], 1.0f) || !isFPEqual(standardDosingRates[1], 0.5f) || !isFPEqual(standardDosingRates[2], 0.5f);
    if (hasStandardDosings) { objectOut[F("standardDosingRates")] = commaStringFromArray(standardDosingRates, 3); }
    if (totalFeedingsDay > 0) { objectOut[F("totalFeedingsDay")] = totalFeedingsDay; }
    if (preFeedAeratorMins != 30) { objectOut[F("preFeedAeratorMins")] = preFeedAeratorMins; }
    if (preLightSprayMins != 60) { objectOut[F("preLightSprayMins")] = preLightSprayMins; }
}

void HydroponicsSchedulerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    // purposeful no call to base method (ignores type)

    baseFeedMultiplier = objectIn[F("baseFeedMultiplier")] | baseFeedMultiplier;
    JsonVariantConst weeklyDosingRatesVar = objectIn[F("weeklyDosingRates")];
    commaStringToArray(weeklyDosingRatesVar, weeklyDosingRates, HYDRUINO_CROP_GROWWEEKS_MAX);
    for (int weekIndex = 0; weekIndex < HYDRUINO_CROP_GROWWEEKS_MAX; ++weekIndex) { 
        weeklyDosingRates[weekIndex] = weeklyDosingRatesVar[weekIndex] | weeklyDosingRates[weekIndex];
    }
    JsonVariantConst standardDosingRatesVar = objectIn[F("standardDosingRates")];
    commaStringToArray(standardDosingRatesVar, standardDosingRates, 3);
    for (int resIndex = 0; resIndex < 3; ++resIndex) { 
        standardDosingRates[resIndex] = standardDosingRatesVar[resIndex] | standardDosingRates[resIndex];
    }
    totalFeedingsDay = objectIn[F("totalFeedingsDay")] | totalFeedingsDay;
    preFeedAeratorMins = objectIn[F("preFeedAeratorMins")] | preFeedAeratorMins;
    preLightSprayMins = objectIn[F("preLightSprayMins")] | preLightSprayMins;
}
