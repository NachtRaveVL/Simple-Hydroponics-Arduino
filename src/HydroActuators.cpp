/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Actuators
*/

#include "Hydruino.h"

HydroActuator *newActuatorObjectFromData(const HydroActuatorData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
            case (int8_t)HydroActuator::Relay:
                return new HydroRelayActuator((const HydroActuatorData *)dataIn);
            case (int8_t)HydroActuator::RelayPump:
                return new HydroRelayPumpActuator((const HydroPumpActuatorData *)dataIn);
            case (int8_t)HydroActuator::Variable:
                return new HydroVariableActuator((const HydroActuatorData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroActivationHandle::HydroActivationHandle(HydroActuator *actuator, Hydro_DirectionMode directionIn, float intensityIn, millis_t duration, bool force)
    : actuator(actuator), update(nullptr), direction(directionIn), intensity(constrain(intensityIn, 0.0f, 1.0f)), start(0), duration(duration), forced(force)
{
    if (actuator) { actuator->_handles.push_back(this); actuator->_needsUpdate = true; }
}

HydroActivationHandle::HydroActivationHandle(const HydroActivationHandle &handle)
    : actuator(handle.actuator), update(nullptr), direction(handle.direction), intensity(constrain(handle.intensity, 0.0f, 1.0f)),
      start(0), duration(handle.duration), forced(handle.forced)
{
    if (actuator) { actuator->_handles.push_back(this); actuator->_needsUpdate = true; }
}

HydroActivationHandle::~HydroActivationHandle()
{
    unset();
    if (update) { delete update; update = nullptr; }
}

HydroActivationHandle &HydroActivationHandle::operator=(const HydroActivationHandle &handle)
{
    if (handle.actuator != actuator) {
        unset();
        actuator = handle.actuator;
        if (actuator) { actuator->_handles.push_back(this); actuator->_needsUpdate = true; }
    } else { start = 0; }
    direction = handle.direction;
    intensity = constrain(handle.intensity, 0.0f, 1.0f);
    duration = handle.duration;
    forced = handle.forced;
    return *this;
}

void HydroActivationHandle::setUpdate(Slot<millis_t> &slot)
{
    if (update) { delete update; }
    update = slot.clone();
}

void HydroActivationHandle::unset()
{
    if (actuator) {
        for (auto handleIter = actuator->_handles.end() - 1; handleIter != actuator->_handles.begin() - 1; --handleIter) {
            if ((*handleIter) == this) {
                actuator->_handles.erase(handleIter); actuator->_needsUpdate = true;
                break;
            }
        }
        actuator = nullptr;
    }
    start = 0;
}


HydroActuator::HydroActuator(Hydro_ActuatorType actuatorType,
                             Hydro_PositionIndex actuatorIndex,
                             int classTypeIn)
    : HydroObject(HydroIdentity(actuatorType, actuatorIndex)), classType((typeof(classType))classTypeIn),
      _enabled(false), _enableMode(Hydro_EnableMode_Undefined), _rail(this), _reservoir(this)
{ ; }

HydroActuator::HydroActuator(const HydroActuatorData *dataIn)
    : HydroObject(dataIn), classType((typeof(classType))dataIn->id.object.classType),
      _enabled(false),
      _contPowerUsage(&(dataIn->contPowerUsage)),
      _rail(this), _reservoir(this)
{
    _rail.setObject(dataIn->railName);
    _reservoir.setObject(dataIn->reservoirName);
}

void HydroActuator::update()
{
    HydroObject::update();

    _rail.resolve();
    _reservoir.resolve();

    // Enablement checking
    bool wasEnabled = _enabled;
    bool canEnable = _handles.size() && getCanEnable();
    for (auto handleIter = _handles.begin(); handleIter != _handles.end() && !canEnable; ++handleIter) {
        canEnable = (*handleIter)->forced;
    }

    // If enabled and shouldn't be (unless force enabled)
    if ((_enabled || _needsUpdate) && !canEnable) {
        _disableActuator();
    } else if (canEnable && (!_enabled || _needsUpdate)) {
        float drivingIntensity = 0.0f; // Determine what driving intensity [-1,1] actuator should use

        switch (_enableMode) {
            case Hydro_EnableMode_Highest:
            case Hydro_EnableMode_DesOrder: {
                drivingIntensity = -__FLT_MAX__;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    auto handleIntensity = (*handleIter)->getDriveIntensity();
                    if (handleIntensity > drivingIntensity) { drivingIntensity = handleIntensity; }
                }
            } break;

            case Hydro_EnableMode_Lowest:
            case Hydro_EnableMode_AscOrder: {
                drivingIntensity = __FLT_MAX__;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    auto handleIntensity = (*handleIter)->getDriveIntensity();
                    if (handleIntensity < drivingIntensity) { drivingIntensity = handleIntensity; }
                }
            } break;

            case Hydro_EnableMode_Average: {
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    drivingIntensity += (*handleIter)->getDriveIntensity();
                }
                drivingIntensity /= _handles.size();
            } break;

            case Hydro_EnableMode_Multiply: {
                drivingIntensity = (*_handles.begin())->getDriveIntensity();
                for (auto handleIter = _handles.begin() + 1; handleIter != _handles.end(); ++handleIter) {
                    drivingIntensity *= (*handleIter)->getDriveIntensity();
                }
            } break;

            case Hydro_EnableMode_InOrder: {
                drivingIntensity = (*_handles.begin())->getDriveIntensity();
            } break;

            case Hydro_EnableMode_RevOrder: {
                drivingIntensity = (*(_handles.end() - 1))->getDriveIntensity();
            } break;

            default:
                break;
        }

        _enableActuator(drivingIntensity);
    }

    // Don't worry about updating if just began, give some time for things to run before update
    if (_enabled && wasEnabled) {
        millis_t time = millis();

        switch (_enableMode) {
            case Hydro_EnableMode_Highest:
            case Hydro_EnableMode_Lowest:
            case Hydro_EnableMode_Average:
            case Hydro_EnableMode_Multiply: {
                // Parallel actuators all run in unison
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->update) {
                        (*handleIter)->update->operator()(time - (*handleIter)->start);
                    }
                }
            } break;

            case Hydro_EnableMode_InOrder: {
                auto handleIter = _handles.begin();
                if ((*handleIter)->update) {
                    (*handleIter)->update->operator()(time - (*handleIter)->start);
                }
            } break;

            case Hydro_EnableMode_RevOrder: {
                auto handleIter = _handles.end() - 1;
                if ((*handleIter)->update) {
                    (*handleIter)->update->operator()(time - (*handleIter)->start);
                }
            } break;

            case Hydro_EnableMode_DesOrder:
            case Hydro_EnableMode_AscOrder: {
                float drivingIntensity = getDriveIntensity();

                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if (isFPEqual((*handleIter)->intensity, drivingIntensity)) {
                        if ((*handleIter)->update) {
                            (*handleIter)->update->operator()(time - (*handleIter)->start);
                        }
                        break;
                    }
                }
            } break;

            default:
                break;
        }
    }

    _needsUpdate = false;
}

