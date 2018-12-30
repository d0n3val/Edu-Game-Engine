#include "Globals.h"
#include "ModuleFileSystem.h"
#include "ModuleResources.h"
#include "ResourceMaterial.h"
#include "ResourceTexture.h"
#include "Application.h"
#include "Assimp/include/types.h"
#include "Assimp/include/material.h"

#include "mmgr/mmgr.h"

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

        simple::mem_istream<std::true_type> read_stream(buffer, size);

		read_stream >> diffuse_color.x >> diffuse_color.y >> diffuse_color.z >> diffuse_color.w;
		read_stream >> specular_color.x >> specular_color.y >> specular_color.z;
		read_stream >> emissive_color.x >> emissive_color.y >> emissive_color.z;

        for(uint i=0; i< TextureCount; ++i)
        {
            read_stream >> textures[i];
        }

		read_stream >> k_ambient >> k_diffuse >> k_specular;

        read_stream >> shininess;

        for(uint i=0; i< TextureCount; ++i)
        {
			if (textures[i] != 0)
			{
				Resource* tex_res = App->resources->Get(textures[i]);
				if (tex_res)
				{
					tex_res->LoadToMemory();
				}
			}
        }

		delete[] buffer;

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
bool ResourceMaterial::Save(std::string& output) 
{
    simple::mem_ostream<std::true_type> write_stream;

    SaveToStream(write_stream);

    const std::vector<char>& data = write_stream.get_internal_vec();

    return App->fs->SaveUnique(output, &data[0], data.size(), LIBRARY_MATERIAL_FOLDER, "material", "edumaterial");
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

        return App->fs->Save(full_path, &data[0], data.size()) > 0;
    }

	std::string output;

	if (App->fs->SaveUnique(output, &data[0], data.size(), LIBRARY_MATERIAL_FOLDER, "material", "edumaterial"))
	{
        App->fs->SplitFilePath(output.c_str(), nullptr, &exported_file);

		return true;
    }

	return false;
}

// ---------------------------------------------------------
void ResourceMaterial::SaveToStream(simple::mem_ostream<std::true_type>& write_stream) const
{
    write_stream << diffuse_color.x << diffuse_color.y << diffuse_color.z << diffuse_color.w;
    write_stream << specular_color.x << specular_color.y << specular_color.z;
    write_stream << emissive_color.x << emissive_color.y << emissive_color.z;

    for(uint i=0; i< TextureCount; ++i)
    {
        write_stream << textures[i];
    }

    write_stream << k_ambient << k_diffuse << k_specular;

    write_stream << shininess;

}

// ---------------------------------------------------------
UID ResourceMaterial::Import(const aiMaterial* material, const char* source_file)
{
    std::string base_path;
    App->fs->SplitFilePath(source_file, &base_path, nullptr, nullptr);

    ResourceMaterial* m = static_cast<ResourceMaterial*>(App->resources->CreateNewResource(Resource::material));

    material->Get(AI_MATKEY_COLOR_DIFFUSE, m->diffuse_color);
    material->Get(AI_MATKEY_COLOR_SPECULAR, m->specular_color);
    material->Get(AI_MATKEY_COLOR_EMISSIVE, m->emissive_color);
    material->Get(AI_MATKEY_SHININESS, m->shininess);

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

    if (material->GetTexture(aiTextureType_AMBIENT, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureOcclusion] = App->resources->ImportFile(full_path.C_Str(), true);
    }

    if (material->GetTexture(aiTextureType_EMISSIVE, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureEmissive] = App->resources->ImportFile(full_path.C_Str(), true);
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

void ResourceMaterial::SetTexture(Texture texture, UID uid)
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

const ResourceTexture* ResourceMaterial::GetTextureRes(Texture t) const
{
    return static_cast<const ResourceTexture*>(App->resources->Get(textures[t]));
}

ResourceTexture* ResourceMaterial::GetTextureRes(Texture t) 
{
    return static_cast<ResourceTexture*>(App->resources->Get(textures[t]));
}

