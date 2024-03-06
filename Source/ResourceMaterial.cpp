#include "Globals.h"

#include "gltf.h"

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

#include<SDL_assert.h>

#include <filesystem>

#define MATERIAL_VERSION 0.6f
//#define FORCE_COMPRESS_ON_LOAD

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// ---------------------------------------------------------
ResourceMaterial::ResourceMaterial(UID id) : Resource(id, Resource::Type::material)
{
}

// ---------------------------------------------------------
ResourceMaterial::~ResourceMaterial()
{
    ReleaseTextures();
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

            SDL_assert(mark == MAKEFOURCC('E', 'D', 'U', 'E'));

            read_stream >> version;

            uint workFlowId;
            read_stream >> workFlowId;

            workFlow = MaterialWorkFlow(workFlowId);

            if(workFlow == SpecularGlossiness)
            {
                SpecularGlossData sgData;
                read_stream >> sgData.diffuse_color.x >> sgData.diffuse_color.y >> sgData.diffuse_color.z >> sgData.diffuse_color.w;
                read_stream >> sgData.specular_color.x >> sgData.specular_color.y >> sgData.specular_color.z;
                read_stream >> sgData.emissive_color.x >> sgData.emissive_color.y >> sgData.emissive_color.z;
                read_stream >> sgData.emissive_intensity;
                read_stream >> sgData.specular_intensity;

                for (uint i = 0; i < SG_TextureCount; ++i)
                {
                    read_stream >> sgData.textures[i];

                    if (sgData.textures[i] != 0)
                    {
                        ResourceTexture *tex_res = static_cast<ResourceTexture *>(App->resources->Get(sgData.textures[i]));

                        if (tex_res)
                        {
                            tex_res->LoadToMemory();
                        }
                    }
                }

                read_stream >> sgData.smoothness;
                read_stream >> sgData.normal_strength;
                read_stream >> sgData.occlusion_strength;

                data = sgData;
            }
            else
            {
                SDL_assert(workFlow == MetallicRoughness);
                MetallicRoughData mrData;

                read_stream >> mrData.baseColor.x >> mrData.baseColor.y >> mrData.baseColor.z >> mrData.baseColor.w;
                read_stream >> mrData.metalness;
                read_stream >> mrData.roughness;
                read_stream >> mrData.emissive_color.x >> mrData.emissive_color.y, mrData.emissive_color.z ;
                read_stream >> mrData.emissive_intensity;

                for (uint i = 0; i < MR_TextureCount; ++i)
                {
                    read_stream >> mrData.textures[i];

                    if (mrData.textures[i] != 0)
                    {
                        ResourceTexture *tex_res = static_cast<ResourceTexture *>(App->resources->Get(mrData.textures[i]));

                        if (tex_res)
                        {
                            tex_res->LoadToMemory();
                        }
                    }
                }

                read_stream >> mrData.normal_strength;
                read_stream >> mrData.occlusion_strength;

                data = mrData;
            }

            read_stream >> double_sided;
            read_stream >> alpha_test;

            read_stream >> uv_tiling;
            read_stream >> uv_offset;
            read_stream >> scnd_uv_tiling;
            read_stream >> scnd_uv_offset;

            read_stream >> planarReflections;

            delete[] buffer;

            uboDirty = true;

            return true;
        }
    }

    return false;
}

/*
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
        materialData.mapMask = GetMask();


        materialUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(MaterialData), nullptr));
        materialUBO->SetData(0, sizeof(MaterialData), &materialData);
    }

    uboDirty = false;
}
*/

void ResourceMaterial::ReleaseTextures(const SpecularGlossData& data)
{
    for (uint i = 0; i < SG_TextureCount; ++i)
    {
        if (data.textures[i] != 0)
        {
            Resource* res = App->resources->Get(data.textures[i]);
            if (res)
            {
                res->Release();
            }
        }
    }
}

void ResourceMaterial::ReleaseTextures(const MetallicRoughData& data)
{
    for (uint i = 0; i < MR_TextureCount; ++i)
    {
        if (data.textures[i] != 0)
        {
            Resource* res = App->resources->Get(data.textures[i]);
            if (res)
            {
                res->Release();
            }
        }
    }
}

void ResourceMaterial::LoadTextures(const SpecularGlossData& data)
{
    for (uint i = 0; i < SG_TextureCount; ++i)
    {
        if (data.textures[i] != 0)
        {
            Resource* res = App->resources->Get(data.textures[i]);
            if (res)
            {
                res->LoadToMemory();
            }
        }
    }
}

