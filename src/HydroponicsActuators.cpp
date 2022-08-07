/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Actuators
*/

#include "Hydroponics.h"

HydroponicsActuator *newActuatorObjectFromData(const HydroponicsActuatorData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
            case 0: // Relay
                return new HydroponicsRelayActuator((const HydroponicsRelayActuatorData *)dataIn);
            case 1: // RelayPump
                return new HydroponicsPumpRelayActuator((const HydroponicsPumpRelayActuatorData *)dataIn);
            case 2: // PWM
                return new HydroponicsPWMActuator((const HydroponicsPWMActuatorData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroponicsActuator::HydroponicsActuator(Hydroponics_ActuatorType actuatorType,
                                         Hydroponics_PositionIndex actuatorIndex,
                                         pintype_t outputPin,
                                         int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(actuatorType, actuatorIndex)), classType((typeof(classType))classTypeIn),
      _outputPin(outputPin), _enabled(false), _rail(this), _reservoir(this)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_outputPin), SFP(HStr_Err_InvalidPinOrType));
    #if !HYDRUINO_SYS_DRY_RUN_ENABLE
        if (isValidPin(_outputPin)) {
            pinMode(_outputPin, OUTPUT);
        }
    #endif
}

HydroponicsActuator::HydroponicsActuator(const HydroponicsActuatorData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))dataIn->id.object.classType),
      _outputPin(dataIn->outputPin), _enabled(false),
      _contPowerUsage(&(dataIn->contPowerUsage)),
      _rail(this), _reservoir(this)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_outputPin), SFP(HStr_Err_InvalidPinOrType));
    #if !HYDRUINO_SYS_DRY_RUN_ENABLE
        if (isValidPin(_outputPin)) {
            pinMode(_outputPin, OUTPUT);
        }
    #endif
    _rail.setObject(dataIn->railName);
    _reservoir.setObject(dataIn->reservoirName);
}

void HydroponicsActuator::update()
{
    HydroponicsObject::update();

    _rail.resolve();
    _reservoir.resolve();

    if (_enabled && getActuatorInWaterFromType(getActuatorType())) {
        auto reservoir = getReservoir();
        if (reservoir && reservoir->isEmpty()) {
            disableActuator();
        }
    }
}

bool HydroponicsActuator::getCanEnable()
{
    if (getRail() && !getRail()->canActivate(this)) { return false; }
    if (getReservoir() && !getReservoir()->canActivate(this)) { return false; }
    return true;
}

void HydroponicsActuator::setContinuousPowerUsage(float contPowerUsage, Hydroponics_UnitsType contPowerUsageUnits)
{
    _contPowerUsage.value = contPowerUsage;
    _contPowerUsage.units = contPowerUsageUnits;
    _contPowerUsage.updateTimestamp();
    _contPowerUsage.updateFrame(1);
}

void HydroponicsActuator::setContinuousPowerUsage(HydroponicsSingleMeasurement contPowerUsage)
{
    _contPowerUsage = contPowerUsage;
    _contPowerUsage.setMinFrame(1);
}

const HydroponicsSingleMeasurement &HydroponicsActuator::getContinuousPowerUsage()
{
    return _contPowerUsage;
}

HydroponicsAttachment &HydroponicsActuator::getParentRail(bool resolve)
{
    if (resolve) { _rail.resolve(); }
    return _rail;
}

HydroponicsAttachment &HydroponicsActuator::getParentReservoir(bool resolve)
{
    if (resolve) { _reservoir.resolve(); }
    return _reservoir;
}

Signal<HydroponicsActuator *> &HydroponicsActuator::getActivationSignal()
{
    return _activateSignal;
}

