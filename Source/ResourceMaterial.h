#ifndef __RESOURCE_MATERIAL_H__
#define __RESOURCE_MATERIAL_H__

#include "Resource.h"
#include "Math.h"
#include "utils/SimpleBinStream.h"

struct aiMaterial;
class ResourceTexture;
class Buffer;

enum MaterialTexture
{
    TextureDiffuse = 0,
    TextureSpecular,
    TextureNormal,
    TextureOcclusion,
    TextureEmissive,
    TextureLightmap,
    TextureDetailMask,
    TextureScndDiffuse,
    TextureScndSpecular,
    TextureScndNormal,
    TextureCount
};

namespace tinygltf { struct Material; class Model; }

class ResourceMaterial : public Resource
{
private:
    typedef std::unique_ptr<Buffer> BufferPtr;

    float4      diffuse_color          = float4::one;
    float3      specular_color         = float3::one;
    float3      emissive_color         = float3::zero;
    float       emissive_intensity     = 1.0;
    float       specular_intensity     = 1.0;
    UID         textures[TextureCount] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    float       smoothness             = 1.0f;
    float       normal_strength        = 1.0f;
    float       occlusion_strength     = 1.0f; // TODO: Apply
    bool        double_sided           = false;
    float       alpha_test             = 0.0f;
    float2      uv_tiling              = float2(1, 1);
    float2      uv_offset              = float2(0, 0);
    float2      scnd_uv_tiling         = float2(1, 1);
    float2      scnd_uv_offset         = float2(0, 0);
    bool        planarReflections   = false;
    BufferPtr   materialUBO;
    bool        uboDirty               = true;


public:

    explicit ResourceMaterial(UID id);
    virtual ~ResourceMaterial();

	bool                    LoadInMemory        () override;
    void                    ReleaseFromMemory   () override;

    bool                    Save                (std::string& output) const;
    bool                    Save                ();

    static UID              Import              (const tinygltf::Model& model, const tinygltf::Material& material, const char* file);
    static UID              Import              (const aiMaterial* material, const char* source_file);

    const float4&           GetDiffuseColor     () const { return diffuse_color;}
    void                    SetDiffuseColor     (const float4& value) { diffuse_color = value; uboDirty = true;}

    const float3&           GetSpecularColor    () const { return specular_color;}
    void                    SetSpecularColor    (const float3& value) { specular_color = value; uboDirty = true;}

    const float3&           GetEmissiveColor    () const { return emissive_color;}
    void                    SetEmissiveColor    (const float3& value) { emissive_color = value; uboDirty = true;}

    float                   GetEmissiveIntensity () const { return emissive_intensity; }
    void                    SetEmissiveIntensity (float intensity) { emissive_intensity = intensity; }

    float                   GetSpecularIntensity () const { return specular_intensity; }
    void                    SetSpecularIntensity (float intensity) { specular_intensity = intensity; }

    UID                     GetTexture          (MaterialTexture texture) const { return textures[texture]; }
    const ResourceTexture*  GetTextureRes       (MaterialTexture texture) const;
    ResourceTexture*        GetTextureRes       (MaterialTexture texture) ;
    void                    SetTexture          (MaterialTexture texture, UID uid);

    float                   GetSmoothness       () const { return smoothness; }
    void                    SetSmoothness       (float s) { smoothness = s; uboDirty = true;}

    float                   GetNormalStrength   () const { return normal_strength; }
    void                    SetNormalStrength   (float s) { normal_strength = s; uboDirty = true;}

    bool                    GetDoubleSided      () const { return double_sided; }
    void                    SetDoubleSided      (bool dside)  { double_sided = dside; uboDirty = true;}

    float                   GetAlphaTest        () const { return alpha_test; }
    void                    SetAlphaTest        (float atest)  { alpha_test = atest; uboDirty = true;}

    uint                    GetMask             () const;

    float2                  GetUVTiling         () const { return uv_tiling; }
    float2                  GetSecondUVTiling   () const { return scnd_uv_tiling; }
    float2                  GetUVOffset         () const { return uv_offset; }
    float2                  GetSecondUVOffset   () const { return scnd_uv_offset; }

    void                    SetUVTiling         (const float2& tiling) { uv_tiling = tiling; uboDirty = true;}
    void                    SetSecondUVTiling   (const float2& tiling) { scnd_uv_tiling = tiling; uboDirty = true;}
    void                    SetUVOffset         (const float2& offset) { uv_offset = offset; uboDirty = true;}
    void                    SetSecondUVOffset   (const float2& offset) { scnd_uv_offset = offset; uboDirty = true;}

    bool                    GetPlanarReflections() const {return planarReflections;}                    
    void                    SetPlanarReflections(bool active) { planarReflections = active; }
    
    static Resource::Type   GetClassType        () {return Resource::material;}

    const Buffer*           GetMaterialUBO      ();

private:
	void                    SaveToStream(simple::mem_ostream<std::true_type>& write_stream) const;
    void                    GenerateUBO         ();
};


#endif /* __RESOURCE_MATERIAL_H__ */
