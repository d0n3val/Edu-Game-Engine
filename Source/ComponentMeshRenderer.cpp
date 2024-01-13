#include "Globals.h"

#include "ComponentMeshRenderer.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"
#include "ModuleRenderer.h"
#include "ModuleLevelManager.h"
#include "BatchManager.h"

#include "OpenGL.h"
#include "Application.h"
#include "GameObject.h"

#include "ModuleResources.h"
#include "ModulePrograms.h"

#include "ResourceTexture.h"

#include "Leaks.h"

ComponentMeshRenderer::ComponentMeshRenderer(GameObject* go) : Component(go, Types::MeshRenderer)
{
}

ComponentMeshRenderer::~ComponentMeshRenderer()
{
    if (batch_index != UINT_MAX)
    {
        App->renderer->GetBatchManager()->Remove(this);
        batch_index = UINT_MAX;
    }

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

    delete [] node_cache;
    node_cache = nullptr;
}

void ComponentMeshRenderer::OnSave(Config& config) const 
{
	config.AddUID("MeshResource", mesh_resource);
	config.AddBool("Visible", visible);
	config.AddUInt("Root", rootGO->GetUID());

	config.AddUID("MaterialResource", material_resource);
	config.AddBool("DebugDrawTangent", debug_draw_tangent);
	config.AddBool("CastShadows", cast_shadows);
	config.AddBool("RecvShadows", recv_shadows);
    config.AddUInt("RenderMode", render_mode);
    config.AddString("BatchName", batch_name.C_str());
    config.AddArray("SkinInfo");

    for(uint i=0; i< numBones; ++i)
    {
        const Bone& bone = bones[i];
        Config boneCfg;
        boneCfg.AddUID("go", bone.go->GetUID());
        boneCfg.AddFloat4x4("bind", bone.bind);
        config.AddArrayEntry(boneCfg);
    }
}

void ComponentMeshRenderer::OnLoad(Config* config) 
{
    ModuleLevelManager* level = App->level;

    visible            = config->GetBool("Visible", true);
    rootGO             = level->Find(config->GetUInt("Root"));

    debug_draw_tangent = config->GetBool("DebugDrawTangent", false);
    cast_shadows       = config->GetBool("CastShadows", true);
    recv_shadows       = config->GetBool("RecvShadows", true);
    render_mode        = config->GetUInt("RenderMode", RENDER_OPAQUE) == uint(RENDER_OPAQUE) ? RENDER_OPAQUE : RENDER_TRANSPARENT;

	SetMaterialRes(config->GetUID("MaterialResource", 0));
	SetMeshRes(config->GetUID("MeshResource", 0));

    if (GetMeshUID() != 0 && GetMaterialUID() != 0)
    {
        HashString batchName(config->GetString("BatchName"));
        SetBatchName(batchName ? batchName : HashString("default"));
    }

    numBones = config->GetArrayCount("SkinInfo");

    bones = std::make_unique<Bone[]>(numBones);

    for(uint i=0; i< numBones; ++i)
    {
        Bone& bone = bones[i];

        Config boneCfg = config->GetArray("SkinInfo", i);

        bone.go   = level->Find(boneCfg.GetUInt("go"));
        bone.bind = boneCfg.GetFloat4x4("bind");
    }
}

void ComponentMeshRenderer::GetBoundingBox (AABB& box) const 
{
    ResourceMesh* res = static_cast<ResourceMesh*>(App->resources->Get(mesh_resource));

    if(res != nullptr)
    {
        box.Enclose(res->bbox);
    }
}

void ComponentMeshRenderer::SetBatchName(const HashString& name)
{
    if(batch_index != UINT_MAX)
    {
        App->renderer->GetBatchManager()->Remove(this);
        batch_index = UINT_MAX;
    }

    batch_name = name;

    if(batch_name)
    {
        batch_index = App->renderer->GetBatchManager()->Add(this, batch_name);
    }
}

