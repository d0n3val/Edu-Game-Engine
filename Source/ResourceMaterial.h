#ifndef __RESOURCE_MATERIAL_H__
#define __RESOURCE_MATERIAL_H__

#include "Resource.h"
#include "Math.h"
#include "utils/SimpleBinStream.h"
#include <variant>

struct aiMaterial;
class ResourceTexture;
class Buffer;

enum SpecularGlossTextures
{
    SG_TextureDiffuse = 0,
    SG_TextureSpecular,
    SG_TextureNormal,
    SG_TextureOcclusion,
    SG_TextureEmissive,
    SG_TextureLightmap,
    SG_TextureDetailMask,
    SG_TextureScndDiffuse,
    SG_TextureScndSpecular,
    SG_TextureScndNormal,
    SG_TextureCount
};

enum MetallicRoughTextures
{
    MR_TextureBaseColor = 0,
    MR_TextureMetallicRough,
    MR_TextureNormal,
    MR_TextureOcclusion,
    MR_TextureEmissive,
    MR_TextureCount
};

enum MaterialWorkFlow
{
    SpecularGlossiness = 0,
    MetallicRoughness
};

struct SpecularGlossData
{
    float4 diffuse_color = float4::one;
    float3 specular_color = float3::one;
    float3 emissive_color = float3::zero;
    float emissive_intensity = 1.0;
    float specular_intensity = 1.0;
    UID textures[SG_TextureCount] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    float smoothness = 1.0f;
    float normal_strength = 1.0f;
    float occlusion_strength = 1.0f; // TODO: Apply
};

struct MetallicRoughData
{
    float4 baseColor = float4::one;
    float metalness = 0.0;
    float roughness = 0.0;
    float3 emissive_color = float3::zero;
    float emissive_intensity = 1.0;
    UID textures[MR_TextureCount] = {0, 0, 0, 0, 0};
    float normal_strength = 1.0f;
    float occlusion_strength = 1.0f;
};

namespace tinygltf { struct Material; class Model; class Value;  }

class ResourceMaterial : public Resource
{
private:
    typedef std::unique_ptr<Buffer> BufferPtr;

    typedef std::variant<SpecularGlossData, MetallicRoughData> MaterialData;

    MaterialWorkFlow workFlow = MetallicRoughness;
    MaterialData     data;

    bool             double_sided           = false;
    float            alpha_test             = 0.0f;
    float2           uv_tiling              = float2(1, 1);
    float2           uv_offset              = float2(0, 0);
    float2           scnd_uv_tiling         = float2(1, 1);
    float2           scnd_uv_offset         = float2(0, 0);
    bool             planarReflections      = false;
    BufferPtr        materialUBO;
    bool             uboDirty               = true;

public:

    explicit ResourceMaterial(UID id);
    virtual ~ResourceMaterial();

	bool                        LoadInMemory        () override;
    void                        ReleaseFromMemory   () override;

    bool                        Save                (std::string& output) const;
    bool                        Save                ();

    static UID                  Import              (const tinygltf::Model& model, const tinygltf::Material& material, const char* file);

    MaterialWorkFlow            GetWorkFlow         () const {return workFlow; }
    const SpecularGlossData&    GetSpecularGlossData() const {return std::get<SpecularGlossData>(data); }
    const MetallicRoughData&    GetMetallicRoughData() const {return std::get<MetallicRoughData>(data); }

    void                        SetSpecularGlossData(const SpecularGlossData& sgData);
    void                        SetMetallicRoughData(const MetallicRoughData& mrData);

    const ResourceTexture*      GetTextureRes       (SpecularGlossTextures texture) const;
    ResourceTexture*            GetTextureRes       (SpecularGlossTextures texture);
    const ResourceTexture*      GetTextureRes       (MetallicRoughTextures texture) const;
    ResourceTexture*            GetTextureRes       (MetallicRoughTextures texture);

    bool                        GetDoubleSided      () const { return double_sided; }
    void                        SetDoubleSided      (bool dside)  { double_sided = dside; uboDirty = true;}

    float                       GetAlphaTest        () const { return alpha_test; }
    void                        SetAlphaTest        (float atest)  { alpha_test = atest; uboDirty = true;}

    uint                        GetMask             () const;

    float2                      GetUVTiling         () const { return uv_tiling; }
    float2                      GetSecondUVTiling   () const { return scnd_uv_tiling; }
    float2                      GetUVOffset         () const { return uv_offset; }
    float2                      GetSecondUVOffset   () const { return scnd_uv_offset; }

    void                        SetUVTiling         (const float2& tiling) { uv_tiling = tiling; uboDirty = true;}
    void                        SetSecondUVTiling   (const float2& tiling) { scnd_uv_tiling = tiling; uboDirty = true;}
    void                        SetUVOffset         (const float2& offset) { uv_offset = offset; uboDirty = true;}
    void                        SetSecondUVOffset   (const float2& offset) { scnd_uv_offset = offset; uboDirty = true;}

    bool                        GetPlanarReflections() const {return planarReflections;}                    
    void                        SetPlanarReflections(bool active) { planarReflections = active; }
    
    static Resource::Type       GetClassType        () {return Resource::material;}

    //const Buffer*               GetMaterialUBO      ();

private:
    void                        ReleaseTextures();
    void                        ReleaseTextures(const SpecularGlossData& data);
    void                        ReleaseTextures(const MetallicRoughData& data);
    void                        LoadTextures(const SpecularGlossData& data);
    void                        LoadTextures(const MetallicRoughData& data);
	void                        SaveToStream(simple::mem_ostream<std::true_type>& write_stream) const;
    //void                        GenerateUBO         ();
    static void                 ImportSpecularGlossiness(SpecularGlossData& data, const std::string& base_path, const tinygltf::Model& model, const tinygltf::Material& material, const tinygltf::Value& materialMap);
    static void                 ImportMetallicRoughness(MetallicRoughData& data, std::string& base_path, const tinygltf::Model& model, const tinygltf::Material& material);
    static UID                  ImportTexture(const std::string& basePath, const tinygltf::Model& model, int textureIdx);
    static float3               ImportColor3(const std::vector<double>& matColor);
    static float4               ImportColor4(const std::vector<double>& matColor);
};


#endif /* __RESOURCE_MATERIAL_H__ */
