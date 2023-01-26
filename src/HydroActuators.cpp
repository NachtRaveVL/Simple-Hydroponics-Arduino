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
                return new HydroPumpRelayActuator((const HydroPumpRelayActuatorData *)dataIn);
            case (int8_t)HydroActuator::Variable:
                return new HydroVariableActuator((const HydroActuatorData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroActivationHandle::HydroActivationHandle(HydroActuator *actuator, Hydro_DirectionMode directionIn, float intensityIn, millis_t forMillis, bool force)
    : actuator(actuator), direction(directionIn), intensity(constrain(intensityIn, 0.0f, 1.0f)), startMillis(0), durationMillis(forMillis), forced(force)
{
    if (actuator) { actuator->_handles.push_back(this); actuator->_needsUpdate = true; }
}

HydroActivationHandle::HydroActivationHandle(const HydroActivationHandle &handle)
    : actuator(handle.actuator), direction(handle.direction), intensity(constrain(handle.intensity, 0.0f, 1.0f)), forced(handle.forced),
      startMillis(handle.startMillis), durationMillis(handle.durationMillis)
{
    if (actuator) { actuator->_handles.push_back(this); }
}

HydroActivationHandle::~HydroActivationHandle()
{
    if (actuator) { actuator->_needsUpdate = true;
        for (auto handleIter = actuator->_handles.end() - 1; handleIter != actuator->_handles.begin() - 1; --handleIter) {
            if ((*handleIter) == this) {
                actuator->_handles.erase(handleIter);
                break;
            }
        }
        actuator = nullptr;
    }
}

HydroActivationHandle &HydroActivationHandle::operator=(const HydroActivationHandle &handle)
{
    if (handle.actuator != actuator) {
        if (actuator) { actuator->_needsUpdate = true;
            for (auto handleIter = actuator->_handles.end() - 1; handleIter != actuator->_handles.begin() - 1; --handleIter) {
                if ((*handleIter) == this) {
                    actuator->_handles.erase(handleIter);
                    break;
                }
            }
        }
        actuator = handle.actuator;
        if (actuator) { actuator->_handles.push_back(this); actuator->_needsUpdate = true; }
    }
    direction = handle.direction;
    intensity = constrain(handle.intensity, 0.0f, 1.0f);
    startMillis = 0;
    durationMillis = handle.durationMillis;
    forced = handle.forced;
    return *this;
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
    bool canEnable = _handles.size() && getCanEnable();
    for (auto handleIter = _handles.begin(); handleIter != _handles.end() && !canEnable; ++handleIter) {
        canEnable = (*handleIter)->forced;
    }

    // If enabled and shouldn't be (unless force enabled)
    if ((_enabled || _needsUpdate) && !canEnable) {
        _disableActuator();
        _needsUpdate = false;
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

        _enableActuator(constrain(drivingIntensity, -1.0f, 1.0f));
        _needsUpdate = false;
    }
}

bool HydroActuator::getCanEnable()
{
    if (getRail() && !getRail()->canActivate(this)) { return false; }
    if (getReservoir() && !getReservoir()->canActivate(this)) { return false; }
    return true;
}

void HydroActuator::setContinuousPowerUsage(float contPowerUsage, Hydro_UnitsType contPowerUsageUnits)
{
    _contPowerUsage.value = contPowerUsage;
    _contPowerUsage.units = contPowerUsageUnits;
    _contPowerUsage.updateTimestamp();
    _contPowerUsage.updateFrame(1);
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

bool HydroRelayActuator::_enableActuator(float intensity)
{
    if (_outputPin.isValid()) {
        bool enabledBefore = _enabled;

        if (!_enabled && !isFPEqual(intensity, 0.0f)) {
            _enabled = true;
            _outputPin.activate();
        }

        if (_enabled && !enabledBefore) {
            millis_t timeMillis = max(1, millis());
            for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                if ((*handleIter) && (*handleIter)->startMillis == 0) {
                    (*handleIter)->startMillis = timeMillis;
                }
            }

            getLoggerInstance()->logActivation(this);

            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<HydroActuator *>(getSharedPtr(), _activateSignal, this);
            #else
                _activateSignal.fire(this);
            #endif
        }
    }

    return _enabled;
}

void HydroRelayActuator::_disableActuator()
{
    if (_outputPin.isValid()) {
        bool enabledBefore = _enabled;

        if (_enabled) {
            _enabled = false;
            _outputPin.deactivate();
        }

        if (!_enabled && enabledBefore) {
            millis_t timeMillis = millis();
            for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                if ((*handleIter) && (*handleIter)->durationMillis) {
                    millis_t durationMillis = timeMillis - (*handleIter)->startMillis;
                    if (durationMillis >= (*handleIter)->durationMillis) {
                        (*handleIter)->durationMillis = 0;
                        (*handleIter)->actuator = nullptr;
                        handleIter = _handles.erase(handleIter) - 1;
                    } else {
                        (*handleIter)->durationMillis -= durationMillis;
                    }
                }
            }

            getLoggerInstance()->logDeactivation(this);

            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<HydroActuator *>(getSharedPtr(), _activateSignal, this);
            #else
                _activateSignal.fire(this);
            #endif
        }
    }
}

bool HydroRelayActuator::getCanEnable()
{
    if (HydroActuator::getCanEnable()) {
        return !getActuatorInWaterFromType(_id.objTypeAs.actuatorType) || !getReservoir() || !getReservoir()->isEmpty();
    }
    return false;
}

bool HydroRelayActuator::isEnabled(float tolerance) const
{
    return _enabled;
}

void HydroRelayActuator::saveToData(HydroData *dataOut)
{
    HydroActuator::saveToData(dataOut);

    _outputPin.saveToData(&((HydroActuatorData *)dataOut)->outputPin);
}


HydroPumpRelayActuator::HydroPumpRelayActuator(Hydro_ActuatorType actuatorType,
                                               Hydro_PositionIndex actuatorIndex,
                                               HydroDigitalPin outputPin,
                                               int classType)
    :  HydroRelayActuator(actuatorType, actuatorIndex, outputPin, classType),
       _flowRateUnits(defaultLiquidFlowUnits()), _flowRate(this), _destReservoir(this), _pumpVolumeAcc(0.0f), _pumpTimeBegMillis(0), _pumpTimeAccMillis(0)
{
    _flowRate.setMeasurementUnits(getFlowRateUnits());
}

HydroPumpRelayActuator::HydroPumpRelayActuator(const HydroPumpRelayActuatorData *dataIn)
    : HydroRelayActuator(dataIn), _pumpVolumeAcc(0.0f), _pumpTimeBegMillis(0), _pumpTimeAccMillis(0),
      _flowRateUnits(definedUnitsElse(dataIn->flowRateUnits, defaultLiquidFlowUnits())),
      _contFlowRate(&(dataIn->contFlowRate)),
      _flowRate(this), _destReservoir(this)
{
    _flowRate.setMeasurementUnits(getFlowRateUnits());
    _destReservoir.setObject(dataIn->destReservoir);
    _flowRate.setObject(dataIn->flowRateSensor);
}

void HydroPumpRelayActuator::update()
{
    HydroActuator::update();

    _destReservoir.resolve();

    _flowRate.updateIfNeeded(true);

    if (_pumpTimeAccMillis) {
        millis_t timeMillis = millis();
        millis_t pumpMillis = timeMillis - _pumpTimeAccMillis;
        if (pumpMillis >= HYDRO_ACT_PUMPCALC_MINWRTMILLIS) {
            handlePumpTime(pumpMillis);
            _pumpTimeAccMillis = max(1, timeMillis);
        }
    }

    if (_enabled) { pollPumpingSensors(); }
}

bool HydroPumpRelayActuator::getCanEnable()
{
    if (HydroRelayActuator::getCanEnable()) {
        auto sourceRes = getInputReservoir();
        auto destRes = getOutputReservoir();
        return (!sourceRes || !sourceRes->isEmpty()) && (!destRes || !destRes->isFilled());
    }
    return false;
}

bool HydroPumpRelayActuator::_enableActuator(float intensity)
{
    bool enabledBefore = _enabled;
    millis_t timeMillis = millis();

    HydroRelayActuator::_enableActuator(intensity);

    if (_enabled && !enabledBefore) {
        _pumpVolumeAcc = 0;
        _pumpTimeBegMillis = _pumpTimeAccMillis = max(1, timeMillis);
    }

    return _enabled;
}

void HydroPumpRelayActuator::_disableActuator()
{
    bool enabledBefore = _enabled;
    millis_t timeMillis = millis();

    HydroRelayActuator::_disableActuator();

    if (!_enabled && enabledBefore) {
        millis_t pumpMillis = timeMillis - _pumpTimeAccMillis;
        if (pumpMillis) { handlePumpTime(pumpMillis); }
        _pumpTimeAccMillis = 0;
        pumpMillis = timeMillis - _pumpTimeBegMillis;
        uint8_t addDecPlaces = getActuatorType() == Hydro_ActuatorType_PeristalticPump ? 2 : 1;

        getLoggerInstance()->logStatus(this, SFP(HStr_Log_MeasuredPumping));
        if (getInputReservoir()) { getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Source_Reservoir), getInputReservoir()->getKeyString()); }
        if (getOutputReservoir()) { getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Destination_Reservoir), getOutputReservoir()->getKeyString()); }
        getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Vol_Measured), measurementToString(_pumpVolumeAcc, baseUnitsFromRate(getFlowRateUnits()), addDecPlaces));
        getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Time_Measured), roundToString(pumpMillis / 1000.0f, 1), String('s'));
    }
}

