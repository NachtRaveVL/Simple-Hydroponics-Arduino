/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Scheduler
*/

#include "Hydroponics.h"

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
    auto linkedCrops = feedRes->getCrops();

    for (auto cropIter = linkedCrops.begin(); cropIter != linkedCrops.end(); ++cropIter) {
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
            feedRes->setWaterPHBalancer(phSetpoint, Hydroponics_UnitsType_pHScale_0_14);
        }
        if (feedRes->getWaterTDSSensor()) {
            feedRes->setWaterTDSBalancer(tdsSetpoint, Hydroponics_UnitsType_Concentration_EC);
        }
    }
    if ((stage == PreFeed || stage == Feed) && feedRes->getWaterTempSensor()) {
        feedRes->setWaterTempBalancer(tempSetpoint, Hydroponics_UnitsType_Temperature_Celsius);
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
                    auto pump = getHydroponicsInstance()->actuatorById(pumpIter->second->getId());
                    if (pump) { actuatorReqs.push_back(pump); }
                }
            } else { // Fresh water peristaltic pumps
                fillPumps = linksFilterActuatorsByType(pumps, Hydroponics_ActuatorType_PeristalticPump);

                for (auto pumpIter = fillPumps.begin(); pumpIter != fillPumps.end(); ++pumpIter) {
                    auto pump = getHydroponicsInstance()->actuatorById(pumpIter->second->getId());
                    if (pump) { actuatorReqs.push_back(pump); }
                }
            }

            HYDRUINO_SOFT_ASSERT(actuatorReqs.size(), F("No linked fresh water pumps"));
        } break;

        case PreFeed: {
            auto aerators = linksFilterActuatorsByType(feedRes->getLinkages(), Hydroponics_ActuatorType_WaterAerator);

            for (auto aeratorIter = aerators.begin(); aeratorIter != aerators.end(); ++aeratorIter) {
                auto aerator = getHydroponicsInstance()->actuatorById(aeratorIter->second->getId());
                if (aerator) { actuatorReqs.push_back(aerator); }
            }
        } break;

        case Feed: {
            {   auto pumps = linksFilterPumpActuatorsByInputReservoir(feedRes->getLinkages(), feedRes.get());
                pumps = linksFilterActuatorsByType(pumps, Hydroponics_ActuatorType_WaterPump);
                auto feedPumps = linksFilterPumpActuatorsByOutputReservoir(pumps, feedRes.get());

                if (feedPumps.size()) { // Recycling feed pumps
                    for (auto pumpIter = feedPumps.begin(); pumpIter != feedPumps.end(); ++pumpIter) {
                        auto pump = getHydroponicsInstance()->actuatorById(pumpIter->second->getId());
                        if (pump) { actuatorReqs.push_back(pump); }
                    }
                } else if (pumps.size() && getHydroponicsInstance()->getSystemMode() == Hydroponics_SystemMode_DrainToWaste) { // DTW feed pumps
                    feedPumps = linksFilterPumpActuatorsByOutputReservoirType(pumps, Hydroponics_ReservoirType_DrainageWater);

                    for (auto pumpIter = feedPumps.begin(); pumpIter != feedPumps.end(); ++pumpIter) {
                        auto pump = getHydroponicsInstance()->actuatorById(pumpIter->second->getId());
                        if (pump) { actuatorReqs.push_back(pump); }
                    }
                }
            }

            HYDRUINO_SOFT_ASSERT(actuatorReqs.size(), F("No linked feed water pumps"));

            {   auto aerators = linksFilterActuatorsByType(feedRes->getLinkages(), Hydroponics_ActuatorType_WaterAerator);

                for (auto aeratorIter = aerators.begin(); aeratorIter != aerators.end(); ++aeratorIter) {
                    auto aerator = getHydroponicsInstance()->actuatorById(aeratorIter->second->getId());
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
                    auto pump = getHydroponicsInstance()->actuatorById(pumpIter->second->getId());
                    if (pump) { actuatorReqs.push_back(pump); }
                }
            } else if (pumps.size()) { // Drainage peristaltic pumps
                drainagePumps = linksFilterActuatorsByType(pumps, Hydroponics_ActuatorType_PeristalticPump);

                for (auto pumpIter = drainagePumps.begin(); pumpIter != drainagePumps.end(); ++pumpIter) {
                    auto pump = getHydroponicsInstance()->actuatorById(pumpIter->second->getId());
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
                auto linkedCrops = feedRes->getCrops();
                for (auto cropIter = linkedCrops.begin(); cropIter != linkedCrops.end(); ++cropIter) {
                    auto crop = (HydroponicsCrop *)(cropIter->second);
                    if (crop->getNeedsFeeding()) { cropsHungry++; }
                }

                if (cropsHungry / (float)linkedCrops.size() >= HYDRUINO_SCHEDULER_FEED_FRACTION - FLT_EPSILON) {
                    stage = TopOff; stageStart = now();
                    setupStaging();
                }
            }
        } break;

        case TopOff: {
            if (feedRes->getIsFull()) {
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
            auto linkedCrops = feedRes->getCrops();
            for (auto cropIter = linkedCrops.begin(); cropIter != linkedCrops.end(); ++cropIter) {
                auto crop = (HydroponicsCrop *)(cropIter->second);
                if (!crop->getNeedsFeeding()) { cropsFed++; }
            }

            if (cropsFed / (float)linkedCrops.size() >= HYDRUINO_SCHEDULER_FEED_FRACTION - FLT_EPSILON) {
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
    auto linkedCrops = feedRes->getCrops();
    feedRes->notifyFeedingBegan();
    for (auto cropIter = linkedCrops.begin(); cropIter != linkedCrops.end(); ++cropIter) {
        auto crop = (HydroponicsCrop *)(cropIter->second);
        crop->notifyFeedingBegan();
    }
}

void HydroponicsFeeding::broadcastFeedingEnded()
{
    auto linkedCrops = feedRes->getCrops();
    feedRes->notifyFeedingEnded();
    for (auto cropIter = linkedCrops.begin(); cropIter != linkedCrops.end(); ++cropIter) {
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
    auto linkedCrops = feedRes->getCrops();
    bool sprayingNeeded = false;

    for (auto cropIter = linkedCrops.begin(); cropIter != linkedCrops.end(); ++cropIter) {
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
    if (time >= sprayStart) { stage = stage + 1; }
    if (time >= lightStart) { stage = stage + 1; }
    if (time >= lightEnd) { stage = stage + 1; }

    switch(stage) {
        case Spray:
            auto sprayers = linksFilterActuatorsByType(feedRes->getLinkages(), Hydroponics_ActuatorType_WaterSprayer);
            for (auto sprayerIter = sprayers.begin(); sprayerIter != sprayers.end(); ++sprayerIter) {
                auto sprayer = getHydroponicsInstance()->actuatorById(sprayerIter->second->getId());
                if (sprayer) { actuatorReqs.push_back(sprayer); }
            }
            break;
        case Light:
            auto lights = linksFilterActuatorsByType(feedRes->getLinkages(), Hydroponics_ActuatorType_GrowLights);
            for (auto lightIter = lights.begin(); lightIter != lights.end(); ++lightIter) {
                auto light = getHydroponicsInstance()->actuatorById(lightIter->second->getId());
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


HydroponicsScheduler::HydroponicsScheduler()
    : _schedulerData(nullptr), _inDaytimeMode(false), _needsRescheduling(false), _lastDayNum(-1)
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
    setNeedsRescheduling();
}

void HydroponicsScheduler::update()
{
    if (_schedulerData) {
        {   DateTime currTime = getCurrentTime();
            bool nowDaytimeMode = currTime.hour() >= HYDRUINO_CROP_NIGHT_END_HR && currTime.hour() < HYDRUINO_CROP_NIGHT_BEGIN_HR;

            if (_inDaytimeMode != nowDaytimeMode) {
                _inDaytimeMode = nowDaytimeMode;
                setNeedsRescheduling();
            }
            if (_lastDayNum != currTime.day()) {
                _lastDayNum = currTime.day();
                setNeedsRescheduling();
                broadcastDayChange();
            }
        }

        if (_needsRescheduling) {
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
    setNeedsRescheduling();
}

void HydroponicsScheduler::handleLowMemory()
{ ; }

void HydroponicsScheduler::setBaseFeedMultiplier(float baseFeedMultiplier)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    if (_schedulerData) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _schedulerData->baseFeedMultiplier = baseFeedMultiplier;
        setNeedsRescheduling();
    }
}

void HydroponicsScheduler::setWeeklyDosingRate(int weekIndex, float dosingRate, Hydroponics_ReservoirType reservoirType)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWEEKS_MAX), F("Invalid week index"));

    if (_schedulerData && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWEEKS_MAX) {
        if (reservoirType == Hydroponics_ReservoirType_NutrientPremix) {
            getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
            _schedulerData->weeklyDosingRates[weekIndex] = dosingRate;

            setNeedsRescheduling();
        } else if (reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 && reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
            HydroponicsCustomAdditiveData newAdditiveData(reservoirType);
            newAdditiveData._bumpRevIfNotAlreadyModded();
            newAdditiveData.weeklyDosingRates[weekIndex] = dosingRate;
            getHydroponicsInstance()->setCustomAdditiveData(&newAdditiveData);

            setNeedsRescheduling();
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

        setNeedsRescheduling();
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
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWEEKS_MAX), F("Invalid week index"));

    if (_schedulerData && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWEEKS_MAX) {
        _schedulerData->weeklyDosingRates[weekIndex] = 0;

        for (Hydroponics_ReservoirType reservoirType = Hydroponics_ReservoirType_CustomAdditive1;
             reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount;
             reservoirType = reservoirType + 1) {
            auto additiveData = getHydroponicsInstance()->getCustomAdditiveData(reservoirType);
            if (additiveData) {
                HydroponicsCustomAdditiveData newAdditiveData = *additiveData;
                newAdditiveData._bumpRevIfNotAlreadyModded();
                newAdditiveData.weeklyDosingRates[weekIndex] = 0;
                getHydroponicsInstance()->setCustomAdditiveData(&newAdditiveData);
            }
        }

        setNeedsRescheduling();
    }
}

void HydroponicsScheduler::setTotalFeedingsDay(unsigned int feedingsDay)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));

    if (_schedulerData) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _schedulerData->totalFeedingsDay = feedingsDay;

        setNeedsRescheduling();
    }
}

