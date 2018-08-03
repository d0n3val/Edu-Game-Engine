#include "ResourceMaterial.h"
#include "Application.h"

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

        uint size = App->fs->Load(LIBRARY_MESH_FOLDER, GetExportedFile(), &buffer);

        std::stringbuf strbuf;
        buffer.pubsetbuf(buffer, size);

        std::istream read_stream(&strbuf);

        read_stream >> ambient;
        read_stream >> diffuse;
        read_stream >> specular;
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

bool ResourceMaterial::Save(string& output) const
{
    char buffer[sizeof(ResourceMaterial)];

    std::stringbuf strbuf;
    buffer.pubsetbuf(buffer, sizeof(ResourceMaterial));
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

	return App->fs->SaveUnique(output, buffer, write_stream.tellp(), LIBRARY_MATERIAL_FOLDER, "material", "edumaterial");
}

bool ResourceMaterial::Import(const aiMaterial* material, const char* base_path, std::string& output)
{
    ResourceMaterial m;

    float shine_strength = 1.0f;

    material->Get(AI_MATKEY_COLOR_AMBIENT, m.ambient);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, m.diffuse);
    material->Get(AI_MATKEY_COLOR_SPECULAR, m.specular);
    material->Get(AI_MATKEY_SHININESS, m.shininess);
    material->Get(AI_MATKEY_SHININESS_STRENGTH, m.shine_strength);

    specular *= shine_strength;

    aiString file;
    aiTextureMapping mapping;
    unsigned uvindex = 0;
    if(material->GetTexture(aiTextureType_DIFFUSE, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path = base_path;
        full_path.Append(file.data);

        m.albedo_map = App->resources->ImportFile(full_path.C_Str(), true);
    }

    if (material->GetTexture(aiTextureType_NORMALS, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path = base_path;
        full_path.Append(file.data);

        m.normal_map = App->resources->ImportFile(full_path.C_Str(), true);
    }

    if (material->GetTexture(aiTextureType_SPECULAR, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path = base_path;
        full_path.Append(file.data);

        m.specular_map = App->resources->ImportFile(full_path.C_Str(), true);
    }

    return Save(m, output);
}

