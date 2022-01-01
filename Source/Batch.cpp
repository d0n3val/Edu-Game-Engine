#include "Globals.h"

#include "Batch.h"

#include "Application.h"
#include "ComponentMeshRenderer.h"
#include "ModuleTextures.h"
#include "ModuleResources.h"
#include "GameObject.h"

#include "ResourceTexture.h"

#include "OpenGL.h"

#include <algorithm>

#include "Leaks.h"

Batch::Batch(const HashString& tag) : tagName(tag), firstFreeObject(UINT32_MAX)
{
}

bool Batch::CanAdd(ComponentMeshRenderer* object) const
{
    if(objects.empty())
    {
        return true;
    }

    const ResourceMesh* mesh         = object->GetMeshRes();
    const ResourceMaterial* material = object->GetMaterialRes();

	return mesh->GetAttribs() == attrib_flags && textures.CanAdd(material);
}

uint Batch::Add(ComponentMeshRenderer* object)
{
    assert(CanAdd(object));

    if(objects.size() == 0)
    {
        Init(object);
    }

    meshes[object->GetMeshRes()->GetUID()].refCount++;

    const ResourceMaterial* material = object->GetMaterialRes();
    if((materials[material->GetUID()].refCount++) == 0)
    {
        textures.Add(material);
    }

    uint objectId = 0;

    if(firstFreeObject != UINT32_MAX)
    {
        objectId = firstFreeObject;
        firstFreeObject = objectHandles[firstFreeObject];
        objectHandles[objectId] = uint(objects.size());
    }
    else
    {
        objectId = uint(objectHandles.size());
        objectHandles.push_back(uint(objects.size()));
    }

    objects.push_back(object);

    ClearRenderData();

    return objectId;
}

void Batch::Init(ComponentMeshRenderer* object)
{
    const ResourceMesh* mesh         = object->GetMeshRes();
    const ResourceMaterial* material = object->GetMaterialRes();

    // Texture sizes

    for(uint i=0; i< TextureCount; ++i)
    {
        const ResourceTexture* texture = material->GetTextureRes(MaterialTexture(i));
        if (texture)
        {
            texture_size[i][0] = texture->GetWidth();
            texture_size[i][1] = texture->GetHeight();
        }
        else
        {
            texture_size[i][0] = 0;
            texture_size[i][1] = 0;
        }
    }

    // Vertex Attribs and vertex size 

    attrib_flags            = mesh->GetAttribs();
    vertex_size             = sizeof(float3);
    attrib_count            = 0;
    attribs[attrib_count++] = {0, 3, GL_FLOAT, GL_FALSE, 0, 0};

    if(mesh->HasAttrib(ATTRIB_TEX_COORDS_0))
    {
        attribs[attrib_count++] = {2, 2, GL_FLOAT, GL_FALSE, 0, vertex_size} ;

        vertex_size += sizeof(float2);
    }

    if(mesh->HasAttrib(ATTRIB_NORMALS))
    {
        attribs[attrib_count++] = {1, 3, GL_FLOAT, GL_TRUE, 0, vertex_size};

        vertex_size += sizeof(float3);
    }

    if(mesh->HasAttrib(ATTRIB_TANGENTS))
    {
        attribs[attrib_count++] = {5, 3, GL_FLOAT, GL_TRUE, 0, vertex_size};

        vertex_size += sizeof(float3);
    }

    for(uint i=0; i< attrib_count; ++i)
    {
        attribs[i].stride = vertex_size;
    }
}

void Batch::CreateRenderData()
{
    CreateVertexBuffers();
    CreateMaterialBuffer();
    CreateTransformBuffer();
    CreateDrawIdBuffer();
    bufferDirty = false;
}

void Batch::ClearRenderData()
{
    vao.reset();
    vbo.reset();
    ibo.reset();
    drawIdVBO.reset();
    commands.clear();

    bufferDirty = true;
}

