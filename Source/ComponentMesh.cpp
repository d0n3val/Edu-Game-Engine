#include "Globals.h"
#include "Application.h"
#include "ComponentMesh.h"
#include "ModuleMeshes.h"
#include "ModuleResources.h"
#include "GameObject.h"

// ---------------------------------------------------------
ComponentMesh::ComponentMesh(GameObject* container) : Component(container)
{
	type = ComponentTypes::Geometry;
	bbox.SetNegativeInfinity();
}

// ---------------------------------------------------------
void ComponentMesh::OnSave(Config& config) const
{
	config.AddUID("Resource", resource);
	config.AddArrayFloat("AABB", (float*) &bbox.minPoint.x, 6);
}

// ---------------------------------------------------------
void ComponentMesh::OnLoad(Config * config)
{
	bbox.minPoint.x = config->GetFloat("AABB", 0.f, 0);
	bbox.minPoint.y = config->GetFloat("AABB", 0.f, 1);
	bbox.minPoint.z = config->GetFloat("AABB", 0.f, 2);

	bbox.maxPoint.x = config->GetFloat("AABB", 0.f, 3);
	bbox.maxPoint.y = config->GetFloat("AABB", 0.f, 4);
	bbox.maxPoint.z = config->GetFloat("AABB", 0.f, 5);
}

// ---------------------------------------------------------
bool ComponentMesh::SetResource(UID resource)
{
	bool ret = false;

	if (resource != 0)
	{
		const Resource* res = App->resources->Get(resource);
		if (res != nullptr && res->GetType() == Resource::mesh)
		{
			if(App->meshes->Load((ResourceMesh*) res))
			{
				this->resource = resource;
				ret = true;
			}
		}
	}

	return true;
}

// ---------------------------------------------------------
const AABB & ComponentMesh::GetBoundingBox() const
{
	return bbox;
}

// ---------------------------------------------------------
const ResourceMesh * ComponentMesh::GetResource() const
{
	return (ResourceMesh*) App->resources->Get(resource);
}
