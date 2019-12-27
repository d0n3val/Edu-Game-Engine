#ifndef __RESOURCE_MATERIAL_H__
#define __RESOURCE_MATERIAL_H__

#include "Resource.h"
#include "Math.h"
#include "utils/SimpleBinStream.h"

struct aiMaterial;
class ResourceTexture;

class ResourceMaterial : public Resource
{
public:
    enum Texture
    {
        TextureDiffuse = 0,
        TextureSpecular,
        TextureNormal,
        TextureOcclusion,
        TextureEmissive,
        TextureCount
    };

private:

    float4      diffuse_color          = float4::one;
    float3      specular_color         = float4::zero;
    float3      emissive_color         = float4::zero;
    UID         textures[TextureCount] = { 0, 0, 0, 0, 0 };
    float       shininess              = 0.5f;

public:

    explicit ResourceMaterial(UID id);
    virtual ~ResourceMaterial();

	bool                    LoadInMemory        () override;
    void                    ReleaseFromMemory   () override;

    bool                    Save                (std::string& output) const;
    bool                    Save                () ;

    static UID              Import              (const aiMaterial* material, const char* source_file);

    void                    UpdateUniforms      () const;

    const float4&           GetDiffuseColor     () const { return diffuse_color;}
    void                    SetDiffuseColor     (const float4& value) { diffuse_color = value; }

    const float3&           GetSpecularColor    () const { return specular_color;}
    void                    SetSpecularColor    (const float3& value) { specular_color = value; }

    const float3&           GetEmissiveColor    () const { return emissive_color;}
    void                    SetEmissiveColor    (const float3& value) { emissive_color = value; }

    UID                     GetTexture          (Texture texture) const { return textures[texture]; }
    const ResourceTexture*  GetTextureRes       (Texture texture) const;
    ResourceTexture*        GetTextureRes       (Texture texture) ;
    void                    SetTexture          (Texture texture, UID uid);

    float                   GetShininess        () const { return shininess; }
    void                    SetShininess        (float s) { shininess = s; }

private:
	void                    SaveToStream(simple::mem_ostream<std::true_type>& write_stream) const;
};


#endif /* __RESOURCE_MATERIAL_H__ */
