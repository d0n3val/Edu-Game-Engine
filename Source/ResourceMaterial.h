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

class ResourceMaterial : public Resource
{
private:

    float4      diffuse_color          = float4::one;
    float3      specular_color         = float4::one;
    float3      emissive_color         = float4::zero;
    UID         textures[TextureCount] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    float       smoothness             = 1.0f;
    float       normal_strength        = 1.0f;
    bool        double_sided           = false;
    float       alpha_test             = 0.0f;
    float2      uv_tiling              = float2(1, 1);
    float2      uv_offset              = float2(0, 0);
    float2      scnd_uv_tiling         = float2(1, 1);
    float2      scnd_uv_offset         = float2(0, 0);
    // \todo: secondary uv set

public:

    explicit ResourceMaterial(UID id);
    virtual ~ResourceMaterial();

	bool                    LoadInMemory        () override;
    void                    ReleaseFromMemory   () override;

    bool                    Save                (std::string& output) const;
    bool                    Save                ();

    static UID              Import              (const aiMaterial* material, const char* source_file);

    const float4&           GetDiffuseColor     () const { return diffuse_color;}
    void                    SetDiffuseColor     (const float4& value) { diffuse_color = value; }

    const float3&           GetSpecularColor    () const { return specular_color;}
    void                    SetSpecularColor    (const float3& value) { specular_color = value; }

    const float3&           GetEmissiveColor    () const { return emissive_color;}
    void                    SetEmissiveColor    (const float3& value) { emissive_color = value; }

    UID                     GetTexture          (MaterialTexture texture) const { return textures[texture]; }
    const ResourceTexture*  GetTextureRes       (MaterialTexture texture) const;
    ResourceTexture*        GetTextureRes       (MaterialTexture texture) ;
    void                    SetTexture          (MaterialTexture texture, UID uid);

    float                   GetSmoothness       () const { return smoothness; }
    void                    SetSmoothness       (float s) { smoothness = s; }

    float                   GetNormalStrength   () const { return normal_strength; }
    void                    SetNormalStrength   (float s) { normal_strength = s; }

    bool                    GetDoubleSided      () const { return double_sided; }
    void                    SetDoubleSided      (bool dside)  { double_sided = dside; }

    float                   GetAlphaTest        () const { return alpha_test; }
    void                    SetAlphaTest        (float atest)  { alpha_test = atest; }

    uint                    GetMapMask          () const;

    float2                  GetUVTiling         () const { return uv_tiling; }
    float2                  GetSecondUVTiling   () const { return scnd_uv_tiling; }
    float2                  GetUVOffset         () const { return uv_offset; }
    float2                  GetSecondUVOffset   () const { return scnd_uv_offset; }

    void                    SetUVTiling         (const float2& tiling) { uv_tiling = tiling; }
    void                    SetSecondUVTiling   (const float2& tiling) { scnd_uv_tiling = tiling; }
    void                    SetUVOffset         (const float2& offset) { uv_offset = offset; }
    void                    SetSecondUVOffset   (const float2& offset) { scnd_uv_offset = offset; }
    
    static Resource::Type   GetClassType        () {return Resource::material;}

private:
	void                    SaveToStream(simple::mem_ostream<std::true_type>& write_stream) const;
};


#endif /* __RESOURCE_MATERIAL_H__ */
