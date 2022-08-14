/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Shared Virtual Pointer (partial template specialization)
*/
// Copied and modified from: https://github.com/hideakitai/ArxSmartPtr

#ifndef HydroponicsSharedVirtualPtr_H
#define HydroponicsSharedVirtualPtr_H

#include "Hydroponics.h"
#ifdef HYDRUINO_USE_VIRTMEM
#include "internal/vptr.h"

// imported and simplified from https://github.com/boostorg/smart_ptr

namespace arx { namespace stdx
{
    template<class T> class shared_ptr<VirtualPtr<T>>
    {
    public:
        typedef typename sp::element<T>::type element_type;

    private:
        typedef shared_ptr<VirtualPtr<T>> this_type;

        template<class Y> friend class shared_ptr;

        VirtualPtr<T> px;
        detail::shared_count pn;

    public:
        shared_ptr() : px(), pn()
        {
        }

        shared_ptr(sp::detail::nullptr_t) : px(), pn()
        {
        }

        template<class Y>
        explicit shared_ptr(Y* p) : px(), pn()
        {
            px.setRawNum((intptr_t)p);
            detail::shared_count(p).swap(pn);
        }

        shared_ptr(const this_type& r) : px(), pn(r.pn)
        {
            px.setRawNum(r.getVPtr().getRawNum());
        }

        template<class Y>
        shared_ptr(const shared_ptr<VirtualPtr<Y>>& r) : px(), pn(r.pn)
        {
            px.setRawNum(r.getVPtr().getRawNum());
        }

        template<class Y>
        shared_ptr(const shared_ptr<VirtualPtr<Y>>& r, VirtualPtr<T> p) : px(), pn(r.pn)
        {
            px.setRawNum(p.getRawNum());
        }

        this_type& operator= (const this_type& r)
        {
            this_type(r).swap(*this);
            return *this;
        }

        template<class Y>
        this_type& operator= (const shared_ptr<VirtualPtr<Y>>& r)
        {
            this_type(r).swap(*this);
            return *this;
        }

        shared_ptr(this_type&& r) : px(), pn()
        {
            pn.swap(r.pn);
            px.setRawNum(r.px.getRawNum());
            r.px.setRawNum(0);
        }

        template<class Y>
        shared_ptr(shared_ptr<VirtualPtr<Y>>&& r) : px(), pn()
        {
            pn.swap( r.pn );
            px.setRawNum(r.px.getRawNum());
            r.px.setRawNum(0);
        }

        this_type& operator= (this_type&& r)
        {
            this_type(static_cast<this_type&&>(r)).swap(*this);
            return *this;
        }

        template<class Y>
        this_type& operator= (shared_ptr<VirtualPtr<Y>>&& r)
        {
            this_type(static_cast<shared_ptr<VirtualPtr<Y>>&&>(r)).swap(*this);
            return *this;
        }

        template<class Y>
        shared_ptr(shared_ptr<VirtualPtr<Y>>&& r, VirtualPtr<Y> p) : px(), pn()
        {
            pn.swap( r.pn );
            px.setRawNum(p.getRawNum());
            r.px.setRawNum(0);
        }

        this_type& operator= (sp::detail::nullptr_t)
        {
            this_type().swap(*this);
            return *this;
        }

        template<class Y>
        this_type& operator= (Y* p)
        {
            this_type(p).swap(*this);
        }

        void reset()
        {
            this_type().swap(*this);
        }

        template<class Y>
        void reset(Y* p)
        {
            this_type(p).swap(*this);
        }

        template<class Y>
        void reset(const shared_ptr<VirtualPtr<Y>>& r, VirtualPtr<T> p)
        {
            this_type(r, p).swap(*this);
        }

        template<class Y>
        void reset(shared_ptr<VirtualPtr<Y>>&& r, VirtualPtr<T> p)
        {
            this_type(static_cast<shared_ptr<VirtualPtr<Y>>&&>(r), p).swap(*this);
        }

        typename VirtualPtr<T>::ValueWrapper &&operator*()
        {
            return px.operator*();
        }

        const typename VirtualPtr<T>::ValueWrapper &operator*() const
        {
            return px.operator*();
        }

        typename VirtualPtr<T>::MemberWrapper &&operator->()
        {
            return px.operator->();
        }

        const typename VirtualPtr<T>::MemberWrapper &operator->() const
        {
            return px.operator->();
        }

        typename VirtualPtr<T>::ValueWrapper &&operator[](size_t i)
        {
            return px.operator[](i);
        }

        const typename VirtualPtr<T>::ValueWrapper &operator[](size_t i) const
        {
            return px.operator[](i);
        }

        T* get()
        {
            return px.operator->().operator->();
        }

        const T* get() const
        {
            return px.operator->().operator->();
        }

        VirtualPtr<T> getVPtr() const
        {
            return px;
        }

        bool isWrapped() const
        {
            #ifdef VIRTMEM_WRAP_CPOINTERS
                return px.isWrapped();
            #else
                return false;
            #endif
        }

        T* unwrap() const
        {
            #ifdef VIRTMEM_WRAP_CPOINTERS
                if (px.isWrapped()) {
                    return reinterpret_cast<T *>(px.unwrap());
                }
            #endif
            return reinterpret_cast<T *>(px.getRawNum());
        }

        explicit operator bool () const
        {
            return px != nullptr;
        }

        bool operator! () const
        {
            return px == nullptr;
        }

        bool unique() const
        {
            return pn.unique();
        }

        long use_count() const
        {
            return pn.use_count();
        }

        void swap(this_type& other)
        {
            sp::detail::swap(px, other.px);
            pn.swap(other.pn);
        }

    }; // shared_ptr<VirtualPtr<T>>