bool ComponentMeshRenderer::SetMeshRes(UID uid) 
{
    delete [] node_cache;
    node_cache = nullptr;

    morph_weights.release();

    Resource* res = App->resources->Get(mesh_resource);

    if(res != nullptr)
    {
        assert(res->GetType() == Resource::mesh);

        res->Release();
    }

	ResourceMesh* mesh = static_cast<ResourceMesh*>(App->resources->Get(uid));

	if (mesh != nullptr)
	{
		assert(mesh->GetType() == Resource::mesh);
		
		if(mesh->LoadToMemory() == true)
        {
            mesh_resource = uid;

#if 0 
            if(mesh->num_bones > 0)
            {
                node_cache   = new const GameObject* [mesh->num_bones];

                for(uint i=0; i< mesh->num_bones; ++i)
                {
                    node_cache[i] = nullptr;
                }
            }
#endif 

            if(mesh->GetNumMorphTargets())
            {
                morph_weights = std::make_unique<float[]>(mesh->GetNumMorphTargets());
                for(uint i=0; i< mesh->GetNumMorphTargets(); ++i)
                {
                    morph_weights[i] = 0.0f;
                }
            }

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
        assert(res->GetType() == Resource::material);

        res->Release();
    }

    res = App->resources->Get(uid);

    if (res != nullptr && res->GetType() == Resource::material)
    {
        if(res->LoadToMemory() == true)
        {
            material_resource = uid;

            return true;
        }
    }

    return false;
}

bool ComponentMeshRenderer::SetSkinInfo(const ResourceModel::Skin& skin, GameObject** gos)
{
    rootGO = skin.rootNode >= 0 ? gos[skin.rootNode] : nullptr;    
    numBones = uint32_t(skin.bones.size());
    bones = std::make_unique<Bone[]>(numBones);

    uint32_t index = 0;
    for (const ResourceModel::SkinBone& srcBone : skin.bones)
    {
        Bone& bone = bones[index++];
        bone.go = gos[srcBone.nodeIdx];
        bone.bind = srcBone.bind;
    }

    return true;
}

const ResourceMaterial* ComponentMeshRenderer::GetMaterialRes () const
{
    return static_cast<const ResourceMaterial*>(App->resources->Get(material_resource));
}


ResourceMaterial* ComponentMeshRenderer::GetMaterialRes () 
{
    return static_cast<ResourceMaterial*>(App->resources->Get(material_resource));
}


void ComponentMeshRenderer::UpdateSkinPalette(float4x4* palette) const
{
    ResourceMesh* mesh = static_cast<ResourceMesh*>(App->resources->Get(mesh_resource));

	if(numBones > 0)
	{
        float4x4 rootT = rootGO && rootGO->GetParent() ? rootGO->GetParent()->GetGlobalTransformation().Inverted() : float4x4::identity;

        for(unsigned i=0; i < numBones; ++i)
        {
            const Bone& bone = bones[i];
            const GameObject* bone_node = bone.go;

            palette[i] = bone_node ? rootT * bone_node->GetGlobalTransformation() * bone.bind :  float4x4::identity;
        }
    }

}

void ComponentMeshRenderer::UpdateCPUMorphTargets() const
{
    const ResourceMesh* mesh = GetMeshRes();

    if(dirty_morphs)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mesh->GetVBO());

        float3* vertices = reinterpret_cast<float3*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, mesh->GetNumVertices() * sizeof(float3), GL_MAP_WRITE_BIT));       
        memcpy(vertices, mesh->GetVertices(), sizeof(float3)*mesh->GetNumVertices());

        for(uint i=0; i< mesh->GetNumMorphTargets(); ++i)
        {
            const ResourceMesh::MorphData& morph_target = mesh->GetMorphTarget(i);

            if (morph_weights[i] > 0.0f)
            {
                for(uint j=0; j< morph_target.num_indices; ++j)
                {
                    uint index = morph_target.src_indices[j];
                    vertices[index] += morph_target.src_vertices[index] * morph_weights[i];
                }
            }
        }

        glUnmapBuffer(GL_ARRAY_BUFFER);

        if(mesh->HasAttrib(ATTRIB_NORMALS))
        {
            float3* normals = reinterpret_cast<float3*>(glMapBufferRange(GL_ARRAY_BUFFER, mesh->GetOffset(ATTRIB_NORMALS), mesh->GetNumVertices() * sizeof(float3), GL_MAP_WRITE_BIT));
            memcpy(normals, mesh->GetNormals(), sizeof(float3)*mesh->GetNumVertices());

            for(uint i=0; i< mesh->GetNumMorphTargets(); ++i)
            {
                const ResourceMesh::MorphData& morph_target = mesh->GetMorphTarget(i);

                // Loas indices????
                if (morph_weights[i] > 0.0f)
                {
                    for(uint j=0; j< morph_target.num_indices; ++j)
                    {
                        uint index = morph_target.src_indices[j];
                        normals[index] += morph_target.src_normals[index] * morph_weights[i];
                        normals[index].Normalize();
                    }
                }
            }

            glUnmapBuffer(GL_ARRAY_BUFFER);

            if(mesh->HasAttrib(ATTRIB_TANGENTS))
            {
                float3* tangents = reinterpret_cast<float3*>(glMapBufferRange(GL_ARRAY_BUFFER, mesh->GetOffset(ATTRIB_TANGENTS), mesh->GetNumVertices() * sizeof(float3), GL_MAP_WRITE_BIT)); 
                memcpy(tangents, mesh->GetTangents(), sizeof(float3)*mesh->GetNumVertices());

                for(uint i=0; i< mesh->GetNumMorphTargets(); ++i)
                {
                    const ResourceMesh::MorphData& morph_target = mesh->GetMorphTarget(i);
                    /* TODO
                    if (morph_weights[i] > 0.0f)
                    {
                        for(uint j=0; j< morph_target.num_indices; ++j)
                        {
                            uint index = morph_target.src_indices[j];
                            tangents[index] += morph_target.src_tangents[index] * morph_weights[i];
                            tangents[index].Normalize();
                        }
                    }
                    */
                }

                glUnmapBuffer(GL_ARRAY_BUFFER);
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        dirty_morphs = false;
    }
}
