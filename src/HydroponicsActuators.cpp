/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Actuators
*/

#include "HydroponicsActuators.h"

HydroponicsActuator::HydroponicsActuator(byte outputPin,
                                         Hydroponics_ActuatorType actuatorType,
                                         Hydroponics_FluidReservoir fluidReservoir)
    : _outputPin(outputPin), _actuatorType(actuatorType), _fluidReservoir(fluidReservoir),
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
    return _actuatorType;
}

Hydroponics_FluidReservoir HydroponicsActuator::getFluidReservoir() const
{
    return _fluidReservoir;
}

bool HydroponicsActuator::getIsActuatorEnabled() const
{
    return _enabled;
}

time_t HydroponicsActuator::getActuatorEnabledUntil() const
{
    return _enabled ? _enabledUntil : 0;
}


HydroponicsRelayActuator::HydroponicsRelayActuator(byte outputPin,
                                                   Hydroponics_ActuatorType actuatorType,
                                                   Hydroponics_RelayRail relayRail,
                                                   Hydroponics_FluidReservoir fluidReservoir,
                                                   bool activeLow)
    : HydroponicsActuator(outputPin, actuatorType, fluidReservoir),
      _relayRail(relayRail), _activeLow(activeLow)
{
    digitalWrite(_outputPin, _activeLow ? HIGH : LOW);  // Disable on startup
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

Hydroponics_RelayRail HydroponicsRelayActuator::getRelayRail() const
{
    return _relayRail;
}

bool HydroponicsRelayActuator::getActiveLow() const
{
    return _activeLow;
}


HydroponicsPWMActuator::HydroponicsPWMActuator(byte outputPin,
                                               Hydroponics_ActuatorType actuatorType,
                                               Hydroponics_FluidReservoir fluidReservoir,
                                               byte writeBitResolution)
    : HydroponicsActuator(outputPin, actuatorType, fluidReservoir),
      _pwmAmount(0.0f),
      #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
          _pwmBitRes(constrain(writeBitResolution, 8, 12)), _pwmMaxAmount(1 << constrain(writeBitResolution, 8, 12))
      #else
          _pwmBitRes(8), _pwmMaxAmount(256)
      #endif
{
    //assert(_pwmBitRes == writeBitResolution && "Resolved resolution mismatch with passed resolution");
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
    toss = _pwmAmount * _pwmMaxAmount;
    return constrain(toss, 0, _pwmMaxAmount);
}

void HydroponicsPWMActuator::setPWMAmount(float amount)
{
    //assert(amount >= 0.0f && amount <= 1.0f && "Amount out of range");
    _pwmAmount = amount;
    if (_pwmAmount <= FLT_EPSILON) _pwmAmount = 0.0f;
    if (_pwmAmount >= 1.0f - FLT_EPSILON) _pwmAmount = 1.0f;

    if (_enabled) { applyPWM(); }
}

void HydroponicsPWMActuator::setPWMAmount(int amount)
{
    //assert(amount >= 0 && amount <= _pwmMaxAmount && "Amount out of range");
    _pwmAmount = amount / (float)_pwmMaxAmount;
    if (_pwmAmount <= FLT_EPSILON) _pwmAmount = 0.0f;
    if (_pwmAmount >= 1.0f - FLT_EPSILON) _pwmAmount = 1.0f;

    if (_enabled) { applyPWM(); }
}

bool HydroponicsPWMActuator::getIsActuatorEnabled(float tolerance) const
{
    return _enabled && _pwmAmount - tolerance >= -FLT_EPSILON;
}

int HydroponicsPWMActuator::getPWMMaxAmount() const
{
    return _pwmMaxAmount;
}

int HydroponicsPWMActuator::getPWMBitResolution() const
{
    return _pwmBitRes;
}

void HydroponicsPWMActuator::applyPWM()
{
    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
        analogWriteResolution(_pwmBitRes);
    #endif
    analogWrite(_outputPin, _enabled ? getPWMAmount(0) : 0);
}
