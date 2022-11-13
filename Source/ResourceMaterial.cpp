#include "Globals.h"
#include "ModuleFileSystem.h"
#include "ModuleResources.h"
#include "ResourceMaterial.h"
#include "ResourceTexture.h"

#include "Application.h"
#include "ModuleHints.h"
#include "ModulePrograms.h"

#include "Assimp/types.h"
#include "Assimp/material.h"

#include "OpenGL.h"

#include "Leaks.h"

#define MATERIAL_VERSION 0.3f

// ---------------------------------------------------------
ResourceMaterial::ResourceMaterial(UID id) : Resource(id, Resource::Type::material)
{
}

// ---------------------------------------------------------
ResourceMaterial::~ResourceMaterial()
{
    for(uint i=0; i< TextureCount; ++i)
    {
        if(textures[i] != 0)
        {
			Resource* res = App->resources->Get(textures[i]);
			if (res)
			{
				res->Release();
			}
            textures[i] = 0;
        }
    }
}

// ---------------------------------------------------------
bool ResourceMaterial::LoadInMemory()
{
	if (GetExportedFile() != nullptr)
    {
        char* buffer = nullptr;

        uint size = App->fs->Load(LIBRARY_MATERIAL_FOLDER, GetExportedFile(), &buffer);

        if (buffer)
        {
            simple::mem_istream<std::true_type> read_stream(buffer, size);

            uint mark = 0;
            float version = 0.0;

            read_stream >> mark;

            assert(mark == MAKEFOURCC('E', 'D', 'U', 'E'));

            read_stream >> version;

            read_stream >> diffuse_color.x >> diffuse_color.y >> diffuse_color.z >> diffuse_color.w;
            read_stream >> specular_color.x >> specular_color.y >> specular_color.z;
            read_stream >> emissive_color.x >> emissive_color.y >> emissive_color.z;

            if(version >= 0.3f)
            {
                read_stream >> emissive_intensity;
            }

            for (uint i = 0; i < TextureCount; ++i)
            {
                read_stream >> textures[i];
            }

            float k_ambient, k_diffuse, k_specular;
            read_stream >> k_ambient >> k_diffuse >> k_specular;

            read_stream >> smoothness;
            read_stream >> normal_strength;
            read_stream >> double_sided;
            read_stream >> alpha_test;

            for (uint i = 0; i < TextureCount; ++i)
            {
                if (textures[i] != 0)
                {
                    ResourceTexture* tex_res = static_cast<ResourceTexture*>(App->resources->Get(textures[i]));

                    if (tex_res)
                    {
                        tex_res->LoadToMemory();
                    }
                }
            }

            if(version >= 0.2f)
            {
                read_stream >> uv_tiling;
                read_stream >> uv_offset;
                read_stream >> scnd_uv_tiling;
                read_stream >> scnd_uv_offset;
            }

            delete[] buffer;

            uboDirty = true;

            return true;
        }
    }

    return false;
}

const Buffer* ResourceMaterial::GetMaterialUBO()
{
    if(uboDirty)
    {
        GenerateUBO();
    }

    return materialUBO.get();
}

void ResourceMaterial::GenerateUBO()
{
    struct MaterialData
    {
        float4   diffuse_color;
        float4   specular_color;
        float4   emissive_color;
        float2   tiling;
        float2   offset;
        float2   secondary_tiling;
        float2   secondary_offset;
        float    smoothness;
        float    normal_strength;
        float    alpha_test;
        uint     mapMask;
    };

    if (material)
    {
        MaterialData materialData;

        materialData.diffuse_color = diffuse_color;
        materialData.specular_color = float4(specular_color, 0.0f);
        materialData.emissive_color = float4(emissive_color, 0.0f);
        materialData.tiling = uv_tiling;
        materialData.offset = uv_offset;
        materialData.secondary_tiling = scnd_uv_tiling;
        materialData.secondary_offset = scnd_uv_offset;
        materialData.smoothness = smoothness;
        materialData.normal_strength = normal_strength;
        materialData.alpha_test = alpha_test;
        materialData.mapMask = GetMapMask();


        materialUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(MaterialData), nullptr));
        materialUBO->SetData(0, sizeof(MaterialData), &materialData);
    }

    uboDirty = false;
}

// ---------------------------------------------------------
void ResourceMaterial::ReleaseFromMemory()
{
    for(uint i=0; i< TextureCount; ++i)
    {
        if(textures[i] != 0)
        {
			Resource* res = App->resources->Get(textures[i]);
			
			if (res)
			{
				res->Release();
			}
			
			textures[i] = 0;
        }
    }
}

// ---------------------------------------------------------
bool ResourceMaterial::Save(std::string& output) const
{
    simple::mem_ostream<std::true_type> write_stream;

    SaveToStream(write_stream);

    const std::vector<char>& data = write_stream.get_internal_vec();

    return App->fs->SaveUnique(output, &data[0], uint(data.size()), LIBRARY_MATERIAL_FOLDER, "material", "edumaterial");
}

// ---------------------------------------------------------
bool ResourceMaterial::Save() 
{
    simple::mem_ostream<std::true_type> write_stream;

    SaveToStream(write_stream);

    const std::vector<char>& data = write_stream.get_internal_vec();

    if(exported_file.length() > 0)
    {
		char full_path[250];

		sprintf_s(full_path, 250, "%s%s", LIBRARY_MATERIAL_FOLDER, exported_file.c_str());

        return App->fs->Save(full_path, &data[0], uint(data.size())) > 0;
    }

	std::string output;

	if (App->fs->SaveUnique(output, &data[0], uint(data.size()), LIBRARY_MATERIAL_FOLDER, "material", "edumaterial"))
	{
        App->fs->SplitFilePath(output.c_str(), nullptr, &exported_file);

		return true;
    }

	return false;
}

