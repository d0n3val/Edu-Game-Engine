#include "Globals.h"
#include "ComponentMaterial.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ModuleTextures.h"

// ---------------------------------------------------------
ComponentMaterial::ComponentMaterial(GameObject* container) : Component(container, Types::Material)
{}

// ---------------------------------------------------------
ComponentMaterial::~ComponentMaterial()
{}

// ---------------------------------------------------------
void ComponentMaterial::OnSave(Config& config) const
{
	ComponentWithResource::OnSaveResource(config);
	config.AddFloat("Alpha Test", alpha_test);
}

// ---------------------------------------------------------
void ComponentMaterial::OnLoad(Config * config)
{
	ComponentWithResource::OnLoadResource(config);
	alpha_test = config->GetFloat("Alpha Test", 0.5f);
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
