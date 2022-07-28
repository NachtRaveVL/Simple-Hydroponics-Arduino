/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Balancers
*/

#include "Hydroponics.h"

HydroponicsBalancer::HydroponicsBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, byte measurementRow, int typeIn)
    : type((typeof(type))typeIn), _targetSetpoint(targetSetpoint), _targetRange(targetRange), _enabled(false),
      _targetUnits(Hydroponics_UnitsType_Undefined), _balancerState(Hydroponics_BalancerState_Undefined),
      _rangeTrigger((HydroponicsObject *)this)
{
    float halfTargetRange = targetRange * 0.5f;

    _rangeTrigger = make_shared<HydroponicsMeasurementRangeTrigger>(sensor, targetSetpoint - halfTargetRange, targetSetpoint + halfTargetRange, true, halfTargetRange, measurementRow);
    HYDRUINO_HARD_ASSERT(_rangeTrigger, SFP(HS_Err_AllocationFailure));
    _rangeTrigger.setUpdateMethod(&HydroponicsBalancer::handleTrigger);
}

HydroponicsBalancer::~HydroponicsBalancer()
{
    //discardFromTaskManager(&_balancerSignal);
    _enabled = false;
    disableAllActuators();
}

void HydroponicsBalancer::setTargetSetpoint(float targetSetpoint)
{
    if (!isFPEqual(_targetSetpoint, targetSetpoint)) {
        _targetSetpoint = targetSetpoint;

        ((HydroponicsMeasurementRangeTrigger *)_rangeTrigger.get())->updateTriggerMidpoint(_targetSetpoint);
    }
}

Hydroponics_BalancerState HydroponicsBalancer::getBalancerState() const
{
    return _balancerState;
}

void HydroponicsBalancer::update()
{
    if (!_enabled) { return; }

    if (_rangeTrigger.getObject()) { _rangeTrigger->update(); }

    _rangeTrigger.updateTriggerIfNeeded();
}

void HydroponicsBalancer::handleLowMemory()
{
    if (_rangeTrigger) { _rangeTrigger->handleLowMemory(); }
}

void HydroponicsBalancer::setTargetUnits(Hydroponics_UnitsType targetUnits)
{
    if (_targetUnits != targetUnits) {
        _targetUnits = targetUnits;

        _rangeTrigger->setToleranceUnits(getTargetUnits());
        _rangeTrigger.setNeedsTriggerState();
    }
}

void HydroponicsBalancer::setIncrementActuators(const Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type &incActuators)
{
    for (auto actuatorIter = _incActuators.begin(); actuatorIter != _incActuators.end(); ++actuatorIter) {
        bool found = false;
        auto key = actuatorIter->first->getKey();

        for (auto actuatorInIter = incActuators.begin(); actuatorInIter != incActuators.end(); ++actuatorInIter) {
            if (key == actuatorInIter->first->getKey()) {
                found = true;
                break;
            }
        }

        if (!found && actuatorIter->first->isEnabled()) { // disables actuators not found in new list, prevents same used actuators from prev cycle from cycling off/on on stage switch
            actuatorIter->first->disableActuator();
        }
    }

    _incActuators.clear();
    for (auto actuatorInIter = incActuators.begin(); actuatorInIter != incActuators.end(); ++actuatorInIter) {
        _incActuators.push_back((*actuatorInIter));
    }
}

void HydroponicsBalancer::setDecrementActuators(const Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type &decActuators)
{
    for (auto actuatorIter = _decActuators.begin(); actuatorIter != _decActuators.end(); ++actuatorIter) {
        bool found = false;
        auto key = actuatorIter->first->getKey();

        for (auto actuatorInIter = decActuators.begin(); actuatorInIter != decActuators.end(); ++actuatorInIter) {
            if (key == actuatorInIter->first->getKey()) {
                found = true;
                break;
            }
        }

        if (!found && actuatorIter->first->isEnabled()) { // disables actuators not found in new list
            actuatorIter->first->disableActuator();
        }
    }

    _decActuators.clear();
    for (auto actuatorInIter = decActuators.begin(); actuatorInIter != decActuators.end(); ++actuatorInIter) {
        _decActuators.push_back((*actuatorInIter));
    }
}

void HydroponicsBalancer::setEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;

        if (_enabled) {
            _rangeTrigger->attachTrigger();
        } else {
            _rangeTrigger->detachTrigger();
        }
        _rangeTrigger.setNeedsTriggerState();
    }
}

Signal<Hydroponics_BalancerState, HYDRUINO_BALANCER_STATE_SLOTS> &HydroponicsBalancer::getBalancerSignal()
{
    return _balancerSignal;
}

