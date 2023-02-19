#include "Globals.h"

#include "GeometryBatch.h"

#include "Application.h"
#include "ComponentMeshRenderer.h"
#include "ModuleTextures.h"
#include "ModuleRenderer.h"
#include "ModuleResources.h"
#include "GameObject.h"
#include "BatchManager.h"

#include "ResourceTexture.h"

#include "OpenGL.h"

#include "Leaks.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include <algorithm>
#include <SDL_assert.h>

GeometryBatch::GeometryBatch(const HashString& tag) : tagName(tag)
{
}

bool GeometryBatch::CanAdd(const ComponentMeshRenderer* object) const
{
    if(objects.empty())
    {
        return true;
    }

    const ResourceMesh* meshRes = object->GetMeshRes();
    if (meshRes->GetAttribs() == attrib_flags)
    {
        return true;
    }

    return false;
}

void GeometryBatch::Add(const ComponentMeshRenderer* object)
{
    assert(CanAdd(object));
    assert(commands.empty());

    if(objects.empty())
    {
        attrib_flags = object->GetMeshRes()->GetAttribs();
    }

    uint index = uint(objects.size());
    meshes[object->GetMeshUID()].refCount++;
    objects[object] = index;

    ClearRenderData();
}

void GeometryBatch::CreateRenderData()
{
    CreateVertexBuffers();
    CreateMaterialBuffer();
    CreateMorphBuffer();
    CreateInstanceBuffer();
    CreateTransformBuffer();
    bufferDirty = false;
    modelUpdates.clear();
}

void GeometryBatch::ClearRenderData()
{
    vao.reset();
    for(uint i=0; i< ATTRIB_COUNT+1; ++i)
    {
        vbo[i].reset();
    }
    ibo.reset();
    commands.clear();

    bufferDirty = true;
}