void HydroponicsScheduler::setPreFeedAeratorMins(unsigned int aeratorMins)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));

    if (_schedulerData) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _schedulerData->preFeedAeratorMins = aeratorMins;

        setNeedsRescheduling();
    }
}

void HydroponicsScheduler::setPreLightSprayMins(unsigned int sprayMins)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));

    if (_schedulerData) {
        getHydroponicsInstance()->_systemData->_bumpRevIfNotAlreadyModded();
        _schedulerData->preLightSprayMins = sprayMins;

        setNeedsRescheduling();
    }
}

void HydroponicsScheduler::setNeedsRescheduling()
{
    _needsRescheduling = (bool)_schedulerData;
}

float HydroponicsScheduler::getCombinedDosingRate(HydroponicsFeedReservoir *feedReservoir, Hydroponics_ReservoirType reservoirType)
{
    HYDRUINO_SOFT_ASSERT(_schedulerData, F("Scheduler data not yet initialized"));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || feedReservoir, F("Invalid feed reservoir"));
    HYDRUINO_SOFT_ASSERT(!_schedulerData || !feedReservoir || (reservoirType >= Hydroponics_ReservoirType_NutrientPremix &&
                                                               reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount), F("Invalid reservoir type"));

    if (_schedulerData && feedReservoir &&
        (reservoirType >= Hydroponics_ReservoirType_NutrientPremix &&
         reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount)) {
        auto crops = feedReservoir->getCrops();
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
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWEEKS_MAX), F("Invalid week index"));

    if (_schedulerData && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWEEKS_MAX) {
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
    HYDRUINO_SOFT_ASSERT(!_schedulerData || (weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWEEKS_MAX), F("Invalid week index"));

    if (_schedulerData && weekIndex >= 0 && weekIndex < HYDRUINO_CROP_GROWEEKS_MAX) {
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
            auto reservoir = reinterpret_pointer_cast<HydroponicsReservoir>(iter->second);
            if (reservoir && reservoir->isFeedClass()) {
                auto feedReservoir = reinterpret_pointer_cast<HydroponicsFeedReservoir>(iter->second);

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

    _needsRescheduling = false;
}

void HydroponicsScheduler::broadcastDayChange() const
{
    for (auto iter = getHydroponicsInstance()->_objects.begin(); iter != getHydroponicsInstance()->_objects.end(); ++iter) {
        if (iter->second) {
            if (iter->second->isReservoirType()) {
                auto reservoir = reinterpret_pointer_cast<HydroponicsReservoir>(iter->second);
                if (reservoir && reservoir->isFeedClass()) {
                    auto feedReservoir = reinterpret_pointer_cast<HydroponicsFeedReservoir>(iter->second);
                    if (feedReservoir) { feedReservoir->notifyDayChanged(); }
                }
            } else if (iter->second->isCropType()) {
                auto crop = reinterpret_pointer_cast<HydroponicsCrop>(iter->second);
                if (crop) { crop->notifyDayChanged(); }
            }
        }
    }
}


HydroponicsSchedulerSubData::HydroponicsSchedulerSubData()
    : HydroponicsSubData(), baseFeedMultiplier(1), weeklyDosingRates{1}, standardDosingRates{1}, totalFeedingsDay(0), preFeedAeratorMins(30), preLightSprayMins(60)
{ ; }

void HydroponicsSchedulerSubData::toJSONObject(JsonObject &objectOut) const
{
    // purposeful no call to base method (ignores type)

    if (!isFPEqual(baseFeedMultiplier, 1.0f)) { objectOut[F("baseFeedMultiplier")] = baseFeedMultiplier; }
    bool hasWeeklyDosings = arrayEqualsAll(weeklyDosingRates, HYDRUINO_CROP_GROWEEKS_MAX, 1.0f);
    if (hasWeeklyDosings) { objectOut[F("weeklyDosingRates")] = commaStringFromArray(weeklyDosingRates, HYDRUINO_CROP_GROWEEKS_MAX); }
    bool hasStandardDosings = arrayEqualsAll(standardDosingRates, 3, 1.0f);
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
    commaStringToArray(weeklyDosingRatesVar, weeklyDosingRates, HYDRUINO_CROP_GROWEEKS_MAX);
    for (int weekIndex = 0; weekIndex < HYDRUINO_CROP_GROWEEKS_MAX; ++weekIndex) { 
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
