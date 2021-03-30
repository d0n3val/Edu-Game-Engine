#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "OGL.h"

class ResourceTexture;
class Config;
class TextureCube;

class Skybox
{

    std::unique_ptr<Buffer>      vbo;
    std::unique_ptr<VertexArray> vao;
	UID                          cubemap = 0;
    Texture*                     test = nullptr;

public:

    Skybox();
    ~Skybox();

    void    Draw        (const float4x4& proj, const float4x4& view);
    void    Load        (const Config& config);
    void    Save        (Config& config) const;

    UID     GetCubemap  () const  { return cubemap; } 
    void    SetCubemap  (UID uid);

    TextureCube* ConvertToCubemap(UID uid, uint width, uint height) const;

};


#endif /* __SKYBOX_H__ */