void ResourceMaterial::LoadTextures(const MetallicRoughData& data)
{
    for (uint i = 0; i < MR_TextureCount; ++i)
    {
        if (data.textures[i] != 0)
        {
            Resource* res = App->resources->Get(data.textures[i]);
            if (res)
            {
                res->LoadToMemory();
            }
        }
    }

}

// ---------------------------------------------------------
void ResourceMaterial::ReleaseTextures()
{
    std::visit([&](auto& data) 
        { 
            ReleaseTextures(data); 
            memset(&data.textures[0], 0, sizeof(data.textures));  
        }, data);
}

// ---------------------------------------------------------
void ResourceMaterial::ReleaseFromMemory()
{
    ReleaseTextures();
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

    write_stream << uint(workFlow);

    if (workFlow == SpecularGlossiness)
    {
        const SpecularGlossData& sgData = std::get<SpecularGlossData>(data);

        write_stream << sgData.diffuse_color.x << sgData.diffuse_color.y << sgData.diffuse_color.z << sgData.diffuse_color.w;
        write_stream << sgData.specular_color.x << sgData.specular_color.y << sgData.specular_color.z;
        write_stream << sgData.emissive_color.x << sgData.emissive_color.y << sgData.emissive_color.z;
        write_stream << sgData.emissive_intensity;
        write_stream << sgData.specular_intensity;

        for (uint i = 0; i < SG_TextureCount; ++i) write_stream << sgData.textures[i];

        write_stream << sgData.smoothness;
        write_stream << sgData.normal_strength;
        write_stream << sgData.occlusion_strength;
    }
    else
    {
        SDL_assert(workFlow == MetallicRoughness);
        const MetallicRoughData& mrData = std::get<MetallicRoughData>(data);

        write_stream << mrData.baseColor.x << mrData.baseColor.y << mrData.baseColor.z << mrData.baseColor.w;
        write_stream << mrData.metalness;
        write_stream << mrData.roughness;
        write_stream << mrData.emissive_color.x << mrData.emissive_color.y, mrData.emissive_color.z;
        write_stream << mrData.emissive_intensity;

        for (uint i = 0; i < MR_TextureCount; ++i) write_stream << mrData.textures[i];

        write_stream << mrData.normal_strength;
        write_stream << mrData.occlusion_strength;
    }

    write_stream << double_sided;
    write_stream << alpha_test;

    write_stream << uv_tiling;
    write_stream << uv_offset;
    write_stream << scnd_uv_tiling;
    write_stream << scnd_uv_offset;

    write_stream << planarReflections;
}

// ---------------------------------------------------------
UID ResourceMaterial::Import(const tinygltf::Model& model, const tinygltf::Material& material, const char* source_file)
{
    std::string base_path;
    App->fs->SplitFilePath(source_file, &base_path, nullptr, nullptr);

    // TODO: name = material.name;
    ResourceMaterial* m = static_cast<ResourceMaterial*>(App->resources->CreateNewResource(Resource::material));

    auto it = material.extensions.find("KHR_materials_pbrSpecularGlossiness");
    if (it != material.extensions.end())
    {
        SpecularGlossData sgData;
        ImportSpecularGlossiness(sgData, base_path, model, material, it->second);

        m->workFlow = SpecularGlossiness;
        m->data = sgData;
    }
    else
    {
        MetallicRoughData mrData;
        ImportMetallicRoughness(mrData, base_path, model, material);

        m->workFlow = MetallicRoughness;
        m->data = mrData;
    }

    if (source_file != nullptr)
    {
        m->file = source_file;
        App->fs->NormalizePath(m->file);
    }

    if (m->Save())
    {
        LOG("Imported successful from aiMaterial [%s] to [%s]", m->GetFile(), m->GetExportedFile());

        App->fs->SplitFilePath(m->file.c_str(), nullptr, &m->user_name, nullptr);

        if (m->user_name.empty())
        {
            m->user_name = m->exported_file;
        }

        size_t pos_dot = m->user_name.find_last_of(".");
        if (pos_dot != std::string::npos)
        {
            m->user_name.erase(m->user_name.begin() + pos_dot, m->user_name.end());
        }

        return m->uid;
    }

    LOG("Importing of BUFFER aiMaterial [%s] FAILED", source_file);

    return 0;
}

