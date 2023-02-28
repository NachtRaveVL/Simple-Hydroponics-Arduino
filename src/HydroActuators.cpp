/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Actuators
*/

#include "Hydruino.h"

HydroActuator *newActuatorObjectFromData(const HydroActuatorData *dataIn)
{
    if (dataIn && isValidType(dataIn->id.object.idType)) return nullptr;
    HYDRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
            case (hid_t)HydroActuator::Relay:
                return new HydroRelayActuator((const HydroActuatorData *)dataIn);
            case (hid_t)HydroActuator::RelayPump:
                return new HydroRelayPumpActuator((const HydroPumpActuatorData *)dataIn);
            case (hid_t)HydroActuator::Variable:
                return new HydroVariableActuator((const HydroActuatorData *)dataIn);
            case (hid_t)HydroActuator::VariablePump:
                //return new HydroVariablePumpActuator((const HydroPumpActuatorData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroActuator::HydroActuator(Hydro_ActuatorType actuatorType, hposi_t actuatorIndex, int classTypeIn)
    : HydroObject(HydroIdentity(actuatorType, actuatorIndex)), classType((typeof(classType))classTypeIn),
      _enabled(false), _enableMode(Hydro_EnableMode_Undefined), _parentRail(this), _parentReservoir(this), _needsUpdate(false)
{ ; }

HydroActuator::HydroActuator(const HydroActuatorData *dataIn)
    : HydroObject(dataIn), classType((typeof(classType))dataIn->id.object.classType),
      _enabled(false), _enableMode(dataIn->enableMode),
      _contPowerUsage(&(dataIn->contPowerUsage)),
      _parentRail(this), _parentReservoir(this), _needsUpdate(false)
{
    _parentRail.initObject(dataIn->railName);
    _parentReservoir.initObject(dataIn->reservoirName);
}

void HydroActuator::update()
{
    HydroObject::update();

    _parentRail.resolve();
    _parentReservoir.resolve();

    millis_t time = nzMillis();

    // Update running handles and elapse them as needed, determine forced status, and remove invalid/finished handles
    bool forced = false;
    if (_handles.size()) {
        for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
            if (_enabled && (*handleIter)->isActive()) {
                (*handleIter)->elapseTo(time);
            }
            if ((*handleIter)->actuator.get() != this || !(*handleIter)->isValid() || (*handleIter)->isDone()) {
                if ((*handleIter)->actuator.get() == this) { (*handleIter)->actuator = nullptr; }
                handleIter = _handles.erase(handleIter) - 1;
                setNeedsUpdate();
                continue;
            }
            forced |= (*handleIter)->isForced();
        }
    }

    // Enablement checking
    bool canEnable = _handles.size() && (forced || getCanEnable());

    if (!canEnable && (_enabled || _needsUpdate)) { // If enabled and shouldn't be (unless force enabled)
        _disableActuator();
    } else if (canEnable && (!_enabled || _needsUpdate)) { // If can enable and isn't (maybe force enabled)
        float drivingIntensity = 0.0f;

        // Determine what driving intensity [-1,1] actuator should use
        switch (_enableMode) {
            case Hydro_EnableMode_Highest:
            case Hydro_EnableMode_DescOrder: {
                drivingIntensity = -__FLT_MAX__;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        auto handleIntensity = (*handleIter)->getDriveIntensity();
                        if (handleIntensity > drivingIntensity) { drivingIntensity = handleIntensity; }
                    }
                }
            } break;

            case Hydro_EnableMode_Lowest:
            case Hydro_EnableMode_AscOrder: {
                drivingIntensity = __FLT_MAX__;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        auto handleIntensity = (*handleIter)->getDriveIntensity();
                        if (handleIntensity < drivingIntensity) { drivingIntensity = handleIntensity; }
                    }
                }
            } break;

            case Hydro_EnableMode_Average: {
                int handleCount = 0;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        drivingIntensity += (*handleIter)->getDriveIntensity();
                        ++handleCount;
                    }
                }
                if (handleCount) { drivingIntensity /= handleCount; }
            } break;

            case Hydro_EnableMode_Multiply: {
                drivingIntensity = (*_handles.begin())->getDriveIntensity();
                for (auto handleIter = _handles.begin() + 1; handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        drivingIntensity *= (*handleIter)->getDriveIntensity();
                    }
                }
            } break;

            case Hydro_EnableMode_InOrder: {
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        drivingIntensity += (*handleIter)->getDriveIntensity();
                        break;
                    }
                }
            } break;

            case Hydro_EnableMode_RevOrder: {
                for (auto handleIter = _handles.end() - 1; handleIter != _handles.begin() - 1; --handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        drivingIntensity += (*handleIter)->getDriveIntensity();
                        break;
                    }
                }
            } break;

            default:
                break;
        }

        // Enable/disable activation handles as needed (serial modes only select 1 at a time)
        switch (_enableMode) {
            case Hydro_EnableMode_InOrder:
            case Hydro_EnableMode_DescOrder: {
                bool selected = false;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if (!selected && (*handleIter)->isValid() && !(*handleIter)->isDone() && isFPEqual((*handleIter)->activation.intensity, getDriveIntensity())) {
                        selected = true; (*handleIter)->checkTime = time;
                    } else if ((*handleIter)->checkTime != 0) {
                        (*handleIter)->checkTime = 0;
                    }
                }
            } break;

            case Hydro_EnableMode_RevOrder:
            case Hydro_EnableMode_AscOrder: {
                bool selected = false;
                for (auto handleIter = _handles.end() - 1; handleIter != _handles.begin() - 1; --handleIter) {
                    if (!selected && (*handleIter)->isValid() && !(*handleIter)->isDone() && isFPEqual((*handleIter)->activation.intensity, getDriveIntensity())) {
                        selected = true; (*handleIter)->checkTime = time;
                    } else if ((*handleIter)->checkTime != 0) {
                        (*handleIter)->checkTime = 0;
                    }
                }
            } break;

            default: {
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone() && (*handleIter)->checkTime == 0) {
                        (*handleIter)->checkTime = time;
                    }
                }
            } break;
        }

        _enableActuator(drivingIntensity);
    }
    _needsUpdate = false;
}

