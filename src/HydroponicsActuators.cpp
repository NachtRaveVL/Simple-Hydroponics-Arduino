/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Actuators
*/

#include "Hydroponics.h"

HydroponicsActuator::HydroponicsActuator(Hydroponics_ActuatorType actuatorType,
                                         Hydroponics_PositionIndex actuatorIndex,
                                         byte outputPin)
    : HydroponicsObject(HydroponicsIdentity(actuatorType, actuatorIndex)),
      _outputPin(outputPin),
      _enabled(false), _enabledUntil(0)
{
    pinMode(_outputPin, OUTPUT);
}

HydroponicsActuator::~HydroponicsActuator()
{ ; }

void HydroponicsActuator::enableActuatorUntil(time_t disableDate)
{
    //assert(disableDate >= now() && "Disable date out of range");
    _enabledUntil = disableDate;
    enableActuator();
}

void HydroponicsActuator::update()
{
    if (_enabled && _enabledUntil && now() >= _enabledUntil) {
        disableActuator();
    }
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


HydroponicsRelayActuator::HydroponicsRelayActuator(Hydroponics_ActuatorType actuatorType,
                                                   Hydroponics_PositionIndex actuatorIndex,
                                                   byte outputPin,
                                                   bool activeLow = true)
    : HydroponicsActuator(actuatorType, actuatorIndex, outputPin),
      _activeLow(activeLow)
{
    digitalWrite(_outputPin, _activeLow ? HIGH : LOW);  // Disable on start
}

HydroponicsRelayActuator::~HydroponicsRelayActuator()
{ ; }

void HydroponicsRelayActuator::disableActuator()
{
    _enabled = false;
    _enabledUntil = 0;
    digitalWrite(_outputPin, _activeLow ? HIGH : LOW);
}

void HydroponicsRelayActuator::enableActuator()
{
    _enabled = true;
    digitalWrite(_outputPin, _activeLow ? LOW : HIGH);
}

bool HydroponicsRelayActuator::getActiveLow() const
{
    return _activeLow;
}


HydroponicsPWMActuator::HydroponicsPWMActuator(Hydroponics_ActuatorType actuatorType,
                                               Hydroponics_PositionIndex actuatorIndex,
                                               byte outputPin,
                                               byte outputBitResolution)
    : HydroponicsActuator(actuatorType, actuatorIndex, outputPin),
      _pwmAmount(0.0f), _pwmResolution(outputBitResolution)
{
    applyPWM();
}

HydroponicsPWMActuator::~HydroponicsPWMActuator()
{ ; }

void HydroponicsPWMActuator::disableActuator()
{
    _enabled = false;
    _enabledUntil = 0;
    applyPWM();
}

void HydroponicsPWMActuator::enableActuator()
{
    _enabled = true;
    applyPWM();
}

float HydroponicsPWMActuator::getPWMAmount() const
{
    return _pwmAmount;
}

int HydroponicsPWMActuator::getPWMAmount(int toss) const
{
    return _pwmResolution.inverseTransform(_pwmAmount);
}

void HydroponicsPWMActuator::setPWMAmount(float amount)
{
    //assert(amount >= 0.0f && amount <= 1.0f && "Amount out of range");
    _pwmAmount = constrain(amount, 0.0f, 1.0f);

    if (_enabled) { applyPWM(); }
}

void HydroponicsPWMActuator::setPWMAmount(int amount)
{
    //assert(amount >= 0 && amount <= _pwmResolution.maxValue && "Amount out of range");
    _pwmAmount = _pwmResolution.transform(amount);

    if (_enabled) { applyPWM(); }
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