void GeometryBatch::CreateVertexBuffers()
{
    SDL_assert(!objects.empty());

    uint numIndices  = 0;
    uint numVertices = 0;

    Buffer* vbo_ptr[ATTRIB_COUNT];

    for (int i = 0; i < ATTRIB_COUNT; ++i) vbo_ptr[i] = nullptr;

    for(std::pair<const UID, MeshData>& meshData  : meshes)
    {
        const ResourceMesh *mesh = static_cast<const ResourceMesh *>(App->resources->Get(meshData.first));

        meshData.second.baseVertex = numVertices;
        meshData.second.baseIndex = numIndices;
        meshData.second.vertexCount = mesh->num_vertices;
        meshData.second.indexCount = mesh->num_indices;

        numVertices += mesh->num_vertices;
        numIndices += mesh->num_indices;
    }

    // Vertex attributes 

    VertexAttrib attribs[ATTRIB_COUNT];

    const ResourceMesh* mesh = objects.begin()->first->GetMeshRes();

    if (mesh->HasAttrib(ATTRIB_POSITIONS))
    {
        uint dataOffset = 0;
        vbo_ptr[ATTRIB_POSITIONS] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float3) * numVertices, nullptr);
        glObjectLabel(GL_BUFFER, vbo_ptr[ATTRIB_POSITIONS]->Id(), -1, "GeometryBatch-POSITION");

        vbo[ATTRIB_POSITIONS].reset(vbo_ptr[ATTRIB_POSITIONS]);
        uint8_t* data = reinterpret_cast<uint8_t*>(vbo[ATTRIB_POSITIONS]->Map(GL_WRITE_ONLY));
        for (const std::pair<const UID, MeshData>& meshData : meshes)
        {
            const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));

            memcpy(&data[dataOffset], &mesh->src_vertices[0], sizeof(float3) * mesh->GetNumVertices());
            dataOffset += sizeof(float3) * mesh->GetNumVertices();
        }
        vbo[ATTRIB_POSITIONS]->Unmap();

        attribs[ATTRIB_POSITIONS] = { POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0 };
    }
    else
    {
        vbo_ptr[ATTRIB_POSITIONS] = nullptr;
        vbo[ATTRIB_POSITIONS].reset();
    }

    if (mesh->HasAttrib(ATTRIB_TEX_COORDS_0))
    {
        uint dataOffset = 0;

        vbo_ptr[ATTRIB_TEX_COORDS_0] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float2) * numVertices, nullptr);
        vbo[ATTRIB_TEX_COORDS_0].reset(vbo_ptr[ATTRIB_TEX_COORDS_0]);
        uint8_t* data = reinterpret_cast<uint8_t*>(vbo[ATTRIB_TEX_COORDS_0]->Map(GL_WRITE_ONLY));
        for (const std::pair<const UID, MeshData>& meshData : meshes)
        {
            const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));

            memcpy(&data[dataOffset], &mesh->src_texcoord0[0], sizeof(float2) * mesh->GetNumVertices());
            dataOffset += sizeof(float2) * mesh->GetNumVertices();
        }
        vbo[ATTRIB_TEX_COORDS_0]->Unmap();

        attribs[ATTRIB_TEX_COORDS_0] = { UV0_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0 };
    }
    else
    {
        vbo_ptr[ATTRIB_TEX_COORDS_0] = nullptr;
        vbo[ATTRIB_TEX_COORDS_0].reset();
    }

    if (mesh->HasAttrib(ATTRIB_NORMALS))
    {
        uint dataOffset = 0;

        vbo_ptr[ATTRIB_NORMALS] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float3) * numVertices, nullptr);
        vbo[ATTRIB_NORMALS].reset(vbo_ptr[ATTRIB_NORMALS]);
        uint8_t* data = reinterpret_cast<uint8_t*>(vbo[ATTRIB_NORMALS]->Map(GL_WRITE_ONLY));
        for (const std::pair<const UID, MeshData>& meshData : meshes)
        {
            const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));
         
            memcpy(&data[dataOffset], &mesh->src_normals[0], sizeof(float3) * mesh->GetNumVertices());
            dataOffset += sizeof(float3) * mesh->GetNumVertices();
        }
        vbo[ATTRIB_NORMALS]->Unmap();

        attribs[ATTRIB_NORMALS] = { NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_TRUE, 0, 0 };
    }
    else
    {
        vbo_ptr[ATTRIB_NORMALS] = nullptr;
        vbo[ATTRIB_NORMALS].reset();
    }

    if (mesh->HasAttrib(ATTRIB_TANGENTS))
    {
        uint dataOffset = 0;
        vbo_ptr[ATTRIB_TANGENTS] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float3) * numVertices, nullptr);
        vbo[ATTRIB_TANGENTS].reset(vbo_ptr[ATTRIB_TANGENTS]);
        uint8_t* data = reinterpret_cast<uint8_t*>(vbo[ATTRIB_TANGENTS]->Map(GL_WRITE_ONLY));
        for (const std::pair<const UID, MeshData>& meshData : meshes)
        {
            const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));

            memcpy(&data[dataOffset], &mesh->src_tangents[0], sizeof(float3) * mesh->GetNumVertices());
            dataOffset += sizeof(float3) * mesh->GetNumVertices();
        }
        vbo[ATTRIB_TANGENTS]->Unmap();

        attribs[ATTRIB_TANGENTS] = { TANGENT_ATTRIB_LOCATION, 3, GL_FLOAT, GL_TRUE, 0, 0 };
    }
    else
    {
        vbo_ptr[ATTRIB_TANGENTS] = nullptr;
        vbo[ATTRIB_TANGENTS].reset();
    }

    if (mesh->HasAttrib(ATTRIB_BONE_INDICES))
    {
        uint dataOffset = 0;
        vbo_ptr[ATTRIB_BONE_INDICES] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(int) * 4 * numVertices, nullptr);
        vbo[ATTRIB_BONE_INDICES].reset(vbo_ptr[ATTRIB_BONE_INDICES]);
        uint8_t* data = reinterpret_cast<uint8_t*>(vbo[ATTRIB_BONE_INDICES]->Map(GL_WRITE_ONLY));
        for (const std::pair<const UID, MeshData>& meshData : meshes)
        {
            const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));
            memcpy(&data[dataOffset], &mesh->src_bone_indices[0], sizeof(int) * 4 * mesh->GetNumVertices());
            dataOffset += sizeof(int) * 4 * mesh->GetNumVertices();
        }
        vbo[ATTRIB_BONE_INDICES]->Unmap();
        attribs[ATTRIB_BONE_INDICES] = { BONE_INDEX_ATTRIB_LOCATION, 4, GL_UNSIGNED_INT, GL_FALSE, 0, 0 };
    }
    else
    {
        vbo_ptr[ATTRIB_BONE_INDICES] = nullptr;
        vbo[ATTRIB_BONE_INDICES].reset();
    }

    if (mesh->HasAttrib(ATTRIB_BONE_WEIGHTS))
    {
        uint dataOffset = 0;
        vbo_ptr[ATTRIB_BONE_WEIGHTS] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float) * 4 * numVertices, nullptr);
        vbo[ATTRIB_BONE_WEIGHTS].reset(vbo_ptr[ATTRIB_BONE_WEIGHTS]);
        uint8_t* data = reinterpret_cast<uint8_t*>(vbo[ATTRIB_BONE_WEIGHTS]->Map(GL_WRITE_ONLY));
        for (const std::pair<const UID, MeshData>& meshData : meshes)
        {
            const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));
            memcpy(&data[dataOffset], &mesh->src_bone_weights[0], sizeof(float) * 4 * mesh->GetNumVertices());
            dataOffset += sizeof(float4) * mesh->GetNumVertices();
        }
        vbo[ATTRIB_BONE_WEIGHTS]->Unmap();
        attribs[ATTRIB_BONE_WEIGHTS] = { BONE_WEIGHT_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, 0, 0 };
    }
    else
    {
        vbo_ptr[ATTRIB_BONE_WEIGHTS] = nullptr;
        vbo[ATTRIB_BONE_WEIGHTS].reset();
    }

    // Indices
    ibo.reset(Buffer::CreateIBO(GL_STATIC_DRAW, sizeof(unsigned) * numIndices, nullptr));
    glObjectLabel(GL_BUFFER, ibo->Id(), -1, "GeometryBatch-INDICES");

    unsigned* indices = reinterpret_cast<unsigned*>(ibo->Map(GL_WRITE_ONLY));

    for (const std::pair<const UID, MeshData>& meshData : meshes)
    {
        const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));
        memcpy(indices, mesh->src_indices.get(), mesh->num_indices * sizeof(uint));
        indices += mesh->num_indices;
    }
   
    ibo->Unmap();

    vao = std::make_unique<VertexArray>(vbo_ptr, ibo.get(), attribs, ATTRIB_COUNT);
}

