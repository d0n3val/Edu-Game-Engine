#ifndef __CUBEMAPUTILS_H__
#define __CUBEMAPUTILS_H__

#include "OGL.h"
#include "Math.h"

class CubemapUtils
{
    std::unique_ptr<Buffer>      vbo;
    std::unique_ptr<VertexArray> vao;

    static const float  skybox_vertices[6*6*3];
    static const float3 front[6];
    static float3       up[6];

public:
    CubemapUtils();

    void         RenderSkybox       (TextureCube* cubemap, const float4x4& proj, const float4x4& view);

    TextureCube* DiffuseIBL         (UID cubemap);
    TextureCube* ConvertToCubemap   (Texture2D* texture, uint width, uint height);

private:
    void InitBuffers();

};

#endif // __CUBEMAPUTILS_H__ //


