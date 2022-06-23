/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Power Rails 
*/

#include "Hydroponics.h"

HydroponicsRail::HydroponicsRail(Hydroponics_RailType railType, Hydroponics_PositionIndex railIndex, int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(railType, railIndex)), classType((typeof(classType))classTypeIn)
{ ; }

HydroponicsRail::~HydroponicsRail()
{
    //discardFromTaskManager(&_capacitySignal);
    for (auto pairObj : getActuators()) { removeActuator(pairObj.second); }
}

bool HydroponicsRail::addActuator(HydroponicsActuator *actuator)
{
    return addLinkage(actuator);
}

bool HydroponicsRail::removeActuator(HydroponicsActuator *actuator)
{
    return removeLinkage(actuator);
}

arx::map<Hydroponics_KeyType, HydroponicsActuator *> HydroponicsRail::getActuators() const
{
    arx::map<Hydroponics_KeyType, HydroponicsActuator *> retVal;
    for (auto pairObj : _links) {
        auto obj = pairObj.second;
        if (obj && obj->isActuatorType()) {
            retVal.insert(pairObj.first, (HydroponicsActuator *)obj);
        }
    }
    return retVal;
}

Hydroponics_RailType HydroponicsRail::getRailType() const
{
    return _id.as.railType;
}

Hydroponics_PositionIndex HydroponicsRail::getRailIndex() const
{
    return _id.posIndex;
}

Signal<HydroponicsRail *> &HydroponicsRail::getCapacitySignal()
{
    return _capacitySignal;
}


HydroponicsSimpleRail::HydroponicsSimpleRail(Hydroponics_RailType railType,
                                             Hydroponics_PositionIndex railIndex,
                                             int maxActiveAtOnce,
                                             int classType)
    : HydroponicsRail(railType, railIndex, classType), _activeCount(0), _maxActiveCount(maxActiveAtOnce)
{ ; }

HydroponicsSimpleRail::~HydroponicsSimpleRail()
{ ; }

bool HydroponicsSimpleRail::canActivate(shared_ptr<HydroponicsActuator> actuator)
{
    return hasActuator(actuator.get()) ? _activeCount < _maxActiveCount
                                       : false;
}

bool HydroponicsSimpleRail::addActuator(HydroponicsActuator *actuator)
{
    if (HydroponicsRail::addActuator(actuator)) {
        auto methodSlot = MethodSlot<HydroponicsSimpleRail, HydroponicsActuator *>(this, &handleActivation);
        actuator->getActivationSignal().attach(methodSlot);
        return true;
    }
    return false;
}

bool HydroponicsSimpleRail::removeActuator(HydroponicsActuator *actuator)
{
    if (HydroponicsRail::removeActuator(actuator)) {
        auto methodSlot = MethodSlot<HydroponicsSimpleRail, HydroponicsActuator *>(this, &handleActivation);
        actuator->getActivationSignal().detach(methodSlot);
        return true;
    }
    return false;
}

int HydroponicsSimpleRail::getActiveCount()
{
    return _activeCount;
}

void HydroponicsSimpleRail::handleActivation(HydroponicsActuator *actuator)
{
    bool hadSpaceBefore = _activeCount < _maxActiveCount;

    if (actuator->getIsActuatorEnabled()) {
        _activeCount += 1;
    } else {
        _activeCount -= 1;
    }

    bool hasSpaceAfter = _activeCount < _maxActiveCount;

    if (hadSpaceBefore != hasSpaceAfter) {
        scheduleSignalFireOnce<HydroponicsRail *>(_capacitySignal, this);
    }
}