void ResourceMaterial::ImportSpecularGlossiness(SpecularGlossData& data, const std::string &base_path, const tinygltf::Model &model, const tinygltf::Material& material, const tinygltf::Value &materialMap)
{
    auto importColor4 = [](float4 &color, const tinygltf::Value &material, const std::string &name)
    {
        const tinygltf::Value &matColor = material.Get(name);
        if (matColor.IsArray() && matColor.ArrayLen() == 4)
        {
            color = float4(float(matColor.Get(0).GetNumberAsDouble()), float(matColor.Get(1).GetNumberAsDouble()),
                           float(matColor.Get(2).GetNumberAsDouble()), float(matColor.Get(3).GetNumberAsDouble()));
        }
    };

    auto importColor3 = [](float3 &color, const tinygltf::Value &material, const std::string &name)
    {
        const tinygltf::Value &matColor = material.Get(name);
        if (matColor.IsArray() && matColor.ArrayLen() == 3)
        {
            color = float3(float(matColor.Get(0).GetNumberAsDouble()), float(matColor.Get(1).GetNumberAsDouble()),
                           float(matColor.Get(2).GetNumberAsDouble()));
        }
    };

    auto importFloat = [](float &value, const tinygltf::Value &material, const std::string &name)
    {
        const tinygltf::Value &matValue = material.Get(name);
        if (matValue.IsNumber())
            value = float(matValue.GetNumberAsDouble());
    };

    auto importTexture = [](UID &texture, std::string base_path, const tinygltf::Model &model, const tinygltf::Value &material, const std::string &name)
    {
        const tinygltf::Value &matTex = material.Get(name);
        if (matTex.IsObject())
        {
            int index = matTex.Get(std::string("index")).GetNumberAsInt();
            texture =ImportTexture(base_path, model, index);
        }
    };

    importColor4(data.diffuse_color, materialMap, "diffuseFactor");
    importColor3(data.specular_color, materialMap, "specularFactor");
    importFloat(data.smoothness, materialMap, "glossinessFactor");
    importTexture(data.textures[SG_TextureDiffuse], base_path, model, materialMap, "diffuseTexture");
    importTexture(data.textures[SG_TextureSpecular], base_path, model, materialMap, "specularGlossinessTexture");

    data.textures[SG_TextureNormal] = ImportTexture(base_path, model, material.normalTexture.index);
    data.normal_strength = float(material.normalTexture.scale);

    data.textures[SG_TextureOcclusion] = ImportTexture(base_path, model, material.occlusionTexture.index);
    data.occlusion_strength = float(material.occlusionTexture.strength);

    data.emissive_color = ImportColor3(material.emissiveFactor);
    data.textures[SG_TextureEmissive] = ImportTexture(base_path, model, material.emissiveTexture.index);
}

void ResourceMaterial::ImportMetallicRoughness(MetallicRoughData &data, std::string &base_path, const tinygltf::Model &model, const tinygltf::Material &material)
{
    data.baseColor = ImportColor4(material.pbrMetallicRoughness.baseColorFactor);
    data.metalness = float(material.pbrMetallicRoughness.metallicFactor);
    data.roughness = float(material.pbrMetallicRoughness.roughnessFactor);
    data.emissive_color = ImportColor3(material.emissiveFactor);
    data.textures[MR_TextureBaseColor] = ImportTexture(base_path, model, material.pbrMetallicRoughness.baseColorTexture.index);
    data.textures[MR_TextureMetallicRough] = ImportTexture(base_path, model, material.pbrMetallicRoughness.metallicRoughnessTexture.index);
    data.textures[MR_TextureNormal] = ImportTexture(base_path, model, material.normalTexture.index);
    data.textures[MR_TextureOcclusion] = ImportTexture(base_path, model, material.occlusionTexture.index);
    data.textures[MR_TextureEmissive] = ImportTexture(base_path, model, material.emissiveTexture.index);

    data.normal_strength = float(material.normalTexture.scale);
    data.occlusion_strength = float(material.occlusionTexture.strength);
}

UID ResourceMaterial::ImportTexture(const std::string &basePath, const tinygltf::Model &model, int textureIdx)
{
    UID ret = 0;
    if(textureIdx >= 0)
    {
        const tinygltf::Texture &texture = model.textures[textureIdx];
        const tinygltf::Image &image = model.images[texture.source];

        std::filesystem::path fullPath = std::filesystem::proximate(basePath + image.uri);
        std::string fullName = fullPath.string();
        std::replace(fullName.begin(), fullName.end(), '\\', '/');
        ret = App->resources->ImportTexture(fullName.c_str(), true, false, false);
    }

    return ret;
}

