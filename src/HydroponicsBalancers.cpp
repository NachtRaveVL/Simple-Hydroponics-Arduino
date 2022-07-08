/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Balancers
*/

#include "Hydroponics.h"

HydroponicsBalancer::HydroponicsBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, int measurementRow, int typeIn)
    : type((typeof(type))typeIn), _rangeTrigger(nullptr), _targetSetpoint(targetSetpoint), _targetRange(targetRange), _enabled(false),
      _targetUnits(Hydroponics_UnitsType_Undefined), _balanceState(Hydroponics_BalancingState_Undefined)
{
    float halfTargetRange = targetRange * 0.5f;
    _rangeTrigger = new HydroponicsMeasurementRangeTrigger(sensor, targetSetpoint - halfTargetRange, targetSetpoint + halfTargetRange, true, halfTargetRange, measurementRow);
    HYDRUINO_SOFT_ASSERT(_rangeTrigger, F("Failure allocating range trigger"));
    if (_rangeTrigger) { attachRangeTrigger(); }
}

HydroponicsBalancer::~HydroponicsBalancer()
{
    _enabled = false;
    if (_rangeTrigger) { detachRangeTrigger(); delete _rangeTrigger; _rangeTrigger = nullptr; }
    disableIncActuators();
    disableDecActuators();
}

void HydroponicsBalancer::setTargetSetpoint(float targetSetpoint)
{
    if (!isFPEqual(_targetSetpoint, targetSetpoint)) {
        _targetSetpoint = targetSetpoint;
        if (_rangeTrigger) { _rangeTrigger->setTriggerToleranceMid(_targetSetpoint); }
    }
}

Hydroponics_BalancingState HydroponicsBalancer::getBalanceState() const
{
    return _balanceState;
}

void HydroponicsBalancer::update()
{
    if (_rangeTrigger) { _rangeTrigger->update(); }
}

void HydroponicsBalancer::resolveLinks()
{
    if (_rangeTrigger) { _rangeTrigger->resolveLinks(); }
}

void HydroponicsBalancer::handleLowMemory()
{
    if (_rangeTrigger) { _rangeTrigger->handleLowMemory(); }
}

void HydroponicsBalancer::setTargetUnits(Hydroponics_UnitsType units)
{
    _targetUnits = units;
}

Hydroponics_UnitsType HydroponicsBalancer::getTargetUnits() const
{
    return _targetUnits;
}

void HydroponicsBalancer::setIncrementActuators(const arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTOBJECTS_MAXSIZE> &incActuators)
{
    for (auto actIter = _incActuators.begin(); actIter != _incActuators.end(); ++actIter) {
        if (incActuators.find(actIter->first) == incActuators.end()) {
            auto actuator = actIter->second.first;
            if (actuator && actuator->getIsEnabled()) { actuator->disableActuator(); }
        }
    }

    _incActuators = incActuators;
}

void HydroponicsBalancer::setDecrementActuators(const arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTOBJECTS_MAXSIZE> &decActuators)
{
    for (auto actIter = _decActuators.begin(); actIter != _decActuators.end(); ++actIter) {
        if (decActuators.find(actIter->first) == decActuators.end()) {
            auto actuator = actIter->second.first;
            if (actuator && actuator->getIsEnabled()) { actuator->disableActuator(); }
        }
    }

    _decActuators = decActuators;
}

const arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTOBJECTS_MAXSIZE> &HydroponicsBalancer::getIncrementActuators()
{
    return _incActuators;
}

const arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTOBJECTS_MAXSIZE> &HydroponicsBalancer::getDecrementActuators()
{
    return _decActuators;
}

void HydroponicsBalancer::setEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;

        if (_enabled) {
            handleRangeTrigger(_rangeTrigger ? _rangeTrigger->getTriggerState() : Hydroponics_TriggerState_Undefined);
        }
    }
}

bool HydroponicsBalancer::getIsEnabled() const
{
    return _enabled;
}

float HydroponicsBalancer::getTargetSetpoint() const
{
    return _targetSetpoint;
}

float HydroponicsBalancer::getTargetRange() const
{
    return _targetRange;
}

void HydroponicsBalancer::disableIncActuators()
{
    for (auto actIter = _incActuators.begin(); actIter != _incActuators.end(); ++actIter) {
        auto actuator = actIter->second.first;
        if (actuator && actuator->getIsEnabled()) { actuator->disableActuator(); }
    }
}

void HydroponicsBalancer::disableDecActuators()
{
    for (auto actIter = _decActuators.begin(); actIter != _decActuators.end(); ++actIter) {
        auto actuator = actIter->second.first;
        if (actuator && actuator->getIsEnabled()) { actuator->disableActuator(); }
    }
}

