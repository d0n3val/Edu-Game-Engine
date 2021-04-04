#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "CubemapUtils.h"

class ResourceTexture;
class Config;
class TextureCube;

class Skybox
{

    CubemapUtils utils;
	UID          cubemap = 0;

public:

    Skybox();
    ~Skybox();

    void    Draw        (const float4x4& proj, const float4x4& view);
    void    Load        (const Config& config);
    void    Save        (Config& config) const;

    UID     GetCubemap  () const  { return cubemap; } 
    void    SetCubemap  (UID uid);
};


#endif /* __SKYBOX_H__ */


