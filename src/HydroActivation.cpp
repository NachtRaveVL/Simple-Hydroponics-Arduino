/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Activations
*/

#include "Hydruino.h"

HydroActivationHandle::HydroActivationHandle(SharedPtr<HydroActuator> actuatorIn, Hydro_DirectionMode direction, float intensity, millis_t duration, bool force)
    : activation(direction, constrain(intensity, 0.0f, 1.0f), duration, (force ? Hydro_ActivationFlags_Forced : Hydro_ActivationFlags_None)), 
      actuator(nullptr), checkTime(0), elapsed(0)
{
    operator=(actuatorIn);
}

HydroActivationHandle::HydroActivationHandle(const HydroActivationHandle &handle)
    : actuator(nullptr), activation(handle.activation), checkTime(0), elapsed(0)
{
    operator=(handle.actuator);
}

HydroActivationHandle::~HydroActivationHandle()
{
    if (actuator) { unset(); }
}

HydroActivationHandle &HydroActivationHandle::operator=(SharedPtr<HydroActuator> actuatorIn)
{
    if (actuator != actuatorIn && isValid()) {
        if (actuator) { unset(); } else { checkTime = 0; }

        actuator = actuatorIn;

        if (actuator) { actuator->_handles.push_back(this); actuator->setNeedsUpdate(); }
    }
    return *this;
}

void HydroActivationHandle::unset()
{
    if (isActive()) { elapseTo(); }
    checkTime = 0;

    if (actuator) {
        for (auto handleIter = actuator->_handles.end() - 1; handleIter != actuator->_handles.begin() - 1; --handleIter) {
            if ((*handleIter) == this) {
                actuator->_handles.erase(handleIter);
                break;
            }
        }
        actuator->setNeedsUpdate();
        actuator = nullptr;
    }
}

void HydroActivationHandle::elapseBy(millis_t delta)
{
    if (delta && isValid() && isActive()) {
        if (!isUntimed()) {
            if (delta <= activation.duration) {
                activation.duration -= delta;
                checkTime += delta;
            } else {
                delta = activation.duration;
                activation.duration = 0;
                checkTime = 0;
                actuator->setNeedsUpdate();
            }
        }
        elapsed += delta;
    }
}
