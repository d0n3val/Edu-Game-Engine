#include "Globals.h"

#include "Batch.h"

#include "ComponentMeshRenderer.h"

#include "ResourceTexture.h"

#include "OpenGL.h"

Batch::Batch(const HashString& tag, uint _max_vertices, uint _max_objects) 
{
    assert(_max_objects > 0);
    assert(_max_vertices > 0);


    tag_name     = tag;
    max_vertices = _max_vertices;
    max_objects  = _max_objects; // \todo: max materials
}

bool Batch::CanAdd(ComponentMeshRenderer* object) const
{
    if(objects.empty())
    {
        return true;
    }

    const ResourceMesh* mesh         = object->GetMeshRes();
    const ResourceMaterial* material = object->GetMaterialRes();

    assert(!mesh->HasAttrib(ATTRIB_BONES));

    bool ok = objects.size()+1 < max_objects && mesh->GetAttribs() == attrib_flags && 
             num_vertices+mesh->GetNumVertices() < max_vertices;

    for(uint i=0; ok && i< TextureCount; ++i)
    {
        const ResourceTexture* texture = material->GetTextureRes(MaterialTexture(i));

        ok = (texture == nullptr && texture_size[i][0] == 0) || 
             (texture->GetWidth() == texture_size[i][0] && 
              texture->GetHeight() == texture_size[i][1]);
    }

    return ok;
}

uint Batch::Add(ComponentMeshRenderer* object)
{
    assert(CanAdd(object));

    if(objects.size() == 0)
    {
        Init(object);
    }

    uint index = objects.size();

    objects.push_back(object);

    const ResourceMaterial* material = object->GetMaterialRes();
    auto it = std::find(unique_materials.begin(), unique_materials.end(), material);

    if(it == unique_materials.end())
    {
        unique_materials.push_back(material);

        for(uint i=0; i<TextureCount; ++i)
            textures[i].reset();
    }

    vao.reset();
    vbo.reset();
    ibo.reset();

    num_vertices += object->GetMeshRes()->GetNumVertices();

    return index;
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

    if((attrib_flags & (1 << ATTRIB_TEX_COORDS_0)))
    {
        attribs[attrib_count++] = {2, 3, GL_FLOAT, GL_FALSE, 0, vertex_size} ;

        vertex_size += sizeof(float3);
    }

    if((attrib_flags & (1 << ATTRIB_NORMALS)))
    {
        attribs[attrib_count++] = {1, 3, GL_FLOAT, GL_TRUE, 0, vertex_size};

        vertex_size += sizeof(float3);
    }

    if((attrib_flags & (1 << ATTRIB_TANGENTS)))
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
    if(objects.size() > 0)
    {
        if(!vbo || !ibo || !vao)
        {
            CreateBuffers();
        }

        if(!textures)
        {
            CreateTextureArray();
        }
   }
}

