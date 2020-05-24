#include "Globals.h"

#include "Batch.h"

#include "Application.h"
#include "ComponentMeshRenderer.h"
#include "ModuleTextures.h"
#include "GameObject.h"

#include "ResourceTexture.h"

#include "OpenGL.h"

#include <algorithm>

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

	bool ok = objects.size()+1 < max_objects && mesh->GetAttribs() == attrib_flags && num_vertices+mesh->GetNumVertices() < max_vertices;

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

    ObjectData data = { object, index == 0 ? 0 : objects[index-1].vertex_offset+objects[index-1].renderer->GetMeshRes()->GetNumVertices() };
    objects.push_back(data);

    const ResourceMaterial* material = object->GetMaterialRes();

    auto it = std::find_if(unique_materials.begin(), unique_materials.end(), [material](const MaterialData& data) -> bool { return data.material == material; });

	if (it == unique_materials.end())
	{
		unique_materials.push_back({ material, 1 });
	}
	else
	{
		++it->ref_count;
	}

    num_vertices += object->GetMeshRes()->GetNumVertices();

    ClearRenderData();

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

    if(mesh->HasAttrib(ATTRIB_TEX_COORDS_0))
    {
        attribs[attrib_count++] = {2, 3, GL_FLOAT, GL_FALSE, 0, vertex_size} ;

        vertex_size += sizeof(float3);
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
    if(objects.size() > 0)
    {
        if(!vbo || !ibo || !vao)
        {
            CreateBuffers();
        }

		bool generate = true;
		for (uint i = 0; generate && i < TextureCount; ++i)
			generate = textures[TextureDiffuse].get() == nullptr;

        if(generate)
        {
            CreateTextureArray();
        }
   }
}

void Batch::ClearRenderData()
{
	// \todo: only if materials changed ???
	for (uint i = 0; i < TextureCount; ++i)
	{
		textures[i].reset();
	}

    vao.reset();
    vbo.reset();
    ibo.reset();

    render_objects.clear();
}

void Batch::CreateBuffers()
{
    vbo.reset(Buffer::CreateVBO(GL_STATIC_DRAW, vertex_size*num_vertices, nullptr));

    uint8_t* data = (uint8_t*)vbo->Map(GL_WRITE_ONLY);

    uint num_indices = 0;

    // \todo: Convert to world coordinates

    for(const ObjectData& object_data : objects)
    {
		if (object_data.renderer == nullptr)
			continue;

        const ResourceMesh* mesh         = object_data.renderer->GetMeshRes();
        const ResourceMaterial* material = object_data.renderer->GetMaterialRes();
        float4x4 model                   = object_data.renderer->GetGameObject()->GetGlobalTransformation();;

        for(uint i=0; i < mesh->GetNumVertices(); ++i)
        {
            (float3&)(data[i*vertex_size]) = model.TransformPos(mesh->src_vertices[i]);
        }

        uint attrib_index = 0;

        if(mesh->HasAttrib(ATTRIB_TEX_COORDS_0))
        {
            uint offset = attribs[++attrib_index].offset;

			uint material_index = std::find_if(unique_materials.begin(), unique_materials.end(), 
                    [material](const MaterialData& data) -> bool { return data.material == material; }) - unique_materials.begin();

            assert(material_index < unique_materials.size());

            for(uint i=0; i < mesh->GetNumVertices(); ++i)
            {
                (float3&)(data[i*vertex_size+offset]) = float3(mesh->src_texcoord0[i].x, mesh->src_texcoord0[i].y, float(material_index));
            }
        }

        if(mesh->HasAttrib(ATTRIB_NORMALS))
        {
            uint offset = attribs[++attrib_index].offset;

            for(uint i=0; i < mesh->GetNumVertices(); ++i)
            {
                (float3&)(data[i*vertex_size+offset]) = model.TransformDir(mesh->src_normals[i]);
            }
        }

        if(mesh->HasAttrib(ATTRIB_TANGENTS))
        {
            uint offset = attribs[++attrib_index].offset;

            for(uint i=0; i < mesh->GetNumVertices(); ++i)
            {
                (float3&)(data[i*vertex_size+offset]) = model.TransformDir(mesh->src_tangents[i]);
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
	const ResourceMaterial* front_material = unique_materials.front().material;

    for(uint i=0; i< TextureCount; ++i)
    {
		if (texture_size[i][0] > 0)
		{
			const ResourceTexture* front_texture = front_material->GetTextureRes(MaterialTexture(i));

			textures[i].reset(Texture2DArray::CreateDefaultRGBA(1, front_texture->GetWidth(), front_texture->GetHeight(), objects.size(), true));

			uint buffer_size = front_texture->GetWidth()*front_texture->GetHeight() * sizeof(unsigned);

			for (uint j = 0; j < unique_materials.size(); ++j)
			{
				const ResourceTexture* texture = unique_materials[j].material->GetTextureRes(MaterialTexture(i));

				// \todo: should not be loaded. If loaded(in another batch) glGetCompressedTexImage
				// \todo: compression!!!! glGetCompressedTexImage
				// \todo: load directly as a texture array
				// \todo: load vb, ib directly as a batch
				// \todo: all maps in the same texture array if they have the same size
				// \todo: doesn´t loads compressed ==> refactor soil to do it internally
				// \todo: doesn´t takes into account different channels/formats

				App->tex->LoadToArray(texture, textures[i].get(), j);
			}
		}
    }
}

void Batch::AddToRender(uint index)
{
    if(num_render_objects == 0)
    {
        CreateRenderData();
    }

    assert(index < objects.size());
    assert(objects[index].renderer != nullptr);

    ComponentMeshRenderer* object = objects[index].renderer;
    const ResourceMesh* mesh      = object->GetMeshRes();
    uint num_indices              = mesh->GetNumIndices();

    if(num_render_objects == render_objects.size() || render_objects[num_render_objects] != index)
    {
        render_objects.erase(render_objects.begin()+num_render_objects, render_objects.end());
		render_objects.push_back(index);

		unsigned* indexes = (unsigned*)ibo->MapRange(GL_MAP_WRITE_BIT, num_render_indices * sizeof(unsigned), num_indices * sizeof(unsigned));

		for (uint i = 0; i < num_indices; ++i)
		{
			indexes[i] = mesh->src_indices[i] + objects[index].vertex_offset;
		}

        
        ibo->Unmap();
    }

    ++num_render_objects;
    num_render_indices += num_indices;
}

void Batch::DoRender()
{
    if(num_render_objects > 0)
    {
        if(textures[TextureDiffuse])
        {
            textures[TextureDiffuse]->Bind(0, 0);
        }

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
	const ResourceMaterial* material = objects[index].renderer->GetMaterialRes();
	const ResourceMesh* mesh = objects[index].renderer->GetMeshRes();

	auto it = std::find_if(unique_materials.begin(), unique_materials.end(), [material](const MaterialData& data) -> bool { return data.material == material; });
	assert(it != unique_materials.end());

	if ((--it->ref_count) == 0)
	{
		unique_materials.erase(it);
	}

	for (uint i = index + 1; i < objects.size(); ++i)
	{
		objects[i].vertex_offset -= mesh->GetNumVertices();
	}

	objects[index].renderer = nullptr;
    objects[index].vertex_offset = 0;

    while(!objects.empty() && objects.back().renderer == nullptr) 
    {
        objects.pop_back();
    }

	ClearRenderData();
}