float3 ResourceMaterial::ImportColor3(const std::vector<double> &matColor)
{
    return float3(float(matColor[0]), float(matColor[1]), float(matColor[2]));
}

float4 ResourceMaterial::ImportColor4(const std::vector<double> &matColor)
{
    return float4(float(matColor[0]), float(matColor[1]), float(matColor[2]), float(matColor[3]));
}

#if 0
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

        m->textures[TextureDiffuse] = App->resources->ImportTexture(full_path.C_Str(), true, true, false);
    }

    if (material->GetTexture(aiTextureType_NORMALS, 0, &file, &mapping, &uvindex) == AI_SUCCESS || 
        material->GetTexture(aiTextureType_HEIGHT, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureNormal] = App->resources->ImportTexture(full_path.C_Str(), true, false, false);
    }

    if (material->GetTexture(aiTextureType_SPECULAR, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureSpecular] = App->resources->ImportTexture(full_path.C_Str(), true, true, false);
    }

    if (material->GetTexture(aiTextureType_AMBIENT, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureOcclusion] = App->resources->ImportTexture(full_path.C_Str(), true, false, false);
    }

    if (material->GetTexture(aiTextureType_EMISSIVE, 0, &file, &mapping, &uvindex) == AI_SUCCESS)
    {
        assert(mapping == aiTextureMapping_UV);
        assert(uvindex == 0);

        aiString full_path(base_path);
        full_path.Append(file.data);

        m->textures[TextureEmissive] = App->resources->ImportTexture(full_path.C_Str(), true, true, false);
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

#endif

uint ResourceMaterial::GetMask() const
{
    uint mask = 0;

    if(workFlow == SpecularGlossiness)
    {
        const SpecularGlossData &sgData = std::get<SpecularGlossData>(data);

        if (sgData.textures[SG_TextureLightmap] != 0)
        {
            mask = 1 << SG_TextureLightmap;
        }
        else
        {
            for (uint i = 0; i < SG_TextureCount; ++i)
            {
                if (i != SG_TextureLightmap)
                {
                    if (sgData.textures[i] != 0)
                    {
                        mask |= 1 << i;
                    }
                }
            }
        }

        if (planarReflections)
        {
            mask |= 1 << SG_TextureCount;
        }
    }
    else
    {
        const MetallicRoughData& mrData = std::get<MetallicRoughData>(data);

        for (uint i = 0; i < MR_TextureCount; ++i)
        {
            if (mrData.textures[i] != 0)
            {
                mask |= 1 << i;
            }
        }

        if (planarReflections)
        {
            mask |= 1 << MR_TextureCount;
        }
    }

    return mask;
}

void ResourceMaterial::SetSpecularGlossData(const SpecularGlossData& sgData)
{
    LoadTextures(sgData);
    std::visit([&](auto& data)
        {
            ReleaseTextures(data);
        }, data);

    data = sgData;
}
void ResourceMaterial::SetMetallicRoughData(const MetallicRoughData& mrData)
{
    LoadTextures(mrData);
    std::visit([&](auto& data)
        {
            ReleaseTextures(data);
        }, data);

    data = mrData;
}

const ResourceTexture* ResourceMaterial::GetTextureRes(SpecularGlossTextures texture) const
{
    const ResourceTexture* ret = nullptr;
    if(workFlow == SpecularGlossiness)
    {
        ret = static_cast<const ResourceTexture*>(App->resources->Get(std::get<SpecularGlossData>(data).textures[texture]));
    }

    return ret;
}

ResourceTexture* ResourceMaterial::GetTextureRes(SpecularGlossTextures texture)
{
    ResourceTexture* ret = nullptr;
    if (workFlow == SpecularGlossiness)
    {
        ret = static_cast<ResourceTexture*>(App->resources->Get(std::get<SpecularGlossData>(data).textures[texture]));
    }

    return ret;
}

const ResourceTexture* ResourceMaterial::GetTextureRes(MetallicRoughTextures texture) const
{
    const ResourceTexture* ret = nullptr;
    if(workFlow == MetallicRoughness)
    {
        ret = static_cast<const ResourceTexture*>(App->resources->Get(std::get<MetallicRoughData>(data).textures[texture]));
    }

    return ret;
}

ResourceTexture* ResourceMaterial::GetTextureRes(MetallicRoughTextures texture) 
{
    ResourceTexture* ret = nullptr;
    if (workFlow == MetallicRoughness)
    {
        ret = static_cast<ResourceTexture*>(App->resources->Get(std::get<MetallicRoughData>(data).textures[texture]));
    }

    return ret;
}

