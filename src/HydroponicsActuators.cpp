/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Actuators
*/

#include "Hydroponics.h"

HydroponicsActuator::HydroponicsActuator(Hydroponics_ActuatorType actuatorType,
                                         Hydroponics_PositionIndex actuatorIndex,
                                         byte outputPin,
                                         int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(actuatorType, actuatorIndex)), classType((typeof(classType))classTypeIn),
      _outputPin(outputPin),
      _enabled(false), _enabledUntil(0)
{
    pinMode(_outputPin, OUTPUT);
}

HydroponicsActuator::~HydroponicsActuator()
{
    //discardFromTaskManager(&_activateSignal);
    if (_powerRail) { _powerRail->removeActuator(this); }
    if (_reservoir) { _reservoir->removeActuator(this); }
}

void HydroponicsActuator::resolveLinks()
{
    HydroponicsObject::resolveLinks();
    if (_powerRail.needsResolved()) { getRail(); }
    if (_reservoir.needsResolved()) { getReservoir(); }
}

void HydroponicsActuator::update()
{
    HydroponicsObject::update();
    if (_enabled && _enabledUntil && now() >= _enabledUntil) {
        disableActuator();
    }
}

void HydroponicsActuator::enableActuatorUntil(time_t disableDate)
{
    HYDRUINO_SOFT_ASSERT(disableDate >= now(), F("Disable date out of range"));
    _enabledUntil = disableDate;
    enableActuator();
}

void HydroponicsActuator::setRail(HydroponicsIdentity powerRailId)
{
    if (_powerRail != powerRailId) {
        if (_powerRail) { _powerRail->removeActuator(this); }
        _powerRail = powerRailId;
    }
}

void HydroponicsActuator::setRail(shared_ptr<HydroponicsRail> powerRail)
{
    if (_powerRail != powerRail) {
        if (_powerRail) { _powerRail->removeActuator(this); }
        _powerRail = powerRail;
        if (_powerRail) { _powerRail->addActuator(this); }
    }
}

shared_ptr<HydroponicsRail> HydroponicsActuator::getRail()
{
    if (_powerRail.resolveIfNeeded()) { _powerRail->addActuator(this); }
    return _powerRail.getObj();
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
    return _id.as.actuatorType;
}

Hydroponics_PositionIndex HydroponicsActuator::getActuatorIndex() const
{
    return _id.posIndex;
}

bool HydroponicsActuator::getIsActuatorEnabled() const
{
    return _enabled;
}

time_t HydroponicsActuator::getActuatorEnabledUntil() const
{
    return _enabled ? _enabledUntil : 0;
}

Signal<HydroponicsActuator *> &HydroponicsActuator::getActivationSignal()
{
    return _activateSignal;
}


HydroponicsRelayActuator::HydroponicsRelayActuator(Hydroponics_ActuatorType actuatorType,
                                                   Hydroponics_PositionIndex actuatorIndex,
                                                   byte outputPin, bool activeLow,
                                                   int classType)
    : HydroponicsActuator(actuatorType, actuatorIndex, outputPin, classType),
      _activeLow(activeLow)
{
    digitalWrite(_outputPin, _activeLow ? HIGH : LOW);  // Disable on start
}

HydroponicsRelayActuator::~HydroponicsRelayActuator()
{ ; }

void HydroponicsRelayActuator::disableActuator()
{
    bool wasEnabledBefore = _enabled;

    _enabled = false;
    _enabledUntil = 0;
    digitalWrite(_outputPin, _activeLow ? HIGH : LOW);

    if (_enabled != wasEnabledBefore) {
        scheduleSignalFireOnce<HydroponicsActuator *>(_activateSignal, this);
    }
}

void HydroponicsRelayActuator::enableActuator()
{
    bool wasEnabledBefore = _enabled;

    _enabled = true;
    digitalWrite(_outputPin, _activeLow ? LOW : HIGH);

    if (_enabled != wasEnabledBefore) {
        scheduleSignalFireOnce<HydroponicsActuator *>(_activateSignal, this);
    }
}

bool HydroponicsRelayActuator::getActiveLow() const
{
    return _activeLow;
}


HydroponicsPumpRelayActuator::HydroponicsPumpRelayActuator(Hydroponics_ActuatorType actuatorType,
                                                           Hydroponics_PositionIndex actuatorIndex,
                                                           byte outputPin, bool activeLow,
                                                           int classType)
    :  HydroponicsRelayActuator(actuatorType, actuatorIndex, outputPin, activeLow, classType)
{ ; }

HydroponicsPumpRelayActuator::~HydroponicsPumpRelayActuator()
{
    if (_flowRateSensor) { detachFlowRateSensor(); }
    if (_outputReservoir) { _outputReservoir->removeActuator(this); }
}

void HydroponicsPumpRelayActuator::resolveLinks()
{
    HydroponicsActuator::resolveLinks();
    if (_flowRateSensor.needsResolved()) { getFlowRateSensor(); }
    if (_outputReservoir.needsResolved()) { getOutputReservoir(); }
}

void HydroponicsPumpRelayActuator::setContinuousFlowRate(float flowRate, Hydroponics_UnitsType flowRateUnits)
{
    _contFlowRate.value = flowRate;
    _contFlowRate.units = flowRateUnits != Hydroponics_UnitsType_Undefined ? flowRateUnits : defaultLiquidFlowUnits();
}

