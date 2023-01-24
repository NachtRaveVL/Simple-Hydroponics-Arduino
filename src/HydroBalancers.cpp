/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Balancers
*/

#include "Hydruino.h"

HydroBalancer::HydroBalancer(SharedPtr<HydroSensor> sensor, float targetSetpoint, float targetRange, uint8_t measurementRow, int typeIn)
    : type((typeof(type))typeIn), _targetSetpoint(targetSetpoint), _targetRange(targetRange), _enabled(false),
      _sensor(this), _balancerState(Hydro_BalancerState_Undefined)
{
    _sensor.setMeasurementRow(measurementRow);
    _sensor.setHandleMethod(&HydroBalancer::handleMeasurement);
    _sensor.setObject(sensor);
}

HydroBalancer::~HydroBalancer()
{
    _enabled = false;
    disableAllActuators();
}

void HydroBalancer::update()
{
    _sensor.updateIfNeeded(true);
}

void HydroBalancer::setTargetSetpoint(float targetSetpoint)
{
    if (!isFPEqual(_targetSetpoint, targetSetpoint)) {
        _targetSetpoint = targetSetpoint;

        _sensor.setNeedsMeasurement();
    }
}

Hydro_BalancerState HydroBalancer::getBalancerState() const
{
    return _balancerState;
}

void HydroBalancer::setIncrementActuators(const Vector<Pair<SharedPtr<HydroActuator>, float>, HYDRO_BAL_INCACTUATORS_MAXSIZE> &incActuators)
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

        if (!found && actuatorIter->first->isEnabled()) { // disables actuators not found in new list, prevents same used actuators from prev cycle from turning off/on on cycle switch
            actuatorIter->first->disableActuator();
        }
    }

    {   _incActuators.clear();
        for (auto actuatorInIter = incActuators.begin(); actuatorInIter != incActuators.end(); ++actuatorInIter) {
            auto actuator = (*actuatorInIter);
            _incActuators.push_back(actuator);
        }
    }
}

void HydroBalancer::setDecrementActuators(const Vector<Pair<SharedPtr<HydroActuator>, float>, HYDRO_BAL_DECACTUATORS_MAXSIZE> &decActuators)
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

    {   _decActuators.clear();
        for (auto actuatorInIter = decActuators.begin(); actuatorInIter != decActuators.end(); ++actuatorInIter) {
            auto actuator = (*actuatorInIter);
            _decActuators.push_back(actuator);
        }
    }
}

Signal<Hydro_BalancerState, HYDRO_BALANCER_STATE_SLOTS> &HydroBalancer::getBalancerSignal()
{
    return _balancerSignal;
}

void HydroBalancer::disableAllActuators()
{
    for (auto actuatorIter = _incActuators.begin(); actuatorIter != _incActuators.end(); ++actuatorIter) {
        actuatorIter->first->disableActuator();
    }
    for (auto actuatorIter = _decActuators.begin(); actuatorIter != _decActuators.end(); ++actuatorIter) {
        actuatorIter->first->disableActuator();
    }
}

void HydroBalancer::handleMeasurement(const HydroMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        auto balancerStateBefore = _balancerState;

        auto measure = getAsSingleMeasurement(measurement, _sensor.getMeasurementRow());
        convertUnits(&measure, getTargetUnits(), _sensor.getMeasurementConvertParam());
        _sensor.setMeasurement(measure);

        if (_enabled) {
            float halfTargetRange = _targetRange * 0.5f;
            if (measure.value > _targetSetpoint - halfTargetRange + FLT_EPSILON &&
                measure.value < _targetSetpoint + halfTargetRange - FLT_EPSILON) {
                _balancerState = Hydro_BalancerState_Balanced;
            } else {
                _balancerState = measure.value > _targetSetpoint ? Hydro_BalancerState_TooHigh : Hydro_BalancerState_TooLow;
            }

            if (_balancerState != balancerStateBefore) {
                #ifdef HYDRO_USE_MULTITASKING
                    scheduleSignalFireOnce<Hydro_BalancerState>(_balancerSignal, _balancerState);
                #else
                    _balancerSignal.fire(_balancerState);
                #endif
            }
        }
    }
}


HydroLinearEdgeBalancer::HydroLinearEdgeBalancer(SharedPtr<HydroSensor> sensor, float targetSetpoint, float targetRange, float edgeOffset, float edgeLength, uint8_t measurementRow)
    : HydroBalancer(sensor, targetSetpoint, targetRange, measurementRow, LinearEdge), _edgeOffset(edgeOffset), _edgeLength(edgeLength)
{ ; }

void HydroLinearEdgeBalancer::update()
{
    HydroBalancer::update();
    if (!_enabled || !_sensor) { return; }

    if (_balancerState != Hydro_BalancerState_Balanced && _balancerState != Hydro_BalancerState_Undefined) {
        auto measure = _sensor.getMeasurement(true);

        float x = fabsf(measure.value - _targetSetpoint);
        float val = _edgeLength > FLT_EPSILON ? mapValue<float>(x, _edgeOffset, _edgeOffset + _edgeLength, 0.0f, 1.0f)
                                                : (x >= _edgeOffset - FLT_EPSILON ? 1.0 : 0.0f);
        val = constrain(val, 0.0f, 1.0f);

        if (_balancerState == Hydro_BalancerState_TooLow) {
            for (auto actuatorIter = _incActuators.begin(); actuatorIter != _incActuators.end(); ++actuatorIter) {
                actuatorIter->first->enableActuator(val * actuatorIter->second);
            }
        } else {
            for (auto actuatorIter = _decActuators.begin(); actuatorIter != _decActuators.end(); ++actuatorIter) {
                actuatorIter->first->enableActuator(val * actuatorIter->second);
            }
        }
    }
}


