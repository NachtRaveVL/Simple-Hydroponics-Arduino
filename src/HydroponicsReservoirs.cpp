/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Reservoirs
*/

#include "Hydroponics.h"

HydroponicsReservoir::HydroponicsReservoir(Hydroponics_ReservoirType reservoirType, Hydroponics_PositionIndex reservoirIndex, int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(reservoirType, reservoirIndex)), classType((typeof(classType))classTypeIn)
{ ; }

HydroponicsReservoir::~HydroponicsReservoir()
{
    {   auto actuators = getActuators();
        for (auto iter = actuators.begin(); iter != actuators.end(); ++iter) { removeActuator(iter->second); }
    }
    {   auto sensors = getSensors();
        for (auto iter = sensors.begin(); iter != sensors.end(); ++iter) { removeSensor(iter->second); }
    }
    {   auto crops = getCrops();
        for (auto iter = crops.begin(); iter != crops.end(); ++iter) { removeCrop(iter->second); }
    }
}

bool HydroponicsReservoir::addActuator(HydroponicsActuator *actuator)
{
    return addLinkage(actuator);
}

bool HydroponicsReservoir::removeActuator(HydroponicsActuator *actuator)
{
    return removeLinkage(actuator);
}

arx::map<Hydroponics_KeyType, HydroponicsActuator *> HydroponicsReservoir::getActuators() const
{
    arx::map<Hydroponics_KeyType, HydroponicsActuator *> retVal;
    for (auto iter = _links.begin(); iter != _links.end(); ++iter) {
        auto obj = iter->second;
        if (obj && obj->isActuatorType()) {
            retVal.insert(iter->first, (HydroponicsActuator *)obj);
        }
    }
    return retVal;
}

bool HydroponicsReservoir::addSensor(HydroponicsSensor *sensor)
{
    return addLinkage(sensor);
}

bool HydroponicsReservoir::removeSensor(HydroponicsSensor *sensor)
{
    return removeLinkage(sensor);
}

arx::map<Hydroponics_KeyType, HydroponicsSensor *> HydroponicsReservoir::getSensors() const
{
    arx::map<Hydroponics_KeyType, HydroponicsSensor *> retVal;
    for (auto iter = _links.begin(); iter != _links.end(); ++iter) {
        auto obj = iter->second;
        if (obj && obj->isSensorType()) {
            retVal.insert(iter->first, (HydroponicsSensor *)obj);
        }
    }
    return retVal;
}

bool HydroponicsReservoir::addCrop(HydroponicsCrop *crop)
{
    return addLinkage(crop);
}

bool HydroponicsReservoir::removeCrop(HydroponicsCrop *crop)
{
    return removeLinkage(crop);
}

arx::map<Hydroponics_KeyType, HydroponicsCrop *> HydroponicsReservoir::getCrops() const
{
    arx::map<Hydroponics_KeyType, HydroponicsCrop *> retVal;
    for (auto iter = _links.begin(); iter != _links.end(); ++iter) {
        auto obj = iter->second;
        if (obj && obj->isCropType()) {
            retVal.insert(iter->first, (HydroponicsCrop *)obj);
        }
    }
    return retVal;
}

Hydroponics_ReservoirType HydroponicsReservoir::getReservoirType() const
{
    return _id.as.reservoirType;
}

Hydroponics_PositionIndex HydroponicsReservoir::getReservoirIndex() const
{
    return _id.posIndex;
}


HydroponicsFluidReservoir::HydroponicsFluidReservoir(Hydroponics_ReservoirType reservoirType,
                                                     Hydroponics_PositionIndex reservoirIndex,
                                                     float maxVolume,
                                                     int channel,
                                                     int classType)
    : HydroponicsReservoir(reservoirType, reservoirIndex, classType), _channel(channel),
      _currVolume(0.0f), _maxVolume(maxVolume), _volumeUnits(defaultLiquidVolumeUnits())
{ ; }

HydroponicsFluidReservoir::~HydroponicsFluidReservoir()
{
    if (_filledTrigger) { delete _filledTrigger; _filledTrigger = nullptr; }
    if (_emptyTrigger) { delete _emptyTrigger; _emptyTrigger = nullptr; }
}

bool HydroponicsFluidReservoir::canActivate(shared_ptr<HydroponicsActuator> actuator)
{
    bool doEmptyCheck;

    switch (actuator->getActuatorType()) {
        case Hydroponics_ActuatorType_WaterPump:
        case Hydroponics_ActuatorType_PeristalticPump: {
            doEmptyCheck = (actuator->getReservoir().get() == this);
        } break;

        case Hydroponics_ActuatorType_WaterAerator:
        case Hydroponics_ActuatorType_WaterHeater: {
            doEmptyCheck = true;
        } break;

        default:
            return true;
    }

    if (doEmptyCheck) {
        auto emptyState = getEmptyState();
        HYDRUINO_SOFT_ASSERT(emptyState != Hydroponics_TriggerState_Disabled, "Cannot drain reservoir without empty state tracking");
        return (emptyState == Hydroponics_TriggerState_NotTriggered);
    } else {
        auto filledState = getFilledState();
        HYDRUINO_SOFT_ASSERT(filledState != Hydroponics_TriggerState_Disabled, "Cannot fill reservoir without fill state tracking");
        return (filledState == Hydroponics_TriggerState_NotTriggered);
    }
}

void HydroponicsFluidReservoir::setVolumeUnits(Hydroponics_UnitsType volumeUnits)
{
    _volumeUnits = volumeUnits;
}

