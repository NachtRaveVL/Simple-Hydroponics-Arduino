/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Actuators
*/

#include "Hydroponics.h"

HydroponicsActuator *newActuatorObjectFromData(const HydroponicsActuatorData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HS_Err_InvalidParameter));

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
    HYDRUINO_HARD_ASSERT(isValidPin(_outputPin), SFP(HS_Err_InvalidPinOrType));
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
    HYDRUINO_HARD_ASSERT(isValidPin(_outputPin), SFP(HS_Err_InvalidPinOrType));
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

        if (_enabled && !wasEnabledBefore) {
            Serial.print("HERE"); Serial.flush(); yield();
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
            digitalWrite(_outputPin, _activeLow ? HIGH : LOW);
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
      _outputReservoir(dataIn->outputReservoir),
      _flowRateSensor(dataIn->flowRateSensor)
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
        time_t pumpMillis = timeMillis - _pumpTimeAccMillis;
        if (pumpMillis >= HYDRUINO_ACT_PUMPCALC_MINWRTMILLIS) {
            handlePumpTime(pumpMillis);
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
    bool wasEnabledBefore = _enabled;
    time_t timeMillis = millis();

    HydroponicsRelayActuator::enableActuator(intensity, override);

    if (_enabled && !wasEnabledBefore) {
        _pumpVolumeAcc = 0;
        _pumpTimeBegMillis = _pumpTimeAccMillis = max(1, timeMillis);
    }
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

        getLoggerInstance()->logPumping(this, SFP(HS_Log_MeasuredPumping));
        getLoggerInstance()->logMessage(SFP(HS_Log_Field_Vol), stringFromMeasurement(_pumpVolumeAcc, baseUnitsFromRate(getFlowRateUnits())));
        getLoggerInstance()->logMessage(SFP(HS_Log_Field_Time), String(timeMillis), String(' ') + String('m') + String('s'));
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
        #ifndef HYDRUINO_DISABLE_MULTITASKING
            if (scheduleActuatorTimedEnableOnce(::getSharedPtr<HydroponicsActuator>(this), timeMillis) != TASKMGR_INVALIDID) {
                getLoggerInstance()->logPumping(this, SFP(HS_Log_EstimatedPumping));
                if (_contFlowRate.value > FLT_EPSILON) {
                    getLoggerInstance()->logMessage(SFP(HS_Log_Field_Vol), stringFromMeasurement(_contFlowRate.value * (timeMillis / (float)secondsToMillis(SECS_PER_MIN)), baseUnitsFromRate(getFlowRateUnits())));
                }
                getLoggerInstance()->logMessage(SFP(HS_Log_Field_Time), String(timeMillis), String(' ') + String('m') + String('s'));
                return true;
            }
        #else
            getLoggerInstance()->logPumping(this, SFP(HS_Log_EstimatedPumping));
            if (_contFlowRate.value > FLT_EPSILON) {
                getLoggerInstance()->logMessage(SFP(HS_Log_Field_Vol), stringFromMeasurement(_contFlowRate.value * (timeMillis / (float)secondsToMillis(SECS_PER_MIN)), baseUnitsFromRate(getFlowRateUnits())));
            }
            getLoggerInstance()->logMessage(SFP(HS_Log_Field_Time), String(timeMillis), String(' ') + String('m') + String('s'));
            enableActuator();
            delayFine(timeMillis);
            disableActuator();
        #endif
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
        strncpy(((HydroponicsPumpRelayActuatorData *)dataOut)->outputReservoir, _outputReservoir.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_flowRateSensor.getId()) {
        strncpy(((HydroponicsPumpRelayActuatorData *)dataOut)->flowRateSensor, _flowRateSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
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
        _pumpVolumeAcc += volumePumped;

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
    HYDRUINO_SOFT_ASSERT(getFlowRateSensor(), SFP(HS_Err_MissingLinkage));
    if (getFlowRateSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsPumpRelayActuator::handleFlowRateMeasure);
        _flowRateSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsPumpRelayActuator::detachFlowRateSensor()
{
    HYDRUINO_SOFT_ASSERT(getFlowRateSensor(), SFP(HS_Err_MissingLinkage));
    if (getFlowRateSensor()) {
        auto methodSlot = MethodSlot<typeof(*this), const HydroponicsMeasurement *>(this, &HydroponicsPumpRelayActuator::handleFlowRateMeasure);
        _flowRateSensor->getMeasurementSignal().detach(methodSlot);
    }
}

void HydroponicsPumpRelayActuator::handleFlowRateMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame && _enabled) {
        setFlowRate(getAsSingleMeasurement(measurement, 0, _contFlowRate.value, _flowRateUnits));
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

    if (isValidPin(outputPin)) { objectOut[SFP(HS_Key_OutputPin)] = outputPin; }
    if (contPowerDraw.value > FLT_EPSILON) {
        JsonObject contPowerDrawObj = objectOut.createNestedObject(SFP(HS_Key_ContPowerDraw));
        contPowerDraw.toJSONObject(contPowerDrawObj);
    }
    if (railName[0]) { objectOut[SFP(HS_Key_Rail)] = stringFromChars(railName, HYDRUINO_NAME_MAXSIZE); }
    if (reservoirName[0]) { objectOut[SFP(HS_Key_Reservoir)] = stringFromChars(reservoirName, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);

    outputPin = objectIn[SFP(HS_Key_OutputPin)] | outputPin;
    JsonVariantConst contPowerDrawVar = objectIn[SFP(HS_Key_ContPowerDraw)];
    if (!contPowerDrawVar.isNull()) { contPowerDraw.fromJSONVariant(contPowerDrawVar); }
    const char *railStr = objectIn[SFP(HS_Key_Rail)];
    if (railStr && railStr[0]) { strncpy(railName, railStr, HYDRUINO_NAME_MAXSIZE); }
    const char *reservoirStr = objectIn[SFP(HS_Key_Reservoir)];
    if (reservoirStr && reservoirStr[0]) { strncpy(reservoirName, reservoirStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsRelayActuatorData::HydroponicsRelayActuatorData()
    : HydroponicsActuatorData(), activeLow(true)
{
    _size = sizeof(*this);
}

void HydroponicsRelayActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsActuatorData::toJSONObject(objectOut);

    objectOut[SFP(HS_Key_ActiveLow)] = activeLow;
}

void HydroponicsRelayActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsActuatorData::fromJSONObject(objectIn);

    activeLow = objectIn[SFP(HS_Key_ActiveLow)] | activeLow;
}

HydroponicsPumpRelayActuatorData::HydroponicsPumpRelayActuatorData()
    : HydroponicsRelayActuatorData(), flowRateUnits(Hydroponics_UnitsType_Undefined), contFlowRate(), outputReservoir{0}, flowRateSensor{0}
{
    _size = sizeof(*this);
}

void HydroponicsPumpRelayActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsRelayActuatorData::toJSONObject(objectOut);

    if (flowRateUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HS_Key_FlowRateUnits)] = unitsTypeToSymbol(flowRateUnits); }
    if (contFlowRate.value > FLT_EPSILON) {
        JsonObject contFlowRateObj = objectOut.createNestedObject(SFP(HS_Key_ContFlowRate));
        contFlowRate.toJSONObject(contFlowRateObj);
    }
    if (outputReservoir[0]) { objectOut[SFP(HS_Key_OutputReservoir)] = stringFromChars(outputReservoir, HYDRUINO_NAME_MAXSIZE); }
    if (flowRateSensor[0]) { objectOut[SFP(HS_Key_FlowRateSensor)] = stringFromChars(flowRateSensor, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsPumpRelayActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsRelayActuatorData::fromJSONObject(objectIn);

    flowRateUnits = unitsTypeFromSymbol(objectIn[SFP(HS_Key_FlowRateUnits)]);
    JsonVariantConst contFlowRateVar = objectIn[SFP(HS_Key_ContFlowRate)];
    if (!contFlowRateVar.isNull()) { contFlowRate.fromJSONVariant(contFlowRateVar); }
    const char *outputReservoirStr = objectIn[SFP(HS_Key_OutputReservoir)];
    if (outputReservoirStr && outputReservoirStr[0]) { strncpy(outputReservoir, outputReservoirStr, HYDRUINO_NAME_MAXSIZE); }
    const char *flowRateSensorStr = objectIn[SFP(HS_Key_FlowRateSensor)];
    if (flowRateSensorStr && flowRateSensorStr[0]) { strncpy(flowRateSensor, flowRateSensorStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsPWMActuatorData::HydroponicsPWMActuatorData()
    : HydroponicsActuatorData(), outputBitResolution(8)
{
    _size = sizeof(*this);
}

void HydroponicsPWMActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsActuatorData::toJSONObject(objectOut);

    if (outputBitResolution != 8) { objectOut[SFP(HS_Key_OutputBitRes)] = outputBitResolution; }
}

void HydroponicsPWMActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsActuatorData::fromJSONObject(objectIn);

    outputBitResolution = objectIn[SFP(HS_Key_OutputBitRes)] | outputBitResolution;
}
