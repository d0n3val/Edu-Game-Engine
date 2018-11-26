#include "Globals.h"
#include "ModuleFileSystem.h"
#include "ModuleResources.h"
#include "ResourceMaterial.h"
#include "ResourceTexture.h"
#include "Application.h"
#include "Assimp/include/types.h"
#include "Assimp/include/material.h"
#include "utils/SimpleBinStream.h"

// ---------------------------------------------------------
ResourceMaterial::ResourceMaterial(UID id) : Resource(id, Resource::Type::material)
{
}

// ---------------------------------------------------------
ResourceMaterial::~ResourceMaterial()
{
    // \todo: Unload resources + unload texture map resources
}

// ---------------------------------------------------------
bool ResourceMaterial::LoadInMemory()
{
	if (GetExportedFile() != nullptr)
    {
        char* buffer = nullptr;

        uint size = App->fs->Load(LIBRARY_MATERIAL_FOLDER, GetExportedFile(), &buffer);

        simple::mem_istream<std::true_type> read_stream(buffer, size);

        for(uint i=0; i< ColorCount; ++i)
        {
            read_stream >> colors[i].x >> colors[i].y >> colors[i].z >> colors[i].w;
        }

        for(uint i=0; i< TextureCount; ++i)
        {
            read_stream >> textures[i];
        }

        read_stream >> shininess;

        for(uint i=0; i< TextureCount; ++i)
        {
            if(textures[i] != 0)
            {
                App->resources->Get(textures[i])->LoadToMemory();
            }
        }

        return true;
    }

    return false;
}

// ---------------------------------------------------------
void ResourceMaterial::ReleaseFromMemory()
{
    for(uint i=0; i< TextureCount; ++i)
    {
        if(textures[i] != 0)
        {
            App->resources->Get(textures[i])->Release();
            textures[i] = 0;
        }
    }
}

// ---------------------------------------------------------
bool ResourceMaterial::Save(std::string& output) const
{
    simple::mem_ostream<std::true_type> write_stream;

    for(uint i=0; i< ColorCount; ++i)
    {
        write_stream << colors[i];
    }

    for(uint i=0; i< TextureCount; ++i)
    {
        write_stream << textures[i];
    }

    write_stream << shininess;

    const std::vector<char>& data = write_stream.get_internal_vec();

    return App->fs->SaveUnique(output, &data[0], data.size(), LIBRARY_MATERIAL_FOLDER, "material", "edumaterial");
}

// ---------------------------------------------------------
UID ResourceMaterial::Import(const aiMaterial* material, const char* source_file)
{
    std::string base_path;
    App->fs->SplitFilePath(source_file, &base_path, nullptr, nullptr);

    ResourceMaterial* m = static_cast<ResourceMaterial*>(App->resources->CreateNewResource(Resource::material));

    float shine_strength = 1.0f;

    material->Get(AI_MATKEY_COLOR_AMBIENT, m->colors[ColorAmbient]);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, m->colors[ColorDiffuse]);
    material->Get(AI_MATKEY_COLOR_SPECULAR, m->colors[ColorSpecular]);
    material->Get(AI_MATKEY_SHININESS, m->shininess);
    material->Get(AI_MATKEY_SHININESS_STRENGTH, shine_strength);

    m->colors[ColorSpecular] *= shine_strength;

    aiString file;
    aiTextureMapping mapping;
    unsigned uvindex = 0;
    if(material->GetTexture(aiTextureType_DIFFUSE, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureDiffuse] = App->resources->ImportFile(full_path.C_Str(), true);
    }

    if (material->GetTexture(aiTextureType_NORMALS, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureNormal] = App->resources->ImportFile(full_path.C_Str(), true);
    }

    if (material->GetTexture(aiTextureType_SPECULAR, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureSpecular] = App->resources->ImportFile(full_path.C_Str(), true);
    }

    std::string output;

    bool save_ok = m->Save(output);
    m->ReleaseFromMemory();

    if(save_ok)
    {
		if (source_file != nullptr) 
        {
			m->file = source_file;
			App->fs->NormalizePath(m->file);
		}

		std::string file;
		App->fs->SplitFilePath(output.c_str(), nullptr, &file);
		m->exported_file = file;

		LOG("Imported successful from aiMaterial [%s] to [%s]", m->GetFile(), m->GetExportedFile());

        return m->uid;
    }

    LOG("Importing of BUFFER aiMaterial [%s] FAILED", source_file);

    return 0;
}

void ResourceMaterial::SetTexture(Texture texture, UID uid)
{
    if(textures[texture] != 0)
    {
        App->resources->Get(textures[texture])->Release();
        textures[texture] = 0;
    }

    Resource* res = App->resources->Get(uid);

    if (res != nullptr && res->GetType() == Resource::texture && res->LoadToMemory() == true)
    {
        textures[texture] = uid;
    }
}

const ResourceTexture* ResourceMaterial::GetTextureRes(Texture t) const
{
    return static_cast<const ResourceTexture*>(App->resources->Get(textures[t]));
}