bool HydroActuator::getCanEnable()
{
    if (getParentRail() && !getParentRail()->canActivate(this)) { return false; }
    if (getParentReservoir() && !getParentReservoir()->canActivate(this)) { return false; }
    return true;
}

void HydroActuator::setContinuousPowerUsage(HydroSingleMeasurement contPowerUsage)
{
    _contPowerUsage = contPowerUsage;
    _contPowerUsage.setMinFrame(1);
    bumpRevisionIfNeeded();
}

const HydroSingleMeasurement &HydroActuator::getContinuousPowerUsage()
{
    return _contPowerUsage;
}

HydroAttachment &HydroActuator::getParentRailAttachment()
{
    return _parentRail;
}

HydroAttachment &HydroActuator::getParentReservoirAttachment()
{
    return _parentReservoir;
}

void HydroActuator::setUserCalibrationData(HydroCalibrationData *userCalibrationData)
{
    if (_calibrationData && _calibrationData != userCalibrationData) { bumpRevisionIfNeeded(); }
    if (getController()) {
        if (userCalibrationData && getController()->setUserCalibrationData(userCalibrationData)) {
            _calibrationData = getController()->getUserCalibrationData(_id.key);
        } else if (!userCalibrationData && _calibrationData && getController()->dropUserCalibrationData(_calibrationData)) {
            _calibrationData = nullptr;
        }
    } else {
        _calibrationData = userCalibrationData;
    }
}

