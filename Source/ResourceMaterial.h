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
    TextureCount
};

class ResourceMaterial : public Resource
{
private:

    struct MaterialBuffer
    {
        float4    diffuse_color;
        float4    specular_color;
        float4    emissive_color;
        float     smoothness;
        float     normal_strength;
        float     alpha_test;
        uint      mapMask;
    };

    typedef std::unique_ptr<Buffer> BufferPtr;

    BufferPtr   materialUBO;
    bool        dirty_ubo = true;


    float4      diffuse_color          = float4::one;
    float3      specular_color         = float4::zero;
    float3      emissive_color         = float4::zero;
    UID         textures[TextureCount] = { 0, 0, 0, 0, 0, 0 };
    float       smoothness             = 0.5f;
    float       normal_strength        = 1.0f;
    bool        double_sided           = false;
    float       alpha_test             = 0.0f;

public:

    explicit ResourceMaterial(UID id);
    virtual ~ResourceMaterial();

	bool                    LoadInMemory        () override;
    void                    ReleaseFromMemory   () override;

    bool                    Save                (std::string& output) const;
    bool                    Save                ();

    static UID              Import              (const aiMaterial* material, const char* source_file);

    void                    UpdateUniforms      ();
    void                    BindTextures        () const;
    void                    UnbindTextures      () const;

    const float4&           GetDiffuseColor     () const { return diffuse_color;}
    void                    SetDiffuseColor     (const float4& value) { diffuse_color = value; dirty_ubo = true; }

    const float3&           GetSpecularColor    () const { return specular_color;}
    void                    SetSpecularColor    (const float3& value) { specular_color = value; dirty_ubo = true; }

    const float3&           GetEmissiveColor    () const { return emissive_color;}
    void                    SetEmissiveColor    (const float3& value) { emissive_color = value; dirty_ubo = true; }

    UID                     GetTexture          (MaterialTexture texture) const { return textures[texture]; }
    const ResourceTexture*  GetTextureRes       (MaterialTexture texture) const;
    ResourceTexture*        GetTextureRes       (MaterialTexture texture) ;
    void                    SetTexture          (MaterialTexture texture, UID uid);

    float                   GetSmoothness       () const { return smoothness; }
    void                    SetSmoothness       (float s) { smoothness = s; dirty_ubo = true; }

    float                   GetNormalStrength   () const { return normal_strength; }
    void                    SetNormalStrength   (float s) { normal_strength = s; dirty_ubo = true; }

    bool                    GetDoubleSided      () const { return double_sided; }
    void                    SetDoubleSided      (bool dside)  { double_sided = dside; }

    float                   GetAlphaTest        () const { return alpha_test; }
    void                    SetAlphaTest        (float atest)  { alpha_test = atest; dirty_ubo = true; }

    uint                    GetMapMask          () const;
    
    static Resource::Type   GetClassType        () {return Resource::material;}

private:
	void                    SaveToStream(simple::mem_ostream<std::true_type>& write_stream) const;
    void                    UpdateUBO           ();
};


#endif /* __RESOURCE_MATERIAL_H__ */
