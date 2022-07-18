/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Actuators
*/

#include "Hydroponics.h"

HydroponicsActuator *newActuatorObjectFromData(const HydroponicsActuatorData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), F("Invalid data"));

    if (dataIn && dataIn->isObjectData()) {
        switch(dataIn->id.object.classType) {
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
                                         byte outputPin,
                                         int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(actuatorType, actuatorIndex)), classType((typeof(classType))classTypeIn),
      _outputPin(outputPin), _enabled(false)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_outputPin), F("Invalid output pin"));
    if (isValidPin(_outputPin)) {
        pinMode(_outputPin, OUTPUT);
    }
}

HydroponicsActuator::HydroponicsActuator(const HydroponicsActuatorData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))dataIn->id.object.classType),
      _outputPin(dataIn->outputPin), _enabled(false),
      _contPowerDraw(&(dataIn->contPowerDraw)),
      _rail(dataIn->railName), _reservoir(dataIn->reservoirName)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_outputPin), F("Invalid output pin"));
    if (isValidPin(_outputPin)) {
        pinMode(_outputPin, OUTPUT);
    }
}

HydroponicsActuator::~HydroponicsActuator()
{
    if (_rail) { _rail->removeActuator(this); }
    if (_reservoir) { _reservoir->removeActuator(this); }
}

void HydroponicsActuator::update()
{
    HydroponicsObject::update();

    if (_enabled && getActuatorInWaterFromType(getActuatorType())) {
        auto reservoir = getReservoir();
        if (reservoir && reservoir->getIsEmpty()) {
            disableActuator();
        }
    }
}

void HydroponicsActuator::resolveLinks()
{
    HydroponicsObject::resolveLinks();

    if (_rail.needsResolved()) { getRail(); }
    if (_reservoir.needsResolved()) { getReservoir(); }
}

void HydroponicsActuator::handleLowMemory()
{ ; }

bool HydroponicsActuator::getCanEnable()
{
    if (getRail() && !_rail->canActivate(this)) { return false; }
    if (getReservoir() && !_reservoir->canActivate(this)) { return false; }
    return true;
}

void HydroponicsActuator::setContinuousPowerDraw(float contPowerDraw, Hydroponics_UnitsType contPowerDrawUnits)
{
    _contPowerDraw.value = contPowerDraw;
    _contPowerDraw.units = contPowerDrawUnits;
    _contPowerDraw.updateTimestamp();
    _contPowerDraw.updateFrame(1);
}

void HydroponicsActuator::setContinuousPowerDraw(HydroponicsSingleMeasurement contPowerDraw)
{
    _contPowerDraw = contPowerDraw;
    _contPowerDraw.setMinFrame(1);
}

const HydroponicsSingleMeasurement &HydroponicsActuator::getContinuousPowerDraw()
{
    return _contPowerDraw;
}

void HydroponicsActuator::setRail(HydroponicsIdentity powerRailId)
{
    if (_rail != powerRailId) {
        if (_rail) { _rail->removeActuator(this); }
        _rail = powerRailId;
    }
}

void HydroponicsActuator::setRail(shared_ptr<HydroponicsRail> powerRail)
{
    if (_rail != powerRail) {
        if (_rail) { _rail->removeActuator(this); }
        _rail = powerRail;
        if (_rail) { _rail->addActuator(this); }
    }
}

shared_ptr<HydroponicsRail> HydroponicsActuator::getRail()
{
    if (_rail.resolveIfNeeded()) { _rail->addActuator(this); }
    return _rail.getObj();
}

void HydroponicsActuator::setReservoir(HydroponicsIdentity reservoirId)
{
    if (_reservoir != reservoirId) {
        if (_reservoir) { _reservoir->removeActuator(this); }
        _reservoir = reservoirId;
    }
}

void HydroponicsActuator::setReservoir(shared_ptr<HydroponicsReservoir> reservoir)
{
    if (_reservoir != reservoir) {
        if (_reservoir) { _reservoir->removeActuator(this); }
        _reservoir = reservoir;
        if (_reservoir) { _reservoir->addActuator(this); }
    }
}

