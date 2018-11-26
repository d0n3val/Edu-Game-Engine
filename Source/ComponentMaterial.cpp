#include "Globals.h"
#include "ComponentMaterial.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceMaterial.h"

ComponentMaterial::ComponentMaterial(GameObject* container) : Component(container, Types::Material)
{
}

ComponentMaterial::~ComponentMaterial()
{
	Resource* res = App->resources->Get(resource);
	if (res != nullptr)
	{
		res->Release();
	}
}

void ComponentMaterial::OnLoad(Config * config)
{
	SetResource(config->GetUID("Resource", 0));

    cast_shadows = config->GetBool("CastShadows", true);
    recv_shadows = config->GetBool("RecvShadows", true);

	Resource* res = App->resources->Get(resource);
	if (res != nullptr)
	{
		res->LoadToMemory();
	}
}

void ComponentMaterial::OnSave(Config& config) const
{
	config.AddUID("Resource", resource);
	config.AddBool("CastShadows", cast_shadows);
	config.AddBool("RecvShadows", recv_shadows);
}

bool ComponentMaterial::SetResource (UID uid)
{
    Resource* res = App->resources->Get(uid);

    if (res != nullptr && res->GetType() == Resource::material)
    {
        if(res->LoadToMemory() == true)
        {
            resource = uid;

            return true;
        }
    }

    return false;
}

const ResourceMaterial* ComponentMaterial::GetResource () const
{
    return static_cast<const ResourceMaterial*>(App->resources->Get(resource));
}


ResourceMaterial* ComponentMaterial::GetResource () 
{
    return static_cast<ResourceMaterial*>(App->resources->Get(resource));
}

