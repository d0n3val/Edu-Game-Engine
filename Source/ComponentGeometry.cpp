#include "Globals.h"

#include "ComponentGeometry.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceMesh.h"

ComponentGeometry::ComponentGeometry(GameObject* go) : Component(go, Types::Geometry)
{
}

ComponentGeometry::~ComponentGeometry()
{
    for(uint i=0; i< meshes.size(); ++i)
    {
        App->resources->Get(meshes[i])->Release();
    }
}

void ComponentGeometry::OnSave(Config& config) const 
{
    ComponentWithResource::OnSaveResource(config);
}

void ComponentGeometry::OnLoad(Config* config) 
{
    ComponentWithResource::OnLoadResource(config);

    App->resources->Get(resource)->LoadToMemory();
}

void ComponentGeometry::GetBoundingBox (AABB& box) const 
{
    box.Enclose(static_cast<ResourceMesh*>(App->resources->Get(resource))->bbox);
}

bool ComponentGeometry::SetResource(UID uid) 
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
