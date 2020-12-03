#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "OGL.h"

class ResourceTexture;

class Skybox
{

    std::unique_ptr<Buffer>      vbo;
    std::unique_ptr<VertexArray> vao;
	UID                          cubemap = 0;

public:

    Skybox();
    ~Skybox();

    void    Draw        (const float4x4& proj, const float4x4& view);

    UID     GetCubemap  () const  { return cubemap; } 
    void    SetCubemap  (UID uid);
};


#endif /* __SKYBOX_H__ */