void GeometryBatch::CreateTransformBuffer()
{
    for (uint i = 0; i < NUM_BUFFERS; ++i)
    {
        transformSSBO[i] = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, objects.size() * sizeof(float4x4), nullptr, true);
        transformsData[i] = reinterpret_cast<float4x4*>(transformSSBO[i]->MapRange(GL_MAP_WRITE_BIT, 0, uint(objects.size() * sizeof(float4x4))));
    }
}

void GeometryBatch::CreateCommandBuffer()
{
    if(commandBufferSize < uint(commands.size()))
    {
        commandBuffer = std::make_unique<Buffer>(GL_DRAW_INDIRECT_BUFFER, GL_STATIC_DRAW, commands.size()*sizeof(DrawCommand), &commands[0], false);
        commandBufferSize = uint(commands.size());
    }
    else
    {
        commandBuffer->SetData(0, uint(commands.size()*sizeof(DrawCommand)), &commands[0]);
    }
}

void GeometryBatch::CreateMaterialBuffer()
{
    struct MaterialData
    {
        float4               diffuse_color;
        float4               specular_color;
        float4               emissive_color;
        float2               tiling;
        float2               offset;
        float2               secondary_tiling;
        float2               secondary_offset;
        float                smoothness;
        float                normal_strength;
        float                alpha_test;
        uint                 mapMask;
        uint64_t             handles[TextureCount];
    };


    materialSSBO = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT, sizeof(MaterialData)*objects.size(), nullptr, true);
    MaterialData* matData = reinterpret_cast<MaterialData*>(materialSSBO->Map(GL_WRITE_ONLY));
    for(const std::pair<const ComponentMeshRenderer*, uint>& object : objects)
    {
        MaterialData& out = matData[object.second];
        const ResourceMaterial* material = reinterpret_cast<const ResourceMaterial*>(App->resources->Get(object.first->GetMaterialRes()->GetUID()));

        out.diffuse_color = material->GetDiffuseColor();
        out.specular_color   = float4(material->GetSpecularColor(), 0.0f);
        out.emissive_color   = float4(material->GetEmissiveColor()*material->GetEmissiveIntensity(), 0.0f);
        out.tiling           = material->GetUVTiling();
        out.offset           = material->GetUVOffset();
        out.secondary_tiling = material->GetSecondUVTiling();
        out.secondary_offset = material->GetSecondUVOffset();
        out.smoothness       = material->GetSmoothness();
        out.normal_strength  = material->GetNormalStrength();
        out.alpha_test       = material->GetAlphaTest();
        out.mapMask          = material->GetMapMask();       

        for(uint i=0; i< TextureCount; ++i)
        {
            const ResourceTexture* texture = material->GetTextureRes(MaterialTexture(i));
            if (texture != nullptr)
            {
                out.handles[i] = texture->GetTexture()->GetBindlessHandle();
            }
        }        
    }

    materialSSBO->Unmap();
}

