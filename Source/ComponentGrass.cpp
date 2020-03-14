#include "Globals.h"

#include "ComponentGrass.h"

#include "ResourceTexture.h"

#include "ModuleResources.h"

#include "Application.h"

#include "mmgr/mmgr.h"

ComponentGrass::ComponentGrass(GameObject* object) : Component(object, Types::Animation)
{
}

ComponentGrass::~ComponentGrass()
{
	Resource* res = App->resources->Get(albedo);
	if (res != nullptr)
	{
		res->Release();
	}

	res = App->resources->Get(normal);
	if (res != nullptr)
	{
		res->Release();
	}
}

void ComponentGrass::OnSave(Config& config) const 
{
	config.AddUID("Albedo", albedo);
	config.AddUID("Normal", normal);
}

void ComponentGrass::OnLoad(Config* config) 
{
	SetAlbedo(config->GetUID("Albedo", 0));
	SetNormal(config->GetUID("Normal", 0));
}

bool ComponentGrass::SetAlbedo(UID uid)
{
    Resource* res = App->resources->Get(albedo);
    if(res != nullptr)
    {
        assert(res->GetType() == Resource::texture);

        res->Release();
    }

    res = App->resources->Get(uid);

    if(res != nullptr && res->GetType() == Resource::texture)
    {
        if(res->LoadToMemory())
        {
            albedo = uid;

            return true;
        }
    }

    return false;
}

const ResourceTexture* ComponentGrass::GetAlbedo () const
{
    return static_cast<const ResourceTexture*>(App->resources->Get(albedo));
}
   
bool ComponentGrass::SetNormal(UID uid)
{
    Resource* res = App->resources->Get(normal);
    if(res != nullptr)
    {
        assert(res->GetType() == Resource::texture);

        res->Release();
    }

    res = App->resources->Get(uid);

    if(res != nullptr && res->GetType() == Resource::texture)
    {
        if(res->LoadToMemory())
        {
            normal = uid;

            return true;
        }
    }

    return false;
}

const ResourceTexture* ComponentGrass::GetNormal() const
{
    return static_cast<const ResourceTexture*>(App->resources->Get(albedo));
}
