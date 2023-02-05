/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Balancers
*/

#include "Hydruino.h"

HydroBalancer::HydroBalancer(SharedPtr<HydroSensor> sensor, float targetSetpoint, float targetRange, uint8_t measurementRow, int typeIn)
    : type((typeof(type))typeIn), _targetSetpoint(targetSetpoint), _targetRange(targetRange),
      _sensor(this), _balancingState(Hydro_BalancingState_Undefined), _enabled(false)
{
    _sensor.setMeasurementRow(measurementRow);
    _sensor.setHandleMethod(&HydroBalancer::handleMeasurement);
    _sensor.setObject(sensor);
}

HydroBalancer::~HydroBalancer()
{
    _enabled = false;
    disableAllActivations();
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

Hydro_BalancingState HydroBalancer::getBalancingState() const
{
    return _balancingState;
}

void HydroBalancer::setIncrementActuators(const Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> &incActuators)
{
    for (auto activationIter = _incActuators.begin(); activationIter != _incActuators.end(); ++activationIter) {
        bool found = false;
        auto key = activationIter->getKey();

        for (auto activationInIter = incActuators.begin(); activationInIter != incActuators.end(); ++activationInIter) {
            if (key == activationInIter->getKey()) {
                found = true;
                break;
            }
        }

        if (!found) { // disables activations not found in new list, prevents same used actuators from prev cycle from turning off/on on cycle switch
            activationIter->disableActivation();
        }
    }

    {   _incActuators.clear();
        for (auto activationInIter = incActuators.begin(); activationInIter != incActuators.end(); ++activationInIter) {
            auto activation = (*activationInIter);
            _incActuators.push_back(activation);
        }
    }
}

void HydroBalancer::setDecrementActuators(const Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> &decActuators)
{
    for (auto activationIter = _decActuators.begin(); activationIter != _decActuators.end(); ++activationIter) {
        bool found = false;
        auto key = activationIter->getKey();

        for (auto activationInIter = decActuators.begin(); activationInIter != decActuators.end(); ++activationInIter) {
            if (key == activationInIter->getKey()) {
                found = true;
                break;
            }
        }

        if (!found) { // disables activations not found in new list
            activationIter->disableActivation();
        }
    }

    {   _decActuators.clear();
        for (auto activationInIter = decActuators.begin(); activationInIter != decActuators.end(); ++activationInIter) {
            auto activation = (*activationInIter);
            _decActuators.push_back(activation);
        }
    }
}

Signal<Hydro_BalancingState, HYDRO_BALANCER_SIGNAL_SLOTS> &HydroBalancer::getBalancingSignal()
{
    return _balancingSignal;
}

void HydroBalancer::disableAllActivations()
{
    for (auto activationIter = _incActuators.begin(); activationIter != _incActuators.end(); ++activationIter) {
        activationIter->disableActivation();
    }
    for (auto activationIter = _decActuators.begin(); activationIter != _decActuators.end(); ++activationIter) {
        activationIter->disableActivation();
    }
}

void HydroBalancer::handleMeasurement(const HydroMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        auto balancingStateBefore = _balancingState;

        auto measure = getAsSingleMeasurement(measurement, _sensor.getMeasurementRow());
        convertUnits(&measure, getTargetUnits(), _sensor.getMeasurementConvertParam());
        _sensor.setMeasurement(measure);

        if (_enabled) {
            float halfTargetRange = _targetRange * 0.5f;
            if (measure.value > _targetSetpoint - halfTargetRange + FLT_EPSILON &&
                measure.value < _targetSetpoint + halfTargetRange - FLT_EPSILON) {
                _balancingState = Hydro_BalancingState_Balanced;
            } else {
                _balancingState = measure.value > _targetSetpoint ? Hydro_BalancingState_TooHigh : Hydro_BalancingState_TooLow;
            }

            if (_balancingState != balancingStateBefore) {
                #ifdef HYDRO_USE_MULTITASKING
                    scheduleSignalFireOnce<Hydro_BalancingState>(_balancingSignal, _balancingState);
                #else
                    _balancingSignal.fire(_balancingState);
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

    if (_balancingState != Hydro_BalancingState_Balanced && _balancingState != Hydro_BalancingState_Undefined) {
        auto measure = _sensor.getMeasurement(true);

        float x = fabsf(measure.value - _targetSetpoint);
        float val = _edgeLength > FLT_EPSILON ? mapValue<float>(x, _edgeOffset, _edgeOffset + _edgeLength, 0.0f, 1.0f)
                                              : (x >= _edgeOffset - FLT_EPSILON ? 1.0 : 0.0f);
        val = constrain(val, 0.0f, 1.0f);

        if (_balancingState == Hydro_BalancingState_TooLow) {
            for (auto activationIter = _incActuators.begin(); activationIter != _incActuators.end(); ++activationIter) {
                activationIter->setupActivation(val * activationIter->getRateMultiplier());
                activationIter->enableActivation();
            }
        } else {
            for (auto activationIter = _decActuators.begin(); activationIter != _decActuators.end(); ++activationIter) {
                activationIter->setupActivation(val * activationIter->getRateMultiplier());
                activationIter->enableActivation();
            }
        }
    }
}


HydroTimedDosingBalancer::HydroTimedDosingBalancer(SharedPtr<HydroSensor> sensor, float targetSetpoint, float targetRange, millis_t baseDosing, time_t mixTime, uint8_t measurementRow)
    : HydroBalancer(sensor, targetSetpoint, targetRange, measurementRow, TimedDosing),
      _lastDosingTime(0), _lastDosingValue(0.0f), _dosing(0), _dosingDir(Hydro_BalancingState_Undefined), _dosingActIndex(-1),
      _baseDosing(baseDosing), _mixTime(mixTime)
{ ; }

HydroTimedDosingBalancer::HydroTimedDosingBalancer(SharedPtr<HydroSensor> sensor, float targetSetpoint, float targetRange, float reservoirVolume, Hydro_UnitsType volumeUnits, uint8_t measurementRow)
    : HydroBalancer(sensor, targetSetpoint, targetRange, measurementRow, TimedDosing),
      _lastDosingTime(0), _lastDosingValue(0.0f), _dosing(0), _dosingDir(Hydro_BalancingState_Undefined), _dosingActIndex(-1)
{
    if (volumeUnits != Hydro_UnitsType_LiqVolume_Gallons) {
        convertUnits(&reservoirVolume, &volumeUnits, Hydro_UnitsType_LiqVolume_Gallons);
    }
    // TODO: Verify these values work
    _baseDosing = mapValue<float>(reservoirVolume, 5, 30, 500, 3000);
    _mixTime = mapValue<float>(reservoirVolume, 30, 200, (10 * SECS_PER_MIN), (30 * SECS_PER_MIN));
}

void HydroTimedDosingBalancer::update()
{
    HydroBalancer::update();
    if (!_enabled || !_sensor) { return; }

    if (_balancingState != Hydro_BalancingState_Balanced && _balancingState != Hydro_BalancingState_Undefined &&
        (!_lastDosingTime || unixNow() > _lastDosingTime + _mixTime)) {
        if (_dosingDir != _balancingState) { // reset dir control on dir change
            _dosing = 0;
            _dosingActIndex = 0;
            _dosingDir = Hydro_BalancingState_Undefined;
            disableAllActivations();
        }

        float dosing = _baseDosing;
        auto dosingValue = getMeasurementValue(_sensor->getLatestMeasurement(), _sensor.getMeasurementRow());
        if (_dosing) {
            auto dosingRatePerMs = (dosingValue - _lastDosingValue) / _dosing;
            dosing = (_targetSetpoint - dosingValue) * dosingRatePerMs;
            dosing = constrain(dosing, _baseDosing * HYDRO_DOSETIME_FRACTION_MIN,
                                       _baseDosing * HYDRO_DOSETIME_FRACTION_MAX);
        }

        _lastDosingValue = dosingValue;
        _dosing = dosing;
        _dosingActIndex = 0;
        _dosingDir = _balancingState;

        _lastDosingTime = unixNow();
    }

    if (_dosingActIndex >= 0) { // has dosing that needs performed
        switch (_dosingDir) {
            case Hydro_BalancingState_TooLow:
                while (_dosingActIndex < _incActuators.size()) {
                    auto activationIter = _incActuators.begin(); // advance iter to index
                    for (int actuatorIndex = 0; activationIter != _incActuators.end() && actuatorIndex < _dosingActIndex; ++activationIter, ++actuatorIndex) { ; }

                    if (activationIter != _incActuators.end()) {
                        if (activationIter->get()->isAnyBinaryClass()) {
                            activationIter->setupActivation(1.0f, activationIter->getRateMultiplier() * _dosing);
                        } else {
                            activationIter->setupActivation(activationIter->getRateMultiplier(), _dosing);
                        }
                        activationIter->enableActivation();
                        #ifdef HYDRO_DISABLE_MULTITASKING
                            break; // only one dosing pass per call when done this way
                        #endif
                    } else { break; }
                }
                if (_dosingActIndex >= _incActuators.size()) {
                    _dosingActIndex = -1; // dosing completed
                }
                break;

            case Hydro_BalancingState_TooHigh:
                while (_dosingActIndex < _decActuators.size()) {
                    auto activationIter = _decActuators.begin();  // advance iter to index
                    for (int actuatorIndex = 0; activationIter != _decActuators.end() && actuatorIndex < _dosingActIndex; ++activationIter, ++actuatorIndex) { ; }

                    if (activationIter != _decActuators.end()) {
                        if (activationIter->get()->isAnyBinaryClass()) {
                            activationIter->setupActivation(1.0f, activationIter->getRateMultiplier() * _dosing);
                        } else {
                            activationIter->setupActivation(activationIter->getRateMultiplier(), _dosing);
                        }
                        activationIter->enableActivation();
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
