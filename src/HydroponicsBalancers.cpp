/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Balancers
*/

#include "Hydroponics.h"

HydroponicsBalancer::HydroponicsBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, int measurementRow, int typeIn)
    : type((typeof(type))typeIn), _rangeTrigger(nullptr), _targetSetpoint(targetSetpoint), _targetRange(targetRange), _enabled(false), _needsTriggerUpdate(true),
      _targetUnits(Hydroponics_UnitsType_Undefined), _balancerState(Hydroponics_BalancerState_Undefined)
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
    //discardFromTaskManager(&_balancerSignal);
}

void HydroponicsBalancer::setTargetSetpoint(float targetSetpoint)
{
    if (!isFPEqual(_targetSetpoint, targetSetpoint)) {
        _targetSetpoint = targetSetpoint;
        if (_rangeTrigger) { _rangeTrigger->setTriggerToleranceMid(_targetSetpoint); }
    }
}

Hydroponics_BalancerState HydroponicsBalancer::getBalancerState() const
{
    return _balancerState;
}

void HydroponicsBalancer::update()
{
    if (_rangeTrigger) { _rangeTrigger->update(); }
    if (!_enabled || !_rangeTrigger) { return; }

    if (_needsTriggerUpdate && _rangeTrigger) {
        handleRangeTrigger(_rangeTrigger->getTriggerState());
    }    
}

void HydroponicsBalancer::resolveLinks()
{
    if (_rangeTrigger) { _rangeTrigger->resolveLinks(); }
}

void HydroponicsBalancer::handleLowMemory()
{
    if (_rangeTrigger) { _rangeTrigger->handleLowMemory(); }
}

void HydroponicsBalancer::setTargetUnits(Hydroponics_UnitsType targetUnits)
{
    if (_targetUnits != targetUnits) {
        _targetUnits = targetUnits;
    }
}

Hydroponics_UnitsType HydroponicsBalancer::getTargetUnits() const
{
    return _targetUnits;
}

void HydroponicsBalancer::setIncrementActuators(const arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> &incActuators)
{
    for (auto actIter = _incActuators.begin(); actIter != _incActuators.end(); ++actIter) {
        if (incActuators.find(actIter->first) == incActuators.end()) {
            auto actuator = actIter->second.first;
            if (actuator && actuator->getIsEnabled()) { actuator->disableActuator(); }
        }
    }

    _incActuators = incActuators;
}

void HydroponicsBalancer::setDecrementActuators(const arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> &decActuators)
{
    for (auto actIter = _decActuators.begin(); actIter != _decActuators.end(); ++actIter) {
        if (decActuators.find(actIter->first) == decActuators.end()) {
            auto actuator = actIter->second.first;
            if (actuator && actuator->getIsEnabled()) { actuator->disableActuator(); }
        }
    }

    _decActuators = decActuators;
}

const arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> &HydroponicsBalancer::getIncrementActuators()
{
    return _incActuators;
}

const arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> &HydroponicsBalancer::getDecrementActuators()
{
    return _decActuators;
}

void HydroponicsBalancer::setEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;

        _needsTriggerUpdate = true;
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
        if (actuator) { actuator->disableActuator(); }
    }
}

void HydroponicsBalancer::disableDecActuators()
{
    for (auto actIter = _decActuators.begin(); actIter != _decActuators.end(); ++actIter) {
        auto actuator = actIter->second.first;
        if (actuator) { actuator->disableActuator(); }
    }
}

void HydroponicsBalancer::handleBalancerState()
{ ; }

void HydroponicsBalancer::attachRangeTrigger()
{
    HYDRUINO_SOFT_ASSERT(_rangeTrigger, F("Range trigger not linked, failure attaching"));
    if (_rangeTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &handleRangeTrigger);
        _rangeTrigger->getTriggerSignal().attach(methodSlot);
    }
}

void HydroponicsBalancer::detachRangeTrigger()
{
    HYDRUINO_SOFT_ASSERT(_rangeTrigger, F("Range trigger not linked, failure detaching"));
    if (_rangeTrigger) {
        auto methodSlot = MethodSlot<typeof(*this), Hydroponics_TriggerState>(this, &handleRangeTrigger);
        _rangeTrigger->getTriggerSignal().detach(methodSlot);
    }
}

void HydroponicsBalancer::handleRangeTrigger(Hydroponics_TriggerState triggerState)
{
    if (!_enabled || !_rangeTrigger) { return; }

    auto sensor = _rangeTrigger->getSensor();
    if (sensor) {
        _needsTriggerUpdate = false;
        auto balancerStateBefore = _balancerState;
        auto measurementValue = measurementValueAt(sensor->getLatestMeasurement(), _rangeTrigger->getMeasurementRow());
        auto measurementUnits = measurementUnitsAt(sensor->getLatestMeasurement(), _rangeTrigger->getMeasurementRow());

        convertStdUnits(&measurementValue, &measurementUnits, _targetUnits);

        float halfTargetRange = _targetRange * 0.5f;
        if (measurementValue > _targetSetpoint - halfTargetRange + FLT_EPSILON &&
            measurementValue < _targetSetpoint + halfTargetRange - FLT_EPSILON) {
            _balancerState = Hydroponics_BalancerState_Balanced;
        } else {
            _balancerState = measurementValue > _targetSetpoint ? Hydroponics_BalancerState_TooHigh : Hydroponics_BalancerState_TooLow;
        }

        if (_balancerState != balancerStateBefore) {
            handleBalancerState();

            scheduleSignalFireOnce<Hydroponics_BalancerState>(_balancerSignal, _balancerState);
        }
    }
}


