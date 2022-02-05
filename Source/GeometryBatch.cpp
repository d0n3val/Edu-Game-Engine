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

#include <algorithm>

#include "Leaks.h"

GeometryBatch::GeometryBatch(const HashString& tag) : tagName(tag)
{
}

bool GeometryBatch::CanAdd(const ComponentMeshRenderer* object) const
{
    if(objects.empty())
    {
        return true;
    }

 	return object->GetMeshRes()->GetAttribs() == attrib_flags && textures.CanAdd(object->GetMaterialRes());
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
    objects[object] = 0;

    textures.Add(object->GetMaterialRes());

    ClearRenderData();
}

void GeometryBatch::CreateRenderData()
{
    CreateVertexBuffers();
    CreateDrawIdBuffer();
    CreateMaterialBuffer();
    CreateTransformBuffer();
    bufferDirty = false;
    modelUpdates.clear();
}

void GeometryBatch::ClearRenderData()
{
    vao.reset();
    vbo.reset();
    ibo.reset();
    drawIdVBO.reset();
    commands.clear();

    bufferDirty = true;
}

void GeometryBatch::CreateVertexBuffers()
{
    uint         vertex_size  = 0;
    uint         attrib_count = 0;
    VertexAttrib attribs[ATTRIB_COUNT];

    GetVertexAttribs(attribs, attrib_count, vertex_size);

    uint numIndices  = 0;
    uint numVertices = 0;

    for(const std::pair<const UID, MeshData>& meshData  : meshes)
    {
        const ResourceMesh *mesh = static_cast<const ResourceMesh *>(App->resources->Get(meshData.first));
        numVertices += mesh->num_vertices;
        numIndices  += mesh->num_indices;
    }

    vbo.reset(Buffer::CreateVBO(GL_STATIC_DRAW, vertex_size*numVertices, nullptr));
    ibo.reset(Buffer::CreateIBO(GL_STATIC_DRAW, sizeof(unsigned)*numIndices, nullptr));

    uint8_t* data     = reinterpret_cast<uint8_t*>(vbo->Map(GL_WRITE_ONLY));
    unsigned* indices = reinterpret_cast<unsigned*>(ibo->Map(GL_WRITE_ONLY));

    uint baseVertex = 0;
    uint baseIndex = 0;

    for(std::pair<const UID, MeshData>& meshData  : meshes)
    {
        const ResourceMesh* mesh = static_cast<const ResourceMesh*>(App->resources->Get(meshData.first));

        meshData.second.baseVertex  = baseVertex;
        meshData.second.baseIndex   = baseIndex;
        meshData.second.vertexCount = mesh->num_vertices;
        meshData.second.indexCount  = mesh->num_indices;

        baseVertex += mesh->num_vertices;
        baseIndex += mesh->num_indices;

        for(uint i=0; i < mesh->GetNumVertices(); ++i)
        {
            (float3&)(data[i*vertex_size]) = mesh->src_vertices[i];
        }

        uint attrib_index = 0;

        if(mesh->HasAttrib(ATTRIB_TEX_COORDS_0))
        {
            uint offset = attribs[++attrib_index].offset;

            for(uint i=0; i < mesh->GetNumVertices(); ++i)
            {
                (float2&)(data[i*vertex_size+offset]) = float2(mesh->src_texcoord0[i].x, mesh->src_texcoord0[i].y);
            }
        }

        if(mesh->HasAttrib(ATTRIB_NORMALS))
        {
            uint offset = attribs[++attrib_index].offset;

            for(uint i=0; i < mesh->GetNumVertices(); ++i)
            {
                (float3&)(data[i*vertex_size+offset]) = mesh->src_normals[i];
            }
        }

        if(mesh->HasAttrib(ATTRIB_TANGENTS))
        {
            uint offset = attribs[++attrib_index].offset;

            for(uint i=0; i < mesh->GetNumVertices(); ++i)
            {
                (float3&)(data[i*vertex_size+offset]) = mesh->src_tangents[i];
            }
        }

        memcpy(indices, mesh->src_indices.get(), mesh->num_indices*sizeof(uint));

        data += mesh->num_vertices*vertex_size;
        indices += mesh->num_indices;
    }

    vbo->Unmap();
    ibo->Unmap();

    vao = std::make_unique<VertexArray>(vbo.get(), ibo.get(), attribs, attrib_count);
}

