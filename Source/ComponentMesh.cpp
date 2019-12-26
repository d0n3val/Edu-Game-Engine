#include "Globals.h"

#include "ComponentMesh.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"

#include "ResourceMesh.h"
#include "ResourceMaterial.h"

#include "GameObject.h"
#include "ComponentMaterial.h"

#include "OpenGL.h"

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
	config.AddUID("Root", root_uid);
}

void ComponentMesh::OnLoad(Config* config) 
{
	SetResource(config->GetUID("Resource", 0));
    visible = config->GetBool("Visible", true);
    root_uid = config->GetUID("Root", root_uid);
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
				resource = uid;

                if(res->num_bones > 0)
                {
                    skin_palette = new float4x4[res->num_bones];
                    node_cache   = new  const GameObject* [res->num_bones];

                    for(uint i=0; i< res->num_bones; ++i)
                    {
                        node_cache[i] = nullptr;
                    }
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

const float4x4* ComponentMesh::UpdateSkinPalette() const
{
    ResourceMesh* mesh = static_cast<ResourceMesh*>(App->resources->Get(resource));
    const GameObject* root   = GetGameObject();

    while(root != nullptr && root->GetUID() != root_uid)
    {
        root = root->GetParent();
    }

	if(mesh && mesh->num_bones > 0)
	{
		for(unsigned i=0; i < mesh->num_bones; ++i)
		{
			const ResourceMesh::Bone& bone = mesh->bones[i];
			const GameObject* bone_node    = node_cache[i];

            if(bone_node == nullptr)
            {
				bone_node = node_cache[i] = root ? root->FindChild(bone.name.C_str(), true) : nullptr;
            }

			if(bone_node)
			{
                skin_palette[i] = bone_node->GetGlobalTransformation()*bone.bind;
			}
			else
			{
				skin_palette[i] = float4x4::identity;
			}
		}
	}

    return skin_palette;
}

void ComponentMesh::Draw() const
{
    const GameObject* go = GetGameObject();
	const ResourceMesh* mesh = GetResource();
	const ComponentMaterial* material = go->FindFirstComponent<ComponentMaterial>();

    if(material != nullptr && mesh != nullptr)
    {
        float4x4 transform          = go->GetGlobalTransformation();
        const ResourceMaterial* mat = material->GetResource();

        glUniformMatrix4fv(App->programs->GetUniformLocation("model"), 1, GL_TRUE, reinterpret_cast<const float*>(&transform));

		if (mat)
		{
			mat->UpdateUniforms();
			mesh->UpdateUniforms(UpdateSkinPalette());
			mesh->Draw();
		}
    }
}

void ComponentMesh::DrawShadowPass() const
{
    const GameObject* go              = GetGameObject();
    const ResourceMesh* mesh          = GetResource();
    float4x4 transform                = go->GetGlobalTransformation();
    const ComponentMaterial* material = go->FindFirstComponent<ComponentMaterial>();

	if (mesh != nullptr && material != nullptr && material->CastShadows())
	{
		glUniformMatrix4fv(App->programs->GetUniformLocation("model"), 1, GL_TRUE, reinterpret_cast<const float*>(&transform));

		mesh->UpdateUniforms(UpdateSkinPalette());
		mesh->Draw();
	}
}

