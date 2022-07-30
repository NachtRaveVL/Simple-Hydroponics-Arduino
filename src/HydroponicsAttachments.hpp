/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Attachment Points
*/

#include "Hydroponics.h"

template<class U>
shared_ptr<U> HydroponicsDLinkObject::getObject()
{
    if (needsResolved() && Hydroponics::_activeInstance) {
        _obj = static_pointer_cast<HydroponicsObjInterface>(Hydroponics::_activeInstance->_objects[_key]);
    }
    if (isResolved() && _keyStr) {
        free((void *)_keyStr); _keyStr = nullptr;
    }
    return static_pointer_cast<U>(_obj);
}


template<class U>
void HydroponicsAttachment::setObject(U obj)
{
    if (_obj != obj) {
        if (isResolved()) { detachObject(); }
        _obj = obj;
        if (isResolved()) { attachObject(); }
    }
}

template<class U>
shared_ptr<U> HydroponicsAttachment::getObject()
{
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
        _handleMethod = MethodSlot<HydroponicsObjInterface,ParameterType>((HydroponicsObjInterface *)handleMethod.getObject(), (HandleMethodPtr)handleMethod.getFunct());
        if (isResolved() && _handleMethod) { (get()->*_signalGetter)().attach(_handleMethod); }
    }
}

inline Hydroponics_TriggerState HydroponicsTriggerAttachment::getTriggerState() { return get()->getTriggerState(); }

inline Hydroponics_BalancerState HydroponicsBalancerAttachment::getBalancerState() { return get()->getBalancerState(); }
