#include "Globals.h"

#include "ComponentMesh.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceMesh.h"

#include "mmgr/mmgr.h"

ComponentMesh::ComponentMesh(GameObject* go) : Component(go, Types::Mesh)
{
}

ComponentMesh::~ComponentMesh()
{
	Resource* res = App->resources->Get(resource);
	if (res != nullptr)
	{
		res->Release();
	}
}

void ComponentMesh::OnSave(Config& config) const 
{
	config.AddUID("Resource", resource);
	config.AddBool("Visible", visible);
}

void ComponentMesh::OnLoad(Config* config) 
{
	SetResource(config->GetUID("Resource", 0));
    visible = config->GetBool("Visible", true);
}

void ComponentMesh::GetBoundingBox (AABB& box) const 
{
    ResourceMesh* res = static_cast<ResourceMesh*>(App->resources->Get(resource));

    if(res != nullptr)
    {
        box.Enclose(res->bbox);
    }
}

bool ComponentMesh::SetResource(UID uid) 
{
	if (uid != 0)
	{
		Resource* res = App->resources->Get(uid);
		if (res != nullptr && res->GetType() == Resource::mesh)
		{
			if(res->LoadToMemory() == true)
			{
				resource = uid;

                return true;
			}
		}
	}

	return false;
}

const ResourceMesh* ComponentMesh::GetResource() const
{
	return static_cast<const ResourceMesh*>(App->resources->Get(resource));
}