// ---------------------------------------------------------
void ResourceMaterial::SaveToStream(simple::mem_ostream<std::true_type>& write_stream) const
{
    write_stream << MAKEFOURCC('E', 'D', 'U','E');
    write_stream << MATERIAL_VERSION;
    write_stream << diffuse_color.x << diffuse_color.y << diffuse_color.z << diffuse_color.w;
    write_stream << specular_color.x << specular_color.y << specular_color.z;
    write_stream << emissive_color.x << emissive_color.y << emissive_color.z;
    write_stream << emissive_intensity;

    for(uint i=0; i< TextureCount; ++i)
    {
        write_stream << textures[i];
    }

    write_stream << 0.0f << 0.0f << 0.0f;

    write_stream << smoothness;
    write_stream << normal_strength;
    write_stream << double_sided;
    write_stream << alpha_test;
    write_stream << uv_tiling;
    write_stream << uv_offset;
    write_stream << scnd_uv_tiling;
    write_stream << scnd_uv_offset;
}

// ---------------------------------------------------------
UID ResourceMaterial::Import(const aiMaterial* material, const char* source_file)
{
    std::string base_path;
    App->fs->SplitFilePath(source_file, &base_path, nullptr, nullptr);

    ResourceMaterial* m = static_cast<ResourceMaterial*>(App->resources->CreateNewResource(Resource::material));

    auto copy_color4 = [](math::float4& out, const aiColor4D& color)
    {
        out.x = color.r;
        out.y = color.g;
        out.z = color.b;
        out.w = color.a;
	};

    auto copy_color3 = [](math::float3& out, const aiColor4D& color)
    {
        out.x = color.r;
        out.y = color.g;
        out.z = color.b;
	};

	aiColor4D color;

	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == aiReturn_SUCCESS)
	{
		copy_color4(m->diffuse_color, color);
	}

    if(material->Get(AI_MATKEY_COLOR_SPECULAR, color) == aiReturn_SUCCESS)
    {
        copy_color3(m->specular_color, color);
    }

    if(material->Get(AI_MATKEY_COLOR_EMISSIVE, color) == aiReturn_SUCCESS)
    {
        copy_color3(m->emissive_color, color);
    }

    material->Get(AI_MATKEY_SHININESS, m->smoothness);

    aiString file;
    aiTextureMapping mapping;
    unsigned uvindex = 0;
    if(material->GetTexture(aiTextureType_DIFFUSE, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureDiffuse] = App->resources->ImportTexture(full_path.C_Str(), true, false);
    }

    if (material->GetTexture(aiTextureType_NORMALS, 0, &file, &mapping, &uvindex) == AI_SUCCESS || 
        material->GetTexture(aiTextureType_HEIGHT, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureNormal] = App->resources->ImportTexture(full_path.C_Str(), true, false);
    }

    if (material->GetTexture(aiTextureType_SPECULAR, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureSpecular] = App->resources->ImportTexture(full_path.C_Str(), true, false);
    }

    if (material->GetTexture(aiTextureType_AMBIENT, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureOcclusion] = App->resources->ImportTexture(full_path.C_Str(), true, false);
    }

    if (material->GetTexture(aiTextureType_EMISSIVE, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureEmissive] = App->resources->ImportTexture(full_path.C_Str(), true, false);
    }

    if (source_file != nullptr) 
    {
        m->file = source_file;
        App->fs->NormalizePath(m->file);
    }

    if(m->Save())
    {
        LOG("Imported successful from aiMaterial [%s] to [%s]", m->GetFile(), m->GetExportedFile());

        App->fs->SplitFilePath(m->file.c_str(), nullptr, &m->user_name, nullptr);

        if (m->user_name.empty())
        {
            m->user_name = m->exported_file;
        }

        size_t pos_dot = m->user_name.find_last_of(".");
        if(pos_dot != std::string::npos)
        {
            m->user_name.erase(m->user_name.begin()+pos_dot, m->user_name.end());
        }

        return m->uid;
    }

    LOG("Importing of BUFFER aiMaterial [%s] FAILED", source_file);

    return 0;
}

void ResourceMaterial::SetTexture(MaterialTexture texture, UID uid)
{
    if(textures[texture] != 0)
    {
		Resource* tex_res = App->resources->Get(textures[texture]);
		if (tex_res)
		{
			tex_res->Release();
		}

        textures[texture] = 0;
    }

    Resource* res = App->resources->Get(uid);

    if (res != nullptr && res->GetType() == Resource::texture && res->LoadToMemory() == true)
    {
        textures[texture] = uid;
    }
}

const ResourceTexture* ResourceMaterial::GetTextureRes(MaterialTexture t) const
{
    return static_cast<const ResourceTexture*>(App->resources->Get(textures[t]));
}

ResourceTexture* ResourceMaterial::GetTextureRes(MaterialTexture t) 
{
    return static_cast<ResourceTexture*>(App->resources->Get(textures[t]));
}

uint ResourceMaterial::GetMapMask() const
{
    uint mapMask = 0;

    if(GetTextureRes(TextureLightmap))
    {
        mapMask = 1 << TextureLightmap;
    }
    else
    {
        for(uint i=0; i< TextureCount; ++i)
        {
            if(i != TextureLightmap)
            {
                if(GetTextureRes(MaterialTexture(i)))
                {
                    mapMask |= 1 << i;
                }
            }
        }
    }

    return mapMask;
}
