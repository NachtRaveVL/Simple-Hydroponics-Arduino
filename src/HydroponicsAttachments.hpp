/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Attachment Points
*/

#include "Hydroponics.h"

template<class T>
HydroponicsDLinkObject<T>::HydroponicsDLinkObject()
    : _id(), _obj(nullptr)
{ ; }

template<class T>
HydroponicsDLinkObject<T>::HydroponicsDLinkObject(const HydroponicsIdentity &id)
    : _id(id), _obj(nullptr)
{ ; }

template<class T>
HydroponicsDLinkObject<T>::HydroponicsDLinkObject(const char *idKeyStr)
    : _id(HydroponicsIdentity(idKeyStr)), _obj(nullptr)
{ ; }

template<class T> template<class U>
HydroponicsDLinkObject<T>::HydroponicsDLinkObject(shared_ptr<U> obj)
    : _id(obj ? obj->getId() : HydroponicsIdentity()), _obj(static_pointer_cast<T>(obj))
{ ; }

template<class T> template<class U>
HydroponicsDLinkObject<T>::HydroponicsDLinkObject(const U *obj)
    : _id(obj ? obj->getId() : HydroponicsIdentity()), _obj(obj ? getSharedPtr<T>((HydroponicsObject *)obj) : nullptr)
{ ; }

template<class T> template<class U>
HydroponicsDLinkObject<T>::HydroponicsDLinkObject(const HydroponicsDLinkObject<U> &obj)
    : _id(obj.id), _obj(static_pointer_cast<T>(obj._obj))
{ ; }

template<class T>
shared_ptr<T> HydroponicsDLinkObject<T>::getObject()
{
    if (!_obj && (bool)_id && Hydroponics::_activeInstance) {
        _obj = Hydroponics::_activeInstance->objectById(_id);
        if ((bool)_obj) { _id = _obj->getId(); } // ensures complete id
    }
    return _obj;
}

template<class T> template<class U>
HydroponicsDLinkObject<T> &HydroponicsDLinkObject<T>::operator=(const U *rhs)
{
    _id = (rhs ? rhs->getId() : _id); // saves id if nulling out, used for attachment tracking
    _obj = (rhs ? getSharedPtr<T>(rhs) : nullptr);
    return *this;
}


template<class T>
HydroponicsAttachment<T>::HydroponicsAttachment(HydroponicsObject *parent, const HydroponicsIdentity &id)
    : _parent(parent), _obj(id)
{
    HYDRUINO_HARD_ASSERT(_parent, SFP(HS_Err_InvalidParameter));
}

template<class T>
HydroponicsAttachment<T>::HydroponicsAttachment(HydroponicsObject *parent, const char *idKeyStr)
    : _parent(parent), _obj(HydroponicsIdentity(idKeyStr))
{
    HYDRUINO_HARD_ASSERT(_parent, SFP(HS_Err_InvalidParameter));
}

template<class T>
HydroponicsAttachment<T>::HydroponicsAttachment(HydroponicsObject *parent, shared_ptr<T> obj)
    : _parent(parent), _obj(obj)
{
    HYDRUINO_HARD_ASSERT(_parent, SFP(HS_Err_InvalidParameter));
    if (isResolved()) {
        _obj->addLinkage(_parent);
    }
}

template<class T>
HydroponicsAttachment<T>::HydroponicsAttachment(HydroponicsObject *parent, const T *obj)
    : _parent(parent), _obj(obj)
{
    HYDRUINO_HARD_ASSERT(_parent, SFP(HS_Err_InvalidParameter));
    if (isResolved()) {
        _obj->addLinkage(_parent);
    }
}

template<class T>
HydroponicsAttachment<T>::~HydroponicsAttachment()
{
    if (isResolved()) {
        _obj->removeLinkage(_parent);
    }
}

template<class T>
void HydroponicsAttachment<T>::attachObject()
{
    if (_obj.resolveIfNeeded()) {
        _obj->addLinkage(_parent);
    }
}

template<class T>
void HydroponicsAttachment<T>::detachObject()
{
    if (isResolved()) {
        _obj->removeLinkage(_parent);
        _obj = (T *)nullptr;
    }
}


template<class T, class ParameterType, int Slots, class U>
HydroponicsSignalAttachment<T,ParameterType,Slots,U>::HydroponicsSignalAttachment(HydroponicsObject *parent, const HydroponicsIdentity &id, SignalGetterPtr signalGetter, MethodSlot<U,ParameterType> handleMethod)
    : HydroponicsAttachment<T>(parent, id), _signalGetter(signalGetter), _handleMethod(handleMethod)
{ ; }

template<class T, class ParameterType, int Slots, class U>
HydroponicsSignalAttachment<T,ParameterType,Slots,U>::HydroponicsSignalAttachment(HydroponicsObject *parent, const char *idKeyStr, SignalGetterPtr signalGetter, MethodSlot<U,ParameterType> handleMethod)
    : HydroponicsAttachment<T>(parent, HydroponicsIdentity(idKeyStr)), _signalGetter(signalGetter), _handleMethod(handleMethod)
{ ; }

template<class T, class ParameterType, int Slots, class U>
HydroponicsSignalAttachment<T,ParameterType,Slots,U>::HydroponicsSignalAttachment(HydroponicsObject *parent, shared_ptr<T> obj, SignalGetterPtr signalGetter, MethodSlot<U,ParameterType> handleMethod)
    : HydroponicsAttachment<T>(parent, obj), _signalGetter(signalGetter), _handleMethod(handleMethod)
{
    if (isResolved()) {
        (get()->*_signalGetter)().attach(_handleMethod);
    }
}

template<class T, class ParameterType, int Slots, class U>
HydroponicsSignalAttachment<T,ParameterType,Slots,U>::HydroponicsSignalAttachment(HydroponicsObject *parent, const T *obj, SignalGetterPtr signalGetter, MethodSlot<U,ParameterType> handleMethod)
    : HydroponicsAttachment<T>(parent, obj), _signalGetter(signalGetter), _handleMethod(handleMethod)
{
    if (isResolved()) {
        (get()->*_signalGetter)().attach(_handleMethod);
    }
}

template<class T, class ParameterType, int Slots, class U>
HydroponicsSignalAttachment<T,ParameterType,Slots,U>::~HydroponicsSignalAttachment()
{
    if (isResolved()) {
        (get()->*_signalGetter)().detach(_handleMethod);
    }
}

template<class T, class ParameterType, int Slots, class U>
void HydroponicsSignalAttachment<T,ParameterType,Slots,U>::attachObject()
{
    if (needsResolved()) {
        HydroponicsAttachment<T>::attachObject();

        if (isResolved()) {
            (get()->*_signalGetter)().attach(_handleMethod);
        }
    }
}

template<class T, class ParameterType, int Slots, class U>
void HydroponicsSignalAttachment<T,ParameterType,Slots,U>::detachObject()
{
    if (isResolved()) {
        HydroponicsAttachment<T>::detachObject();

        if (!isResolved()) {
            (get()->*_signalGetter)().detach(_handleMethod);
        }
    }
}