HydroponicsData *HydroponicsActuator::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void HydroponicsActuator::saveToData(HydroponicsData *dataOut)
{
    HydroponicsObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
    ((HydroponicsActuatorData *)dataOut)->outputPin = _outputPin;
    if (_contPowerUsage.frame) {
        _contPowerUsage.saveToData(&(((HydroponicsActuatorData *)dataOut)->contPowerUsage));
    }
    if (_reservoir.getId()) {
        strncpy(((HydroponicsActuatorData *)dataOut)->reservoirName, _reservoir.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_rail.getId()) {
        strncpy(((HydroponicsActuatorData *)dataOut)->railName, _rail.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}


HydroponicsRelayActuator::HydroponicsRelayActuator(Hydroponics_ActuatorType actuatorType,
                                                   Hydroponics_PositionIndex actuatorIndex,
                                                   pintype_t outputPin, bool activeLow,
                                                   int classType)
    : HydroponicsActuator(actuatorType, actuatorIndex, outputPin, classType),
      _activeLow(activeLow)
{
    #if !HYDRUINO_SYS_DRY_RUN_ENABLE
        if (isValidPin(_outputPin)) {
            digitalWrite(_outputPin, _activeLow ? HIGH : LOW);
        }
    #endif
}

HydroponicsRelayActuator::HydroponicsRelayActuator(const HydroponicsRelayActuatorData *dataIn)
    : HydroponicsActuator(dataIn), _activeLow(dataIn->activeLow)
{
    #if !HYDRUINO_SYS_DRY_RUN_ENABLE
        if (isValidPin(_outputPin)) {
            digitalWrite(_outputPin, _activeLow ? HIGH : LOW);
        }
    #endif
}

HydroponicsRelayActuator::~HydroponicsRelayActuator()
{
    if (_enabled) {
        _enabled = false;
        #if !HYDRUINO_SYS_DRY_RUN_ENABLE
            if (isValidPin(_outputPin)) {
                digitalWrite(_outputPin, _activeLow ? HIGH : LOW);
            }
        #endif
    }
}

bool HydroponicsRelayActuator::enableActuator(float intensity, bool force)
{
    if (isValidPin(_outputPin)) {
        bool wasEnabledBefore = _enabled;

        if (!_enabled && (force || getCanEnable())) {
            _enabled = true;
            #if !HYDRUINO_SYS_DRY_RUN_ENABLE
                digitalWrite(_outputPin, _activeLow ? LOW : HIGH);
            #endif
        }

        if (_enabled && !wasEnabledBefore) {
            getLoggerInstance()->logActivation(this);

            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<HydroponicsActuator *>(getSharedPtr(), _activateSignal, this);
            #else
                _activateSignal.fire(this);
            #endif
        }
    }

    return _enabled;
}

void HydroponicsRelayActuator::disableActuator()
{
    if (isValidPin(_outputPin)) {
        bool wasEnabledBefore = _enabled;

        if (_enabled) {
            _enabled = false;
            #if !HYDRUINO_SYS_DRY_RUN_ENABLE
                digitalWrite(_outputPin, _activeLow ? HIGH : LOW);
            #endif
        }

        if (!_enabled && wasEnabledBefore) {
            getLoggerInstance()->logDeactivation(this);

            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<HydroponicsActuator *>(getSharedPtr(), _activateSignal, this);
            #else
                _activateSignal.fire(this);
            #endif
        }
    }
}

bool HydroponicsRelayActuator::isEnabled(float tolerance) const
{
    return _enabled;
}

void HydroponicsRelayActuator::saveToData(HydroponicsData *dataOut)
{
    HydroponicsActuator::saveToData(dataOut);

    ((HydroponicsRelayActuatorData *)dataOut)->activeLow = _activeLow;
}


HydroponicsPumpRelayActuator::HydroponicsPumpRelayActuator(Hydroponics_ActuatorType actuatorType,
                                                           Hydroponics_PositionIndex actuatorIndex,
                                                           pintype_t outputPin, bool activeLow,
                                                           int classType)
    :  HydroponicsRelayActuator(actuatorType, actuatorIndex, outputPin, activeLow, classType),
       _flowRateUnits(defaultLiquidFlowUnits()), _flowRate(this), _destReservoir(this), _pumpVolumeAcc(0.0f), _pumpTimeBegMillis(0), _pumpTimeAccMillis(0)
{
    _flowRate.setMeasurementUnits(getFlowRateUnits());
}

HydroponicsPumpRelayActuator::HydroponicsPumpRelayActuator(const HydroponicsPumpRelayActuatorData *dataIn)
    : HydroponicsRelayActuator(dataIn), _pumpVolumeAcc(0.0f), _pumpTimeBegMillis(0), _pumpTimeAccMillis(0),
      _flowRateUnits(definedUnitsElse(dataIn->flowRateUnits, defaultLiquidFlowUnits())),
      _contFlowRate(&(dataIn->contFlowRate)),
      _flowRate(this), _destReservoir(this)
{
    _flowRate.setMeasurementUnits(getFlowRateUnits());
    _destReservoir.setObject(dataIn->destReservoir);
    _flowRate.setObject(dataIn->flowRateSensor);
}

void HydroponicsPumpRelayActuator::update()
{
    HydroponicsActuator::update();

    _destReservoir.resolve();

    _flowRate.updateIfNeeded(true);

    if (_pumpTimeAccMillis) {
        time_t timeMillis = millis();
        time_t pumpMillis = timeMillis - _pumpTimeAccMillis;
        if (pumpMillis >= HYDRUINO_ACT_PUMPCALC_MINWRTMILLIS) {
            handlePumpTime(pumpMillis);
            _pumpTimeAccMillis = max(1, timeMillis);
        }
    }

    if (_enabled) { checkPumpingReservoirs(); }

    if (_enabled) { pollPumpingSensors(); }
}

bool HydroponicsPumpRelayActuator::enableActuator(float intensity, bool force)
{
    bool wasEnabledBefore = _enabled;
    time_t timeMillis = millis();

    HydroponicsRelayActuator::enableActuator(intensity, force);

    if (_enabled && !wasEnabledBefore) {
        _pumpVolumeAcc = 0;
        _pumpTimeBegMillis = _pumpTimeAccMillis = max(1, timeMillis);
    }

    return _enabled;
}

void HydroponicsPumpRelayActuator::disableActuator()
{
    bool wasEnabledBefore = _enabled;
    time_t timeMillis = millis();

    HydroponicsRelayActuator::disableActuator();

    if (!_enabled && wasEnabledBefore) {
        time_t pumpMillis = timeMillis - _pumpTimeAccMillis;
        if (pumpMillis) { handlePumpTime(pumpMillis); }
        _pumpTimeAccMillis = 0;
        pumpMillis = timeMillis - _pumpTimeBegMillis;
        uint8_t addDecPlaces = getActuatorType() == Hydroponics_ActuatorType_PeristalticPump ? 3 - defaultDecimalPlaces() : 1;

        getLoggerInstance()->logStatus(this, SFP(HStr_Log_MeasuredPumping));
        getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Vol_Measured), measurementToString(_pumpVolumeAcc, baseUnitsFromRate(getFlowRateUnits()), addDecPlaces));
        getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Time_Measured), roundToString(pumpMillis / 1000.0f, 1), String('s'));
    }
}

