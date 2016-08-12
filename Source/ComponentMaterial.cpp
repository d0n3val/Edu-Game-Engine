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
	config.AddUID("Resource", resource);
}

// ---------------------------------------------------------
void ComponentMaterial::OnLoad(Config * config)
{
	SetResource(config->GetUID("Resource", 0));
}

// ---------------------------------------------------------
bool ComponentMaterial::SetResource(UID resource)
{
	bool ret = false;

	if (resource != 0)
	{
		const Resource* res = App->resources->Get(resource);
		if (res != nullptr && res->type == Resource::image)
		{
			texture = App->tex->Load(res->exported_file.c_str());
			if (texture != nullptr)
			{
				this->resource = resource;
				ret = true;
			}
		}
	}

	return ret;
}