HydroponicsLinearEdgeBalancer::HydroponicsLinearEdgeBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, float edgeOffset, float edgeLength, int measurementRow)
    : HydroponicsBalancer(sensor, targetSetpoint, targetRange, measurementRow, LinearEdge), _edgeOffset(edgeOffset), _edgeLength(edgeLength)
{ ; }

HydroponicsLinearEdgeBalancer::~HydroponicsLinearEdgeBalancer()
{ ; }

void HydroponicsLinearEdgeBalancer::update()
{
    HydroponicsBalancer::update();
    if (!_enabled || !_rangeTrigger) { return; }

    if (_balancerState != Hydroponics_BalancerState_Balanced && _balancerState != Hydroponics_BalancerState_Undefined) {
        auto sensor = _rangeTrigger->getSensor();
        if (sensor) {
            auto measure = singleMeasurementAt(sensor->getLatestMeasurement(), _rangeTrigger->getMeasurementRow());
            convertStdUnits(&measure.value, &measure.units, _targetUnits);

            float x = fabsf(measure.value - _targetSetpoint);
            float val = _edgeLength > FLT_EPSILON ? mapValue<float>(x, _edgeOffset, _edgeOffset + _edgeLength, 0.0f, 1.0f)
                                                  : (x >= _edgeOffset - FLT_EPSILON ? 1.0 : 0.0f);
            val = constrain(val, 0.0f, 1.0f);

            typeof(_incActuators) &actuatorsDir = _balancerState == Hydroponics_BalancerState_TooLow ? _incActuators : _decActuators;
            for (auto iter = actuatorsDir.begin(); iter != actuatorsDir.end(); ++iter) {
                auto actuator = iter->second.first;
                if (actuator) {
                    actuator->enableActuator(val * iter->second.second);
                }
            }
        }
    }
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
    : HydroponicsBalancer(sensor, targetSetpoint, targetRange, measurementRow, TimedDosing), _baseDosingMillis(baseDosingMillis), _mixTimeMins(mixTimeMins),
      _lastDosingTime(0), _dosingMillis(0), _dosingDir(Hydroponics_BalancerState_Undefined), _dosingActIndex(-1)
{ ; }

HydroponicsTimedDosingBalancer::HydroponicsTimedDosingBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, float reservoirVolume, Hydroponics_UnitsType volumeUnits, int measurementRow)
    : HydroponicsBalancer(sensor, targetSetpoint, targetRange, measurementRow, TimedDosing),
      _lastDosingTime(0), _dosingMillis(0), _dosingDir(Hydroponics_BalancerState_Undefined), _dosingActIndex(-1)
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
    if (!_enabled || !_rangeTrigger) { return; }

    if (_balancerState != Hydroponics_BalancerState_Balanced && _balancerState != Hydroponics_BalancerState_Undefined &&
        (!_lastDosingTime || now() > _lastDosingTime + (_mixTimeMins * SECS_PER_MIN))) {
        performDosing();
    }

    if (_dosingActIndex >= 0) {
        typeof(_incActuators) &actuatorsDir = _dosingDir == Hydroponics_BalancerState_TooLow ? _incActuators : _decActuators;

        while (_dosingActIndex < actuatorsDir.size()) {
            auto iter = actuatorsDir.begin();
            for (int actIndex = 0; iter != actuatorsDir.end() && actIndex != _dosingActIndex; ++iter, ++actIndex) { ; }
            if (iter != actuatorsDir.end() && scheduleActuatorTimedEnableOnce(iter->second.first, 1.0f, iter->second.second * _dosingMillis) != TASKMGR_INVALIDID) {
                _dosingActIndex++;
                continue;
            } else {
                break;
            }
        }

        if (_dosingActIndex >= actuatorsDir.size()) {
            _dosingActIndex = -1; // Dosing completed
        }
    }
}

time_t HydroponicsTimedDosingBalancer::getBaseDosingMillis() const
{
    return _baseDosingMillis;
}

unsigned int HydroponicsTimedDosingBalancer::getMixTimeMins() const
{
    return _mixTimeMins;
}

void HydroponicsTimedDosingBalancer::performDosing()
{
    auto sensor = _rangeTrigger ? _rangeTrigger->getSensor() : nullptr;
    if (sensor) {
        if (_dosingDir != _balancerState) {
            _dosingMillis = 0;
            _dosingDir = _balancerState;
        }

        float dosingMillis = _baseDosingMillis;
        auto dosingValue = measurementValueAt(sensor->getLatestMeasurement(), _rangeTrigger->getMeasurementRow());
        if (_dosingMillis) {
            auto dosingRatePerMs = (dosingValue - _lastDosingValue) / _dosingMillis;
            dosingMillis = (_targetSetpoint - dosingValue) * dosingRatePerMs;
            dosingMillis = constrain(dosingMillis, _baseDosingMillis * HYDRUINO_DOSETIME_MIN_FRACTION,
                                                   _baseDosingMillis * HYDRUINO_DOSETIME_MAX_FRACTION);
        }
        _lastDosingValue = dosingValue;
        _dosingMillis = dosingMillis;
        _dosingActIndex = 0;

        _lastDosingTime = now();
    }
}
