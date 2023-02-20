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

GeometryBatch::GeometryBatch(const HashString& tag, Program* program) : tagName(tag), skinningProgram(program)
{
}

GeometryBatch::~GeometryBatch()
{
    for(int i=0; i< NUM_BUFFERS; ++i)
    {
        if (sync[frameCount])
        {
            glDeleteSync((GLsync)sync[frameCount]);
        }
    }
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

    meshes[object->GetMeshUID()].refCount++;
    objects[object] = -1;

    ClearRenderData();
}

void GeometryBatch::CreateRenderData()
{
    if (objects.empty())
    {
        ClearRenderData();
    }
    else
    {
        uint index = 0;
        for (auto& object : objects) object.second = index++;

        CreateVertexBuffers();

        CreateMaterialBuffer();
        CreateMorphBuffer();
        CreateInstanceBuffer();
        CreateTransformBuffer();

        if (totalBones > 0)
        {
            CreateSkinningBuffers();
        }

        bufferDirty = false;
        modelUpdates.clear();
    }
}

void GeometryBatch::ClearRenderData()
{
    vao.reset();
    for(uint i=0; i< ATTRIB_COUNT+1; ++i)
    {
        vbo[i].release();
    }
    ibo.release();
    tpose_positions.release();
    tpose_normals.release();
    tpose_tangents.release();;
    commands.clear();

    bufferDirty = true;
}

void GeometryBatch::CreateSkinningBuffers()
{
    SDL_assert(totalBones > 0);

    tpose_positions.reset(Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float4) * totalVertices, nullptr));
    float4* positions = reinterpret_cast<float4*>(tpose_positions->Map(GL_WRITE_ONLY));
    uint offset = 0;

    for (const std::pair<const UID, MeshData> &meshData : meshes)
    {
        const ResourceMesh *mesh = static_cast<const ResourceMesh *>(App->resources->Get(meshData.first));

        for(uint i=0; i < mesh->GetNumVertices(); ++i)
        {
            const float3& vtx = mesh->src_vertices[i];
            positions[offset+i].Set(vtx.x, vtx.y, vtx.z, 1.0);
        }
        offset += mesh->GetNumVertices();
    }

    tpose_positions->Unmap();

    tpose_normals.reset(Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float4) * totalVertices, nullptr));
    float4* normals = reinterpret_cast<float4*>(tpose_normals->Map(GL_WRITE_ONLY));
    offset = 0;

    for (const std::pair<const UID, MeshData> &meshData : meshes)
    {
        const ResourceMesh *mesh = static_cast<const ResourceMesh *>(App->resources->Get(meshData.first));

        for(uint i=0; i < mesh->GetNumVertices(); ++i)
        {
            const float3& vtx = mesh->src_normals[i];
            normals[offset+i].Set(vtx.x, vtx.y, vtx.z, 0.0);
        }

        offset += mesh->GetNumVertices();
    }

    tpose_normals->Unmap();

    tpose_tangents.reset(Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float4) * totalVertices, nullptr));
    float4* tangents = reinterpret_cast<float4*>(tpose_tangents->Map(GL_WRITE_ONLY));
    offset = 0;

    for (const std::pair<const UID, MeshData> &meshData : meshes)
    {
        const ResourceMesh *mesh = static_cast<const ResourceMesh *>(App->resources->Get(meshData.first));

        for(uint i=0; i < mesh->GetNumVertices(); ++i)
        {
            const float3& vtx = mesh->src_tangents[i];
            tangents[offset+i].Set(vtx.x, vtx.y, vtx.z, 0.0);
        }

        offset += mesh->GetNumVertices();
    }

    tpose_tangents->Unmap();

    offset = 0;
    bone_indices.reset(Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(int) * 4 * totalVertices, nullptr));
    uint8_t* indices = reinterpret_cast<uint8_t*>(bone_indices->Map(GL_WRITE_ONLY));
    for (const std::pair<const UID, MeshData>& meshData : meshes)
    {
        const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));
        memcpy(&indices[offset], &mesh->src_bone_indices[0], sizeof(int) * 4 * mesh->GetNumVertices());
        offset += sizeof(int) * 4 * mesh->GetNumVertices();
    }
    bone_indices->Unmap();

    offset = 0;
    bone_weights.reset(Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float) * 4 * totalVertices, nullptr));
    uint8_t* weights = reinterpret_cast<uint8_t*>(bone_weights->Map(GL_WRITE_ONLY));
    for (const std::pair<const UID, MeshData>& meshData : meshes)
    {
        const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));
        memcpy(&weights[offset], &mesh->src_bone_weights[0], sizeof(float) * 4 * mesh->GetNumVertices());
        offset += sizeof(float4) * mesh->GetNumVertices();
    }
    bone_weights->Unmap();
}

