#ifndef __RESOURCE_MATERIAL_H__
#define __RESOURCE_MATERIAL_H__

#include "Resource.h"
#include "Math.h"

struct aiMaterial;

class ResourceMaterial : public Resource
{
public:
    enum Texture
    {
        TextureDiffuse = 0,
        TextureSpecular,
        TextureNormal,
        TextureOcclusion,
        TextureCount
    };

    enum Colors
    {
        ColorAmbient = 0,
        ColorDiffuse,
        ColorSpecular,
        ColorCount
    };

private:

    float4      colors[ColorCount]     = { { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } };
    UID         textures[TextureCount] = { 0, 0, 0, 0 };
    float       shininess                = 40.0f;

public:

    explicit ResourceMaterial(UID id);
    virtual ~ResourceMaterial();

	bool                    LoadInMemory        () override;
    void                    ReleaseFromMemory   () override;
    bool                    Save                (std::string& output) const;

    static UID              Import              (const aiMaterial* material, const char* source_file);

    UID                     GetTexture          (Texture texture) const { return textures[texture]; }
    const ResourceTexture*  GetTextureRes       (Texture texture) const;
    void                    SetTexture          (Texture texture, UID uid);

    float                   GetShininess        () const { return shininess; }
};


#endif /* __RESOURCE_MATERIAL_H__ */