Signal<HydroActuator *, HYDRO_ACTUATOR_SIGNAL_SLOTS> &HydroActuator::getActivationSignal()
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
    if (_contPowerUsage.isSet()) {
        _contPowerUsage.saveToData(&(((HydroActuatorData *)dataOut)->contPowerUsage));
    }
    if (_parentReservoir.isSet()) {
        strncpy(((HydroActuatorData *)dataOut)->reservoirName, _parentReservoir.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_parentRail.isSet()) {
        strncpy(((HydroActuatorData *)dataOut)->railName, _parentRail.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    ((HydroActuatorData *)dataOut)->enableMode = _enableMode;
}

void HydroActuator::handleActivation()
{
    if (_enabled) {
        getLogger()->logActivation(this);
    } else {
        for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
            if ((*handleIter)->checkTime) { (*handleIter)->checkTime = 0; }
        }

        getLogger()->logDeactivation(this);
    }

    #ifdef HYDRO_USE_MULTITASKING
        scheduleSignalFireOnce<HydroActuator *>(getSharedPtr(), _activateSignal, this);
    #else
        _activateSignal.fire(this);
    #endif
}


HydroRelayActuator::HydroRelayActuator(Hydro_ActuatorType actuatorType, hposi_t actuatorIndex, HydroDigitalPin outputPin, int classType)
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

float HydroRelayActuator::getDriveIntensity() const
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
    //intensity = roundf(intensity); // unnecessary frn

    if (_outputPin.isValid()) {
        if (intensity > FLT_EPSILON) {
            _enabled = true;
            _outputPin.activate();
        } else {
            _outputPin.deactivate();
        }

        if (!wasEnabled) { handleActivation(); }
    }
}

void HydroRelayActuator::_disableActuator()
{
    bool wasEnabled = _enabled;

    if (_outputPin.isValid()) {
        _enabled = false;
        _outputPin.deactivate();

        if (wasEnabled) { handleActivation(); }
    }
}

void HydroRelayActuator::saveToData(HydroData *dataOut)
{
    HydroActuator::saveToData(dataOut);

    _outputPin.saveToData(&((HydroActuatorData *)dataOut)->outputPin);
}


HydroRelayPumpActuator::HydroRelayPumpActuator(Hydro_ActuatorType actuatorType, hposi_t actuatorIndex, HydroDigitalPin outputPin, int classType)
    : HydroRelayActuator(actuatorType, actuatorIndex, outputPin, classType),
      HydroFlowRateUnitsInterfaceStorage(defaultFlowRateUnits()),
      _flowRate(this), _destReservoir(this),
      _pumpVolumeAccum(0.0f), _pumpTimeStart(0), _pumpTimeAccum(0)
{
    _flowRate.setMeasurementUnits(getFlowRateUnits());
}

HydroRelayPumpActuator::HydroRelayPumpActuator(const HydroPumpActuatorData *dataIn)
    : HydroRelayActuator(dataIn),
      HydroFlowRateUnitsInterfaceStorage(definedUnitsElse(dataIn->flowRateUnits, defaultFlowRateUnits())),
      _pumpVolumeAccum(0.0f), _pumpTimeStart(0), _pumpTimeAccum(0),
      _contFlowRate(&(dataIn->contFlowRate)),
      _flowRate(this), _destReservoir(this)
{
    _flowRate.setMeasurementUnits(getFlowRateUnits());
    _destReservoir.initObject(dataIn->destReservoir);
    _flowRate.initObject(dataIn->flowRateSensor);
}

void HydroRelayPumpActuator::update()
{
    HydroActuator::update();

    _destReservoir.resolve();

    _flowRate.updateIfNeeded(true);

    if (_pumpTimeStart) {
        millis_t time = nzMillis();
        millis_t duration = time - _pumpTimeStart;
        if (duration >= HYDRO_ACT_PUMPCALC_UPDATEMS) {
            handlePumpTime(time);
        }
    }
}

