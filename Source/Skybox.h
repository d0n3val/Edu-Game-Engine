#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "CubemapUtils.h"
#include <memory>

class ResourceTexture;
class Config;
class TextureCube;

class Skybox
{
    typedef std::unique_ptr<TextureCube> TextureCubePtr;

    CubemapUtils   utils;
	UID            cubemap = 0;
    TextureCubePtr diffuseIBL = nullptr;    
    TextureCubePtr prefilteredIBL = nullptr;

public:

    Skybox();
    ~Skybox();

    void    Draw                (const float4x4& proj, const float4x4& view);
    void    DrawDiffuseIBL      (const float4x4& proj, const float4x4& view);
    void    DrawPrefilteredIBL  (const float4x4& proj, const float4x4& view);
    void    Load                (const Config& config);
    void    Save                (Config& config) const;

    UID     GetCubemap          () const  { return cubemap; } 
    void    SetCubemap          (UID uid);

    const TextureCube* GetDiffuseIBL () const { return diffuseIBL.get(); }
    const TextureCube* GetPrefilterdIBL() const { return prefilteredIBL.get(); }
};


#endif /* __SKYBOX_H__ */