void GeometryBatch::CreateTransformBuffer()
{
    transformSSBO = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT, objects.size()*sizeof(float4x4), nullptr, true);
    float4x4* transforms = reinterpret_cast<float4x4*>(transformSSBO->Map(GL_WRITE_ONLY));
    for(std::pair<const ComponentMeshRenderer* const, uint>& object : objects)
    {
        transforms[object.second] = object.first->GetGameObject()->GetGlobalTransformation();
    }
    transformSSBO->Unmap();
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
        TextureBatch::Handle handles[TextureCount];
    };

    textures.GenerateTextures();

    materialSSBO = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW, sizeof(MaterialData)*objects.size(), nullptr);
    MaterialData* matData = reinterpret_cast<MaterialData*>(materialSSBO->Map(GL_WRITE_ONLY));
    for(const std::pair<const ComponentMeshRenderer*, uint>& object : objects)
    {
        MaterialData& out = matData[object.second];
        const ResourceMaterial* material = reinterpret_cast<const ResourceMaterial*>(App->resources->Get(object.first->GetMaterialRes()->GetUID()));

        out.diffuse_color    = material->GetDiffuseColor();
        out.specular_color   = float4(material->GetSpecularColor(), 0.0f);
        out.emissive_color   = float4(material->GetEmissiveColor(), 0.0f);
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
                textures.GetHandle(texture, out.handles[i]);
            }
        }
    }

    materialSSBO->Unmap();
}

void GeometryBatch::CreateDrawIdBuffer()
{
    drawIdVBO.reset(Buffer::CreateVBO(GL_STATIC_DRAW, uint(objects.size()*sizeof(int)), nullptr));
    int* drawIds = (int*)drawIdVBO->Map(GL_WRITE_ONLY);

    uint instanceId = 0;
    for(std::pair<const ComponentMeshRenderer* const, uint>& object : objects)
    {
        object.second = instanceId++;
        drawIds[object.second] = object.second;
    }
    drawIdVBO->Unmap();

    // add drawId to vao
    vao->Bind();
    drawIdVBO->Bind();
    glEnableVertexAttribArray(7);
    glVertexAttribIPointer(7, 1, GL_INT, sizeof(int), (void *)0);
    glVertexAttribDivisor(7, 1);
    vao->Unbind();
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
        DrawCommand command = {itMesh->second.indexCount, 1, itMesh->second.baseIndex, itMesh->second.baseVertex, itObject->second };
        commands.push_back(command);
    }
}

void GeometryBatch::DoRender(uint transformsIndex, uint materialsIndex)
{
    if (!commands.empty())
    {
        CreateCommandBuffer();
        UpdateModels();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, transformsIndex, transformSSBO->Id());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, materialsIndex, materialSSBO->Id());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, materialsIndex, materialSSBO->Id());
        commandBuffer->Bind();

        textures.Bind();

        vao->Bind();
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0, int(commands.size()), 0);
        vao->Unbind();

        commands.clear();
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
    textures.Remove(object->GetMaterialRes());

	ClearRenderData();
}

void GeometryBatch::GetVertexAttribs(VertexAttrib *attribs, uint &count, uint& vertex_size) const
{
    vertex_size      = sizeof(float3);
    count            = 0;
    attribs[count++] = {0, 3, GL_FLOAT, GL_FALSE, 0, 0};

    const ResourceMesh* mesh = objects.begin()->first->GetMeshRes();

    if(mesh->HasAttrib(ATTRIB_TEX_COORDS_0))
    {
        attribs[count++] = {2, 2, GL_FLOAT, GL_FALSE, 0, vertex_size} ;
        vertex_size += sizeof(float2);
    }

    if(mesh->HasAttrib(ATTRIB_NORMALS))
    {
        attribs[count++] = {1, 3, GL_FLOAT, GL_TRUE, 0, vertex_size};
        vertex_size += sizeof(float3);
    }

    if(mesh->HasAttrib(ATTRIB_TANGENTS))
    {
        attribs[count++] = {5, 3, GL_FLOAT, GL_TRUE, 0, vertex_size};
        vertex_size += sizeof(float3);
    }

    for(uint i=0; i< count; ++i)
    {
        attribs[i].stride = vertex_size;
    }
}

void GeometryBatch::UpdateModel(const ComponentMeshRenderer *object)
{
    modelUpdates.push_back(object);
}

void GeometryBatch::UpdateModels()
{
    if(!modelUpdates.empty())
    {
        float4x4* transforms = reinterpret_cast<float4x4*>(transformSSBO->MapRange(GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT, 0, uint(objects.size() * sizeof(float4x4))));
        for (const ComponentMeshRenderer *object : modelUpdates)
        {
            auto it = objects.find(object);
            if (it != objects.end())
            {
                transforms[it->second] = object->GetGameObject()->GetGlobalTransformation();
            }
        }
        transformSSBO->Unmap();

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