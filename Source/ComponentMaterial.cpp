#include "Globals.h"
#include "ComponentMaterial.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ModuleTextures.h"

// ---------------------------------------------------------
ComponentMaterial::ComponentMaterial(GameObject* container) : Component(container)
{
	type = ComponentTypes::Material;
}

// ---------------------------------------------------------
ComponentMaterial::~ComponentMaterial()
{}

// ---------------------------------------------------------
void ComponentMaterial::OnSave(Config& config) const
{
	ComponentWithResource::OnSaveResource(config);
}

// ---------------------------------------------------------
void ComponentMaterial::OnLoad(Config * config)
{
	ComponentWithResource::OnLoadResource(config);
}

// ---------------------------------------------------------
bool ComponentMaterial::SetResource(UID resource)
{
	bool ret = false;

	if (resource != 0)
	{
		Resource* res = App->resources->Get(resource);
		if (res != nullptr && res->GetType() == Resource::texture)
		{
			if(res->LoadToMemory() == true)
			{
				this->resource = resource;
				ret = true;
			}
		}
	}

	return ret;
}