bool HydroActuator::getCanEnable()
{
    if (getRail() && !getRail()->canActivate(this)) { return false; }
    if (getReservoir() && !getReservoir()->canActivate(this)) { return false; }
    return true;
}

void HydroActuator::setContinuousPowerUsage(HydroSingleMeasurement contPowerUsage)
{
    _contPowerUsage = contPowerUsage;
    _contPowerUsage.setMinFrame(1);
}

const HydroSingleMeasurement &HydroActuator::getContinuousPowerUsage()
{
    return _contPowerUsage;
}

HydroAttachment &HydroActuator::getParentRail(bool resolve)
{
    if (resolve) { _rail.resolve(); }
    return _rail;
}

HydroAttachment &HydroActuator::getParentReservoir(bool resolve)
{
    if (resolve) { _reservoir.resolve(); }
    return _reservoir;
}

Signal<HydroActuator *> &HydroActuator::getActivationSignal()
{
    return _activateSignal;
}

HydroData *HydroActuator::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void HydroActuator::saveToData(HydroData *dataOut)
{
    HydroObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
    if (_contPowerUsage.frame) {
        _contPowerUsage.saveToData(&(((HydroActuatorData *)dataOut)->contPowerUsage));
    }
    if (_reservoir.getId()) {
        strncpy(((HydroActuatorData *)dataOut)->reservoirName, _reservoir.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_rail.getId()) {
        strncpy(((HydroActuatorData *)dataOut)->railName, _rail.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    ((HydroActuatorData *)dataOut)->enableMode = _enableMode;
}

void HydroActuator::handleActivation()
{
    millis_t time = millis();

    if (_enabled) {
        for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
            if ((*handleIter)->actuator.get() == this && (*handleIter)->start == 0) {
                (*handleIter)->start = max(1, time);
            }
        }

        getLoggerInstance()->logActivation(this);
    } else {
        for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
            if ((*handleIter)->duration) {
                millis_t duration = time - (*handleIter)->start;

                if (duration >= (*handleIter)->duration) {
                    duration = (*handleIter)->duration; (*handleIter)->duration = 0;
                    if ((*handleIter)->update) {
                        (*handleIter)->update->operator()(duration);
                    }
                    (*handleIter)->start = 0;
                    (*handleIter)->actuator = nullptr;
                    handleIter = _handles.erase(handleIter) - 1;
                } else {
                    (*handleIter)->duration -= duration;
                    if ((*handleIter)->update) {
                        (*handleIter)->update->operator()(duration);
                    }
                    (*handleIter)->start = 0;
                }
            }
        }

        getLoggerInstance()->logDeactivation(this);
    }

    #ifdef HYDRO_USE_MULTITASKING
        scheduleSignalFireOnce<HydroActuator *>(getSharedPtr(), _activateSignal, this);
    #else
        _activateSignal.fire(this);
    #endif
}


HydroRelayActuator::HydroRelayActuator(Hydro_ActuatorType actuatorType,
                                       Hydro_PositionIndex actuatorIndex,
                                       HydroDigitalPin outputPin,
                                       int classType)
    : HydroActuator(actuatorType, actuatorIndex, classType),
      _outputPin(outputPin)
{
    HYDRO_HARD_ASSERT(_outputPin.isValid(), SFP(HStr_Err_InvalidPinOrType));
    _outputPin.init();
    _outputPin.deactivate();
}

HydroRelayActuator::HydroRelayActuator(const HydroActuatorData *dataIn)
    : HydroActuator(dataIn), _outputPin(&dataIn->outputPin)
{
    HYDRO_HARD_ASSERT(_outputPin.isValid(), SFP(HStr_Err_InvalidPinOrType));
    _outputPin.init();
    _outputPin.deactivate();
}

HydroRelayActuator::~HydroRelayActuator()
{
    if (_enabled) {
        _enabled = false;
        _outputPin.deactivate();
    }
}

bool HydroRelayActuator::getCanEnable()
{
    return _outputPin.isValid() && HydroActuator::getCanEnable();
}

float HydroRelayActuator::getDriveIntensity()
{
    return _enabled ? 1.0f : 0.0f;
}

bool HydroRelayActuator::isEnabled(float tolerance) const
{
    return _enabled;
}

void HydroRelayActuator::_enableActuator(float intensity)
{
    bool wasEnabled = _enabled;

    if (_outputPin.isValid() && !_enabled) {
        if (!isFPEqual(intensity, 0.0f)) {
            _enabled = true;
            _outputPin.activate();
            if (!wasEnabled) { handleActivation(); }
        } else {
            _outputPin.deactivate();
        }
    }
}

void HydroRelayActuator::_disableActuator()
{
    if (_outputPin.isValid() && _enabled) {
        _enabled = false;
        _outputPin.deactivate();
        handleActivation();
    }
}

void HydroRelayActuator::saveToData(HydroData *dataOut)
{
    HydroActuator::saveToData(dataOut);

    _outputPin.saveToData(&((HydroActuatorData *)dataOut)->outputPin);
}


HydroRelayPumpActuator::HydroRelayPumpActuator(Hydro_ActuatorType actuatorType,
                                               Hydro_PositionIndex actuatorIndex,
                                               HydroDigitalPin outputPin,
                                               int classType)
    :  HydroRelayActuator(actuatorType, actuatorIndex, outputPin, classType),
       _flowRateUnits(defaultLiquidFlowUnits()), _flowRate(this), _destReservoir(this),
       _pumpVolumeAccum(0.0f), _pumpTimeStart(0), _pumpTimeAccum(0)
{
    _flowRate.setMeasurementUnits(getFlowRateUnits());
}

HydroRelayPumpActuator::HydroRelayPumpActuator(const HydroPumpActuatorData *dataIn)
    : HydroRelayActuator(dataIn), _pumpVolumeAccum(0.0f), _pumpTimeStart(0), _pumpTimeAccum(0),
      _flowRateUnits(definedUnitsElse(dataIn->flowRateUnits, defaultLiquidFlowUnits())),
      _contFlowRate(&(dataIn->contFlowRate)),
      _flowRate(this), _destReservoir(this)
{
    _flowRate.setMeasurementUnits(getFlowRateUnits());
    _destReservoir.setObject(dataIn->destReservoir);
    _flowRate.setObject(dataIn->flowRateSensor);
}

void HydroRelayPumpActuator::update()
{
    HydroActuator::update();

    _destReservoir.resolve();

    _flowRate.updateIfNeeded(true);

    if (_pumpTimeStart) {
        millis_t time = max(1, millis());
        millis_t duration = time - _pumpTimeStart;
        if (duration >= HYDRO_ACT_PUMPCALC_UPDATEMS) {
            handlePumpTime(time);
        }
    }

    if (_enabled) { pollPumpingSensors(); }
}

bool HydroRelayPumpActuator::getCanEnable()
{
    if (HydroRelayActuator::getCanEnable()) {
        if (getOutputReservoir() && !getOutputReservoir()->canActivate(this)) { return false; }
        return true;
    }
    return false;
}

void HydroRelayPumpActuator::handleActivation()
{
    millis_t time = millis();
    HydroActuator::handleActivation();

    if (_enabled) {
        _pumpVolumeAccum = 0;
        _pumpTimeStart = _pumpTimeAccum = max(1, time);
    } else {
        if (_pumpTimeAccum < time) { handlePumpTime(time); }
        _pumpTimeAccum = 0;
        float duration = time - _pumpTimeStart;
        uint8_t addDecPlaces = getActuatorType() == Hydro_ActuatorType_PeristalticPump ? 2 : 1;

        getLoggerInstance()->logStatus(this, SFP(HStr_Log_MeasuredPumping));
        if (getInputReservoir()) { getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Source_Reservoir), getInputReservoir()->getKeyString()); }
        if (getOutputReservoir()) { getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Destination_Reservoir), getOutputReservoir()->getKeyString()); }
        getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Vol_Measured), measurementToString(_pumpVolumeAccum, baseUnitsFromRate(getFlowRateUnits()), addDecPlaces));
        getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Time_Measured), roundToString(duration / 1000.0f, 1), String('s'));
    }
}

