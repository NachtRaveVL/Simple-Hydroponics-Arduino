/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Attachment Points
*/

#include "Hydroponics.h"

template<class U>
shared_ptr<U> HydroponicsDLinkObject::getObject()
{
    if (_obj) { return reinterpret_pointer_cast<U>(_obj); }
    if (_key == (Hydroponics_KeyType)-1) { return nullptr; }

    if (needsResolved() && Hydroponics::_activeInstance) {
        _obj = static_pointer_cast<HydroponicsObjInterface>(Hydroponics::_activeInstance->_objects[_key]);
    }
    if (isResolved() && _keyStr) {
        free((void *)_keyStr); _keyStr = nullptr;
    }

    return reinterpret_pointer_cast<U>(_obj);
}

template<class U>
inline HydroponicsDLinkObject &HydroponicsDLinkObject::operator=(shared_ptr<U> &rhs)
{
    _key = (rhs ? rhs->getKey() : (Hydroponics_KeyType)-1);
    _obj = reinterpret_pointer_cast<HydroponicsObjInterface>(rhs);
    if (_keyStr) { free((void *)_keyStr); _keyStr = nullptr; } return *this;
}


template<class U>
void HydroponicsAttachment::setObject(U obj)
{
    if (!(_obj == obj)) {
        if (_obj.isResolved()) { detachObject(); }
        _obj = obj;
        if (_obj.isResolved()) { attachObject(); }
    }
}

template<class U>
shared_ptr<U> HydroponicsAttachment::getObject()
{
    if (_obj.isResolved()) { return _obj.getObject<U>(); }
    if (_obj.getKey() == (Hydroponics_KeyType)-1) { return nullptr; }

    if (_obj.needsResolved() && _obj.resolve()) {
        attachObject();
    }
    return _obj.getObject<U>();
}


template<class ParameterType, int Slots> template<class T>
HydroponicsSignalAttachment<ParameterType,Slots>::HydroponicsSignalAttachment(HydroponicsObjInterface *parent, Signal<ParameterType,Slots> &(T::*signalGetter)(void))
    : HydroponicsAttachment(parent), _signalGetter((SignalGetterPtr)signalGetter)
{
    HYDRUINO_HARD_ASSERT(_signalGetter, SFP(HStr_Err_InvalidParameter));
}

template<class ParameterType, int Slots>
HydroponicsSignalAttachment<ParameterType,Slots>::~HydroponicsSignalAttachment()
{
    if (isResolved() && _handleMethod) {
        (get()->*_signalGetter)().detach(_handleMethod);
    }
}

template<class ParameterType, int Slots>
void HydroponicsSignalAttachment<ParameterType,Slots>::attachObject()
{
    HydroponicsAttachment::attachObject();

    if (isResolved() && _handleMethod) {
        (get()->*_signalGetter)().attach(_handleMethod);
    }
}

template<class ParameterType, int Slots>
void HydroponicsSignalAttachment<ParameterType,Slots>::detachObject()
{
    if (isResolved() && _handleMethod) {
        (get()->*_signalGetter)().detach(_handleMethod);
    }

    HydroponicsAttachment::detachObject();
}

template<class ParameterType, int Slots> template<class U>
void HydroponicsSignalAttachment<ParameterType,Slots>::setHandleMethod(MethodSlot<U,ParameterType> handleMethod)
{
    if (!(_handleMethod == handleMethod)) {
        if (isResolved() && _handleMethod) { (get()->*_signalGetter)().detach(_handleMethod); }
        _handleMethod = MethodSlot<HydroponicsObjInterface,ParameterType>(handleMethod.getObject(), handleMethod.getFunct());
        if (isResolved() && _handleMethod) { (get()->*_signalGetter)().attach(_handleMethod); }
    }
}


inline Hydroponics_TriggerState HydroponicsTriggerAttachment::getTriggerState() { return isResolved() ? get()->getTriggerState() : Hydroponics_TriggerState_Undefined; }

inline void HydroponicsTriggerAttachment::updateIfNeeded() { if (resolve()) { get()->update(); } }

inline Hydroponics_BalancerState HydroponicsBalancerAttachment::getBalancerState() { return isResolved() ? get()->getBalancerState() : Hydroponics_BalancerState_Undefined; }

inline void HydroponicsBalancerAttachment::updateIfNeeded() { if (resolve()) { get()->update(); } }