void HydroponicsPumpRelayActuator::setContinuousFlowRate(HydroponicsSingleMeasurement flowRate)
{
    _contFlowRate = flowRate;
}

const HydroponicsSingleMeasurement &HydroponicsPumpRelayActuator::getContinuousFlowRate() const
{
    return _contFlowRate;
}

void HydroponicsPumpRelayActuator::setFlowRateSensor(HydroponicsIdentity flowRateSensorId)
{
    if (_flowRateSensor != flowRateSensorId) {
        if (_flowRateSensor) { detachFlowRateSensor(); }
        _flowRateSensor = flowRateSensorId;
    }
}

void HydroponicsPumpRelayActuator::setFlowRateSensor(shared_ptr<HydroponicsSensor> flowRateSensor)
{
    if (_flowRateSensor != flowRateSensor) {
        if (_flowRateSensor) { detachFlowRateSensor(); }
        _flowRateSensor = flowRateSensor;
        if (_flowRateSensor) { attachFlowRateSensor(); }
    }
}

shared_ptr<HydroponicsSensor> HydroponicsPumpRelayActuator::getFlowRateSensor()
{
    if (_flowRateSensor.resolveIfNeeded()) { attachFlowRateSensor(); }
    return _flowRateSensor.getObj();
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

void HydroponicsPumpRelayActuator::attachFlowRateSensor()
{
    HYDRUINO_SOFT_ASSERT(_flowRateSensor, F("Flow rate sensor not linked, failure attaching"));
    if (_flowRateSensor) {
        auto methodSlot = MethodSlot<HydroponicsPumpRelayActuator, HydroponicsMeasurement *>(this, &handleFlowRateMeasure);
        _flowRateSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsPumpRelayActuator::detachFlowRateSensor()
{
    HYDRUINO_SOFT_ASSERT(_flowRateSensor, F("Flow rate sensor not linked, failure detaching"));
    if (_flowRateSensor) {
        auto methodSlot = MethodSlot<HydroponicsPumpRelayActuator, HydroponicsMeasurement *>(this, &handleFlowRateMeasure);
        _flowRateSensor->getMeasurementSignal().attach(methodSlot);
    }
}

void HydroponicsPumpRelayActuator::handleFlowRateMeasure(HydroponicsMeasurement *measurement)
{
    if (measurement) {
        if (measurement->isBinaryType()) {
            _instFlowRate.value = ((HydroponicsBinaryMeasurement *)measurement)->state ? _contFlowRate.value : 0.0f;
            _instFlowRate.units = _contFlowRate.units;
        } else if (measurement->isSingleType()) {
            _instFlowRate = *((HydroponicsSingleMeasurement *)measurement);
        } else if (measurement->isDoubleType()) {
            _instFlowRate = ((HydroponicsDoubleMeasurement *)measurement)->asSingleMeasurement(0);
        } else if (measurement->isTripleType()) {
            _instFlowRate = ((HydroponicsTripleMeasurement *)measurement)->asSingleMeasurement(0);
        }
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
    applyPWM();
}

HydroponicsPWMActuator::~HydroponicsPWMActuator()
{ ; }

void HydroponicsPWMActuator::disableActuator()
{
    bool wasEnabledBefore = _enabled;

    _enabled = false;
    _enabledUntil = 0;
    applyPWM();

    if (_enabled != wasEnabledBefore) {
        scheduleSignalFireOnce<HydroponicsActuator *>(_activateSignal, this);
    }
}

void HydroponicsPWMActuator::enableActuator()
{
    bool wasEnabledBefore = _enabled;

    _enabled = true;
    applyPWM();

    if (_enabled != wasEnabledBefore) {
        scheduleSignalFireOnce<HydroponicsActuator *>(_activateSignal, this);
    }
}

float HydroponicsPWMActuator::getPWMAmount() const
{
    return _pwmAmount;
}

int HydroponicsPWMActuator::getPWMAmount(int discard) const
{
    return _pwmResolution.inverseTransform(_pwmAmount);
}

void HydroponicsPWMActuator::setPWMAmount(float amount)
{
    HYDRUINO_SOFT_ASSERT(amount >= 0.0f && amount <= 1.0f, F("Amount out of range"));
    _pwmAmount = constrain(amount, 0.0f, 1.0f);

    if (_enabled) {
        if (amount > FLT_EPSILON) { applyPWM(); }
        else { disableActuator(); }
    } else if (amount > FLT_EPSILON) { enableActuator(); }
}

void HydroponicsPWMActuator::setPWMAmount(int amount)
{
    HYDRUINO_SOFT_ASSERT(amount >= 0 && amount <= _pwmResolution.maxVal, F("Amount out of range"));
    _pwmAmount = _pwmResolution.transform(amount);

    if (_enabled) {
        if (amount) { applyPWM(); }
        else { disableActuator(); }
    } else if (amount) { enableActuator(); }
}

bool HydroponicsPWMActuator::getIsActuatorEnabled(float tolerance) const
{
    return _enabled && _pwmAmount - tolerance >= -FLT_EPSILON;
}

HydroponicsBitResolution HydroponicsPWMActuator::getPWMResolution() const
{
    return _pwmResolution;
}

void HydroponicsPWMActuator::applyPWM()
{
    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
        analogWriteResolution(_pwmResolution.bitRes);
    #endif
    analogWrite(_outputPin, _enabled ? getPWMAmount(0) : 0);
}
