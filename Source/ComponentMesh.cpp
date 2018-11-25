#include "Globals.h"

#include "ComponentMesh.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceMesh.h"

ComponentMesh::ComponentMesh(GameObject* go) : Component(go, Types::Mesh)
{
}

ComponentMesh::~ComponentMesh()
{
}

void ComponentMesh::OnSave(Config& config) const 
{
    ComponentWithResource::OnSaveResource(config);
}

void ComponentMesh::OnLoad(Config* config) 
{
    ComponentWithResource::OnLoadResource(config);

    App->resources->Get(resource)->LoadToMemory();
}

void ComponentMesh::GetBoundingBox (AABB& box) const 
{
    box.Enclose(static_cast<ResourceMesh*>(App->resources->Get(resource))->bbox);
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