void GeometryBatch::CreateVertexBuffers()
{
    SDL_assert(!objects.empty());

    Buffer* vbo_ptr[ATTRIB_COUNT];

    for (int i = 0; i < ATTRIB_COUNT; ++i) vbo_ptr[i] = nullptr;

    totalVertices = 0;
    totalIndices = 0;

    for(std::pair<const UID, MeshData>& meshData  : meshes)
    {
        const ResourceMesh *mesh = static_cast<const ResourceMesh *>(App->resources->Get(meshData.first));

        meshData.second.baseVertex = totalVertices;
        meshData.second.baseIndex = totalIndices;
        meshData.second.vertexCount = mesh->num_vertices;
        meshData.second.indexCount = mesh->num_indices;

        totalVertices += mesh->num_vertices;
        totalIndices += mesh->num_indices;
    }

    // Vertex attributes 

    VertexAttrib attribs[ATTRIB_COUNT];

    const ResourceMesh* mesh = objects.begin()->first->GetMeshRes();

    if (mesh->HasAttrib(ATTRIB_POSITIONS))
    {
        uint offset = 0;
        vbo_ptr[ATTRIB_POSITIONS] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float4) * totalVertices, nullptr);
        glObjectLabel(GL_BUFFER, vbo_ptr[ATTRIB_POSITIONS]->Id(), -1, "GeometryBatch-POSITION");

        vbo[ATTRIB_POSITIONS].reset(vbo_ptr[ATTRIB_POSITIONS]);
        float4* data = reinterpret_cast<float4*>(vbo[ATTRIB_POSITIONS]->Map(GL_WRITE_ONLY));
        for (const std::pair<const UID, MeshData>& meshData : meshes)
        {
            const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));

            for (uint i = 0; i < mesh->GetNumVertices(); ++i)
            {
                const float3 &vtx = mesh->src_vertices[i];
                data[offset + i].Set(vtx.x, vtx.y, vtx.z, 1.0);
            }

            offset += mesh->GetNumVertices();
        }
        vbo[ATTRIB_POSITIONS]->Unmap();

        attribs[ATTRIB_POSITIONS] = { POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, false, sizeof(float4), 0, 0};
    }
    else
    {
        vbo_ptr[ATTRIB_POSITIONS] = nullptr;
        vbo[ATTRIB_POSITIONS].reset();
    }

    if (mesh->HasAttrib(ATTRIB_TEX_COORDS_0))
    {
        uint dataOffset = 0;

        vbo_ptr[ATTRIB_TEX_COORDS_0] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float2) * totalVertices, nullptr);
        vbo[ATTRIB_TEX_COORDS_0].reset(vbo_ptr[ATTRIB_TEX_COORDS_0]);
        uint8_t* data = reinterpret_cast<uint8_t*>(vbo[ATTRIB_TEX_COORDS_0]->Map(GL_WRITE_ONLY));
        for (const std::pair<const UID, MeshData>& meshData : meshes)
        {
            const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));

            memcpy(&data[dataOffset], &mesh->src_texcoord0[0], sizeof(float2) * mesh->GetNumVertices());
            dataOffset += sizeof(float2) * mesh->GetNumVertices();
        }
        vbo[ATTRIB_TEX_COORDS_0]->Unmap();

        attribs[ATTRIB_TEX_COORDS_0] = { UV0_ATTRIB_LOCATION, 2, GL_FLOAT, false, 0, 0 , 0};
    }
    else
    {
        vbo_ptr[ATTRIB_TEX_COORDS_0] = nullptr;
        vbo[ATTRIB_TEX_COORDS_0].reset();
    }

    if (mesh->HasAttrib(ATTRIB_NORMALS))
    {
        uint offset = 0;

        vbo_ptr[ATTRIB_NORMALS] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float4) * totalVertices, nullptr);
        vbo[ATTRIB_NORMALS].reset(vbo_ptr[ATTRIB_NORMALS]);
        float4* data = reinterpret_cast<float4*>(vbo[ATTRIB_NORMALS]->Map(GL_WRITE_ONLY));
        for (const std::pair<const UID, MeshData>& meshData : meshes)
        {
            const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));
         
            for (uint i = 0; i < mesh->GetNumVertices(); ++i)
            {
                const float3 &vtx = mesh->src_normals[i];
                data[offset + i].Set(vtx.x, vtx.y, vtx.z, 0.0);
            }

            offset += mesh->GetNumVertices();
        }
        vbo[ATTRIB_NORMALS]->Unmap();

        attribs[ATTRIB_NORMALS] = { NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, false, sizeof(float4), 0, 0};
    }
    else
    {
        vbo_ptr[ATTRIB_NORMALS] = nullptr;
        vbo[ATTRIB_NORMALS].reset();
    }

    if (mesh->HasAttrib(ATTRIB_TANGENTS))
    {
        uint offset = 0;
        vbo_ptr[ATTRIB_TANGENTS] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float4) * totalVertices, nullptr);
        vbo[ATTRIB_TANGENTS].reset(vbo_ptr[ATTRIB_TANGENTS]);
        float4* data = reinterpret_cast<float4*>(vbo[ATTRIB_TANGENTS]->Map(GL_WRITE_ONLY));
        for (const std::pair<const UID, MeshData>& meshData : meshes)
        {
            const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));

            for (uint i = 0; i < mesh->GetNumVertices(); ++i)
            {
                const float3 &vtx = mesh->src_tangents[i];
                data[offset + i].Set(vtx.x, vtx.y, vtx.z, 0.0);
            }

            offset += mesh->GetNumVertices();
        }
        vbo[ATTRIB_TANGENTS]->Unmap();

        attribs[ATTRIB_TANGENTS] = { TANGENT_ATTRIB_LOCATION, 3, GL_FLOAT, false, sizeof(float4), 0, 0};
    }
    else
    {
        vbo_ptr[ATTRIB_TANGENTS] = nullptr;
        vbo[ATTRIB_TANGENTS].reset();
    }

    // Indices
    ibo.reset(Buffer::CreateIBO(GL_STATIC_DRAW, sizeof(unsigned) * totalIndices, nullptr));
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

    frameCount = (frameCount + 1) % NUM_BUFFERS;

    if (sync[frameCount])
    {
        GLenum waitReturn;

        do
        {
            waitReturn = glClientWaitSync((GLsync)sync[frameCount], GL_SYNC_FLUSH_COMMANDS_BIT, 1000);
        } while (waitReturn == GL_TIMEOUT_EXPIRED);
    }

    UpdateModels();

    if(totalBones > 0 || totalTargets > 0) 
    {
        UpdateSkinning();
    }

    modelUpdates.clear();
}

