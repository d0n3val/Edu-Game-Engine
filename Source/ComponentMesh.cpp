#include "Globals.h"

#include "ComponentMesh.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceMesh.h"
#include "GameObject.h"

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

    delete [] skin_palette;
    skin_palette = nullptr;

    delete [] node_cache;
    node_cache = nullptr;
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
    if(resource != 0)
    {
		Resource* res = App->resources->Get(uid);
        if(res != nullptr) 
        {
            assert(res->GetType() == Resource::mesh);

            res->Release();
        }
    }

    delete [] skin_palette;
    skin_palette = nullptr;

    delete [] node_cache;
    node_cache = nullptr;

	if (uid != 0)
	{
		ResourceMesh* res = static_cast<ResourceMesh*>(App->resources->Get(uid));

		if (res != nullptr) 
		{
            assert(res->GetType() == Resource::mesh);

			if(res->LoadToMemory() == true)
			{
				resource     = uid;

                if(res->num_bones > 0)
                {
                    skin_palette = new float4x4[res->num_bones];
                    node_cache   = new  const GameObject* [res->num_bones];
                }

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

const float4x4* ComponentMesh::UpdateSkinPalette()
{
    ResourceMesh* mesh = static_cast<ResourceMesh*>(App->resources->Get(resource));
    GameObject* root   = GetGameObject();

    if(auto_generated && root)
    {
        root = root->GetParent();
    }

	if(root && mesh && mesh->num_bones > 0)
	{
        float4x4 inverse = root->GetGlobalTransformation();
        inverse.InverseOrthonormal();

		for(unsigned i=0; i < mesh->num_bones; ++i)
		{
			const ResourceMesh::Bone& bone = mesh->bones[i];
			const GameObject* bone_node    = node_cache[i];

            if(bone_node == nullptr)
            {
                bone_node = node_cache[i] = root->FindChild(bone.name.C_str(), true);
            }

			assert(bone_node != nullptr);

			if(bone_node)
			{	
                skin_palette[i] = inverse*bone_node->GetGlobalTransformation()*bone.bind;
			}
			else
			{
				skin_palette[i] = float4x4::identity;
			}
		}
	}

    return skin_palette;
}

