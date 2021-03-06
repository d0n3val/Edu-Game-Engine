#ifndef __CUBEMAPUTILS_H__
#define __CUBEMAPUTILS_H__

#include "OGL.h"
#include "Math.h"

class CubemapUtils
{
    std::unique_ptr<Buffer>      vbo;
    std::unique_ptr<VertexArray> vao;
    std::unique_ptr<Program>     skybox;
    std::unique_ptr<Program>     equirectangular;
    std::unique_ptr<Program>     diffuseIBL;

    static const float  skybox_vertices[6*6*3];
    static const float3 front[6];
    static float3       up[6];

public:
    CubemapUtils();

    void         RenderSkybox       (TextureCube* cubemap, const float4x4& proj, const float4x4& view);

    TextureCube* DiffuseIBL         (TextureCube* texture, uint width, uint height);
    TextureCube* ConvertToCubemap   (Texture2D* texture, uint width, uint height);

private:
    void Init();
    TextureCube* RenderCube(Program* program, Texture* texture, uint width, uint height);

};

#endif // __CUBEMAPUTILS_H__ //