bool HydroRelayPumpActuator::getCanEnable()
{
    if (HydroRelayActuator::getCanEnable()) {
        if (getDestinationReservoir() && !getDestinationReservoir()->canActivate(this)) { return false; }
        return true;
    }
    return false;
}

void HydroRelayPumpActuator::handleActivation()
{
    millis_t time = nzMillis();
    HydroActuator::handleActivation();

    if (_enabled) {
        _pumpVolumeAccum = 0;
        _pumpTimeStart = _pumpTimeAccum = time;
    } else {
        if (_pumpTimeAccum < time) { handlePumpTime(time); }
        _pumpTimeAccum = 0;
        float duration = time - _pumpTimeStart;
        uint8_t addDecPlaces = getActuatorType() == Hydro_ActuatorType_PeristalticPump ? 2 : 1;

        getLogger()->logStatus(this, SFP(HStr_Log_MeasuredPumping));
        if (getSourceReservoir()) { getLogger()->logMessage(SFP(HStr_Log_Field_Source_Reservoir), getSourceReservoir()->getKeyString()); }
        if (getDestinationReservoir()) { getLogger()->logMessage(SFP(HStr_Log_Field_Destination_Reservoir), getDestinationReservoir()->getKeyString()); }
        getLogger()->logMessage(SFP(HStr_Log_Field_Vol_Measured), measurementToString(_pumpVolumeAccum, baseUnits(getFlowRateUnits()), addDecPlaces));
        getLogger()->logMessage(SFP(HStr_Log_Field_Time_Measured), roundToString(duration / 1000.0f, 1), String('s'));
    }
}

bool HydroRelayPumpActuator::canPump(float volume, Hydro_UnitsType volumeUnits)
{
    if (getSourceReservoir() && _contFlowRate.value > FLT_EPSILON) {
        auto waterVolume = getSourceReservoir()->getWaterVolumeSensorAttachment().getMeasurement().asUnits(getVolumeUnits());
        return volume <= waterVolume.value + FLT_EPSILON;
    }
    return false;
}

HydroActivationHandle HydroRelayPumpActuator::pump(float volume, Hydro_UnitsType volumeUnits)
{
    if (getSourceReservoir() && _contFlowRate.value > FLT_EPSILON) {
        convertUnits(&volume, &volumeUnits, getVolumeUnits());
        return pump((millis_t)((volume / _contFlowRate.value) * secondsToMillis(SECS_PER_MIN)));
    }
    return HydroActivationHandle();
}

bool HydroRelayPumpActuator::canPump(millis_t time)
{
    if (getSourceReservoir() && _contFlowRate.value > FLT_EPSILON) {
        return canPump(_contFlowRate.value * (time / (float)secondsToMillis(SECS_PER_MIN)), getVolumeUnits());
    }
    return false;
}

