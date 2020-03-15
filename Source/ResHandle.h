#ifndef __RESHANDLE_H__
#define __RESHANDLE_H__

#include "Resource.h"

class ResHandle
{
    UID id = 0;

public:

    ResHandle();
    ~ResHandle();

    explicit ResHandle(UID u);
    ResHandle(const ResHandle& rhs);
    ResHandle(ResHandle&& rhs);

    ResHandle& operator=(UID u);
    ResHandle& operator=(const ResHandle& rhs);
    ResHandle& operator=(ResHandle&& rhs);

    Resource::Type  GetType () const;
    Resource*       GetPtr  ();
    const Resource* GetPtr  () const;
    UID             GetUID() const { return id; }

    template<class T>
    const T* GetPtr() const { return T::GetClassType() == GetType() ? reinterpret_cast<const T*>(GetPtr()) : nullptr; }

    template<class T>
    T*       GetPtr() { return T::GetClassType() == GetType() ? reinterpret_cast<T*>(GetPtr()) : nullptr; }

    operator bool() const { return id != 0; }

};

#endif /* __RESOURCE_H__ */