bool HydroPumpRelayActuator::canPump(float volume, Hydro_UnitsType volumeUnits)
{
    if (getReservoir() && _contFlowRate.value > FLT_EPSILON) {
        auto waterVolume = getReservoir()->getWaterVolume().getMeasurement();
        convertUnits(&volume, &volumeUnits, waterVolume.units);
        return volume <= waterVolume.value + FLT_EPSILON;
    }
    return false;
}

bool HydroPumpRelayActuator::pump(float volume, Hydro_UnitsType volumeUnits)
{
    if (getReservoir() && _contFlowRate.value > FLT_EPSILON) {
        convertUnits(&volume, &volumeUnits, baseUnitsFromRate(getFlowRateUnits()));
        return pump((millis_t)((volume / _contFlowRate.value) * secondsToMillis(SECS_PER_MIN)));
    }
    return false;
}

bool HydroPumpRelayActuator::canPump(millis_t timeMillis)
{
    if (getReservoir() && _contFlowRate.value > FLT_EPSILON) {
        return canPump(_contFlowRate.value * (timeMillis / (float)secondsToMillis(SECS_PER_MIN)), baseUnitsFromRate(getFlowRateUnits()));
    }
    return false;
}

bool HydroPumpRelayActuator::pump(millis_t timeMillis)
{
    if (getReservoir()) {
        #ifdef HYDRO_USE_MULTITASKING
            if (isValidTask(scheduleActuatorTimedEnableOnce(::getSharedPtr<HydroActuator>(this), timeMillis))) {
                getLoggerInstance()->logStatus(this, SFP(HStr_Log_CalculatedPumping));
                if (getInputReservoir()) { getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Source_Reservoir), getInputReservoir()->getKeyString()); }
                if (getOutputReservoir()) { getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Destination_Reservoir), getOutputReservoir()->getKeyString()); }
                if (_contFlowRate.value > FLT_EPSILON) {
                    uint8_t addDecPlaces = getActuatorType() == Hydro_ActuatorType_PeristalticPump ? 2 : 1;
                    getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Vol_Calculated), measurementToString(_contFlowRate.value * (timeMillis / (float)secondsToMillis(SECS_PER_MIN)), baseUnitsFromRate(getFlowRateUnits()), addDecPlaces));
                }
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Time_Calculated), roundToString(timeMillis / 1000.0f, 1), String('s'));
                return true;
            }
        #else
            getLoggerInstance()->logStatus(this, SFP(HStr_Log_CalculatedPumping));
            if (getInputReservoir()) { getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Source_Reservoir), getInputReservoir()->getKeyString()); }
            if (getOutputReservoir()) { getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Destination_Reservoir), getOutputReservoir()->getKeyString()); }
            if (_contFlowRate.value > FLT_EPSILON) {
                uint8_t addDecPlaces = getActuatorType() == Hydro_ActuatorType_PeristalticPump ? 2 : 1;
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Vol_Calculated), measurementToString(_contFlowRate.value * (timeMillis / (float)secondsToMillis(SECS_PER_MIN)), baseUnitsFromRate(getFlowRateUnits()), addDecPlaces));
            }
            getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Time_Calculated), roundToString(timeMillis / 1000.0f, 1), String('s'));
            enableActuator();
            delayFine(timeMillis);
            disableActuator();
            return true;
        #endif
    }
    return false;
}