HydroActivationHandle HydroRelayPumpActuator::pump(millis_t time)
{
    if (getSourceReservoir()) {
        #ifdef HYDRO_USE_MULTITASKING
            getLogger()->logStatus(this, SFP(HStr_Log_CalculatedPumping));
            if (getSourceReservoir()) { getLogger()->logMessage(SFP(HStr_Log_Field_Source_Reservoir), getSourceReservoir()->getKeyString()); }
            if (getDestinationReservoir()) { getLogger()->logMessage(SFP(HStr_Log_Field_Destination_Reservoir), getDestinationReservoir()->getKeyString()); }
            if (_contFlowRate.value > FLT_EPSILON) {
                uint8_t addDecPlaces = getActuatorType() == Hydro_ActuatorType_PeristalticPump ? 2 : 1;
                getLogger()->logMessage(SFP(HStr_Log_Field_Vol_Calculated), measurementToString(_contFlowRate.value * (time / (float)secondsToMillis(SECS_PER_MIN)), baseUnits(getFlowRateUnits()), addDecPlaces));
            }
            getLogger()->logMessage(SFP(HStr_Log_Field_Time_Calculated), roundToString(time / 1000.0f, 1), String('s'));
            return enableActuator(time);
        #else
            getLogger()->logStatus(this, SFP(HStr_Log_CalculatedPumping));
            if (getSourceReservoir()) { getLogger()->logMessage(SFP(HStr_Log_Field_Source_Reservoir), getSourceReservoir()->getKeyString()); }
            if (getOutputReservoir()) { getLogger()->logMessage(SFP(HStr_Log_Field_Destination_Reservoir), getOutputReservoir()->getKeyString()); }
            if (_contFlowRate.value > FLT_EPSILON) {
                uint8_t addDecPlaces = getActuatorType() == Hydro_ActuatorType_PeristalticPump ? 2 : 1;
                getLogger()->logMessage(SFP(HStr_Log_Field_Vol_Calculated), measurementToString(_contFlowRate.value * (time / (float)secondsToMillis(SECS_PER_MIN)), baseUnits(getFlowRateUnits()), addDecPlaces));
            }
            getLogger()->logMessage(SFP(HStr_Log_Field_Time_Calculated), roundToString(time / 1000.0f, 1), String('s'));
            return enableActuator(time);
        #endif
    }
    return HydroActivationHandle();
}

HydroAttachment &HydroRelayPumpActuator::getSourceReservoirAttachment()
{
    return _parentReservoir;
}

HydroAttachment &HydroRelayPumpActuator::getDestinationReservoirAttachment()
{
    return _destReservoir;
}

void HydroRelayPumpActuator::setFlowRateUnits(Hydro_UnitsType flowRateUnits)
{
    if (_flowRateUnits != flowRateUnits) {
        _flowRateUnits = flowRateUnits;

        convertUnits(&_contFlowRate, getFlowRateUnits());
        _flowRate.setMeasurementUnits(getFlowRateUnits());
        bumpRevisionIfNeeded();
    }
}

void HydroRelayPumpActuator::setContinuousFlowRate(HydroSingleMeasurement contFlowRate)
{
    _contFlowRate = contFlowRate;
    _contFlowRate.setMinFrame(1);

    convertUnits(&_contFlowRate, getFlowRateUnits());
    bumpRevisionIfNeeded();
}

const HydroSingleMeasurement &HydroRelayPumpActuator::getContinuousFlowRate()
{
    return _contFlowRate;
}

HydroSensorAttachment &HydroRelayPumpActuator::getFlowRateSensorAttachment()
{
    return _flowRate;
}

