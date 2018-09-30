#include "Globals.h"
#include "ComponentMaterial.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ModuleTextures.h"

// ---------------------------------------------------------
ComponentMaterial::ComponentMaterial(GameObject* container) : Component(container, Types::Material)
{
	// \todo: this component should disappear
}

// ---------------------------------------------------------
ComponentMaterial::~ComponentMaterial()
{}

// ---------------------------------------------------------
void ComponentMaterial::OnSave(Config& config) const
{
	ComponentWithResource::OnSaveResource(config);
	config.AddFloat("Alpha Test", alpha_test);
	config.AddArrayFloat("Transform", &tex_transform.v[0][0], 16);
}

// ---------------------------------------------------------
void ComponentMaterial::OnLoad(Config * config)
{
	ComponentWithResource::OnLoadResource(config);
	alpha_test = config->GetFloat("Alpha Test", 0.5f);

	// Todo create custom method in config to load common types live float3, quat or float4x4
	tex_transform.v[0][0] = config->GetFloat("Transform", 1.f, 0);
	tex_transform.v[0][1] = config->GetFloat("Transform", 0.f, 1);
	tex_transform.v[0][2] = config->GetFloat("Transform", 0.f, 2);
	tex_transform.v[0][3] = config->GetFloat("Transform", 0.f, 3);

	tex_transform.v[1][0] = config->GetFloat("Transform", 0.f, 4);
	tex_transform.v[1][1] = config->GetFloat("Transform", 1.f, 5);
	tex_transform.v[1][2] = config->GetFloat("Transform", 0.f, 6);
	tex_transform.v[1][3] = config->GetFloat("Transform", 0.f, 7);

	tex_transform.v[2][0] = config->GetFloat("Transform", 0.f, 8);
	tex_transform.v[2][1] = config->GetFloat("Transform", 0.f, 9);
	tex_transform.v[2][2] = config->GetFloat("Transform", 1.f, 10);
	tex_transform.v[2][3] = config->GetFloat("Transform", 0.f, 11);

	tex_transform.v[3][0] = config->GetFloat("Transform", 0.f, 12);
	tex_transform.v[3][1] = config->GetFloat("Transform", 0.f, 13);
	tex_transform.v[3][2] = config->GetFloat("Transform", 0.f, 14);
	tex_transform.v[3][3] = config->GetFloat("Transform", 1.f, 15);
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
