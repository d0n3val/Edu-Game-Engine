#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "CubemapUtils.h"
#include <memory>

class ResourceTexture;
class Config;
class TextureCube;
class Texture2D;

class Skybox
{
    typedef std::unique_ptr<TextureCube> TextureCubePtr;
    typedef std::unique_ptr<Texture2D> Texture2DPtr;

    CubemapUtils   utils;
	UID            cubemap = 0;
    TextureCubePtr diffuseIBL;    
    TextureCubePtr prefilteredIBL;
    uint           prefilteredLevels = 0;
    Texture2DPtr   environmentBRDF;

public:

    Skybox();
    ~Skybox();

    bool    Draw                (const float4x4& proj, const float4x4& view);
    void    DrawDiffuseIBL      (const float4x4& proj, const float4x4& view);
    void    DrawPrefilteredIBL  (const float4x4& proj, const float4x4& view, float roughness);
    void    Load                (const Config& config);
    void    Save                (Config& config) const;

    UID     GetCubemap          () const  { return cubemap; } 
    void    SetCubemap          (UID uid);

    void    BindIBL             ();

    const TextureCube* GetDiffuseIBL () const { return diffuseIBL.get(); }
    const TextureCube* GetPrefilterdIBL() const { return prefilteredIBL.get(); }
    uint               GetPrefilterdLevels() const { return prefilteredLevels; }
    const Texture2D*   GetEnvironmentBRDF() const { return environmentBRDF.get(); }
};


#endif /* __SKYBOX_H__ */