void GeometryBatch::CreateInstanceBuffer()
{
    instances.resize(objects.size());
    
    totalBones = 0;
    totalTargets = 0;

    for(const std::pair<const ComponentMeshRenderer*, uint>& object : objects)
    {
        PerInstance& out = instances[object.second];

        const MeshData& meshData = meshes[object.first->GetMeshUID()];
        const ResourceMesh* mesh = object.first->GetMeshRes();

        uint numBones        = mesh->GetNumBones();
        uint numMorphAttribs = mesh->GetMorphNumAttribs();
        uint numVertices     = mesh->GetNumVertices();

        out.numBones         = numBones;
        out.baseBone         = totalBones;
        out.numTargets       = meshData.numTargets;
        out.baseTarget       = meshData.baseTarget;
        out.baseTargetWeight = totalTargets;
        out.targetStride     = numMorphAttribs*numVertices;
        out.normalsStride    = numVertices;
        out.tangentsStride   = numVertices*2;

        totalBones        += numBones;
        totalTargets      += meshData.numTargets;
    }

    instanceSSBO = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, 0, sizeof(PerInstance)*objects.size(), &instances[0], true);

    if (totalBones > 0)
    {
        for (uint i = 0; i < NUM_BUFFERS; ++i)
        {
            skinning[i] = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, totalBones * sizeof(float4x4), nullptr, true);
            skinningData[i] = reinterpret_cast<float4x4*>(skinning[i]->MapRange(GL_MAP_WRITE_BIT, 0, uint(totalBones * sizeof(float4x4))));
        }
    }

    if (totalTargets > 0)
    {
        for (uint i = 0; i < NUM_BUFFERS; ++i)
        {
            morphWeights[i] = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, totalTargets * sizeof(float), nullptr, true);
            morphWeightsData[i] = reinterpret_cast<float*>(morphWeights[i]->MapRange(GL_MAP_WRITE_BIT, 0, uint(totalTargets * sizeof(float))));
        }
    }
}

void GeometryBatch::CreateMorphBuffer()
{
    uint totalTargetOffset = 0;

    for(std::pair<const UID, MeshData>& meshData  : meshes)
    {
        const ResourceMesh *mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));

        MeshData& data = meshData.second;

        uint numTargets     = mesh->GetNumMorphTargets();
        uint targetsAttribs = mesh->GetMorphNumAttribs();
        uint numVertices    = mesh->GetNumVertices();

        data.numTargets = numTargets;
        data.baseTarget = totalTargetOffset;

        totalTargetOffset += targetsAttribs*numTargets*numVertices;
    }

    if (totalTargetOffset > 0)
    {
        morphBuffer = std::make_unique<Buffer>(GL_TEXTURE_BUFFER, GL_MAP_WRITE_BIT, totalTargetOffset * sizeof(float3), nullptr, true);
        float3* data = reinterpret_cast<float3*>(morphBuffer->Map(GL_WRITE_ONLY));

        for (const std::pair<const UID, MeshData>& meshData : meshes)
        {
            const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));

            uint numTargets = mesh->GetNumMorphTargets();

            for (uint i = 0; i < numTargets; ++i)
            {
                const ResourceMesh::MorphData& morphData = mesh->GetMorphTarget(i);

                uint morphSize = mesh->GetMorphNumAttribs() ;
                uint numVertices = mesh->GetNumVertices();

                memcpy(&data[numVertices * morphSize * i], morphData.src_vertices.get(), numVertices*sizeof(float3));

                if (mesh->HasAttrib(ATTRIB_NORMALS))
                {
                    memcpy(&data[numVertices * morphSize * i + numVertices], morphData.src_normals.get(), numVertices * sizeof(float3));
                    if (mesh->HasAttrib(ATTRIB_TANGENTS))
                    {
                        memcpy(&data[numVertices * morphSize * i + numVertices * 2], morphData.src_tangents.get(), numVertices * sizeof(float3));
                    }
                }
            }
        }

        morphBuffer->Unmap();

        morphTexture = std::make_unique<TextureBuffer>(morphBuffer.get(), GL_RGB32F);
    }
}


