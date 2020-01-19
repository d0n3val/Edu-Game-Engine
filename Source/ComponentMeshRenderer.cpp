#include "Globals.h"

#include "ComponentMeshRenderer.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"

#include "OpenGL.h"
#include "Application.h"
#include "GameObject.h"

#include "ModuleResources.h"
#include "ModulePrograms.h"

#include "mmgr/mmgr.h"


ComponentMeshRenderer::ComponentMeshRenderer(GameObject* go) : Component(go, Types::Mesh)
{
}

ComponentMeshRenderer::~ComponentMeshRenderer()
{
	Resource* res = App->resources->Get(mesh_resource);
	if (res != nullptr)
	{
		res->Release();
	}

	res = App->resources->Get(material_resource);
	if (res != nullptr)
	{
		res->Release();
	}

    delete [] skin_palette;
    skin_palette = nullptr;

    delete [] node_cache;
    node_cache = nullptr;
}

void ComponentMeshRenderer::OnSave(Config& config) const 
{
	config.AddUID("MeshResource", mesh_resource);
	config.AddBool("Visible", visible);
	config.AddUID("Root", root_uid);

	config.AddUID("MaterialResource", material_resource);
	config.AddBool("DebugDrawTangent", debug_draw_tangent);
	config.AddBool("CastShadows", cast_shadows);
	config.AddBool("RecvShadows", recv_shadows);
    config.AddUInt("RenderMode", render_mode);
}

void ComponentMeshRenderer::OnLoad(Config* config) 
{
	SetMeshRes(config->GetUID("MeshResource", 0));
    visible = config->GetBool("Visible", true);
    root_uid = config->GetUID("Root", root_uid);

	SetMaterialRes(config->GetUID("MaterialResource", 0));

    debug_draw_tangent = config->GetBool("DebugDrawTangent", false);
    cast_shadows = config->GetBool("CastShadows", true);
    recv_shadows = config->GetBool("RecvShadows", true);
    render_mode  = config->GetUInt("RenderMode", RENDER_OPAQUE) == uint(RENDER_OPAQUE) ? RENDER_OPAQUE : RENDER_TRANSPARENT;
}

void ComponentMeshRenderer::GetBoundingBox (AABB& box) const 
{
    ResourceMesh* res = static_cast<ResourceMesh*>(App->resources->Get(mesh_resource));

    if(res != nullptr)
    {
        box.Enclose(res->bbox);
    }
}

bool ComponentMeshRenderer::SetMeshRes(UID uid) 
{
    delete [] skin_palette;
    skin_palette = nullptr;

    delete [] node_cache;
    node_cache = nullptr;

    Resource* res = App->resources->Get(mesh_resource);

    if(res != nullptr)
    {
        assert(res->GetType() == Resource::mesh);

        res->Release();
    }

	ResourceMesh* mesh = static_cast<ResourceMesh*>(App->resources->Get(uid));
	assert(mesh->GetType() == Resource::mesh);

	if (mesh != nullptr)
	{
        if(mesh->LoadToMemory() == true)
        {
            mesh_resource = uid;

            if(mesh->num_bones > 0)
            {
                skin_palette = new float4x4[mesh->num_bones];
                node_cache   = new const GameObject* [mesh->num_bones];

                for(uint i=0; i< mesh->num_bones; ++i)
                {
                    node_cache[i] = nullptr;
                }
            }

			mesh->Release();

            return true;
        }
    }

	return false;
}

const ResourceMesh* ComponentMeshRenderer::GetMeshRes() const
{
	return static_cast<const ResourceMesh*>(App->resources->Get(mesh_resource));
}

ResourceMesh* ComponentMeshRenderer::GetMeshRes() 
{
	return static_cast<ResourceMesh*>(App->resources->Get(mesh_resource));
}

bool ComponentMeshRenderer::SetMaterialRes(UID uid)
{
    Resource* res = App->resources->Get(material_resource);

    if(material_resource != 0 && res != nullptr)
    {
        assert(res->GetType() == Resource::mesh);

        res->Release();
    }

    res = App->resources->Get(uid);

    if (res != nullptr && res->GetType() == Resource::material)
    {
        if(res->LoadToMemory() == true)
        {
            material_resource = uid;


            res->Release();

            return true;
        }
    }

    return false;
}

const ResourceMaterial* ComponentMeshRenderer::GetMaterialRes () const
{
    return static_cast<const ResourceMaterial*>(App->resources->Get(material_resource));
}


ResourceMaterial* ComponentMeshRenderer::GetMaterialRes () 
{
    return static_cast<ResourceMaterial*>(App->resources->Get(material_resource));
}


const float4x4* ComponentMeshRenderer::UpdateSkinPalette() const
{
    ResourceMesh* mesh = static_cast<ResourceMesh*>(App->resources->Get(mesh_resource));
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

void ComponentMeshRenderer::Draw() const
{
    const GameObject* go             = GetGameObject();
	const ResourceMesh* mesh         = GetMeshRes();
	const ResourceMaterial* material = GetMaterialRes();

    if(material != nullptr && mesh != nullptr)
    {
        float4x4 transform = go->GetGlobalTransformation();

        glUniformMatrix4fv(App->programs->GetUniformLocation("model"), 1, GL_TRUE, reinterpret_cast<const float*>(&transform));

        mesh->UpdateUniforms(UpdateSkinPalette());
        material->UpdateUniforms();
        material->BindTextures();
        mesh->Draw();
        material->UnbindTextures();

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void ComponentMeshRenderer::DrawShadowPass() const
{
    const GameObject* go              = GetGameObject();
    const ResourceMesh* mesh          = GetMeshRes();
    float4x4 transform                = go->GetGlobalTransformation();

	if (mesh != nullptr && cast_shadows)
	{
		glUniformMatrix4fv(App->programs->GetUniformLocation("model"), 1, GL_TRUE, reinterpret_cast<const float*>(&transform));

		mesh->UpdateUniforms(UpdateSkinPalette());
		mesh->Draw();
	}
}

