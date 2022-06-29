/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Actuators
*/

#include "Hydroponics.h"

HydroponicsActuator *newActuatorObjectFromData(const HydroponicsActuatorData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjData(), F("Invalid data"));

    if (dataIn && dataIn->isObjData()) {
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
      _outputPin(outputPin), _disableTime(0)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_outputPin), F("Invalid output pin"));
    pinMode(_outputPin, OUTPUT);
}

HydroponicsActuator::HydroponicsActuator(const HydroponicsActuatorData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))dataIn->id.object.classType),
      _outputPin(dataIn->outputPin), _disableTime(0),
      _rail(dataIn->railName), _reservoir(dataIn->reservoirName)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_outputPin), F("Invalid output pin"));
    pinMode(_outputPin, OUTPUT);
}

HydroponicsActuator::~HydroponicsActuator()
{
    //discardFromTaskManager(&_activateSignal);
    if (_rail) { _rail->removeActuator(this); }
    if (_reservoir) { _reservoir->removeActuator(this); }
}

void HydroponicsActuator::update()
{
    HydroponicsObject::update();

    if (_disableTime && now() >= _disableTime) {
        disableActuator();
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

void HydroponicsActuator::disableAt(time_t disableTime)
{
    _disableTime = disableTime;
}

bool HydroponicsActuator::getCanEnable()
{
    if (_rail && !_rail->canActivate(this)) { return false; }
    if (_reservoir && !_reservoir->canActivate(this)) { return false; }
    return true;
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
    return _id.typeAs.actuatorType;
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

void HydroponicsActuator::saveToData(HydroponicsData *dataOut) const
{
    HydroponicsObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
    ((HydroponicsActuatorData *)dataOut)->outputPin = _outputPin;
}


HydroponicsRelayActuator::HydroponicsRelayActuator(Hydroponics_ActuatorType actuatorType,
                                                   Hydroponics_PositionIndex actuatorIndex,
                                                   byte outputPin, bool activeLow,
                                                   int classType)
    : HydroponicsActuator(actuatorType, actuatorIndex, outputPin, classType),
      _activeLow(activeLow), _enabled(false)
{
    digitalWrite(_outputPin, _activeLow ? HIGH : LOW); // Disable on start
}

HydroponicsRelayActuator::HydroponicsRelayActuator(const HydroponicsRelayActuatorData *dataIn)
    : HydroponicsActuator(dataIn), _activeLow(dataIn->activeLow), _enabled(false)
{
    digitalWrite(_outputPin, _activeLow ? HIGH : LOW); // Disable on start
}

HydroponicsRelayActuator::~HydroponicsRelayActuator()
{ ; }

bool HydroponicsRelayActuator::enableActuator(bool override, float intensity)
{
    bool wasEnabledBefore = _enabled;
    bool canEnable = _enabled || override || getCanEnable();

    if (!_enabled && canEnable) {
        _enabled = true;
        digitalWrite(_outputPin, _activeLow ? LOW : HIGH);
    }

    if (_enabled != wasEnabledBefore) {
        scheduleSignalFireOnce<HydroponicsActuator *>(_activateSignal, this);
    }
}

void HydroponicsRelayActuator::disableActuator()
{
    bool wasEnabledBefore = _enabled;

    if (_enabled) {
        _enabled = false;
        _disableTime = 0;
        digitalWrite(_outputPin, _activeLow ? HIGH : LOW);
    }

    if (_enabled != wasEnabledBefore) {
        scheduleSignalFireOnce<HydroponicsActuator *>(_activateSignal, this);
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

void HydroponicsRelayActuator::saveToData(HydroponicsData *dataOut) const
{
    HydroponicsActuator::saveToData(dataOut);

    ((HydroponicsRelayActuatorData *)dataOut)->activeLow = _activeLow;
}


HydroponicsPumpRelayActuator::HydroponicsPumpRelayActuator(Hydroponics_ActuatorType actuatorType,
                                                           Hydroponics_PositionIndex actuatorIndex,
                                                           byte outputPin, bool activeLow,
                                                           int classType)
    :  HydroponicsRelayActuator(actuatorType, actuatorIndex, outputPin, activeLow, classType),
       _flowRateUnits(defaultLiquidFlowUnits())
{ ; }

HydroponicsPumpRelayActuator::HydroponicsPumpRelayActuator(const HydroponicsPumpRelayActuatorData *dataIn)
    : HydroponicsRelayActuator(dataIn),
      _outputReservoir(dataIn->outputReservoirName), _flowRateSensor(dataIn->flowRateSensorName),
      _flowRateUnits(defaultLiquidFlowUnits()), _contFlowRate(&(dataIn->contFlowRate))
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

bool HydroponicsPumpRelayActuator::canPump(float volume, Hydroponics_UnitsType volumeUnits)
{
    // TODO
    return false;
}

void HydroponicsPumpRelayActuator::pump(float volume, Hydroponics_UnitsType volumeUnits)
{
    // TODO
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
    _flowRateUnits = flowRateUnits;
}

Hydroponics_UnitsType HydroponicsPumpRelayActuator::getFlowRateUnits() const
{
    return _flowRateUnits;
}

void HydroponicsPumpRelayActuator::setContinuousFlowRate(float contFlowRate, Hydroponics_UnitsType contFlowRateUnits)
{
    _contFlowRate.value = contFlowRate;
    _contFlowRate.units = contFlowRateUnits != Hydroponics_UnitsType_Undefined ? contFlowRateUnits : defaultLiquidFlowUnits();
}

void HydroponicsPumpRelayActuator::setContinuousFlowRate(HydroponicsSingleMeasurement contFlowRate)
{
    _contFlowRate = contFlowRate;
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

void HydroponicsPumpRelayActuator::setInstantaneousFlowRate(float instFlowRate, Hydroponics_UnitsType instFlowRateUnits)
{
    _instFlowRate.value = instFlowRate;
    _instFlowRate.units = instFlowRateUnits != Hydroponics_UnitsType_Undefined ? instFlowRateUnits : defaultLiquidFlowUnits();
}

void HydroponicsPumpRelayActuator::setInstantaneousFlowRate(HydroponicsSingleMeasurement instFlowRate)
{
    _instFlowRate = instFlowRate;
}

const HydroponicsSingleMeasurement &HydroponicsPumpRelayActuator::getInstantaneousFlowRate() const
{
    return _instFlowRate;
}

void HydroponicsPumpRelayActuator::saveToData(HydroponicsData *dataOut) const
{
    HydroponicsRelayActuator::saveToData(dataOut);

    if (_outputReservoir.getId()) {
        strncpy(((HydroponicsPumpRelayActuatorData *)dataOut)->outputReservoirName, _outputReservoir.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_flowRateSensor.getId()) {
        strncpy(((HydroponicsPumpRelayActuatorData *)dataOut)->flowRateSensorName, _flowRateSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    ((HydroponicsPumpRelayActuatorData *)dataOut)->flowRateUnits = _flowRateUnits;
    if (!_contFlowRate.isUnknownType()) {
        _contFlowRate.saveToData(&(((HydroponicsPumpRelayActuatorData *)dataOut)->contFlowRate));
    }
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
            setInstantaneousFlowRate(((HydroponicsBinaryMeasurement *)measurement)->state ? _contFlowRate.value : 0.0f, _contFlowRate.units);
        } else if (measurement->isSingleType()) {
            setInstantaneousFlowRate(*((HydroponicsSingleMeasurement *)measurement));
        } else if (measurement->isDoubleType()) {
            setInstantaneousFlowRate(((HydroponicsDoubleMeasurement *)measurement)->asSingleMeasurement(0)); // TODO: Correct row reference, based on sensor
        } else if (measurement->isTripleType()) {
            setInstantaneousFlowRate(((HydroponicsTripleMeasurement *)measurement)->asSingleMeasurement(0)); // TODO: Correct row reference, based on sensor
        }
    }
}


HydroponicsPWMActuator::HydroponicsPWMActuator(Hydroponics_ActuatorType actuatorType,
                                               Hydroponics_PositionIndex actuatorIndex,
                                               byte outputPin,
                                               byte outputBitResolution,
                                               int classType)
    : HydroponicsActuator(actuatorType, actuatorIndex, outputPin, classType),
      _enabled(false), _pwmAmount(0.0f), _pwmResolution(outputBitResolution)
{
    applyPWM();
}

HydroponicsPWMActuator::HydroponicsPWMActuator(const HydroponicsPWMActuatorData *dataIn)
    : HydroponicsActuator(dataIn),
      _enabled(false), _pwmAmount(0.0f), _pwmResolution(dataIn->outputBitResolution)
{
    applyPWM();
}

HydroponicsPWMActuator::~HydroponicsPWMActuator()
{ ; }

bool HydroponicsPWMActuator::enableActuator(bool override, float intensity)
{
    bool wasEnabledBefore = _enabled;
    bool canEnable = _enabled || override || getCanEnable();

    if ((!_enabled && canEnable) || (_enabled && !isFPEqual(_pwmAmount, intensity))) {
        _enabled = true;
        _pwmAmount = constrain(intensity, 0.0f, 1.0f);
        applyPWM();
    }

    if (_enabled != wasEnabledBefore) {
        scheduleSignalFireOnce<HydroponicsActuator *>(_activateSignal, this);
    }
    return true;
}

void HydroponicsPWMActuator::disableActuator()
{
    bool wasEnabledBefore = _enabled;

    if (_enabled) {
        _enabled = false;
        _disableTime = 0;
        applyPWM();
    }

    if (_enabled != wasEnabledBefore) {
        scheduleSignalFireOnce<HydroponicsActuator *>(_activateSignal, this);
    }
}

bool HydroponicsPWMActuator::getIsEnabled(float tolerance) const
{
    return _enabled && _pwmAmount  >= tolerance - FLT_EPSILON;
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
    HYDRUINO_SOFT_ASSERT(amount >= 0.0f && amount <= 1.0f, F("PWM amount out of range"));
    _pwmAmount = constrain(amount, 0.0f, 1.0f);

    if (_enabled) {
        if (amount > FLT_EPSILON) { applyPWM(); }
        else { disableActuator(); }
    }
}

void HydroponicsPWMActuator::setPWMAmount(int amount)
{
    HYDRUINO_SOFT_ASSERT(amount >= 0 && amount <= _pwmResolution.maxVal, F("PWM amount out of range"));
    _pwmAmount = _pwmResolution.transform(amount);

    if (_enabled) {
        if (amount) { applyPWM(); }
        else { disableActuator(); }
    }
}

HydroponicsBitResolution HydroponicsPWMActuator::getPWMResolution() const
{
    return _pwmResolution;
}

void HydroponicsPWMActuator::saveToData(HydroponicsData *dataOut) const
{
    HydroponicsActuator::saveToData(dataOut);

    ((HydroponicsPWMActuatorData *)dataOut)->outputBitResolution = _pwmResolution.bitRes;
}

void HydroponicsPWMActuator::applyPWM()
{
    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
        analogWriteResolution(_pwmResolution.bitRes);
    #endif
    analogWrite(_outputPin, _enabled ? getPWMAmount(0) : 0);
}


HydroponicsActuatorData::HydroponicsActuatorData()
    : HydroponicsObjectData(), outputPin(-1), railName{0}, reservoirName{0}
{
    _size = sizeof(*this);
}

void HydroponicsActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsObjectData::toJSONObject(objectOut);

    if (isValidPin(outputPin)) { objectOut[F("outputPin")] = outputPin; }
    if (railName[0]) { objectOut[F("railName")] = stringFromChars(railName, HYDRUINO_NAME_MAXSIZE); }
    if (reservoirName[0]) { objectOut[F("reservoirName")] = stringFromChars(reservoirName, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);

    outputPin = objectIn[F("outputPin")] | outputPin;
    const char *railNameStr = objectIn[F("railName")];
    if (railNameStr && railNameStr[0]) { strncpy(railName, railNameStr, HYDRUINO_NAME_MAXSIZE); }
    const char *reservoirNameStr = objectIn[F("reservoirName")];
    if (reservoirNameStr && reservoirNameStr[0]) { strncpy(reservoirName, reservoirNameStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsRelayActuatorData::HydroponicsRelayActuatorData()
    : HydroponicsActuatorData(), activeLow(false)
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
    : HydroponicsRelayActuatorData(), outputReservoirName{0}, flowRateSensorName{0},
      flowRateUnits(Hydroponics_UnitsType_Undefined), contFlowRate()
{
    _size = sizeof(*this);
}

void HydroponicsPumpRelayActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsRelayActuatorData::toJSONObject(objectOut);

    if (outputReservoirName[0]) { objectOut[F("outputReservoirName")] = stringFromChars(outputReservoirName, HYDRUINO_NAME_MAXSIZE); }
    if (flowRateSensorName[0]) { objectOut[F("flowRateSensorName")] = stringFromChars(flowRateSensorName, HYDRUINO_NAME_MAXSIZE); }
    if (flowRateUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("flowRateUnits")] = flowRateUnits; }
    if (contFlowRate.type != -1) {
        JsonObject contFlowRateObj = objectOut.createNestedObject(F("contFlowRate"));
        contFlowRate.toJSONObject(contFlowRateObj);
    }
}

void HydroponicsPumpRelayActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsRelayActuatorData::fromJSONObject(objectIn);

    const char *outputReservoirNameStr = objectIn[F("outputReservoirName")];
    if (outputReservoirNameStr && outputReservoirNameStr[0]) { strncpy(outputReservoirName, outputReservoirNameStr, HYDRUINO_NAME_MAXSIZE); }
    const char *flowRateSensorNameStr = objectIn[F("flowRateSensorName")];
    if (flowRateSensorNameStr && flowRateSensorNameStr[0]) { strncpy(flowRateSensorName, flowRateSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    flowRateUnits = objectIn[F("flowRateUnits")] | flowRateUnits;
    JsonVariantConst contFlowRateVar = objectIn[F("contFlowRate")];
    if (!contFlowRateVar.isNull()) { contFlowRate.fromJSONVariant(contFlowRateVar); }
}

HydroponicsPWMActuatorData::HydroponicsPWMActuatorData()
    : HydroponicsActuatorData(), outputBitResolution(0)
{
    _size = sizeof(*this);
}

void HydroponicsPWMActuatorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsActuatorData::toJSONObject(objectOut);

    objectOut[F("outputBitResolution")] = outputBitResolution;
}

void HydroponicsPWMActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsActuatorData::fromJSONObject(objectIn);

    outputBitResolution = objectIn[F("outputBitResolution")] | outputBitResolution;
}
