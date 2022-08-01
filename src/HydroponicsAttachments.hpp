/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Attachment Points
*/

#include "Hydroponics.h"

inline HydroponicsDLinkObject &HydroponicsDLinkObject::operator=(HydroponicsIdentity rhs)
{
    _key = rhs.key;
    _obj = nullptr;
    auto len = rhs.keyString.length();
    if (len) {
        _keyStr = (const char *)malloc(len + 1);
        strncpy((char *)_keyStr, rhs.keyString.c_str(), len + 1);
    }
    return *this;
}

inline HydroponicsDLinkObject &HydroponicsDLinkObject::operator=(const char *rhs)
{
    _key = stringHash(rhs);
    _obj = nullptr;
    auto len = strnlen(rhs, HYDRUINO_NAME_MAXSIZE);
    if (len) {
        _keyStr = (const char *)malloc(len + 1);
        strncpy((char *)_keyStr, rhs, len + 1);
    }
    return *this;
}

inline HydroponicsDLinkObject &HydroponicsDLinkObject::operator=(const HydroponicsObjInterface *rhs)
{
    _key = rhs ? rhs->getKey() : (Hydroponics_KeyType)-1;
    _obj = rhs ? getSharedPtr<HydroponicsObjInterface>(rhs) : nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }
    return *this;
}

template<class U>
inline HydroponicsDLinkObject &HydroponicsDLinkObject::operator=(shared_ptr<U> &rhs)
{
    _key = rhs ? rhs->getKey() : (Hydroponics_KeyType)-1;
    _obj = rhs ? reinterpret_pointer_cast<HydroponicsObjInterface>(rhs) : nullptr;
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; }
    return *this;
}


template<class U>
void HydroponicsAttachment::setObject(U obj)
{
    if (!(_obj == obj)) {
        if (_obj.isResolved()) { detachObject(); }

        _obj = obj; // will be replaced by templated operator= inline

        if (_obj.isResolved()) { attachObject(); }
    }
}

template<class U>
shared_ptr<U> HydroponicsAttachment::getObject()
{
    if (_obj) { return _obj.getObject<U>(); }
    if (_obj.getKey() == (Hydroponics_KeyType)-1) { return nullptr; }

    if (_obj.needsResolved() && _obj._getObject()) {
        attachObject();
    }
    return _obj.getObject<U>();
}


template<class ParameterType, int Slots> template<class U>
HydroponicsSignalAttachment<ParameterType,Slots>::HydroponicsSignalAttachment(HydroponicsObjInterface *parent, Signal<ParameterType,Slots> &(U::*signalGetter)(void))
    : HydroponicsAttachment(parent), _signalGetter((SignalGetterPtr)signalGetter), _handleMethod(nullptr)
{
    HYDRUINO_HARD_ASSERT(_signalGetter, SFP(HStr_Err_InvalidParameter));
}

template<class ParameterType, int Slots>
HydroponicsSignalAttachment<ParameterType,Slots>::HydroponicsSignalAttachment(const HydroponicsSignalAttachment<ParameterType,Slots> &attachment)
    : HydroponicsAttachment(attachment._parent), _signalGetter((SignalGetterPtr)attachment._signalGetter), _handleMethod((HandleMethodSlotPtr)(attachment._handleMethod ? attachment._handleMethod->clone() : nullptr))
{
    HYDRUINO_HARD_ASSERT(_signalGetter, SFP(HStr_Err_InvalidParameter));
}

template<class ParameterType, int Slots>
HydroponicsSignalAttachment<ParameterType,Slots>::~HydroponicsSignalAttachment()
{
    if (isResolved() && _handleMethod) {
        (get()->*_signalGetter)().detach(*_handleMethod);
    }
}

template<class ParameterType, int Slots>
void HydroponicsSignalAttachment<ParameterType,Slots>::attachObject()
{
    HydroponicsAttachment::attachObject();

    if (_handleMethod) {
        (get()->*_signalGetter)().attach(*_handleMethod);
    }
}

template<class ParameterType, int Slots>
void HydroponicsSignalAttachment<ParameterType,Slots>::detachObject()
{
    if (isResolved() && _handleMethod) {
        (get()->*_signalGetter)().detach(*_handleMethod);
    }

    HydroponicsAttachment::detachObject();
}

template<class ParameterType, int Slots> template<class U>
void HydroponicsSignalAttachment<ParameterType,Slots>::setHandleMethod(MethodSlot<U,ParameterType> handleMethod)
{
    if (!_handleMethod || (*_handleMethod == handleMethod)) {
        if (isResolved() && _handleMethod) { (get()->*_signalGetter)().detach(*_handleMethod); }

        if (_handleMethod) { delete _handleMethod; _handleMethod = nullptr; }
        _handleMethod = (HandleMethodSlotPtr)handleMethod.clone();

        if (isResolved() && _handleMethod) { (get()->*_signalGetter)().attach(*_handleMethod); }
    }
}

inline void HydroponicsSensorAttachment::updateIfNeeded(bool poll)
{
    if (resolve() && (_needsMeasurement || poll)) {
        if (_handleMethod) { _handleMethod->operator()(get()->getLatestMeasurement()); }
        else { handleMeasurement(get()->getLatestMeasurement()); }

        get()->takeMeasurement((_needsMeasurement || poll));
    }
}


inline Hydroponics_TriggerState HydroponicsTriggerAttachment::getTriggerState()
{
    return resolve() ? get()->getTriggerState() : Hydroponics_TriggerState_Undefined;
}

inline void HydroponicsTriggerAttachment::updateIfNeeded()
{
    if (resolve()) { get()->update(); }
}


inline Hydroponics_BalancerState HydroponicsBalancerAttachment::getBalancerState()
{
    return resolve() ? get()->getBalancerState() : Hydroponics_BalancerState_Undefined;
}

inline void HydroponicsBalancerAttachment::updateIfNeeded()
{
    if (resolve()) { get()->update(); }
}
