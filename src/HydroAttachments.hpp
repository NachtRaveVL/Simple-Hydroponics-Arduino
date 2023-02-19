/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Attachment Points
*/

#include "Hydruino.h"

inline HydroDLinkObject &HydroDLinkObject::operator=(HydroIdentity rhs)
{
    _key = rhs.key;
    _obj = nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    auto len = rhs.keyString.length();
    if (len) {
        _keyStr = (const char *)malloc(len + 1);
        strncpy((char *)_keyStr, rhs.keyString.c_str(), len + 1);
    }
    return *this;
}

inline HydroDLinkObject &HydroDLinkObject::operator=(const char *rhs)
{
    _key = stringHash(rhs);
    _obj = nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    auto len = strnlen(rhs, HYDRO_NAME_MAXSIZE);
    if (len) {
        _keyStr = (const char *)malloc(len + 1);
        strncpy((char *)_keyStr, rhs, len + 1);
    }
    return *this;
}

inline HydroDLinkObject &HydroDLinkObject::operator=(const HydroObjInterface *rhs)
{
    _key = rhs ? rhs->getKey() : hkey_none;
    _obj = rhs ? getSharedPtr<HydroObjInterface>(rhs) : nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    return *this;
}

inline HydroDLinkObject &HydroDLinkObject::operator=(const HydroAttachment *rhs)
{
    _key = rhs ? rhs->getKey() : hkey_none;
    _obj = rhs && rhs->isResolved() ? rhs->getSharedPtr() : nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    if (rhs && !rhs->isResolved()) {
        String keyString = rhs->getKeyString();
        auto len = keyString.length();
        if (len) {
            _keyStr = (const char *)malloc(len + 1);
            strncpy((char *)_keyStr, keyString.c_str(), len + 1);
        }
    }
    return *this;
}

template<class U>
inline HydroDLinkObject &HydroDLinkObject::operator=(SharedPtr<U> &rhs)
{
    _key = rhs ? rhs->getKey() : hkey_none;
    _obj = rhs ? reinterpret_pointer_cast<HydroObjInterface>(rhs) : nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }

    return *this;
}


template<class U>
void HydroAttachment::setObject(U obj)
{
    if (!(_obj == obj)) {
        if (_obj.isResolved()) { detachObject(); }

        _obj = obj; // will be replaced by templated operator= inline

        if (_obj.isResolved()) { attachObject(); }
    }
}

template<class U>
SharedPtr<U> HydroAttachment::getObject()
{
    if (_obj) { return _obj.getObject<U>(); }
    if (_obj.getKey() == hkey_none) { return nullptr; }

    if (_obj.needsResolved() && _obj._getObject()) {
        attachObject();
    }
    return _obj.getObject<U>();
}


template<class ParameterType, int Slots> template<class U>
HydroSignalAttachment<ParameterType,Slots>::HydroSignalAttachment(HydroObjInterface *parent, Signal<ParameterType,Slots> &(U::*signalGetter)(void))
    : HydroAttachment(parent), _signalGetter((SignalGetterPtr)signalGetter), _handleSlot(nullptr)
{ ; }

template<class ParameterType, int Slots>
HydroSignalAttachment<ParameterType,Slots>::HydroSignalAttachment(const HydroSignalAttachment<ParameterType,Slots> &attachment)
    : HydroAttachment(attachment), _signalGetter((SignalGetterPtr)attachment._signalGetter),
      _handleSlot(attachment._handleSlot ? attachment._handleSlot->clone() : nullptr)
{ ; }

template<class ParameterType, int Slots>
HydroSignalAttachment<ParameterType,Slots>::~HydroSignalAttachment()
{
    if (isResolved() && _handleSlot && _signalGetter) {
        (get()->*_signalGetter)().detach(*_handleSlot);
    }
    if (_handleSlot) {
        delete _handleSlot; _handleSlot = nullptr;
    }
}

template<class ParameterType, int Slots>
void HydroSignalAttachment<ParameterType,Slots>::attachObject()
{
    HydroAttachment::attachObject();

    if (isResolved() && _handleSlot && _signalGetter) {
        (get()->*_signalGetter)().attach(*_handleSlot);
    }
}

template<class ParameterType, int Slots>
void HydroSignalAttachment<ParameterType,Slots>::detachObject()
{
    if (isResolved() && _handleSlot && _signalGetter) {
        (get()->*_signalGetter)().detach(*_handleSlot);
    }

    HydroAttachment::detachObject();
}

template<class ParameterType, int Slots> template<class U>
void HydroSignalAttachment<ParameterType,Slots>::setSignalGetter(Signal<ParameterType,Slots> &(U::*signalGetter)(void))
{
    if (_signalGetter != signalGetter) {
        if (isResolved() && _handleSlot && _signalGetter) { (get()->*_signalGetter)().detach(*_handleSlot); }

        _signalGetter = signalGetter;

        if (isResolved() && _handleSlot && _signalGetter) { (get()->*_signalGetter)().attach(*_handleSlot); }
    }
}

template<class ParameterType, int Slots>
void HydroSignalAttachment<ParameterType,Slots>::setHandleSlot(const Slot<ParameterType> &handleSlot)
{
    if (!_handleSlot || !_handleSlot->operator==(&handleSlot)) {
        if (isResolved() && _handleSlot && _signalGetter) { (get()->*_signalGetter)().detach(*_handleSlot); }

        if (_handleSlot) { delete _handleSlot; _handleSlot = nullptr; }
        _handleSlot = handleSlot.clone();

        if (isResolved() && _handleSlot && _signalGetter) { (get()->*_signalGetter)().attach(*_handleSlot); }
    }
}


inline Hydro_TriggerState HydroTriggerAttachment::getTriggerState(bool poll)
{
    return resolve() ? get()->getTriggerState(poll) : Hydro_TriggerState_Undefined;
}


inline Hydro_BalancingState HydroBalancerAttachment::getBalancingState(bool poll)
{
    return resolve() ? get()->getBalancingState(poll) : Hydro_BalancingState_Undefined;
}