void Batch::CreateBuffers()
{
    vbo.reset(Buffer::CreateVBO(GL_STATIC_DRAW, vertex_size*num_vertices, nullptr));

    uint8_t* data = (uint8_t*)vbo->Map(GL_WRITE_ONLY);

    uint num_indices = 0;

    // \todo: Convert to world coordinates

    for(ComponentMeshRenderer* object : objects)
    {
        const ResourceMesh* mesh         = object->GetMeshRes();
        const ResourceMaterial* material = object->GetMaterialRes();

        for(uint i=0; i < mesh->GetNumVertices(); ++i)
        {
            (float3&)(data[i*vertex_size]) = mesh->src_vertices[i];
        }

        uint attrib_index = 0;

        if((attrib_flags & ATTRIB_TEX_COORDS_0))
        {
            uint offset = attribs[++attrib_index].offset;

            uint material_index = std::find(unique_materials.begin(), unique_materials.end(), material)-unique_materials.begin();
            assert(material_index < unique_materials.size());

            for(uint i=0; i < mesh->GetNumVertices(); ++i)
            {
                (float3&)(data[i*vertex_size+offset]) = float3(mesh->src_texcoord0[i].x, mesh->src_texcoord0[i].y, float(material_index));
            }
        }

        if((attrib_flags & ATTRIB_NORMALS))
        {
            uint offset = attribs[++attrib_index].offset;

            for(uint i=0; i < mesh->GetNumVertices(); ++i)
            {
                (float3&)(data[i*vertex_size+offset]) = mesh->src_normals[i];
            }
        }

        if((attrib_flags & ATTRIB_TANGENTS))
        {
            uint offset = attribs[++attrib_index].offset;

            for(uint i=0; i < mesh->GetNumVertices(); ++i)
            {
                (float3&)(data[i*vertex_size+offset]) = mesh->src_tangents[i];
            }
        }

        data += mesh->GetNumVertices()*vertex_size;

        num_indices += mesh->GetNumIndices();
    }

    vbo->Unmap();

    ibo.reset(Buffer::CreateIBO(GL_DYNAMIC_DRAW, sizeof(unsigned)*num_indices, nullptr)); 
    vao = std::make_unique<VertexArray>(vbo.get(), ibo.get(), attribs, attrib_count);
}

void Batch::CreateTextureArray()
{
    const ResourceMaterial* front_material = objects.front()->GetMaterialRes();

    for(uint i=0; i< TextureCount; ++i)
    {
        const ResourceTexture* front_texture = front_material->GetTextureRes(MaterialTexture(i));

        textures[i].reset(Texture2DArray::CreateDefaultRGBA(1, front_texture->GetWidth(), front_texture->GetHeight(), objects.size()));

        uint buffer_size = front_texture->GetWidth()*front_texture->GetHeight()*sizeof(unsigned);
        uint8_t* buffer  = (uint8_t*)malloc(buffer_size);

        for(uint j=0; j< unique_materials.size(); ++j)
        {
            const ResourceMaterial* material = unique_materials[j];

            Texture2D* texture = material->GetTextureRes(MaterialTexture(i))->GetTexture();

            // \todo: should not be loaded. If loaded(in another batch) glGetCompressedTexImage
            // \todo: compression!!!! glGetCompressedTexImage
            // \todo: load directly as a texture array
            // \todo: load vb, ib directly as a batch
            // \todo: all maps in the same texture array if they have the same size
            glGetTextureImage(texture->Id(), 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer_size, buffer);

            textures[i]->SetDefaultRGBASubData(0, j, buffer);
        }

        free(buffer);
    }
}

void Batch::AddToRender(uint index)
{
    if(num_render_objects == 0)
    {
        CreateRenderData();
    }

    assert(index < objects.size());
    assert(objects[index] != nullptr);

    ComponentMeshRenderer* object = objects[index];
    const ResourceMesh* mesh      = object->GetMeshRes();
    uint num_indices              = mesh->GetNumIndices();

    if(num_render_objects == render_objects.size() || render_objects[num_render_objects] != index)
    {
        render_objects.erase(render_objects.begin()+num_render_objects, render_objects.end());

        render_objects.push_back(index);

        memcpy(ibo->MapRange(GL_MAP_WRITE_BIT, num_render_indices*sizeof(unsigned), num_indices*sizeof(unsigned)), mesh->src_indices.get(), num_indices*sizeof(unsigned));
        ibo->Unmap();
    }

    ++num_render_objects;
    num_render_indices += num_indices;
}

void Batch::DoRender()
{
    if(num_render_objects > 0)
    {
        vao->Bind();

        // uniform buffer objects for material binding + texture arrays
        // \todo: material binding
        // test with color
        
		glDrawElements(GL_TRIANGLES, num_render_indices, GL_UNSIGNED_INT, nullptr);

		vao->Unbind();

        num_render_indices = 0;
        num_render_objects = 0;
    }
}

void Batch::Remove(uint index)
{
    objects[index] = nullptr;

    while(!objects.empty() && objects.back() == nullptr) 
    {
        objects.pop_back();
    }
}

