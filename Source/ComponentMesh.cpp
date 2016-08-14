#include "Globals.h"
#include "Application.h"
#include "ComponentMesh.h"
#include "ModuleMeshes.h"
#include "ModuleResources.h"
#include "ResourceMesh.h"
#include "GameObject.h"

// ---------------------------------------------------------
ComponentMesh::ComponentMesh(GameObject* container) : Component(container)
{
	type = ComponentTypes::Geometry;
}

// ---------------------------------------------------------
void ComponentMesh::OnSave(Config& config) const
{
	ComponentWithResource::OnSaveResource(config);
}

// ---------------------------------------------------------
void ComponentMesh::OnLoad(Config * config)
{
	ComponentWithResource::OnLoadResource(config);
}

// ---------------------------------------------------------
bool ComponentMesh::SetResource(UID resource)
{
	bool ret = false;

	if (resource != 0)
	{
		Resource* res = App->resources->Get(resource);
		if (res != nullptr && res->GetType() == Resource::mesh)
		{
			if(res->LoadToMemory() == true)
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
	const ResourceMesh* res = (const ResourceMesh*) App->resources->Get(resource);
	if (res != nullptr)
		return res->bbox;
	static AABB inf;
	inf.SetNegativeInfinity();
	return inf;
}