HydroTimedDosingBalancer::HydroTimedDosingBalancer(SharedPtr<HydroSensor> sensor, float targetSetpoint, float targetRange, time_t baseDosingMillis, time_t mixTime, uint8_t measurementRow)
    : HydroBalancer(sensor, targetSetpoint, targetRange, measurementRow, TimedDosing),
      _lastDosingTime(0), _lastDosingValue(0.0f), _dosingMillis(0), _dosingDir(Hydro_BalancerState_Undefined), _dosingActIndex(-1),
      _baseDosingMillis(baseDosingMillis), _mixTime(mixTime)
{ ; }

HydroTimedDosingBalancer::HydroTimedDosingBalancer(SharedPtr<HydroSensor> sensor, float targetSetpoint, float targetRange, float reservoirVolume, Hydro_UnitsType volumeUnits, uint8_t measurementRow)
    : HydroBalancer(sensor, targetSetpoint, targetRange, measurementRow, TimedDosing),
      _lastDosingTime(0), _lastDosingValue(0.0f), _dosingMillis(0), _dosingDir(Hydro_BalancerState_Undefined), _dosingActIndex(-1)
{
    if (volumeUnits != Hydro_UnitsType_LiqVolume_Gallons) {
        convertUnits(&reservoirVolume, &volumeUnits, Hydro_UnitsType_LiqVolume_Gallons);
    }
    // TODO: Verify these values work
    _baseDosingMillis = mapValue<float>(reservoirVolume, 5, 30, 500, 3000);
    _mixTime = mapValue<float>(reservoirVolume, 30, 200, (10 * SECS_PER_MIN), (30 * SECS_PER_MIN));
}

void HydroTimedDosingBalancer::update()
{
    HydroBalancer::update();
    if (!_enabled || !_sensor) { return; }

    if (_balancerState != Hydro_BalancerState_Balanced && _balancerState != Hydro_BalancerState_Undefined &&
        (!_lastDosingTime || unixNow() > _lastDosingTime + _mixTime)) {
        performDosing();
    }

    if (_dosingActIndex >= 0) { // has dosing that needs performed
        switch (_dosingDir) {
            case Hydro_BalancerState_TooLow:
                while (_dosingActIndex < _incActuators.size()) {
                    auto actuatorIter = _incActuators.begin(); // advance iter to index
                    for (int actuatorIndex = 0; actuatorIter != _incActuators.end() && actuatorIndex < _dosingActIndex; ++actuatorIter, ++actuatorIndex) { ; }

                    if (actuatorIter != _incActuators.end()) {
                        performDosing(actuatorIter->first, actuatorIter->second * _dosingMillis);
                        #ifdef HYDRO_DISABLE_MULTITASKING
                            break; // only one dosing pass per call when done this way
                        #endif
                    } else { break; }
                }
                if (_dosingActIndex >= _incActuators.size()) {
                    _dosingActIndex = -1; // dosing completed
                }
                break;

            case Hydro_BalancerState_TooHigh:
                while (_dosingActIndex < _decActuators.size()) {
                    auto actuatorIter = _decActuators.begin();  // advance iter to index
                    for (int actuatorIndex = 0; actuatorIter != _decActuators.end() && actuatorIndex < _dosingActIndex; ++actuatorIter, ++actuatorIndex) { ; }

                    if (actuatorIter != _decActuators.end()) {
                        performDosing(actuatorIter->first, actuatorIter->second * _dosingMillis);
                        #ifdef HYDRO_DISABLE_MULTITASKING
                            break; // only one dosing pass per call when done this way
                        #endif
                    } else { break; }
                }
                if (_dosingActIndex >= _decActuators.size()) {
                    _dosingActIndex = -1; // dosing completed
                }
                break;

            default:
                HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
                break;
        }
    }
}

void HydroTimedDosingBalancer::performDosing()
{
    if (_dosingDir != _balancerState) { // reset dir control on dir change
        _dosingMillis = 0;
        _dosingActIndex = 0;
        _dosingDir = Hydro_BalancerState_Undefined;
        disableAllActuators();
    }

    float dosingMillis = _baseDosingMillis;
    auto dosingValue = getMeasurementValue(_sensor->getLatestMeasurement(), _sensor.getMeasurementRow());
    if (_dosingMillis) {
        auto dosingRatePerMs = (dosingValue - _lastDosingValue) / _dosingMillis;
        dosingMillis = (_targetSetpoint - dosingValue) * dosingRatePerMs;
        dosingMillis = constrain(dosingMillis, _baseDosingMillis * HYDRO_DOSETIME_FRACTION_MIN,
                                                _baseDosingMillis * HYDRO_DOSETIME_FRACTION_MAX);
    }

    _lastDosingValue = dosingValue;
    _dosingMillis = dosingMillis;
    _dosingActIndex = 0;
    _dosingDir = _balancerState;

    _lastDosingTime = unixNow();
}

void HydroTimedDosingBalancer::performDosing(SharedPtr<HydroActuator> &actuator, time_t timeMillis)
{
    if (actuator->isRelayPumpClass()) {
        static_pointer_cast<HydroPumpRelayActuator>(actuator)->pump(timeMillis); // pumps have nice logging output
    } else {
        #ifdef HYDRO_USE_MULTITASKING
            scheduleActuatorTimedEnableOnce(actuator, timeMillis);
        #else
            actuator->enableActuator();
            delayFine(timeMillis);
            actuator->disableActuator();
        #endif
    }
    _dosingActIndex++;
}
