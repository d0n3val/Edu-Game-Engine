#include "Globals.h"
#include "ModuleFileSystem.h"
#include "ModuleResources.h"
#include "ResourceMaterial.h"
#include "Application.h"
#include "Assimp/include/types.h"
#include "Assimp/include/material.h"

#include <iostream>

ResourceMaterial::ResourceMaterial(UID id) : Resource(id, Resource::Type::material)
{
}

ResourceMaterial::~ResourceMaterial()
{
}

bool ResourceMaterial::LoadInMemory()
{
	if (GetExportedFile() != nullptr)
    {
        char* buffer = nullptr;

        uint size = App->fs->Load(LIBRARY_MATERIAL_FOLDER, GetExportedFile(), &buffer);

        std::stringbuf strbuf;
        strbuf.pubsetbuf(buffer, size);

        std::istream read_stream(&strbuf);

        read_stream >> ambient.x >> ambient.y >> ambient.z >> ambient.w;
        read_stream >> diffuse.x >> diffuse.y >> diffuse.z >> diffuse.w;
        read_stream >> specular.x >> specular.y >> specular.z >> specular.w;
        read_stream >> shininess;
        read_stream >> albedo_map;
        read_stream >> normal_map;
        read_stream >> specular_map;
        read_stream >> cast_shadows;
        read_stream >> recv_shadows;

        return true;
    }

    return false;
}

bool ResourceMaterial::Save(std::string& output) const
{
    char buffer[sizeof(ResourceMaterial)];

    std::stringbuf strbuf;
    strbuf.pubsetbuf(buffer, sizeof(ResourceMaterial));
    std::ostream write_stream(&strbuf);

    write_stream << ambient;
    write_stream << diffuse;
    write_stream << specular;
    write_stream << shininess;
    write_stream << albedo_map;
    write_stream << normal_map;
    write_stream << specular_map;
    write_stream << cast_shadows;
    write_stream << recv_shadows;

	return App->fs->SaveUnique(output, buffer, (uint)write_stream.tellp(), LIBRARY_MATERIAL_FOLDER, "material", "edumaterial");
}

UID ResourceMaterial::Import(const aiMaterial* material, const char* source_file)
{
    std::string base_path;
    App->fs->SplitFilePath(source_file, &base_path, nullptr, nullptr);

    ResourceMaterial* m = static_cast<ResourceMaterial*>(App->resources->CreateNewResource(Resource::material));

    float shine_strength = 1.0f;

    material->Get(AI_MATKEY_COLOR_AMBIENT, m->ambient);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, m->diffuse);
    material->Get(AI_MATKEY_COLOR_SPECULAR, m->specular);
    material->Get(AI_MATKEY_SHININESS, m->shininess);
    material->Get(AI_MATKEY_SHININESS_STRENGTH, shine_strength);

    m->specular *= shine_strength;

    aiString file;
    aiTextureMapping mapping;
    unsigned uvindex = 0;
    if(material->GetTexture(aiTextureType_DIFFUSE, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->albedo_map = App->resources->ImportFile(full_path.C_Str(), true);
    }

    if (material->GetTexture(aiTextureType_NORMALS, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->normal_map = App->resources->ImportFile(full_path.C_Str(), true);
    }

    if (material->GetTexture(aiTextureType_SPECULAR, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->specular_map = App->resources->ImportFile(full_path.C_Str(), true);
    }

    std::string output;

    if(m->Save(output))
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

    // \todo: remove resource

    LOG("Importing of BUFFER aiMaterial [%s] FAILED", source_file);

    return 0;
}