bool HydroRelayPumpActuator::canPump(float volume, Hydro_UnitsType volumeUnits)
{
    if (getReservoir() && _contFlowRate.value > FLT_EPSILON) {
        auto waterVolume = getReservoir()->getWaterVolume().getMeasurement();
        convertUnits(&volume, &volumeUnits, waterVolume.units);
        return volume <= waterVolume.value + FLT_EPSILON;
    }
    return false;
}

HydroActivationHandle HydroRelayPumpActuator::pump(float volume, Hydro_UnitsType volumeUnits)
{
    if (getReservoir() && _contFlowRate.value > FLT_EPSILON) {
        convertUnits(&volume, &volumeUnits, baseUnitsFromRate(getFlowRateUnits()));
        return pump((millis_t)((volume / _contFlowRate.value) * secondsToMillis(SECS_PER_MIN)));
    }
    return HydroActivationHandle();
}

bool HydroRelayPumpActuator::canPump(millis_t time)
{
    if (getReservoir() && _contFlowRate.value > FLT_EPSILON) {
        return canPump(_contFlowRate.value * (time / (float)secondsToMillis(SECS_PER_MIN)), baseUnitsFromRate(getFlowRateUnits()));
    }
    return false;
}

HydroActivationHandle HydroRelayPumpActuator::pump(millis_t time)
{
    if (getReservoir()) {
        #ifdef HYDRO_USE_MULTITASKING
            getLoggerInstance()->logStatus(this, SFP(HStr_Log_CalculatedPumping));
            if (getInputReservoir()) { getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Source_Reservoir), getInputReservoir()->getKeyString()); }
            if (getOutputReservoir()) { getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Destination_Reservoir), getOutputReservoir()->getKeyString()); }
            if (_contFlowRate.value > FLT_EPSILON) {
                uint8_t addDecPlaces = getActuatorType() == Hydro_ActuatorType_PeristalticPump ? 2 : 1;
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Vol_Calculated), measurementToString(_contFlowRate.value * (time / (float)secondsToMillis(SECS_PER_MIN)), baseUnitsFromRate(getFlowRateUnits()), addDecPlaces));
            }
            getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Time_Calculated), roundToString(time / 1000.0f, 1), String('s'));
            return enableActuator(time);
        #else
            getLoggerInstance()->logStatus(this, SFP(HStr_Log_CalculatedPumping));
            if (getInputReservoir()) { getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Source_Reservoir), getInputReservoir()->getKeyString()); }
            if (getOutputReservoir()) { getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Destination_Reservoir), getOutputReservoir()->getKeyString()); }
            if (_contFlowRate.value > FLT_EPSILON) {
                uint8_t addDecPlaces = getActuatorType() == Hydro_ActuatorType_PeristalticPump ? 2 : 1;
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Vol_Calculated), measurementToString(_contFlowRate.value * (time / (float)secondsToMillis(SECS_PER_MIN)), baseUnitsFromRate(getFlowRateUnits()), addDecPlaces));
            }
            getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Time_Calculated), roundToString(time / 1000.0f, 1), String('s'));
            return enableActuator(time);
        #endif
    }
}