Hydroponics_UnitsType HydroponicsFluidReservoir::getVolumeUnits() const
{
    return _volumeUnits;
}

void HydroponicsFluidReservoir::update()
{
    HydroponicsReservoir::update();
    if (_filledTrigger) { _filledTrigger->update(); }
    if (_emptyTrigger) { _emptyTrigger->update(); }
}

void HydroponicsFluidReservoir::resolveLinks()
{
    HydroponicsReservoir::resolveLinks();
    if (_filledTrigger) { _filledTrigger->attach(); }
    if (_emptyTrigger) { _emptyTrigger->attach(); }
}

Hydroponics_TriggerState HydroponicsFluidReservoir::getFilledState() const
{
    return _filledTrigger ? _filledTrigger->getTriggerState() : Hydroponics_TriggerState_Disabled;
}

Hydroponics_TriggerState HydroponicsFluidReservoir::getEmptyState() const
{
    return _emptyTrigger ? _emptyTrigger->getTriggerState() : Hydroponics_TriggerState_Disabled;
}

Signal<Hydroponics_TriggerState> *HydroponicsFluidReservoir::getFilledSignal()
{
    return _filledTrigger ? &(_filledTrigger->getTriggerSignal()) : nullptr;
}

Signal<Hydroponics_TriggerState> *HydroponicsFluidReservoir::getEmptySignal()
{
    return _emptyTrigger ? &(_emptyTrigger->getTriggerSignal()) : nullptr;
}

void HydroponicsFluidReservoir::setFilledTrigger(HydroponicsTrigger *filledTrigger)
{
    if (_filledTrigger != filledTrigger) {
        if (_filledTrigger) { delete _filledTrigger; }
        _filledTrigger = filledTrigger;
        if (_filledTrigger) { _filledTrigger->attach(); }
    }
}

const HydroponicsTrigger *HydroponicsFluidReservoir::getFilledTrigger() const
{
    return _filledTrigger;
}

void HydroponicsFluidReservoir::setEmptyTrigger(HydroponicsTrigger *emptyTrigger)
{
    if (_emptyTrigger != emptyTrigger) {
        if (_emptyTrigger) { delete _emptyTrigger; }
        _emptyTrigger = emptyTrigger;
        if (_emptyTrigger) { _emptyTrigger->attach(); }
    }
}

const HydroponicsTrigger *HydroponicsFluidReservoir::getEmptyTrigger() const
{
    return _emptyTrigger;
}

void HydroponicsFluidReservoir::setChannelNumber(int channel)
{
    _channel = channel;
}

int HydroponicsFluidReservoir::getChannelNumber() const
{
    return _channel;
}

arx::map<Hydroponics_KeyType, HydroponicsPumpObjectInterface *> HydroponicsFluidReservoir::getInputPumpActuators() const
{
    arx::map<Hydroponics_KeyType, HydroponicsPumpObjectInterface *> retVal;
    for (auto iter = _links.begin(); iter != _links.end(); ++iter) {
        auto obj = iter->second;
        if (obj && obj->isActuatorType() && ((HydroponicsActuator *)obj)->isAnyPumpClass() &&
            ((HydroponicsActuator *)obj)->getReservoir().get() == this) {
            retVal.insert(iter->first, (HydroponicsPumpObjectInterface *)obj);
        }
    }
    return retVal;
}

arx::map<Hydroponics_KeyType, HydroponicsPumpObjectInterface *> HydroponicsFluidReservoir::getOutputPumpActuators() const
{
    arx::map<Hydroponics_KeyType, HydroponicsPumpObjectInterface *> retVal;
    for (auto iter = _links.begin(); iter != _links.end(); ++iter) {
        auto obj = iter->second;
        if (obj && obj->isActuatorType() && ((HydroponicsActuator *)obj)->isAnyPumpClass() &&
            ((HydroponicsPumpObjectInterface *)obj)->getOutputReservoir().get() == this) {
            retVal.insert(iter->first, (HydroponicsPumpObjectInterface *)obj);
        }
    }
    return retVal;
}


HydroponicsInfiniteReservoir::HydroponicsInfiniteReservoir(Hydroponics_ReservoirType reservoirType,
                                                           Hydroponics_PositionIndex reservoirIndex,
                                                           bool alwaysFilled,
                                                           int classType)
    : HydroponicsReservoir(reservoirType, reservoirIndex, classType), _alwaysFilled(alwaysFilled)
{ ; }

HydroponicsInfiniteReservoir::~HydroponicsInfiniteReservoir()
{ ; }

Hydroponics_TriggerState HydroponicsInfiniteReservoir::getFilledState() const
{
    return _alwaysFilled ? Hydroponics_TriggerState_Triggered : Hydroponics_TriggerState_NotTriggered;
}

Hydroponics_TriggerState HydroponicsInfiniteReservoir::getEmptyState() const
{
    return !_alwaysFilled ? Hydroponics_TriggerState_Triggered : Hydroponics_TriggerState_NotTriggered;
}

Signal<Hydroponics_TriggerState> *HydroponicsInfiniteReservoir::getFilledSignal()
{
    return nullptr;
}

Signal<Hydroponics_TriggerState> *HydroponicsInfiniteReservoir::getEmptySignal()
{
    return nullptr;
}

bool HydroponicsInfiniteReservoir::canActivate(shared_ptr<HydroponicsActuator> actuator)
{
    return true;
}