shared_ptr<HydroponicsReservoir> HydroponicsActuator::getReservoir()
{
    if (_reservoir.resolveIfNeeded()) { _reservoir->addActuator(this); }
    return _reservoir.getObj();
}

byte HydroponicsActuator::getOutputPin() const
{
    return _outputPin;
}

Hydroponics_ActuatorType HydroponicsActuator::getActuatorType() const
{
    return _id.objTypeAs.actuatorType;
}

Hydroponics_PositionIndex HydroponicsActuator::getActuatorIndex() const
{
    return _id.posIndex;
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
    if (_contPowerDraw.frame) {
        _contPowerDraw.saveToData(&(((HydroponicsActuatorData *)dataOut)->contPowerDraw));
    }
    if (_reservoir.getId()) {
        strncpy(((HydroponicsActuatorData *)dataOut)->reservoirName, _reservoir.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_rail.getId()) {
        strncpy(((HydroponicsActuatorData *)dataOut)->railName, _rail.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}


HydroponicsRelayActuator::HydroponicsRelayActuator(Hydroponics_ActuatorType actuatorType,
                                                   Hydroponics_PositionIndex actuatorIndex,
                                                   byte outputPin, bool activeLow,
                                                   int classType)
    : HydroponicsActuator(actuatorType, actuatorIndex, outputPin, classType),
      _activeLow(activeLow)
{
    if (isValidPin(_outputPin)) {
        digitalWrite(_outputPin, _activeLow ? HIGH : LOW);
    }
}

HydroponicsRelayActuator::HydroponicsRelayActuator(const HydroponicsRelayActuatorData *dataIn)
    : HydroponicsActuator(dataIn), _activeLow(dataIn->activeLow)
{
    if (isValidPin(_outputPin)) {
        digitalWrite(_outputPin, _activeLow ? HIGH : LOW);
    }
}

HydroponicsRelayActuator::~HydroponicsRelayActuator()
{
    if (_enabled) {
        _enabled = false;
        if (isValidPin(_outputPin)) {
            digitalWrite(_outputPin, _activeLow ? HIGH : LOW);
        }
    }
}

bool HydroponicsRelayActuator::enableActuator(float intensity, bool override)
{
    if (isValidPin(_outputPin)) {
        bool wasEnabledBefore = _enabled;

        if (!_enabled && (override || getCanEnable())) {
            _enabled = true;
            digitalWrite(_outputPin, _activeLow ? LOW : HIGH);
        }

        if (_enabled != wasEnabledBefore) {
            scheduleSignalFireOnce<HydroponicsActuator *>(getSharedPtr(), _activateSignal, this);
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
            digitalWrite(_outputPin, _activeLow ? HIGH : LOW);
        }

        if (_enabled != wasEnabledBefore) {
            scheduleSignalFireOnce<HydroponicsActuator *>(getSharedPtr(), _activateSignal, this);
        }
    }
}

bool HydroponicsRelayActuator::getIsEnabled(float tolerance) const
{
    return _enabled;
}

bool HydroponicsRelayActuator::getActiveLow() const
{
    return _activeLow;
}

void HydroponicsRelayActuator::saveToData(HydroponicsData *dataOut)
{
    HydroponicsActuator::saveToData(dataOut);

    ((HydroponicsRelayActuatorData *)dataOut)->activeLow = _activeLow;
}


HydroponicsPumpRelayActuator::HydroponicsPumpRelayActuator(Hydroponics_ActuatorType actuatorType,
                                                           Hydroponics_PositionIndex actuatorIndex,
                                                           byte outputPin, bool activeLow,
                                                           int classType)
    :  HydroponicsRelayActuator(actuatorType, actuatorIndex, outputPin, activeLow, classType),
       _flowRateUnits(defaultLiquidFlowUnits()), _needsFlowRate(true), _pumpTimeAccMillis(0)
{ ; }

HydroponicsPumpRelayActuator::HydroponicsPumpRelayActuator(const HydroponicsPumpRelayActuatorData *dataIn)
    : HydroponicsRelayActuator(dataIn), _needsFlowRate(true), _pumpTimeAccMillis(0),
      _flowRateUnits(definedUnitsElse(dataIn->flowRateUnits, defaultLiquidFlowUnits())),
      _contFlowRate(&(dataIn->contFlowRate)),
      _outputReservoir(dataIn->outputReservoirName),
      _flowRateSensor(dataIn->flowRateSensorName)
{ ; }

HydroponicsPumpRelayActuator::~HydroponicsPumpRelayActuator()
{
    if (_outputReservoir) { _outputReservoir->removeActuator(this); }
    if (_flowRateSensor) { detachFlowRateSensor(); }
}

void HydroponicsPumpRelayActuator::update()
{
    HydroponicsActuator::update();

    if (_needsFlowRate && getFlowRateSensor()) {
        handleFlowRateMeasure(_flowRateSensor->getLatestMeasurement());
    }

    if (_pumpTimeAccMillis) {
        time_t timeMillis = millis();
        time_t timePassedMillis = timeMillis - _pumpTimeAccMillis;
        if (_pumpTimeAccMillis >= HYDRUINO_ACT_PUMPCALC_MINWRTMILLIS) {
            handlePumpTime(timePassedMillis);
            _pumpTimeAccMillis = max(1, timeMillis);
        }
    }

    if (_enabled) { checkPumpingReservoirs(); }

    if (_enabled) { pulsePumpingSensors(); }
}

void HydroponicsPumpRelayActuator::resolveLinks()
{
    HydroponicsActuator::resolveLinks();

    if (_flowRateSensor.needsResolved()) { getFlowRateSensor(); }
    if (_outputReservoir.needsResolved()) { getOutputReservoir(); }
}

bool HydroponicsPumpRelayActuator::enableActuator(float intensity, bool override)
{
    if (HydroponicsRelayActuator::enableActuator(intensity, override) && !_pumpTimeAccMillis) {
        time_t timeMillis = millis();
        _pumpTimeAccMillis = max(1, timeMillis);
    }
}

void HydroponicsPumpRelayActuator::disableActuator()
{
    bool wasEnabledBefore = _enabled;
    HydroponicsRelayActuator::disableActuator();
    if (!_enabled && wasEnabledBefore && _pumpTimeAccMillis) {
        handlePumpTime(millis() - _pumpTimeAccMillis);
        _pumpTimeAccMillis = 0;
    }
}

bool HydroponicsPumpRelayActuator::canPump(float volume, Hydroponics_UnitsType volumeUnits)
{
    auto reservoir = getReservoir();
    if (reservoir && _contFlowRate.value > FLT_EPSILON) {
        auto waterVolume = reservoir->getWaterVolume();
        convertUnits(&volume, &volumeUnits, waterVolume.units);
        return volume <= waterVolume.value + FLT_EPSILON;
    }
    return false;
}

bool HydroponicsPumpRelayActuator::pump(float volume, Hydroponics_UnitsType volumeUnits)
{
    auto reservoir = getReservoir();
    if (reservoir && _contFlowRate.value > FLT_EPSILON) {
        convertUnits(&volume, &volumeUnits, baseUnitsFromRate(getFlowRateUnits()));
        return pump((time_t)((volume / _contFlowRate.value) * secondsToMillis(SECS_PER_MIN)));
    }
    return false;
}

bool HydroponicsPumpRelayActuator::canPump(time_t timeMillis)
{
    auto reservoir = getReservoir();
    if (reservoir && _contFlowRate.value > FLT_EPSILON) {
        return canPump(_contFlowRate.value * (timeMillis / (float)secondsToMillis(SECS_PER_MIN)), baseUnitsFromRate(getFlowRateUnits()));
    }
    return false;
}

bool HydroponicsPumpRelayActuator::pump(time_t timeMillis)
{
    auto reservoir = getReservoir();
    if (reservoir) {
        return scheduleActuatorTimedEnableOnce(::getSharedPtr<HydroponicsActuator>(this), timeMillis) != TASKMGR_INVALIDID;
    }
    return false;
}

void HydroponicsPumpRelayActuator::setReservoir(HydroponicsIdentity reservoirId) {
    HydroponicsActuator::setReservoir(reservoirId);
}

void HydroponicsPumpRelayActuator::setReservoir(shared_ptr<HydroponicsReservoir> reservoir) {
    HydroponicsActuator::setReservoir(reservoir);
}

shared_ptr<HydroponicsReservoir> HydroponicsPumpRelayActuator::getReservoir() {
    return HydroponicsActuator::getReservoir();
}

void HydroponicsPumpRelayActuator::setOutputReservoir(HydroponicsIdentity outputReservoirId)
{
    if (_outputReservoir != outputReservoirId) {
        if (_outputReservoir) { _outputReservoir->removeActuator(this); }
        _outputReservoir = outputReservoirId;
    }
}

void HydroponicsPumpRelayActuator::setOutputReservoir(shared_ptr<HydroponicsReservoir> outputReservoir)
{
    if (_outputReservoir != outputReservoir) {
        if (_outputReservoir) { _outputReservoir->removeActuator(this); }
        _outputReservoir = outputReservoir;
        if (_outputReservoir) { _outputReservoir->addActuator(this); }
    }
}

shared_ptr<HydroponicsReservoir> HydroponicsPumpRelayActuator::getOutputReservoir()
{
    if (_outputReservoir.resolveIfNeeded()) { _outputReservoir->addActuator(this); }
    return _outputReservoir.getObj();
}

void HydroponicsPumpRelayActuator::setFlowRateUnits(Hydroponics_UnitsType flowRateUnits)
{
    if (_flowRateUnits != flowRateUnits) {
        _flowRateUnits = flowRateUnits;

        convertUnits(&_contFlowRate, getFlowRateUnits());
        convertUnits(&_flowRate, getFlowRateUnits());
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

void HydroponicsPumpRelayActuator::setFlowRateSensor(HydroponicsIdentity flowRateSensorId)
{
    if (_flowRateSensor != flowRateSensorId) {
        if (_flowRateSensor) { detachFlowRateSensor(); }
        _flowRateSensor = flowRateSensorId;
        _needsFlowRate = true;
    }
}

void HydroponicsPumpRelayActuator::setFlowRateSensor(shared_ptr<HydroponicsSensor> flowRateSensor)
{
    if (_flowRateSensor != flowRateSensor) {
        if (_flowRateSensor) { detachFlowRateSensor(); }
        _flowRateSensor = flowRateSensor;
        if (_flowRateSensor) { attachFlowRateSensor(); }
        _needsFlowRate = true;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsPumpRelayActuator::getFlowRateSensor()
{
    if (_flowRateSensor.resolveIfNeeded()) { attachFlowRateSensor(); }
    return _flowRateSensor.getObj();
}

void HydroponicsPumpRelayActuator::setFlowRate(float flowRate, Hydroponics_UnitsType flowRateUnits)
{
    _flowRate.value = flowRate;
    _flowRate.units = flowRateUnits;
    _flowRate.updateTimestamp();
    _flowRate.updateFrame(1);

    convertUnits(&_flowRate, getFlowRateUnits());
    _needsFlowRate = false;
}

void HydroponicsPumpRelayActuator::setFlowRate(HydroponicsSingleMeasurement flowRate)
{
    _flowRate = flowRate;
    _flowRate.setMinFrame(1);

    convertUnits(&_flowRate, getFlowRateUnits());
    _needsFlowRate = false;
}

const HydroponicsSingleMeasurement &HydroponicsPumpRelayActuator::getFlowRate()
{
    if (_needsFlowRate && getFlowRateSensor()) {
        handleFlowRateMeasure(_flowRateSensor->getLatestMeasurement());
    }
    return _flowRate;
}

void HydroponicsPumpRelayActuator::saveToData(HydroponicsData *dataOut)
{
    HydroponicsRelayActuator::saveToData(dataOut);

    ((HydroponicsPumpRelayActuatorData *)dataOut)->flowRateUnits = _flowRateUnits;
    if (_contFlowRate.frame) {
        _contFlowRate.saveToData(&(((HydroponicsPumpRelayActuatorData *)dataOut)->contFlowRate));
    }
    if (_outputReservoir.getId()) {
        strncpy(((HydroponicsPumpRelayActuatorData *)dataOut)->outputReservoirName, _outputReservoir.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_flowRateSensor.getId()) {
        strncpy(((HydroponicsPumpRelayActuatorData *)dataOut)->flowRateSensorName, _flowRateSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}

void HydroponicsPumpRelayActuator::checkPumpingReservoirs()
{
    auto sourceRes = getReservoir();
    auto destRes = getOutputReservoir();
    if ((sourceRes && sourceRes->getIsEmpty()) || (destRes && destRes->getIsFilled())) {
        disableActuator();
    }
}

void HydroponicsPumpRelayActuator::pulsePumpingSensors()
{
    if (getFlowRateSensor()) {
        _flowRateSensor->takeMeasurement(true);
    }
    HydroponicsSensor *sourceVolSensor;
    if (getReservoir() && _reservoir->isAnyFluidClass() &&
        (sourceVolSensor = ((HydroponicsFluidReservoir *)_reservoir.obj.get())->getVolumeSensor().get())) {
        sourceVolSensor->takeMeasurement(true);
    }
    HydroponicsSensor *destVolSensor;
    if (getOutputReservoir() && _outputReservoir->isAnyFluidClass() &&
        (destVolSensor = ((HydroponicsFluidReservoir *)_outputReservoir.obj.get())->getVolumeSensor().get()) &&
        destVolSensor != sourceVolSensor) {
        destVolSensor->takeMeasurement(true);
    }
}

void HydroponicsPumpRelayActuator::handlePumpTime(time_t timeMillis)
{
    auto sourceRes = getReservoir();
    auto destRes = getOutputReservoir();
    if (sourceRes != destRes) {
        float flowRateVal = _flowRate.frame && getFlowRateSensor() && !_flowRateSensor->getNeedsPolling(HYDRUINO_ACT_PUMPCALC_MAXFRAMEDIFF) &&
                            _flowRate.value >= (_contFlowRate.value * HYDRUINO_ACT_PUMPCALC_MINFLOWRATE) - FLT_EPSILON ? _flowRate.value : _contFlowRate.value;
        float volumePumped = flowRateVal * (timeMillis / (float)secondsToMillis(SECS_PER_MIN));

        if (sourceRes && sourceRes->isAnyFluidClass()) {
            auto sourceFluidRes = static_pointer_cast<HydroponicsFluidReservoir>(sourceRes);
            if (sourceFluidRes && !sourceFluidRes->getVolumeSensor()) { // only report if there isn't a volume sensor already doing it
                auto volume = sourceFluidRes->getWaterVolume();
                convertUnits(&volume, baseUnitsFromRate(getFlowRateUnits()));
                volume.value -= volumePumped;
                sourceFluidRes->setWaterVolume(volume);
            }
        }
        if (destRes && destRes->isAnyFluidClass()) {
            auto destFluidRes = static_pointer_cast<HydroponicsFluidReservoir>(destRes);
            if (destFluidRes && !destFluidRes->getVolumeSensor()) { // only report if there isn't a volume sensor already doing it
                auto volume = destFluidRes->getWaterVolume();
                convertUnits(&volume, baseUnitsFromRate(getFlowRateUnits()));
                volume.value += volumePumped;
                destFluidRes->setWaterVolume(volume);
            }
        }
    }
}

void HydroponicsPumpRelayActuator::attachFlowRateSensor()
{
    HYDRUINO_SOFT_ASSERT(getFlowRateSensor(), F("Flow rate sensor not linked, failure attaching"));
    if (getFlowRateSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleFlowRateMeasure);
        _flowRateSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsPumpRelayActuator::detachFlowRateSensor()
{
    HYDRUINO_SOFT_ASSERT(getFlowRateSensor(), F("Flow rate sensor not linked, failure detaching"));
    if (getFlowRateSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &handleFlowRateMeasure);
        _flowRateSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsPumpRelayActuator::handleFlowRateMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame && _enabled) {
        setFlowRate(singleMeasurementAt(measurement, 0, _contFlowRate.value, _flowRateUnits));
    }
}


HydroponicsPWMActuator::HydroponicsPWMActuator(Hydroponics_ActuatorType actuatorType,
                                               Hydroponics_PositionIndex actuatorIndex,
                                               byte outputPin,
                                               byte outputBitResolution,
                                               int classType)
    : HydroponicsActuator(actuatorType, actuatorIndex, outputPin, classType),
      _pwmAmount(0.0f), _pwmResolution(outputBitResolution)
{
    if (isValidPin(_outputPin)) {
        analogWrite(_outputPin, 0);
    }
}

HydroponicsPWMActuator::HydroponicsPWMActuator(const HydroponicsPWMActuatorData *dataIn)
    : HydroponicsActuator(dataIn),
      _pwmAmount(0.0f), _pwmResolution(dataIn->outputBitResolution)
{
    if (isValidPin(_outputPin)) {
        analogWrite(_outputPin, 0);
    }
}

HydroponicsPWMActuator::~HydroponicsPWMActuator()
{
    if (_enabled) {
        _enabled = false;
        if (isValidPin(_outputPin)) {
            analogWrite(_outputPin, 0);
        }
    }
}

bool HydroponicsPWMActuator::enableActuator(float intensity, bool override)
{
    if (isValidPin(_outputPin)) {
        bool wasEnabledBefore = _enabled;

        if ((!_enabled && (override || getCanEnable())) || (_enabled && !isFPEqual(_pwmAmount, intensity))) {
            _enabled = true;
            _pwmAmount = constrain(intensity, 0.0f, 1.0f);
            applyPWM();
        }

        if (_enabled != wasEnabledBefore) {
            scheduleSignalFireOnce<HydroponicsActuator *>(getSharedPtr(), _activateSignal, this);
        }
    }
    return _enabled;
}

void HydroponicsPWMActuator::disableActuator()
{
    bool wasEnabledBefore = _enabled;

    if (_enabled) {
        _enabled = false;
        applyPWM();
    }

    if (_enabled != wasEnabledBefore) {
        scheduleSignalFireOnce<HydroponicsActuator *>(getSharedPtr(), _activateSignal, this);
    }
}

bool HydroponicsPWMActuator::getIsEnabled(float tolerance) const
{
    return _enabled && _pwmAmount >= tolerance - FLT_EPSILON;
}

float HydroponicsPWMActuator::getPWMAmount() const
{
    return _pwmAmount;
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

HydroponicsBitResolution HydroponicsPWMActuator::getPWMResolution() const
{
    return _pwmResolution;
}

void HydroponicsPWMActuator::saveToData(HydroponicsData *dataOut)
{
    HydroponicsActuator::saveToData(dataOut);

    ((HydroponicsPWMActuatorData *)dataOut)->outputBitResolution = _pwmResolution.bitRes;
}

void HydroponicsPWMActuator::applyPWM()
{
    if (isValidPin(_outputPin)) {
        #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
            analogWriteResolution(_pwmResolution.bitRes);
        #endif
        analogWrite(_outputPin, _enabled ? getPWMAmount(0) : 0);
    }
}


HydroponicsActuatorData::HydroponicsActuatorData()
    : HydroponicsObjectData(), outputPin(-1), contPowerDraw(), railName{0}, reservoirName{0}
{
    _size = sizeof(*this);
}

void HydroponicsActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsObjectData::toJSONObject(objectOut);

    if (isValidPin(outputPin)) { objectOut[F("outputPin")] = outputPin; }
    if (contPowerDraw.type != -1) {
        JsonObject contPowerDrawObj = objectOut.createNestedObject(F("contPowerDraw"));
        contPowerDraw.toJSONObject(contPowerDrawObj);
    }
    if (railName[0]) { objectOut[F("railName")] = stringFromChars(railName, HYDRUINO_NAME_MAXSIZE); }
    if (reservoirName[0]) { objectOut[F("reservoirName")] = stringFromChars(reservoirName, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);

    outputPin = objectIn[F("outputPin")] | outputPin;
    JsonVariantConst contPowerDrawVar = objectIn[F("contPowerDraw")];
    if (!contPowerDrawVar.isNull()) { contPowerDraw.fromJSONVariant(contPowerDrawVar); }
    const char *railNameStr = objectIn[F("railName")];
    if (railNameStr && railNameStr[0]) { strncpy(railName, railNameStr, HYDRUINO_NAME_MAXSIZE); }
    const char *reservoirNameStr = objectIn[F("reservoirName")];
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

    objectOut[F("activeLow")] = activeLow;
}

void HydroponicsRelayActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsActuatorData::fromJSONObject(objectIn);

    activeLow = objectIn[F("activeLow")] | activeLow;
}

HydroponicsPumpRelayActuatorData::HydroponicsPumpRelayActuatorData()
    : HydroponicsRelayActuatorData(), flowRateUnits(Hydroponics_UnitsType_Undefined), contFlowRate(), outputReservoirName{0}, flowRateSensorName{0}
{
    _size = sizeof(*this);
}

void HydroponicsPumpRelayActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsRelayActuatorData::toJSONObject(objectOut);

    if (flowRateUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("flowRateUnits")] = unitsTypeToSymbol(flowRateUnits); }
    if (contFlowRate.type != -1) {
        JsonObject contFlowRateObj = objectOut.createNestedObject(F("contFlowRate"));
        contFlowRate.toJSONObject(contFlowRateObj);
    }
    if (outputReservoirName[0]) { objectOut[F("outputReservoirName")] = stringFromChars(outputReservoirName, HYDRUINO_NAME_MAXSIZE); }
    if (flowRateSensorName[0]) { objectOut[F("flowRateSensorName")] = stringFromChars(flowRateSensorName, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsPumpRelayActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsRelayActuatorData::fromJSONObject(objectIn);

    flowRateUnits = unitsTypeFromSymbol(objectIn[F("flowRateUnits")]);
    JsonVariantConst contFlowRateVar = objectIn[F("contFlowRate")];
    if (!contFlowRateVar.isNull()) { contFlowRate.fromJSONVariant(contFlowRateVar); }
    const char *outputReservoirNameStr = objectIn[F("outputReservoirName")];
    if (outputReservoirNameStr && outputReservoirNameStr[0]) { strncpy(outputReservoirName, outputReservoirNameStr, HYDRUINO_NAME_MAXSIZE); }
    const char *flowRateSensorNameStr = objectIn[F("flowRateSensorName")];
    if (flowRateSensorNameStr && flowRateSensorNameStr[0]) { strncpy(flowRateSensorName, flowRateSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsPWMActuatorData::HydroponicsPWMActuatorData()
    : HydroponicsActuatorData(), outputBitResolution(8)
{
    _size = sizeof(*this);
}

void HydroponicsPWMActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsActuatorData::toJSONObject(objectOut);

    if (outputBitResolution != 8) { objectOut[F("outputBitResolution")] = outputBitResolution; }
}

void HydroponicsPWMActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsActuatorData::fromJSONObject(objectIn);

    outputBitResolution = objectIn[F("outputBitResolution")] | outputBitResolution;
}