HydroAttachment &HydroRelayPumpActuator::getParentReservoir(bool resolve)
{
    return HydroActuator::getParentReservoir(resolve);
}

HydroAttachment &HydroRelayPumpActuator::getDestinationReservoir(bool resolve)
{
    if (resolve) { _destReservoir.resolve(); }
    return _destReservoir;
}

void HydroRelayPumpActuator::setFlowRateUnits(Hydro_UnitsType flowRateUnits)
{
    if (_flowRateUnits != flowRateUnits) {
        _flowRateUnits = flowRateUnits;

        convertUnits(&_contFlowRate, getFlowRateUnits());
        _flowRate.setMeasurementUnits(getFlowRateUnits());
    }
}

Hydro_UnitsType HydroRelayPumpActuator::getFlowRateUnits() const
{
    return definedUnitsElse(_flowRateUnits, defaultLiquidFlowUnits());
}

void HydroRelayPumpActuator::setContinuousFlowRate(HydroSingleMeasurement contFlowRate)
{
    _contFlowRate = contFlowRate;
    _contFlowRate.setMinFrame(1);

    convertUnits(&_contFlowRate, getFlowRateUnits());
}

const HydroSingleMeasurement &HydroRelayPumpActuator::getContinuousFlowRate()
{
    return _contFlowRate;
}