    template<class T, class U>
    shared_ptr<VirtualPtr<T>> static_vpointer_cast(const shared_ptr<VirtualPtr<U>>& r)
    {
#ifndef VIRTMEM_WRAP_CPOINTERS
        (void)static_cast<T*>(static_cast<U*>(0));
#else
        if (r.isWrapped()) {
            VirtualPtr<T> p = VirtualPtr<T>::wrap(static_cast<T *>(r.unwrap()));
            return shared_ptr<VirtualPtr<T>>(r, p);
        }
#endif
        {   VirtualPtr<T> p; p.setRawNum(r.getVPtr().getRawNum());
            return shared_ptr<VirtualPtr<T>>(r, p);
        }
    }

    template<class T, class U>
    shared_ptr<VirtualPtr<T>> const_vpointer_cast(const shared_ptr<VirtualPtr<U>>& r)
    {
#ifndef VIRTMEM_WRAP_CPOINTERS
        (void)const_cast<T*>(static_cast<U*>(0));
#else
        if (r.isWrapped()) {
            VirtualPtr<T> p = VirtualPtr<T>::wrap(const_cast<T *>(r.unwrap()));
            return shared_ptr<VirtualPtr<T>>(r, p);
        }
#endif
        {   VirtualPtr<T> p; p.setRawNum(r.getVPtr().getRawNum());
            return shared_ptr<VirtualPtr<T>>(r, p);
        }
    }

    template<class T, class U>
    shared_ptr<VirtualPtr<T>> dynamic_vpointer_cast(const shared_ptr<VirtualPtr<U>>& r)
    {
#ifndef VIRTMEM_WRAP_CPOINTERS
        (void)dynamic_cast<T*>(static_cast<U*>(0));
#else
        if (r.isWrapped()) {
            VirtualPtr<T> p = VirtualPtr<T>::wrap(dynamic_cast<T *>(r.unwrap()));
            return shared_ptr<VirtualPtr<T>>(r, p);
        }
#endif
        {   VirtualPtr<T> p; p.setRawNum(r.getVPtr().getRawNum());
            return shared_ptr<VirtualPtr<T>>(r, p);
        }
    }

    template<class T, class U>
    shared_ptr<VirtualPtr<T>> reinterpret_vpointer_cast(const shared_ptr<VirtualPtr<U>>& r)
    {
#ifndef VIRTMEM_WRAP_CPOINTERS
        (void)reinterpret_cast<T*>(static_cast<U*>(0));
#else
        if (r.isWrapped()) {
            VirtualPtr<T> p = VirtualPtr<T>::wrap(reinterpret_cast<T *>(r.unwrap()));
            return shared_ptr<VirtualPtr<T>>(r, p);
        }
#endif
        {   VirtualPtr<T> p; p.setRawNum(r.getVPtr().getRawNum());
            return shared_ptr<VirtualPtr<T>>(r, p);
        }
    }

    template<class T, class U>
    shared_ptr<VirtualPtr<T>> static_vpointer_cast(shared_ptr<VirtualPtr<U>>&& r)
    {
#ifndef VIRTMEM_WRAP_CPOINTERS
        (void)static_cast<T*>(static_cast<U*>(0));
#else
        if (r.isWrapped()) {
            VirtualPtr<T> p = VirtualPtr<T>::wrap(static_cast<T *>(r.unwrap()));
            return shared_ptr<VirtualPtr<T>>(r, p);
        }
#endif
        {   VirtualPtr<T> p; p.setRawNum(r.getVPtr().getRawNum());
            return shared_ptr<VirtualPtr<T>>(sp::detail::move(r), p);
        }
    }

