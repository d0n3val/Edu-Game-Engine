#include "Globals.h"

#include "ResHandle.h"

#include "ModuleResources.h"

#include "Application.h"

ResHandle::ResHandle() 
{
}

ResHandle::~ResHandle()
{
    if(id != 0)
    {
        Resource* res = App->resources->Get(id);
        if(res != nullptr)
        {
            res->Release();
        }
    }
}

ResHandle::ResHandle(UID u) 
{
    if(u != 0)
    {
        Resource* res = App->resources->Get(u);
        if(res != nullptr && res->LoadToMemory())
        {
            id = u;
        }
    }
}

ResHandle::ResHandle(const ResHandle& rhs) 
{
    if(rhs.id != 0)
    {
        Resource* res = App->resources->Get(rhs.id);
        if(res != nullptr && res->LoadToMemory())
        {
            id = rhs.id;
        }
    }
}

ResHandle::ResHandle(ResHandle&& rhs)
{
    id     = rhs.id;
    rhs.id = 0;
}

ResHandle& ResHandle::operator=(UID u)
{
    if(id != 0)
    {
        Resource* res = App->resources->Get(id);
        if(res != nullptr)
        {
            res->Release();
        }

        id = 0;
    }

    if(u != 0)
    {
        Resource* res = App->resources->Get(u);
        if(res != nullptr && res->LoadToMemory())
        {
            id = u;
        }
    }

    return *this;
}

ResHandle& ResHandle::operator=(const ResHandle& rhs)
{
    if(id != 0)
    {
        Resource* res = App->resources->Get(id);
        if(res != nullptr)
        {
            res->Release();
        }

        id = 0;
    }

    if(rhs.id != 0)
    {
        Resource* res = App->resources->Get(rhs.id);
        if(res != nullptr && res->LoadToMemory())
        {
            id = rhs.id;
        }
    }

    return *this;
}

ResHandle& ResHandle::operator=(ResHandle&& rhs)
{
    if(id != 0)
    {
        Resource* res = App->resources->Get(id);
        if(res != nullptr)
        {
            res->Release();
        }
    }

    id     = rhs.id;
    rhs.id = 0;

    return *this;
}

Resource::Type ResHandle::GetType() const
{
    if(id != 0)
    {
        Resource* res = App->resources->Get(id);
        if(res)
        {
            return res->GetType();
        }
    }

    return Resource::unknown;
}

Resource* ResHandle::GetPtr()
{
    return id != 0 ? App->resources->Get(id) : nullptr;
}

const Resource* ResHandle::GetPtr() const
{
    return id != 0 ? App->resources->Get(id) : nullptr;
}