void HydroponicsBalancer::attachRangeTrigger()
{
    HYDRUINO_SOFT_ASSERT(_rangeTrigger, F("Range trigger not linked, failure attaching"));
    if (_rangeTrigger) {
        auto methodSlot = MethodSlot<HydroponicsBalancer, Hydroponics_TriggerState>(this, &handleRangeTrigger);
        _rangeTrigger->getTriggerSignal().attach(methodSlot);
    }
}

void HydroponicsBalancer::detachRangeTrigger()
{
    HYDRUINO_SOFT_ASSERT(_rangeTrigger, F("Range trigger not linked, failure detaching"));
    if (_rangeTrigger) {
        auto methodSlot = MethodSlot<HydroponicsBalancer, Hydroponics_TriggerState>(this, &handleRangeTrigger);
        _rangeTrigger->getTriggerSignal().detach(methodSlot);
    }
}

void HydroponicsBalancer::handleRangeTrigger(Hydroponics_TriggerState triggerState)
{
    if (!_enabled || !_rangeTrigger) { return; }

    auto sensor = _rangeTrigger->getSensor();
    auto measurementValue = measurementValueAt(sensor->getLatestMeasurement(), _rangeTrigger->getMeasurementRow());
    auto measurementUnits = measurementUnitsAt(sensor->getLatestMeasurement(), _rangeTrigger->getMeasurementRow());

    if (_targetUnits != Hydroponics_UnitsType_Undefined && measurementUnits != _targetUnits) {
        convertStdUnits(&measurementValue, &measurementUnits, _targetUnits);
        HYDRUINO_SOFT_ASSERT(measurementUnits == _targetUnits, F("Failure converting measurement value to target units"));
    }

    float halfTargetRange = _targetRange * 0.5f;
    if (measurementValue > _targetSetpoint - halfTargetRange + FLT_EPSILON &&
        measurementValue < _targetSetpoint + halfTargetRange - FLT_EPSILON) {
        _balanceState = Hydroponics_BalancingState_Balanced;
        disableIncActuators();
        disableDecActuators();
    } else {
        _balanceState = measurementValue > _targetSetpoint ? Hydroponics_BalancingState_TooHigh : Hydroponics_BalancingState_TooLow;
    }

    update();
}


HydroponicsLinearEdgeBalancer::HydroponicsLinearEdgeBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, float edgeOffset, float edgeLength, int measurementRow)
    : HydroponicsBalancer(sensor, targetSetpoint, targetRange, measurementRow, LinearEdge), _edgeOffset(edgeOffset), _edgeLength(edgeLength)
{ ; }

HydroponicsLinearEdgeBalancer::~HydroponicsLinearEdgeBalancer()
{ ; }

void HydroponicsLinearEdgeBalancer::update()
{
    HydroponicsBalancer::update();

    // TODO
}

float HydroponicsLinearEdgeBalancer::getEdgeOffset() const
{
    return _edgeOffset;
}

float HydroponicsLinearEdgeBalancer::getEdgeLength() const
{
    return _edgeLength;
}


HydroponicsTimedDosingBalancer::HydroponicsTimedDosingBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, time_t baseDosingMillis, unsigned int mixTimeMins, int measurementRow)
    : HydroponicsBalancer(sensor, targetSetpoint, targetRange, measurementRow, TimedDosing), _baseDosingMillis(baseDosingMillis), _mixTimeMins(mixTimeMins)
{ ; }

HydroponicsTimedDosingBalancer::HydroponicsTimedDosingBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, float reservoirVolume, Hydroponics_UnitsType volumeUnits, int measurementRow)
    : HydroponicsBalancer(sensor, targetSetpoint, targetRange, measurementRow, TimedDosing)
{
    if (volumeUnits != Hydroponics_UnitsType_LiquidVolume_Gallons) {
        convertStdUnits(&reservoirVolume, &volumeUnits, Hydroponics_UnitsType_LiquidVolume_Gallons);
    }
    _baseDosingMillis = mapValue<float>(reservoirVolume, 5, 30, 500, 3000);
    _mixTimeMins = mapValue<float>(reservoirVolume, 30, 200, 10, 30);
}

HydroponicsTimedDosingBalancer::~HydroponicsTimedDosingBalancer()
{ ; }

void HydroponicsTimedDosingBalancer::update()
{
    HydroponicsBalancer::update();

    // TODO
}

void HydroponicsTimedDosingBalancer::setDosingDrift(float dosingDrift)
{
    _dosingDrift = dosingDrift;
}

time_t HydroponicsTimedDosingBalancer::getBaseDosingMillis() const
{
    return _baseDosingMillis;
}

unsigned int HydroponicsTimedDosingBalancer::getMixTimeMins() const
{
    return _mixTimeMins;
}

float HydroponicsTimedDosingBalancer::getDosingDrift() const
{
    return _dosingDrift;
}