void GeometryBatch::Render(const ComponentMeshRenderer* object)
{
    if (bufferDirty)
    {
        CreateRenderData();
    }

    auto itMesh = meshes.find(object->GetMeshUID());
    auto itObject = objects.find(object);

    if(itMesh != meshes.end() && itObject != objects.end())
    {
        DrawCommand command = {itMesh->second.indexCount, 1, itMesh->second.baseIndex, itMesh->second.baseVertex, itObject->second};

        commands.push_back(command);        
    }
}

void GeometryBatch::DoUpdate()
{
    if (bufferDirty)
    {
        CreateRenderData();
    }

    UpdateModels();
}


void GeometryBatch::DoRender(uint flags)
{
    if (!commands.empty())
    {
        CreateCommandBuffer();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, MODEL_SSBO_BINDING, transformSSBO[frameCount]->Id());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PERINSTANCE_SSBO_BINDING, instanceSSBO->Id());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, MATERIAL_SSBO_BINDING, materialSSBO->Id());

        if (skinning[frameCount])
        {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PALETTE_SSBO_BINDING, skinning[frameCount]->Id());
        }

        if (morphWeights[frameCount])
        {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, MORPH_WEIGHT_SSBO_BINDING, morphWeights[frameCount]->Id());
        }

        commandBuffer->Bind();

        if(morphTexture) 
        {
            morphTexture->Bind(MORPH_TARGET_TBO_BINDING);
        }

        vao->Bind();
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0, int(commands.size()), 0);
        vao->Unbind();

        commands.clear();

        if(sync[frameCount])
        {
            glDeleteSync((GLsync)sync[frameCount]);
        }

        sync[frameCount] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }
}

void GeometryBatch::Remove(const ComponentMeshRenderer* object)
{
    assert(commands.empty());

    auto it = meshes.find(object->GetMeshUID());
    if((--it->second.refCount) == 0)
    {
        meshes.erase(it);
    }

    objects.erase(object);

	ClearRenderData();
}

void GeometryBatch::MarkForUpdate(const ComponentMeshRenderer *object)
{
    modelUpdates.push_back(object);
}

void GeometryBatch::UpdateModels()
{
    if(!modelUpdates.empty())
    {
        frameCount = (frameCount + 1) % NUM_BUFFERS;

        if(sync[frameCount])
        {
            GLenum waitReturn;
            
            do
            {
                waitReturn = glClientWaitSync((GLsync)sync[frameCount], GL_SYNC_FLUSH_COMMANDS_BIT, 1000);
            }while(waitReturn == GL_TIMEOUT_EXPIRED);
        }

        float4x4* transforms = transformsData[frameCount]; 
        float4x4* palette    = nullptr;
        float* weights       = nullptr;

        if (totalBones > 0)
        {
            palette = skinningData[frameCount]; 
        }

        if (totalTargets > 0)
        {
            weights = morphWeightsData[frameCount]; 
        }

        for (const ComponentMeshRenderer *object : modelUpdates)
        {
            auto it = objects.find(object);
            if (it != objects.end())
            {
                transforms[it->second] = object->GetGameObject()->GetGlobalTransformation();

                static bool stop = false;

                // skinning update
                const PerInstance& instanceData = instances[it->second];
                if(instanceData.numBones > 0 && !stop)
                {
                    object->UpdateSkinPalette(&palette[instanceData.baseBone]);
                }

                // morph targets
                if (totalTargets)
                {
                    memcpy(&weights[instanceData.baseTargetWeight], object->GetMorphTargetWeights(), sizeof(float) * instanceData.numTargets);
                }
            }
        }

        modelUpdates.clear();
    }
}

void GeometryBatch::OnMaterialModified(UID materialID)
{
    std::vector<const ComponentMeshRenderer*> modifyList;

    modifyList.reserve(objects.size());

    for (std::pair<const ComponentMeshRenderer *const, uint> &object : objects)
    {
        if(object.first->GetMaterialUID() == materialID)
        {
            modifyList.push_back(object.first);
        }
    }

    for(const ComponentMeshRenderer* object : modifyList)
    {
        Remove(object);
    }

    for(const ComponentMeshRenderer* object : modifyList)
    {
        App->renderer->GetBatchManager()->Add(object, tagName);
    }
}