    template<class T, class U>
    shared_ptr<VirtualPtr<T>> const_vpointer_cast(shared_ptr<VirtualPtr<U>>&& r)
    {
#ifndef VIRTMEM_WRAP_CPOINTERS
        (void)const_cast<T*>(static_cast<U*>(0));
#else
        if (r.isWrapped()) {
            VirtualPtr<T> p = VirtualPtr<T>::wrap(const_cast<T *>(r.unwrap()));
            return shared_ptr<VirtualPtr<T>>(r, p);
        }
#endif
        {   VirtualPtr<T> p; p.setRawNum(r.getVPtr().getRawNum());
            return shared_ptr<VirtualPtr<T>>(sp::detail::move(r), p);
        }
    }

    template<class T, class U>
    shared_ptr<VirtualPtr<T>> dynamic_vpointer_cast(shared_ptr<VirtualPtr<U>>&& r)
    {
#ifndef VIRTMEM_WRAP_CPOINTERS
        (void)dynamic_cast<T*>(static_cast<U*>(0));
#else
        if (r.isWrapped()) {
            VirtualPtr<T> p = VirtualPtr<T>::wrap(dynamic_cast<T *>(r.unwrap()));
            return shared_ptr<VirtualPtr<T>>(r, p);
        }
#endif
        {   VirtualPtr<T> p; p.setRawNum(r.getVPtr().getRawNum());
            return shared_ptr<VirtualPtr<T>>(sp::detail::move(r), p);
        }
    }

    template<class T, class U>
    shared_ptr<VirtualPtr<T>> reinterpret_vpointer_cast(shared_ptr<VirtualPtr<U>>&& r)
    {
#ifndef VIRTMEM_WRAP_CPOINTERS
        (void)reinterpret_cast<T*>(static_cast<U*>(0));
#else
        if (r.isWrapped()) {
            VirtualPtr<T> p = VirtualPtr<T>::wrap(reinterpret_cast<T *>(r.unwrap()));
            return shared_ptr<VirtualPtr<T>>(r, p);
        }
#endif
        {   VirtualPtr<T> p; p.setRawNum(r.getVPtr().getRawNum());
            return shared_ptr<VirtualPtr<T>>(sp::detail::move(r), p);
        }
    }

    template<class T>
    shared_ptr<VirtualPtr<T>> make_vshared()
    {
        return shared_ptr<VirtualPtr<T>>(new T);
    }

    template<class T, class... Args>
    shared_ptr<VirtualPtr<T>> make_vshared(Args&&... args)
    {
        return shared_ptr<VirtualPtr<T>>(new T(sp::detail::forward<Args>(args)...));
    }

}} // namespace arx::stdx

template<class T, class U> inline SharedPtr<T> static_hyptr_cast(const SharedPtr<U>& r) { return static_vpointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> static_hyptr_cast(SharedPtr<U>&& r) { return static_vpointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> const_hyptr_cast(const SharedPtr<U>& r) { return const_vpointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> const_hyptr_cast(SharedPtr<U>&& r) { return const_vpointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> dynamic_hyptr_cast(const SharedPtr<U>& r) { return dynamic_vpointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> dynamic_hyptr_cast(SharedPtr<U>&& r) { return dynamic_vpointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> reinterpret_hyptr_cast(const SharedPtr<U>& r) { return reinterpret_vpointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> reinterpret_hyptr_cast(SharedPtr<U>&& r) { return reinterpret_vpointer_cast<T,U>(r); }
template<class T> inline SharedPtr<T> make_hyptr_shared() { return make_vshared<T>(); }
template<class T, class... Args> inline SharedPtr<T> make_hyptr_shared(Args&&... args) { return make_vshared<T>(args...); }

#else

template<class T, class U> inline SharedPtr<T> static_hyptr_cast(const SharedPtr<U>& r) { return static_pointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> static_hyptr_cast(SharedPtr<U>&& r) { return static_pointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> const_hyptr_cast(const SharedPtr<U>& r) { return const_pointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> const_hyptr_cast(SharedPtr<U>&& r) { return const_pointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> dynamic_hyptr_cast(const SharedPtr<U>& r) { return dynamic_pointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> dynamic_hyptr_cast(SharedPtr<U>&& r) { return dynamic_pointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> reinterpret_hyptr_cast(const SharedPtr<U>& r) { return reinterpret_pointer_cast<T,U>(r); }
template<class T, class U> inline SharedPtr<T> reinterpret_hyptr_cast(SharedPtr<U>&& r) { return reinterpret_pointer_cast<T,U>(r); }
template<class T> inline SharedPtr<T> make_hyptr_shared() { return arx::stdx::make_shared<T>(); }
template<class T, class... Args> inline SharedPtr<T> make_hyptr_shared(Args&&... args) { return arx::stdx::make_shared<T>(args...); }

#endif // /ifdef HYDRUINO_USE_VIRTMEM

#endif // /ifndef HydroponicsSharedVirtualPtr_H