HydroAttachment &HydroPumpRelayActuator::getParentReservoir(bool resolve)
{
    return HydroActuator::getParentReservoir(resolve);
}

HydroAttachment &HydroPumpRelayActuator::getDestinationReservoir(bool resolve)
{
    if (resolve) { _destReservoir.resolve(); }
    return _destReservoir;
}

void HydroPumpRelayActuator::setFlowRateUnits(Hydro_UnitsType flowRateUnits)
{
    if (_flowRateUnits != flowRateUnits) {
        _flowRateUnits = flowRateUnits;

        convertUnits(&_contFlowRate, getFlowRateUnits());
        _flowRate.setMeasurementUnits(getFlowRateUnits());
    }
}

Hydro_UnitsType HydroPumpRelayActuator::getFlowRateUnits() const
{
    return definedUnitsElse(_flowRateUnits, defaultLiquidFlowUnits());
}

void HydroPumpRelayActuator::setContinuousFlowRate(float contFlowRate, Hydro_UnitsType contFlowRateUnits)
{
    _contFlowRate.value = contFlowRate;
    _contFlowRate.units = contFlowRateUnits;
    _contFlowRate.updateTimestamp();
    _contFlowRate.updateFrame(1);

    convertUnits(&_contFlowRate, getFlowRateUnits());
}