void Batch::CreateVertexBuffers()
{
    uint numIndices  = 0;
    uint numVertices = 0;

    for(const std::pair<UID, MeshData>& meshData  : meshes)
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

        meshData.second.baseVertex = baseVertex;
        meshData.second.baseIndex = baseIndex;
        meshData.second.vertexCount = mesh->num_vertices;
        meshData.second.indexCount = mesh->num_indices;

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

void Batch::CreateTransformBuffer()
{
    transformSSBO = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, objects.size()*sizeof(float4x4), nullptr);
    float4x4* transforms = reinterpret_cast<float4x4*>(transformSSBO->Map(GL_WRITE_ONLY));
    for(uint index = 0, count = uint(objects.size()); index < count; ++index)
    {
        transforms[index] = objects[index]->GetGameObject()->GetGlobalTransformation();
    }
    transformSSBO->Unmap();
}

void Batch::CreateCommandBuffer()
{
    if(commandBufferSize < uint(commands.size()))
    {
        commandBuffer = std::make_unique<Buffer>(GL_DRAW_INDIRECT_BUFFER, GL_DYNAMIC_DRAW, commands.size()*sizeof(DrawCommand), &commands[0]);
        commandBufferSize = uint(commands.size());
    }
    else
    {
        commandBuffer->SetData(0, uint(commands.size()*sizeof(DrawCommand)), &commands[0]);
    }
}

void Batch::CreateMaterialBuffer()
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

    materialSSBO = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW, sizeof(MaterialData)*materials.size(), nullptr);
    MaterialData* matData = reinterpret_cast<MaterialData*>(materialSSBO->Map(GL_WRITE_ONLY));
    for(auto it = materials.begin(); it != materials.end(); ++it)
    {
        MaterialData& out = *(matData++);
        const ResourceMaterial* material = reinterpret_cast<const ResourceMaterial*>(App->resources->Get(it->first));

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

void Batch::CreateDrawIdBuffer()
{
    drawIdVBO.reset(Buffer::CreateVBO(GL_STATIC_DRAW, uint(objects.size()*sizeof(int)), nullptr));
    int* drawIds = (int*)drawIdVBO->Map(GL_WRITE_ONLY);

    for(uint i=0; i< uint(objects.size()); ++i)
    {
        drawIds[i] = i;
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

void Batch::AddToRender(uint index)
{
    assert(index < objectHandles.size());

    uint objIndex = objectHandles[index];

    auto itMesh = meshes.find(objects[objIndex]->GetMeshRes()->GetUID());
    assert(itMesh != meshes.end());

    const MeshData& meshData = itMesh->second;

    DrawCommand command = { meshData.indexCount, 1, meshData.baseIndex, meshData.baseVertex, objIndex };
    commands.push_back(command);
}

void Batch::DoRender(uint transformsIndex, uint materialsIndex, uint texturesLocation)
{
    if (!commands.empty())
    {
        if(bufferDirty)
        {
            CreateRenderData();
        }

        CreateCommandBuffer();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, transformsIndex, transformSSBO->Id());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, materialsIndex, materialSSBO->Id());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, materialsIndex, materialSSBO->Id());
        commandBuffer->Bind();

        textures.SetUniform(texturesLocation);
        textures.Bind();

        vao->Bind();
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0, int(commands.size()), 0);
        vao->Unbind();

        commands.clear();
    }
}

void Batch::Remove(uint index)
{
    uint objIndex = objectHandles[index];
    ComponentMeshRenderer* mesh = objects[objIndex];
    UID meshId  = mesh->GetMeshRes()->GetUID();
    auto itMesh = meshes.find(meshId);
    assert(itMesh != meshes.end());

    if((--itMesh->second.refCount) == 0)
    {
        meshes.erase(itMesh);
    }

    const ResourceMaterial* material = mesh->GetMaterialRes();
    UID materialId  = material->GetUID();
    auto itMaterial = materials.find(materialId);
    assert(itMaterial != materials.end());

    if((--itMaterial->second.refCount) == 0)
    {
        materials.erase(itMaterial);
        textures.Remove(material);
    }

    for (auto it = objectHandles.begin() + index + 1; it != objectHandles.end(); ++it)
    {
        --(*it);
    }

    objectHandles[index] = firstFreeObject;
    
    firstFreeObject = index;

    objects.erase(objects.begin()+objIndex);

	ClearRenderData();
}
