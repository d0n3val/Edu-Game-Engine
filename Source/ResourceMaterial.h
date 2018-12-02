#ifndef __RESOURCE_MATERIAL_H__
#define __RESOURCE_MATERIAL_H__

#include "Resource.h"
#include "Math.h"
#include "utils/SimpleBinStream.h"

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

    enum Color
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
    bool                    Save                () const;

    static UID              Import              (const aiMaterial* material, const char* source_file);

    const float4&           GetColor            (Color color) const { return colors[color];}
    void                    SetColor            (Color color, const float4& value) { colors[color] = value; }

    UID                     GetTexture          (Texture texture) const { return textures[texture]; }
    const ResourceTexture*  GetTextureRes       (Texture texture) const;
    void                    SetTexture          (Texture texture, UID uid);

    float                   GetShininess        () const { return shininess; }
    void                    SetShininess        (float s) { shininess = s; }

private:
	void                    SaveToStream(simple::mem_ostream<std::true_type>& write_stream) const;
};


#endif /* __RESOURCE_MATERIAL_H__ */