void GeometryBatch::DoRender(uint flags)
{
    if (!commands.empty())
    {
        CreateCommandBuffer();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, MODEL_SSBO_BINDING, transformSSBO[frameCount]->Id());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, MATERIAL_SSBO_BINDING, materialSSBO->Id());

        commandBuffer->Bind();

        // due to skinning compute
        if(totalBones > 0 || totalTargets > 0) 
        {
            glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
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
    SDL_assert(objects.find(object) != objects.end());
    modelUpdates.insert(object);
}

void GeometryBatch::UpdateSkinning()
{
    SDL_assert(totalBones > 0);

    float4x4 *palette = skinningData[frameCount];

    for (const ComponentMeshRenderer *object : modelUpdates)
    {
        auto it = objects.find(object);
        SDL_assert(it != objects.end());

        if(it != objects.end())
        {
            const PerInstance& instanceData = instances[it->second];
            const MeshData &meshData = meshes[object->GetMeshUID()];

            skinningProgram->Use();

            skinningProgram->BindUniform(SKINNING_NUM_VERTICES_LOCATION, int(meshData.vertexCount));
            skinningProgram->BindUniform(SKINNING_NUM_BONES_LOCATION, int(instanceData.numBones));
            skinningProgram->BindUniform(SKINNING_NUM_TARGETS_LOCATION, int(instanceData.numTargets));
            skinningProgram->BindUniform(SKINNING_TARGET_STRIDE_LOCATION, int(instanceData.targetStride));
            skinningProgram->BindUniform(SKINNING_NORMAL_STRIDE_LOCATION, int(instanceData.normalsStride));
            skinningProgram->BindUniform(SKINNING_TANGENT_STRIDE_LOCATION, int(instanceData.tangentsStride));

            if (instanceData.numBones > 0)
            {
                object->UpdateSkinPalette(&palette[instanceData.baseBone]);

                // Bind buffers
                skinningProgram->BindSSBO(SKINNING_PALETTE_BINDING, skinning[frameCount].get(), instanceData.baseBone * sizeof(float4x4), instanceData.numBones * sizeof(float4x4));
                skinningProgram->BindSSBO(SKINNING_INDICES_BINDING, bone_indices.get(), meshData.baseVertex * sizeof(int)*4, meshData.vertexCount * sizeof(int) * 4);
                skinningProgram->BindSSBO(SKINNING_WEIGHTS_BINDING, bone_weights.get(), meshData.baseVertex * sizeof(float)*4, meshData.vertexCount * sizeof(float) * 4);

                skinningProgram->BindSSBO(SKINNING_POSITIONS_BINDING, tpose_positions.get(), meshData.baseVertex * sizeof(float4), meshData.vertexCount * sizeof(float4));
                skinningProgram->BindSSBO(SKINNING_INNORMALS_BINDING, tpose_normals.get(), meshData.baseVertex * sizeof(float4), meshData.vertexCount * sizeof(float4));
                skinningProgram->BindSSBO(SKINNING_INTANGENTS_BINDING, tpose_tangents.get(), meshData.baseVertex * sizeof(float4), meshData.vertexCount * sizeof(float4));

                skinningProgram->BindSSBO(SKINNING_OUTPOSITIONS_BINDING, vbo[ATTRIB_POSITIONS].get(), meshData.baseVertex * sizeof(float4), meshData.vertexCount * sizeof(float4));
                skinningProgram->BindSSBO(SKINNING_OUTNORMALS_BINDING, vbo[ATTRIB_NORMALS].get(), meshData.baseVertex * sizeof(float4), meshData.vertexCount * sizeof(float4));
                skinningProgram->BindSSBO(SKINNING_OUTTANGENTS_BINDING, vbo[ATTRIB_TANGENTS].get(), meshData.baseVertex * sizeof(float4), meshData.vertexCount * sizeof(float4));
            }

            if(totalTargets > 0)
            {
                memcpy(&morphWeightsData[frameCount][instanceData.baseTargetWeight], object->GetMorphTargetWeights(), sizeof(float) * instanceData.numTargets);

                skinningProgram->BindSSBO(SKINNING_MORPH_WEIGHTS_BINDING, morphWeights[frameCount].get(), instanceData.baseTargetWeight*sizeof(float), instanceData.numTargets*sizeof(float));
                morphTexture->Bind(SKINNING_MORPH_TARGET_BINDING);
            }

            int numWorkGroups = (meshData.vertexCount + (SKINNING_GROUP_SIZE - 1)) / SKINNING_GROUP_SIZE;
            glDispatchCompute(numWorkGroups, 1, 1);
        }
    }
}

void GeometryBatch::UpdateModels()
{
    if(!modelUpdates.empty())
    {
        float4x4* transforms = transformsData[frameCount]; 
        float4x4* palette    = nullptr;
        float* weights       = nullptr;

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
                const PerInstance& instanceData = instances[it->second];

                // morph targets
                if (totalTargets)
                {
                    memcpy(&weights[instanceData.baseTargetWeight], object->GetMorphTargetWeights(), sizeof(float) * instanceData.numTargets);
                }
            }
        }
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