void HydroponicsBalancer::disableAllActuators()
{
    for (auto actuatorIter = _incActuators.begin(); actuatorIter != _incActuators.end(); ++actuatorIter) {
        auto actuator = actuatorIter->first;
        if (actuator) { actuator->disableActuator(); }
    }
    for (auto actuatorIter = _decActuators.begin(); actuatorIter != _decActuators.end(); ++actuatorIter) {
        auto actuator = actuatorIter->first;
        if (actuator) { actuator->disableActuator(); }
    }
}

void HydroponicsBalancer::handleTrigger(Hydroponics_TriggerState triggerState)
{
    if (!_enabled || triggerState == Hydroponics_TriggerState_Undefined || triggerState == Hydroponics_TriggerState_Disabled) { return; }

    auto sensor = _rangeTrigger->getSensor();
    if (sensor) {
        auto balancerStateBefore = _balancerState;
        auto measure = getAsSingleMeasurement(sensor->getLatestMeasurement(), _rangeTrigger->getMeasurementRow());

        convertUnits(&measure, getTargetUnits());

        float halfTargetRange = _targetRange * 0.5f;
        if (measure.value > _targetSetpoint - halfTargetRange + FLT_EPSILON &&
            measure.value < _targetSetpoint + halfTargetRange - FLT_EPSILON) {
            _balancerState = Hydroponics_BalancerState_Balanced;
        } else {
            _balancerState = measure.value > _targetSetpoint ? Hydroponics_BalancerState_TooHigh : Hydroponics_BalancerState_TooLow;
        }

        if (_balancerState != balancerStateBefore) {
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<Hydroponics_BalancerState>(_balancerSignal, _balancerState);
            #else
                _balancerSignal.fire(_balancerState);
            #endif
        }
    }
}


HydroponicsLinearEdgeBalancer::HydroponicsLinearEdgeBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, float edgeOffset, float edgeLength, byte measurementRow)
    : HydroponicsBalancer(sensor, targetSetpoint, targetRange, measurementRow, LinearEdge), _edgeOffset(edgeOffset), _edgeLength(edgeLength)
{ ; }

void HydroponicsLinearEdgeBalancer::update()
{
    HydroponicsBalancer::update();

    if (!_enabled) { return; }

    if (_balancerState != Hydroponics_BalancerState_Balanced && _balancerState != Hydroponics_BalancerState_Undefined) {
        if (_rangeTrigger->getSensor()) {
            auto measure = getAsSingleMeasurement(_rangeTrigger->getSensor()->getLatestMeasurement(), _rangeTrigger->getMeasurementRow());

            convertUnits(&measure.value, &measure.units, _targetUnits);

            float x = fabsf(measure.value - _targetSetpoint);
            float val = _edgeLength > FLT_EPSILON ? mapValue<float>(x, _edgeOffset, _edgeOffset + _edgeLength, 0.0f, 1.0f)
                                                  : (x >= _edgeOffset - FLT_EPSILON ? 1.0 : 0.0f);
            val = constrain(val, 0.0f, 1.0f);

            if (_balancerState == Hydroponics_BalancerState_TooLow) {
                for (auto actuatorIter = _incActuators.begin(); actuatorIter != _incActuators.end(); ++actuatorIter) {
                    auto actuator = actuatorIter->first;
                    if (actuator) { actuator->enableActuator(val * actuatorIter->second); }
                }
            } else {
                for (auto actuatorIter = _decActuators.begin(); actuatorIter != _decActuators.end(); ++actuatorIter) {
                    auto actuator = actuatorIter->first;
                    if (actuator) { actuator->enableActuator(val * actuatorIter->second); }
                }
            }
        }
    }
}


HydroponicsTimedDosingBalancer::HydroponicsTimedDosingBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, time_t baseDosingMillis, unsigned int mixTimeMins, byte measurementRow)
    : HydroponicsBalancer(sensor, targetSetpoint, targetRange, measurementRow, TimedDosing), _baseDosingMillis(baseDosingMillis), _mixTimeMins(mixTimeMins),
      _lastDosingTime(0), _lastDosingValue(0.0f), _dosingMillis(0), _dosingDir(Hydroponics_BalancerState_Undefined), _dosingActIndex(-1)
{ ; }