bool HydroponicsPumpRelayActuator::canPump(float volume, Hydroponics_UnitsType volumeUnits)
{
    if (getReservoir() && _contFlowRate.value > FLT_EPSILON) {
        auto waterVolume = getReservoir()->getWaterVolume().getMeasurement();
        convertUnits(&volume, &volumeUnits, waterVolume.units);
        return volume <= waterVolume.value + FLT_EPSILON;
    }
    return false;
}

bool HydroponicsPumpRelayActuator::pump(float volume, Hydroponics_UnitsType volumeUnits)
{
    if (getReservoir() && _contFlowRate.value > FLT_EPSILON) {
        convertUnits(&volume, &volumeUnits, baseUnitsFromRate(getFlowRateUnits()));
        return pump((time_t)((volume / _contFlowRate.value) * secondsToMillis(SECS_PER_MIN)));
    }
    return false;
}

bool HydroponicsPumpRelayActuator::canPump(time_t timeMillis)
{
    if (getReservoir() && _contFlowRate.value > FLT_EPSILON) {
        return canPump(_contFlowRate.value * (timeMillis / (float)secondsToMillis(SECS_PER_MIN)), baseUnitsFromRate(getFlowRateUnits()));
    }
    return false;
}

bool HydroponicsPumpRelayActuator::pump(time_t timeMillis)
{
    if (getReservoir()) {
        #ifndef HYDRUINO_DISABLE_MULTITASKING
            if (scheduleActuatorTimedEnableOnce(::getSharedPtr<HydroponicsActuator>(this), timeMillis) != TASKMGR_INVALIDID) {
                getLoggerInstance()->logStatus(this, SFP(HStr_Log_CalculatedPumping));
                if (_contFlowRate.value > FLT_EPSILON) {
                    uint8_t addDecPlaces = getActuatorType() == Hydroponics_ActuatorType_PeristalticPump ? 3 - defaultDecimalPlaces() : 1;
                    getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Vol_Calculated), measurementToString(_contFlowRate.value * (timeMillis / (float)secondsToMillis(SECS_PER_MIN)), baseUnitsFromRate(getFlowRateUnits()), addDecPlaces));
                }
                getLoggerInstance()->logMessage(SFP(HStr_Log_Field_Time_Calculated), roundToString(timeMillis / 1000.0f, 1), String('s'));
                return true;
            }
        #else
            getLoggerInstance()->logStatus(this, SFP(HStr_Log_CalculatedPumping));
            if (_contFlowRate.value > FLT_EPSILON) {
                uint8_t addDecPlaces = getActuatorType() == Hydroponics_ActuatorType_PeristalticPump ? 3 - defaultDecimalPlaces() : 1;
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

HydroponicsAttachment &HydroponicsPumpRelayActuator::getParentReservoir(bool resolve)
{
    return HydroponicsActuator::getParentReservoir(resolve);
}

HydroponicsAttachment &HydroponicsPumpRelayActuator::getDestinationReservoir(bool resolve)
{
    if (resolve) { _destReservoir.resolve(); }
    return _destReservoir;
}

void HydroponicsPumpRelayActuator::setFlowRateUnits(Hydroponics_UnitsType flowRateUnits)
{
    if (_flowRateUnits != flowRateUnits) {
        _flowRateUnits = flowRateUnits;

        convertUnits(&_contFlowRate, getFlowRateUnits());
        _flowRate.setMeasurementUnits(getFlowRateUnits());
    }
}

Hydroponics_UnitsType HydroponicsPumpRelayActuator::getFlowRateUnits() const
{
    return definedUnitsElse(_flowRateUnits, defaultLiquidFlowUnits());
}

void HydroponicsPumpRelayActuator::setContinuousFlowRate(float contFlowRate, Hydroponics_UnitsType contFlowRateUnits)
{
    _contFlowRate.value = contFlowRate;
    _contFlowRate.units = contFlowRateUnits;
    _contFlowRate.updateTimestamp();
    _contFlowRate.updateFrame(1);

    convertUnits(&_contFlowRate, getFlowRateUnits());
}

void HydroponicsPumpRelayActuator::setContinuousFlowRate(HydroponicsSingleMeasurement contFlowRate)
{
    _contFlowRate = contFlowRate;
    _contFlowRate.setMinFrame(1);

    convertUnits(&_contFlowRate, getFlowRateUnits());
}

const HydroponicsSingleMeasurement &HydroponicsPumpRelayActuator::getContinuousFlowRate()
{
    return _contFlowRate;
}

HydroponicsSensorAttachment &HydroponicsPumpRelayActuator::getFlowRate(bool poll)
{
    _flowRate.updateIfNeeded(poll);
    return _flowRate;
}

void HydroponicsPumpRelayActuator::saveToData(HydroponicsData *dataOut)
{
    HydroponicsRelayActuator::saveToData(dataOut);

    ((HydroponicsPumpRelayActuatorData *)dataOut)->flowRateUnits = _flowRateUnits;
    if (_contFlowRate.frame) {
        _contFlowRate.saveToData(&(((HydroponicsPumpRelayActuatorData *)dataOut)->contFlowRate));
    }
    if (_destReservoir.getId()) {
        strncpy(((HydroponicsPumpRelayActuatorData *)dataOut)->destReservoir, _destReservoir.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_flowRate.getId()) {
        strncpy(((HydroponicsPumpRelayActuatorData *)dataOut)->flowRateSensor, _flowRate.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}

void HydroponicsPumpRelayActuator::checkPumpingReservoirs()
{
    auto sourceRes = getInputReservoir();
    auto destRes = getOutputReservoir();
    if ((sourceRes && sourceRes->isEmpty()) || (destRes && destRes->isFilled())) {
        disableActuator();
    }
}

void HydroponicsPumpRelayActuator::pollPumpingSensors()
{
    if (getFlowRateSensor()) {
        _flowRate->takeMeasurement(true);
    }
    HydroponicsSensor *sourceVolSensor;
    if (getInputReservoir() && _reservoir.get<HydroponicsReservoir>()->isAnyFluidClass() &&
        (sourceVolSensor = _reservoir.get<HydroponicsFluidReservoir>()->getWaterVolumeSensor().get())) {
        sourceVolSensor->takeMeasurement(true);
    }
    HydroponicsSensor *destVolSensor;
    if (getOutputReservoir() && _destReservoir.get<HydroponicsReservoir>()->isAnyFluidClass() &&
        (destVolSensor = _destReservoir.get<HydroponicsFluidReservoir>()->getWaterVolumeSensor().get()) &&
        destVolSensor != sourceVolSensor) {
        destVolSensor->takeMeasurement(true);
    }
}

void HydroponicsPumpRelayActuator::handlePumpTime(time_t timeMillis)
{
    if (getInputReservoir() != getOutputReservoir()) {
        float flowRateVal = _flowRate.getMeasurementFrame() && getFlowRateSensor() && !_flowRate->needsPolling(HYDRUINO_ACT_PUMPCALC_MAXFRAMEDIFF) &&
                            _flowRate.getMeasurementValue() >= (_contFlowRate.value * HYDRUINO_ACT_PUMPCALC_MINFLOWRATE) - FLT_EPSILON ? _flowRate.getMeasurementValue() : _contFlowRate.value;
        float volumePumped = flowRateVal * (timeMillis / (float)secondsToMillis(SECS_PER_MIN));
        _pumpVolumeAcc += volumePumped;

        if (getInputReservoir() && _reservoir.get<HydroponicsReservoir>()->isAnyFluidClass()) {
            auto sourceFluidRes = getInputReservoir<HydroponicsFluidReservoir>();
            if (sourceFluidRes && !sourceFluidRes->getWaterVolume()) { // only report if there isn't a volume sensor already doing it
                auto volume = sourceFluidRes->getWaterVolume().getMeasurement(true);
                convertUnits(&volume, baseUnitsFromRate(getFlowRateUnits()));
                volume.value -= volumePumped;
                sourceFluidRes->getWaterVolume().setMeasurement(volume);
            }
        }
        if (getOutputReservoir() && _destReservoir.get<HydroponicsReservoir>()->isAnyFluidClass()) {
            auto destFluidRes = getOutputReservoir<HydroponicsFluidReservoir>();
            if (destFluidRes && !destFluidRes->getWaterVolume()) { // only report if there isn't a volume sensor already doing it
                auto volume = destFluidRes->getWaterVolume().getMeasurement(true);
                convertUnits(&volume, baseUnitsFromRate(getFlowRateUnits()));
                volume.value += volumePumped;
                destFluidRes->getWaterVolume().setMeasurement(volume);
            }
        }
    }
}


HydroponicsPWMActuator::HydroponicsPWMActuator(Hydroponics_ActuatorType actuatorType,
                                               Hydroponics_PositionIndex actuatorIndex,
                                               pintype_t outputPin,
#ifdef ESP_PLATFORM
                                               uint8_t pwmChannel,
                                               float pwmFrequency,
#endif
                                               uint8_t outputBitRes,
                                               int classType)
    : HydroponicsActuator(actuatorType, actuatorIndex, outputPin, classType),
      _pwmAmount(0.0f), _pwmResolution(outputBitRes)
#ifdef ESP_PLATFORM
      , _pwmChannel(pwmChannel), _pwmFrequency(pwmFrequency)
#endif
{
    #if !HYDRUINO_SYS_DRY_RUN_ENABLE
        if (isValidPin(_outputPin)) {
            #ifdef ESP32
                ledcAttachPin(outputPin, _pwmChannel);
                ledcSetup(_pwmChannel, _pwmFrequency, outputBitRes);
                ledcWrite(_pwmChannel, 0);
            #else
                analogWrite(_outputPin, 0);
            #endif
        }
    #endif
}

HydroponicsPWMActuator::HydroponicsPWMActuator(const HydroponicsPWMActuatorData *dataIn)
    : HydroponicsActuator(dataIn),
      _pwmAmount(0.0f), _pwmResolution(dataIn->outputBitRes)
{
    #if !HYDRUINO_SYS_DRY_RUN_ENABLE
        if (isValidPin(_outputPin)) {
            #ifdef ESP32
                ledcWrite(_pwmChannel, 0);
            #else
                analogWrite(_outputPin, 0);
            #endif
        }
    #endif
}

HydroponicsPWMActuator::~HydroponicsPWMActuator()
{
    if (_enabled) {
        _enabled = false;
        #if !HYDRUINO_SYS_DRY_RUN_ENABLE
            if (isValidPin(_outputPin)) {
                #ifdef ESP32
                    ledcWrite(_pwmChannel, 0);
                #else
                    analogWrite(_outputPin, 0);
                #endif
            }
        #endif
    }
}

bool HydroponicsPWMActuator::enableActuator(float intensity, bool force)
{
    if (isValidPin(_outputPin)) {
        bool wasEnabledBefore = _enabled;

        if ((!_enabled && (force || getCanEnable())) || (_enabled && !isFPEqual(_pwmAmount, intensity))) {
            _enabled = true;
            _pwmAmount = constrain(intensity, 0.0f, 1.0f);
            applyPWM();
        }

        if (_enabled && !wasEnabledBefore) {
            getLoggerInstance()->logActivation(this);

            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<HydroponicsActuator *>(getSharedPtr(), _activateSignal, this);
            #else
                _activateSignal.fire(this);
            #endif
        }
    }

    return _enabled;
}

void HydroponicsPWMActuator::disableActuator()
{
    if (isValidPin(_outputPin)) {
        bool wasEnabledBefore = _enabled;

        if (_enabled) {
            _enabled = false;
            applyPWM();
        }

        if (!_enabled && wasEnabledBefore) {
            getLoggerInstance()->logDeactivation(this);

            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<HydroponicsActuator *>(getSharedPtr(), _activateSignal, this);
            #else
                _activateSignal.fire(this);
            #endif
        }
    }
}

bool HydroponicsPWMActuator::isEnabled(float tolerance) const
{
    return _enabled && _pwmAmount >= tolerance - FLT_EPSILON;
}

int HydroponicsPWMActuator::getPWMAmount(int) const
{
    return _pwmResolution.inverseTransform(_pwmAmount);
}

void HydroponicsPWMActuator::setPWMAmount(float amount)
{
    _pwmAmount = constrain(amount, 0.0f, 1.0f);

    if (_enabled) { applyPWM(); }
}

void HydroponicsPWMActuator::setPWMAmount(int amount)
{
    _pwmAmount = _pwmResolution.transform(amount);

    if (_enabled) { applyPWM(); }
}

void HydroponicsPWMActuator::saveToData(HydroponicsData *dataOut)
{
    HydroponicsActuator::saveToData(dataOut);

    ((HydroponicsPWMActuatorData *)dataOut)->outputBitRes = _pwmResolution.bitRes;
}

void HydroponicsPWMActuator::applyPWM()
{
    #if !HYDRUINO_SYS_DRY_RUN_ENABLE
        if (isValidPin(_outputPin)) {
            #ifdef ESP32
                ledcWrite(_pwmChannel, _enabled ? getPWMAmount(0) : 0);
            #else
                #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
                    analogWriteResolution(_pwmResolution.bitRes);
                #elif defined(ESP8266)
                    analogWriteRange(_pwmResolution.maxVal);
                    analogWriteFreq(_pwmFrequency);
                #endif
                analogWrite(_outputPin, _enabled ? getPWMAmount(0) : 0);
            #endif
        }
    #endif
}


HydroponicsActuatorData::HydroponicsActuatorData()
    : HydroponicsObjectData(), outputPin(-1), contPowerUsage(), railName{0}, reservoirName{0}
{
    _size = sizeof(*this);
}

void HydroponicsActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsObjectData::toJSONObject(objectOut);

    if (isValidPin(outputPin)) { objectOut[SFP(HStr_Key_OutputPin)] = outputPin; }
    if (contPowerUsage.value > FLT_EPSILON) {
        JsonObject contPowerUsageObj = objectOut.createNestedObject(SFP(HStr_Key_ContPowerUsage));
        contPowerUsage.toJSONObject(contPowerUsageObj);
    }
    if (railName[0]) { objectOut[SFP(HStr_Key_RailName)] = charsToString(railName, HYDRUINO_NAME_MAXSIZE); }
    if (reservoirName[0]) { objectOut[SFP(HStr_Key_ReservoirName)] = charsToString(reservoirName, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);

    outputPin = objectIn[SFP(HStr_Key_OutputPin)] | outputPin;
    JsonVariantConst contPowerUsageVar = objectIn[SFP(HStr_Key_ContPowerUsage)];
    if (!contPowerUsageVar.isNull()) { contPowerUsage.fromJSONVariant(contPowerUsageVar); }
    const char *railNameStr = objectIn[SFP(HStr_Key_RailName)];
    if (railNameStr && railNameStr[0]) { strncpy(railName, railNameStr, HYDRUINO_NAME_MAXSIZE); }
    const char *reservoirNameStr = objectIn[SFP(HStr_Key_ReservoirName)];
    if (reservoirNameStr && reservoirNameStr[0]) { strncpy(reservoirName, reservoirNameStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsRelayActuatorData::HydroponicsRelayActuatorData()
    : HydroponicsActuatorData(), activeLow(true)
{
    _size = sizeof(*this);
}

void HydroponicsRelayActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsActuatorData::toJSONObject(objectOut);

    objectOut[SFP(HStr_Key_ActiveLow)] = activeLow;
}

void HydroponicsRelayActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsActuatorData::fromJSONObject(objectIn);

    activeLow = objectIn[SFP(HStr_Key_ActiveLow)] | activeLow;
}

HydroponicsPumpRelayActuatorData::HydroponicsPumpRelayActuatorData()
    : HydroponicsRelayActuatorData(), flowRateUnits(Hydroponics_UnitsType_Undefined), contFlowRate(), destReservoir{0}, flowRateSensor{0}
{
    _size = sizeof(*this);
}

void HydroponicsPumpRelayActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsRelayActuatorData::toJSONObject(objectOut);

    if (flowRateUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HStr_Key_FlowRateUnits)] = unitsTypeToSymbol(flowRateUnits); }
    if (contFlowRate.value > FLT_EPSILON) {
        JsonObject contFlowRateObj = objectOut.createNestedObject(SFP(HStr_Key_ContFlowRate));
        contFlowRate.toJSONObject(contFlowRateObj);
    }
    if (destReservoir[0]) { objectOut[SFP(HStr_Key_OutputReservoir)] = charsToString(destReservoir, HYDRUINO_NAME_MAXSIZE); }
    if (flowRateSensor[0]) { objectOut[SFP(HStr_Key_FlowRateSensor)] = charsToString(flowRateSensor, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsPumpRelayActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsRelayActuatorData::fromJSONObject(objectIn);

    flowRateUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_FlowRateUnits)]);
    JsonVariantConst contFlowRateVar = objectIn[SFP(HStr_Key_ContFlowRate)];
    if (!contFlowRateVar.isNull()) { contFlowRate.fromJSONVariant(contFlowRateVar); }
    const char *destReservoirStr = objectIn[SFP(HStr_Key_OutputReservoir)];
    if (destReservoirStr && destReservoirStr[0]) { strncpy(destReservoir, destReservoirStr, HYDRUINO_NAME_MAXSIZE); }
    const char *flowRateSensorStr = objectIn[SFP(HStr_Key_FlowRateSensor)];
    if (flowRateSensorStr && flowRateSensorStr[0]) { strncpy(flowRateSensor, flowRateSensorStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsPWMActuatorData::HydroponicsPWMActuatorData()
    : HydroponicsActuatorData(), outputBitRes(10)
#ifdef ESP_PLATFORM
      , pwmChannel(0), pwmFrequency(0)
#endif
{
    _size = sizeof(*this);
}

void HydroponicsPWMActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsActuatorData::toJSONObject(objectOut);

    #ifdef ESP_PLATFORM
        if (pwmChannel) { objectOut[SFP(HStr_Key_PWMChannel)] = pwmChannel; }
        if (pwmFrequency > FLT_EPSILON) { objectOut[SFP(HStr_Key_PWMFrequency)] = pwmFrequency; }
    #endif
    if (outputBitRes != 10) { objectOut[SFP(HStr_Key_OutputBitRes)] = outputBitRes; }
}

void HydroponicsPWMActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsActuatorData::fromJSONObject(objectIn);

    #ifdef ESP_PLATFORM
        pwmChannel = objectIn[SFP(HStr_Key_PWMChannel)] | pwmChannel;
        pwmFrequency = objectIn[SFP(HStr_Key_PWMFrequency)] | pwmFrequency;
    #endif
    outputBitRes = objectIn[SFP(HStr_Key_OutputBitRes)] | outputBitRes;
}
