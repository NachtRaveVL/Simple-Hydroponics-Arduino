/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Attachment Points
*/

#include "Hydroponics.h"

template<class U>
shared_ptr<U> HydroponicsDLinkObject::getObject()
{
    if (needsResolved() && Hydroponics::_activeInstance) {
        _obj = Hydroponics::_activeInstance->objectById(_id);
        if ((bool)_obj) { _id = _obj->getId(); } // ensures complete id
    }
    return reinterpret_pointer_cast<U>(_obj);
}

template<class U>
HydroponicsDLinkObject &HydroponicsDLinkObject::operator=(const U *rhs)
{
    _id = (rhs ? rhs->getId() : HydroponicsIdentity());
    _obj = (rhs ? rhs->getSharedPtr() : nullptr);
    return *this;
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


template<class ParameterType, int Slots> template<class T, class U>
HydroponicsSignalAttachment<ParameterType,Slots>::HydroponicsSignalAttachment(HydroponicsObject *parent, Signal<ParameterType,Slots> &(T::*signalGetter)(void), MethodSlot<U,ParameterType> handleMethod)
    : HydroponicsAttachment(parent), _signalGetter((SignalGetterPtr)signalGetter), _handleMethod(handleMethod)
{
    HYDRUINO_HARD_ASSERT(_signalGetter && _handleMethod, SFP(HS_Err_InvalidParameter));
}

template<class ParameterType, int Slots>
HydroponicsSignalAttachment<ParameterType,Slots>::~HydroponicsSignalAttachment()
{
    if (isResolved()) {
        (get()->*_signalGetter)().detach(_handleMethod);
    }
}

template<class ParameterType, int Slots>
void HydroponicsSignalAttachment<ParameterType,Slots>::attachObject()
{
    HydroponicsAttachment::attachObject();

    (get()->*_signalGetter)().attach(_handleMethod);
}

template<class ParameterType, int Slots>
void HydroponicsSignalAttachment<ParameterType,Slots>::detachObject()
{
    if (isResolved()) {
        (get()->*_signalGetter)().detach(_handleMethod);
    }

    HydroponicsAttachment::detachObject();
}