HydroponicsTimedDosingBalancer::HydroponicsTimedDosingBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, float reservoirVolume, Hydroponics_UnitsType volumeUnits, byte measurementRow)
    : HydroponicsBalancer(sensor, targetSetpoint, targetRange, measurementRow, TimedDosing),
      _lastDosingTime(0), _lastDosingValue(0.0f), _dosingMillis(0), _dosingDir(Hydroponics_BalancerState_Undefined), _dosingActIndex(-1)
{
    if (volumeUnits != Hydroponics_UnitsType_LiqVolume_Gallons) {
        convertUnits(&reservoirVolume, &volumeUnits, Hydroponics_UnitsType_LiqVolume_Gallons);
    }
    // TODO: Verify these values work
    _baseDosingMillis = mapValue<float>(reservoirVolume, 5, 30, 500, 3000);
    _mixTimeMins = mapValue<float>(reservoirVolume, 30, 200, 10, 30);
}

void HydroponicsTimedDosingBalancer::update()
{
    HydroponicsBalancer::update();
    if (!_enabled) { return; }

    if (_balancerState != Hydroponics_BalancerState_Balanced && _balancerState != Hydroponics_BalancerState_Undefined &&
        (!_lastDosingTime || unixNow() > _lastDosingTime + (_mixTimeMins * SECS_PER_MIN))) {
        performDosing();
    }

    if (_dosingActIndex >= 0) { // has dosing that needs performed
        if (_dosingDir == Hydroponics_BalancerState_TooLow) {
            while (_dosingActIndex < _incActuators.size()) {
                auto actuatorIter = _incActuators.begin();
                for (int actuatorIndex = 0; actuatorIter != _incActuators.end() && actuatorIndex != _dosingActIndex; ++actuatorIter, ++actuatorIndex) { ; }
                if (actuatorIter != _incActuators.end()) {
                    performDosing(static_pointer_cast<HydroponicsActuator>(actuatorIter->first),
                                  actuatorIter->second * _dosingMillis);
                    #ifdef HYDRUINO_DISABLE_MULTITASKING
                        break; // only one dosing per call when done this way
                    #endif
                } else { break; }
            }

            if (_dosingActIndex >= _incActuators.size()) {
                _dosingActIndex = -1; // dosing completed
            }
        } else {
            while (_dosingActIndex < _decActuators.size()) {
                auto actuatorIter = _decActuators.begin();
                for (int actuatorIndex = 0; actuatorIter != _decActuators.end() && actuatorIndex != _dosingActIndex; ++actuatorIter, ++actuatorIndex) { ; }
                if (actuatorIter != _decActuators.end()) {
                    performDosing(static_pointer_cast<HydroponicsActuator>(actuatorIter->first),
                                  actuatorIter->second * _dosingMillis);
                    #ifdef HYDRUINO_DISABLE_MULTITASKING
                        break; // only one dosing per call when done this way
                    #endif
                } else { break; }
            }

            if (_dosingActIndex >= _decActuators.size()) {
                _dosingActIndex = -1; // dosing completed
            }
        }
    }
}

void HydroponicsTimedDosingBalancer::performDosing()
{
    if (_rangeTrigger->getSensor()) {
        if (_dosingDir != _balancerState) { // reset dir control on dir change
            _dosingMillis = 0;
            _dosingActIndex = 0;
            _dosingDir = Hydroponics_BalancerState_Undefined;
            disableAllActuators();
        }

        float dosingMillis = _baseDosingMillis;
        auto dosingValue = getMeasurementValue(_rangeTrigger->getSensor()->getLatestMeasurement(), _rangeTrigger->getMeasurementRow());
        if (_dosingMillis) {
            auto dosingRatePerMs = (dosingValue - _lastDosingValue) / _dosingMillis;
            dosingMillis = (_targetSetpoint - dosingValue) * dosingRatePerMs;
            dosingMillis = constrain(dosingMillis, _baseDosingMillis * HYDRUINO_DOSETIME_FRACTION_MIN,
                                                   _baseDosingMillis * HYDRUINO_DOSETIME_FRACTION_MAX);
        }

        _lastDosingValue = dosingValue;
        _dosingMillis = dosingMillis;
        _dosingActIndex = 0;
        _dosingDir = _balancerState;

        _lastDosingTime = unixNow();
    }
}

void HydroponicsTimedDosingBalancer::performDosing(shared_ptr<HydroponicsActuator> actuator, time_t timeMillis)
{
    if (actuator->isAnyPumpClass()) {
        ((HydroponicsPumpObjectInterface *)(actuator.get()))->pump(timeMillis); // pumps have nice logging output
    } else {
        #ifndef HYDRUINO_DISABLE_MULTITASKING
            scheduleActuatorTimedEnableOnce(actuator, timeMillis);
            _dosingActIndex++;
        #else
            actuator->enableActuator();
            delayFine(timeMillis);
            actuator->disableActuator();
            _dosingActIndex++;
        #endif
    }
}