HydroSensorAttachment &HydroRelayPumpActuator::getFlowRate(bool poll)
{
    _flowRate.updateIfNeeded(poll);
    return _flowRate;
}

void HydroRelayPumpActuator::saveToData(HydroData *dataOut)
{
    HydroRelayActuator::saveToData(dataOut);

    ((HydroPumpActuatorData *)dataOut)->flowRateUnits = _flowRateUnits;
    if (_contFlowRate.frame) {
        _contFlowRate.saveToData(&(((HydroPumpActuatorData *)dataOut)->contFlowRate));
    }
    if (_destReservoir.getId()) {
        strncpy(((HydroPumpActuatorData *)dataOut)->destReservoir, _destReservoir.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_flowRate.getId()) {
        strncpy(((HydroPumpActuatorData *)dataOut)->flowRateSensor, _flowRate.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
}

void HydroRelayPumpActuator::pollPumpingSensors()
{
    if (getFlowRateSensor()) {
        _flowRate->takeMeasurement(true);
    }
    HydroSensor *sourceVolSensor;
    if (getInputReservoir() && _reservoir.get<HydroReservoir>()->isAnyFluidClass() &&
        (sourceVolSensor = _reservoir.get<HydroFluidReservoir>()->getWaterVolumeSensor().get())) {
        sourceVolSensor->takeMeasurement(true);
    }
    HydroSensor *destVolSensor;
    if (getOutputReservoir() && _destReservoir.get<HydroReservoir>()->isAnyFluidClass() &&
        (destVolSensor = _destReservoir.get<HydroFluidReservoir>()->getWaterVolumeSensor().get()) &&
        destVolSensor != sourceVolSensor) {
        destVolSensor->takeMeasurement(true);
    }
}

void HydroRelayPumpActuator::handlePumpTime(millis_t time)
{
    if (getInputReservoir() != getOutputReservoir()) {
        float flowRateVal = getFlowRateSensor() ? _flowRate.getMeasurementValue(true) : _contFlowRate.value;
        flowRateVal = max(_contFlowRate.value * HYDRO_ACT_PUMPCALC_MINFLOWRATE, flowRateVal);
        float volumePumped = flowRateVal * ((time - _pumpTimeAccum) / (float)secondsToMillis(SECS_PER_MIN));
        _pumpVolumeAccum += volumePumped;

        if (getInputReservoir() && _reservoir.get<HydroReservoir>()->isAnyFluidClass()) {
            auto sourceFluidRes = getInputReservoir<HydroFluidReservoir>();
            if (sourceFluidRes && !sourceFluidRes->getWaterVolume()) { // only report if there isn't a volume sensor already doing it
                auto volume = sourceFluidRes->getWaterVolume().getMeasurement(true);
                convertUnits(&volume, baseUnitsFromRate(getFlowRateUnits()));
                volume.value -= volumePumped;
                sourceFluidRes->getWaterVolume().setMeasurement(volume);
            }
        }

        if (getOutputReservoir() && _destReservoir.get<HydroReservoir>()->isAnyFluidClass()) {
            auto destFluidRes = getOutputReservoir<HydroFluidReservoir>();
            if (destFluidRes && !destFluidRes->getWaterVolume()) { // only report if there isn't a volume sensor already doing it
                auto volume = destFluidRes->getWaterVolume().getMeasurement(true);
                convertUnits(&volume, baseUnitsFromRate(getFlowRateUnits()));
                volume.value += volumePumped;
                destFluidRes->getWaterVolume().setMeasurement(volume);
            }
        }
    }

    _pumpTimeAccum = time;
}


HydroVariableActuator::HydroVariableActuator(Hydro_ActuatorType actuatorType,
                                             Hydro_PositionIndex actuatorIndex,
                                             HydroAnalogPin outputPin,
                                             int classType)
    : HydroActuator(actuatorType, actuatorIndex, classType),
      _outputPin(outputPin), _intensity(0.0f)
{
    HYDRO_HARD_ASSERT(_outputPin.isValid(), SFP(HStr_Err_InvalidPinOrType));
    _outputPin.init();
    _outputPin.analogWrite_raw(0);
}

HydroVariableActuator::HydroVariableActuator(const HydroActuatorData *dataIn)
    : HydroActuator(dataIn),
      _outputPin(&dataIn->outputPin), _intensity(0.0f)
{
    HYDRO_HARD_ASSERT(_outputPin.isValid(), SFP(HStr_Err_InvalidPinOrType));
    _outputPin.init();
    _outputPin.analogWrite_raw(0);
}

HydroVariableActuator::~HydroVariableActuator()
{
    if (_enabled) {
        _enabled = false;
        _outputPin.analogWrite_raw(0);
    }
}

bool HydroVariableActuator::getCanEnable()
{
    return _outputPin.isValid() && HydroActuator::getCanEnable();
}

float HydroVariableActuator::getDriveIntensity()
{
    return _intensity;
}

bool HydroVariableActuator::isEnabled(float tolerance) const
{
    return _enabled && _intensity >= tolerance - FLT_EPSILON;
}

void HydroVariableActuator::_enableActuator(float intensity)
{
    bool wasEnabled = _enabled;
    intensity = constrain(intensity, 0.0f, 1.0f);

    if (_outputPin.isValid() && (!_enabled || !isFPEqual(_intensity, intensity))) {
        _intensity = intensity;
        if (!isFPEqual(_intensity, 0.0f)) {
            _enabled = true;
            _outputPin.analogWrite(_intensity);
            if (!wasEnabled) { handleActivation(); }
        } else {
            _outputPin.analogWrite_raw(0);
        }
    }
}

void HydroVariableActuator::_disableActuator()
{
    if (_outputPin.isValid() && _enabled) {
        _enabled = false;
        _outputPin.analogWrite_raw(0);
        handleActivation();
    }
}

void HydroVariableActuator::saveToData(HydroData *dataOut)
{
    HydroActuator::saveToData(dataOut);

    _outputPin.saveToData(&((HydroActuatorData *)dataOut)->outputPin);
}


HydroActuatorData::HydroActuatorData()
    : HydroObjectData(), outputPin(), enableMode(Hydro_EnableMode_Undefined), contPowerUsage(), railName{0}, reservoirName{0}
{
    _size = sizeof(*this);
}

void HydroActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroObjectData::toJSONObject(objectOut);

    if (isValidPin(outputPin.pin)) {
        JsonObject outputPinObj = objectOut.createNestedObject(SFP(HStr_Key_OutputPin));
        outputPin.toJSONObject(outputPinObj);
    }
    if (enableMode != Hydro_EnableMode_Undefined) { objectOut[SFP(HStr_Key_EnableMode)] = enableMode; }
    if (contPowerUsage.value > FLT_EPSILON) {
        JsonObject contPowerUsageObj = objectOut.createNestedObject(SFP(HStr_Key_ContinousPowerUsage));
        contPowerUsage.toJSONObject(contPowerUsageObj);
    }
    if (railName[0]) { objectOut[SFP(HStr_Key_RailName)] = charsToString(railName, HYDRO_NAME_MAXSIZE); }
    if (reservoirName[0]) { objectOut[SFP(HStr_Key_ReservoirName)] = charsToString(reservoirName, HYDRO_NAME_MAXSIZE); }
}

void HydroActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroObjectData::fromJSONObject(objectIn);

    JsonObjectConst outputPinObj = objectIn[SFP(HStr_Key_OutputPin)];
    if (!outputPinObj.isNull()) { outputPin.fromJSONObject(outputPinObj); }
    enableMode = objectIn[SFP(HStr_Key_EnableMode)] | enableMode;
    JsonVariantConst contPowerUsageVar = objectIn[SFP(HStr_Key_ContinousPowerUsage)];
    if (!contPowerUsageVar.isNull()) { contPowerUsage.fromJSONVariant(contPowerUsageVar); }
    const char *railNameStr = objectIn[SFP(HStr_Key_RailName)];
    if (railNameStr && railNameStr[0]) { strncpy(railName, railNameStr, HYDRO_NAME_MAXSIZE); }
    const char *reservoirNameStr = objectIn[SFP(HStr_Key_ReservoirName)];
    if (reservoirNameStr && reservoirNameStr[0]) { strncpy(reservoirName, reservoirNameStr, HYDRO_NAME_MAXSIZE); }
}

HydroPumpActuatorData::HydroPumpActuatorData()
    : HydroActuatorData(), flowRateUnits(Hydro_UnitsType_Undefined), contFlowRate(), destReservoir{0}, flowRateSensor{0}
{
    _size = sizeof(*this);
}

void HydroPumpActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroActuatorData::toJSONObject(objectOut);

    if (flowRateUnits != Hydro_UnitsType_Undefined) { objectOut[SFP(HStr_Key_FlowRateUnits)] = unitsTypeToSymbol(flowRateUnits); }
    if (contFlowRate.value > FLT_EPSILON) {
        JsonObject contFlowRateObj = objectOut.createNestedObject(SFP(HStr_Key_ContinousFlowRate));
        contFlowRate.toJSONObject(contFlowRateObj);
    }
    if (destReservoir[0]) { objectOut[SFP(HStr_Key_OutputReservoir)] = charsToString(destReservoir, HYDRO_NAME_MAXSIZE); }
    if (flowRateSensor[0]) { objectOut[SFP(HStr_Key_FlowRateSensor)] = charsToString(flowRateSensor, HYDRO_NAME_MAXSIZE); }
}

void HydroPumpActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroActuatorData::fromJSONObject(objectIn);

    flowRateUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_FlowRateUnits)]);
    JsonVariantConst contFlowRateVar = objectIn[SFP(HStr_Key_ContinousFlowRate)];
    if (!contFlowRateVar.isNull()) { contFlowRate.fromJSONVariant(contFlowRateVar); }
    const char *destReservoirStr = objectIn[SFP(HStr_Key_OutputReservoir)];
    if (destReservoirStr && destReservoirStr[0]) { strncpy(destReservoir, destReservoirStr, HYDRO_NAME_MAXSIZE); }
    const char *flowRateSensorStr = objectIn[SFP(HStr_Key_FlowRateSensor)];
    if (flowRateSensorStr && flowRateSensorStr[0]) { strncpy(flowRateSensor, flowRateSensorStr, HYDRO_NAME_MAXSIZE); }
}