void HydroRelayPumpActuator::saveToData(HydroData *dataOut)
{
    HydroRelayActuator::saveToData(dataOut);

    ((HydroPumpActuatorData *)dataOut)->flowRateUnits = _flowRateUnits;
    if (_contFlowRate.isSet()) {
        _contFlowRate.saveToData(&(((HydroPumpActuatorData *)dataOut)->contFlowRate));
    }
    if (_destReservoir.isSet()) {
        strncpy(((HydroPumpActuatorData *)dataOut)->destReservoir, _destReservoir.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_flowRate.isSet()) {
        strncpy(((HydroPumpActuatorData *)dataOut)->flowRateSensor, _flowRate.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
}

void HydroRelayPumpActuator::handlePumpTime(millis_t time)
{
    auto flowRate = getFlowRateSensor(true) ? _flowRate.getMeasurement() : _contFlowRate;

    if (flowRate.value >= (_contFlowRate.value * HYDRO_ACT_PUMPCALC_MINFLOWRATE) - FLT_EPSILON) {
        auto timeDelta = (time - _pumpTimeAccum) / (float)secondsToMillis(SECS_PER_MIN);
        auto volDelta = flowRate.value * timeDelta;
        _pumpVolumeAccum += volDelta;

        auto srcRes = getSourceReservoir();
        auto destRes = getDestinationReservoir();
        if (srcRes != destRes) {
            if (srcRes && srcRes->isAnyFluidClass()) {
                auto srcFluidRes = static_pointer_cast<HydroFluidReservoir>(srcRes);

                if (srcFluidRes && !srcFluidRes->getWaterVolumeSensorAttachment()) { // only report if there isn't a volume sensor already doing it
                    auto volume = srcFluidRes->getWaterVolumeSensorAttachment().getMeasurement(true).asUnits(getVolumeUnits());
                    volume.value -= volDelta;
                    srcFluidRes->getWaterVolumeSensorAttachment().setMeasurement(volume);
                }
            }

            if (destRes && destRes->isAnyFluidClass()) {
                auto destFluidRes = static_pointer_cast<HydroFluidReservoir>(destRes);

                if (destFluidRes && !destFluidRes->getWaterVolumeSensorAttachment()) { // only report if there isn't a volume sensor already doing it
                    auto volume = destFluidRes->getWaterVolumeSensorAttachment().getMeasurement(true).asUnits(getVolumeUnits());
                    volume.value += volDelta;
                    destFluidRes->getWaterVolumeSensorAttachment().setMeasurement(volume);
                }
            }
        }
    }

    _pumpTimeAccum = time;
}


HydroVariableActuator::HydroVariableActuator(Hydro_ActuatorType actuatorType, hposi_t actuatorIndex, HydroAnalogPin outputPin, int classType)
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

float HydroVariableActuator::getDriveIntensity() const
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

    if (_outputPin.isValid()) {
        _enabled = true;
        _outputPin.analogWrite((_intensity = intensity));

        if (!wasEnabled) { handleActivation(); }
    }
}

void HydroVariableActuator::_disableActuator()
{
    bool wasEnabled = _enabled;

    if (_outputPin.isValid()) {
        _enabled = false;
        _outputPin.analogWrite_raw(0);

        if (wasEnabled) { handleActivation(); }
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
    if (enableMode != Hydro_EnableMode_Undefined) { objectOut[SFP(HStr_Key_EnableMode)] = enableModeToString(enableMode); }
    if (contPowerUsage.value > FLT_EPSILON) {
        JsonObject contPowerUsageObj = objectOut.createNestedObject(SFP(HStr_Key_ContinuousPowerUsage));
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
    enableMode = enableModeFromString(objectIn[SFP(HStr_Key_EnableMode)]);
    JsonVariantConst contPowerUsageVar = objectIn[SFP(HStr_Key_ContinuousPowerUsage)];
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
        JsonObject contFlowRateObj = objectOut.createNestedObject(SFP(HStr_Key_ContinuousFlowRate));
        contFlowRate.toJSONObject(contFlowRateObj);
    }
    if (destReservoir[0]) { objectOut[SFP(HStr_Key_OutputReservoir)] = charsToString(destReservoir, HYDRO_NAME_MAXSIZE); }
    if (flowRateSensor[0]) { objectOut[SFP(HStr_Key_FlowRateSensor)] = charsToString(flowRateSensor, HYDRO_NAME_MAXSIZE); }
}

void HydroPumpActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroActuatorData::fromJSONObject(objectIn);

    flowRateUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_FlowRateUnits)]);
    JsonVariantConst contFlowRateVar = objectIn[SFP(HStr_Key_ContinuousFlowRate)];
    if (!contFlowRateVar.isNull()) { contFlowRate.fromJSONVariant(contFlowRateVar); }
    const char *destReservoirStr = objectIn[SFP(HStr_Key_OutputReservoir)];
    if (destReservoirStr && destReservoirStr[0]) { strncpy(destReservoir, destReservoirStr, HYDRO_NAME_MAXSIZE); }
    const char *flowRateSensorStr = objectIn[SFP(HStr_Key_FlowRateSensor)];
    if (flowRateSensorStr && flowRateSensorStr[0]) { strncpy(flowRateSensor, flowRateSensorStr, HYDRO_NAME_MAXSIZE); }
}