void HydroPumpRelayActuator::setContinuousFlowRate(HydroSingleMeasurement contFlowRate)
{
    _contFlowRate = contFlowRate;
    _contFlowRate.setMinFrame(1);

    convertUnits(&_contFlowRate, getFlowRateUnits());
}

const HydroSingleMeasurement &HydroPumpRelayActuator::getContinuousFlowRate()
{
    return _contFlowRate;
}

HydroSensorAttachment &HydroPumpRelayActuator::getFlowRate(bool poll)
{
    _flowRate.updateIfNeeded(poll);
    return _flowRate;
}

void HydroPumpRelayActuator::saveToData(HydroData *dataOut)
{
    HydroRelayActuator::saveToData(dataOut);

    ((HydroPumpRelayActuatorData *)dataOut)->flowRateUnits = _flowRateUnits;
    if (_contFlowRate.frame) {
        _contFlowRate.saveToData(&(((HydroPumpRelayActuatorData *)dataOut)->contFlowRate));
    }
    if (_destReservoir.getId()) {
        strncpy(((HydroPumpRelayActuatorData *)dataOut)->destReservoir, _destReservoir.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_flowRate.getId()) {
        strncpy(((HydroPumpRelayActuatorData *)dataOut)->flowRateSensor, _flowRate.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
}

void HydroPumpRelayActuator::pollPumpingSensors()
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

void HydroPumpRelayActuator::handlePumpTime(millis_t timeMillis)
{
    if (getInputReservoir() != getOutputReservoir()) {
        float flowRateVal = _flowRate.getMeasurementFrame() && getFlowRateSensor() && !_flowRate->needsPolling(HYDRO_ACT_PUMPCALC_MAXFRAMEDIFF) &&
                            _flowRate.getMeasurementValue() >= (_contFlowRate.value * HYDRO_ACT_PUMPCALC_MINFLOWRATE) - FLT_EPSILON ? _flowRate.getMeasurementValue() : _contFlowRate.value;
        float volumePumped = flowRateVal * (timeMillis / (float)secondsToMillis(SECS_PER_MIN));
        _pumpVolumeAcc += volumePumped;

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
}


HydroVariableActuator::HydroVariableActuator(Hydro_ActuatorType actuatorType,
                                             Hydro_PositionIndex actuatorIndex,
                                             HydroAnalogPin outputPin,
                                             int classType)
    : HydroActuator(actuatorType, actuatorIndex, classType),
      _outputPin(outputPin), _pwmAmount(0.0f)
{
    HYDRO_HARD_ASSERT(_outputPin.isValid(), SFP(HStr_Err_InvalidPinOrType));
    _outputPin.init();
    _outputPin.analogWrite_raw(0);
}

HydroVariableActuator::HydroVariableActuator(const HydroActuatorData *dataIn)
    : HydroActuator(dataIn),
      _outputPin(&dataIn->outputPin), _pwmAmount(0.0f)
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

bool HydroVariableActuator::_enableActuator(float intensity)
{
    if (_outputPin.isValid()) {
        bool enabledBefore = _enabled;

        if (!_enabled || (_enabled && !isFPEqual(_pwmAmount, intensity))) {
            _enabled = true;
            _pwmAmount = constrain(intensity, 0.0f, 1.0f);
            _outputPin.analogWrite(_pwmAmount);
        }

        if (_enabled && !enabledBefore) {
            getLoggerInstance()->logActivation(this);

            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<HydroActuator *>(getSharedPtr(), _activateSignal, this);
            #else
                _activateSignal.fire(this);
            #endif
        }
    }

    return _enabled;
}

void HydroVariableActuator::_disableActuator()
{
    if (_outputPin.isValid()) {
        bool enabledBefore = _enabled;

        if (_enabled) {
            _enabled = false;
            _outputPin.analogWrite_raw(0);
        }

        if (!_enabled && enabledBefore) {
            getLoggerInstance()->logDeactivation(this);

            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<HydroActuator *>(getSharedPtr(), _activateSignal, this);
            #else
                _activateSignal.fire(this);
            #endif
        }
    }
}

bool HydroVariableActuator::isEnabled(float tolerance) const
{
    return _enabled && _pwmAmount >= tolerance - FLT_EPSILON;
}

int HydroVariableActuator::getPWMAmount(int) const
{
    return _outputPin.bitRes.inverseTransform(_pwmAmount);
}

void HydroVariableActuator::setPWMAmount(float amount)
{
    _pwmAmount = constrain(amount, 0.0f, 1.0f);

    if (_enabled) { _outputPin.analogWrite(_pwmAmount); }
}

void HydroVariableActuator::setPWMAmount(int amount)
{
    _pwmAmount = _outputPin.bitRes.transform(amount);

    if (_enabled) { _outputPin.analogWrite(_pwmAmount); }
}

void HydroVariableActuator::saveToData(HydroData *dataOut)
{
    HydroActuator::saveToData(dataOut);

    _outputPin.saveToData(&((HydroActuatorData *)dataOut)->outputPin);
}


HydroActuatorData::HydroActuatorData()
    : HydroObjectData(), outputPin(), contPowerUsage(), railName{0}, reservoirName{0}, enableMode(Hydro_EnableMode_Undefined)
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
    if (contPowerUsage.value > FLT_EPSILON) {
        JsonObject contPowerUsageObj = objectOut.createNestedObject(SFP(HStr_Key_ContPowerUsage));
        contPowerUsage.toJSONObject(contPowerUsageObj);
    }
    if (railName[0]) { objectOut[SFP(HStr_Key_RailName)] = charsToString(railName, HYDRO_NAME_MAXSIZE); }
    if (reservoirName[0]) { objectOut[SFP(HStr_Key_ReservoirName)] = charsToString(reservoirName, HYDRO_NAME_MAXSIZE); }
    if (enableMode != Hydro_EnableMode_Undefined) { objectOut[SFP(HStr_Key_EnableMode)] = enableMode; }
}

void HydroActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroObjectData::fromJSONObject(objectIn);

    JsonObjectConst outputPinObj = objectIn[SFP(HStr_Key_OutputPin)];
    if (!outputPinObj.isNull()) { outputPin.fromJSONObject(outputPinObj); }
    JsonVariantConst contPowerUsageVar = objectIn[SFP(HStr_Key_ContPowerUsage)];
    if (!contPowerUsageVar.isNull()) { contPowerUsage.fromJSONVariant(contPowerUsageVar); }
    const char *railNameStr = objectIn[SFP(HStr_Key_RailName)];
    if (railNameStr && railNameStr[0]) { strncpy(railName, railNameStr, HYDRO_NAME_MAXSIZE); }
    const char *reservoirNameStr = objectIn[SFP(HStr_Key_ReservoirName)];
    if (reservoirNameStr && reservoirNameStr[0]) { strncpy(reservoirName, reservoirNameStr, HYDRO_NAME_MAXSIZE); }
    enableMode = objectIn[SFP(HStr_Key_EnableMode)] | enableMode;
}

HydroPumpRelayActuatorData::HydroPumpRelayActuatorData()
    : HydroActuatorData(), flowRateUnits(Hydro_UnitsType_Undefined), contFlowRate(), destReservoir{0}, flowRateSensor{0}
{
    _size = sizeof(*this);
}

void HydroPumpRelayActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroActuatorData::toJSONObject(objectOut);

    if (flowRateUnits != Hydro_UnitsType_Undefined) { objectOut[SFP(HStr_Key_FlowRateUnits)] = unitsTypeToSymbol(flowRateUnits); }
    if (contFlowRate.value > FLT_EPSILON) {
        JsonObject contFlowRateObj = objectOut.createNestedObject(SFP(HStr_Key_ContFlowRate));
        contFlowRate.toJSONObject(contFlowRateObj);
    }
    if (destReservoir[0]) { objectOut[SFP(HStr_Key_OutputReservoir)] = charsToString(destReservoir, HYDRO_NAME_MAXSIZE); }
    if (flowRateSensor[0]) { objectOut[SFP(HStr_Key_FlowRateSensor)] = charsToString(flowRateSensor, HYDRO_NAME_MAXSIZE); }
}

void HydroPumpRelayActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroActuatorData::fromJSONObject(objectIn);

    flowRateUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_FlowRateUnits)]);
    JsonVariantConst contFlowRateVar = objectIn[SFP(HStr_Key_ContFlowRate)];
    if (!contFlowRateVar.isNull()) { contFlowRate.fromJSONVariant(contFlowRateVar); }
    const char *destReservoirStr = objectIn[SFP(HStr_Key_OutputReservoir)];
    if (destReservoirStr && destReservoirStr[0]) { strncpy(destReservoir, destReservoirStr, HYDRO_NAME_MAXSIZE); }
    const char *flowRateSensorStr = objectIn[SFP(HStr_Key_FlowRateSensor)];
    if (flowRateSensorStr && flowRateSensorStr[0]) { strncpy(flowRateSensor, flowRateSensorStr, HYDRO_NAME_MAXSIZE); }
}
