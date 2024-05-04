#ifndef __CUBEMAPUTILS_H__
#define __CUBEMAPUTILS_H__

#include "OGL.h"
#include "Math.h"

class CubemapUtils
{
    std::unique_ptr<Buffer>      vbo;
    std::unique_ptr<VertexArray> vao;
    std::unique_ptr<Program>     skybox;
    std::unique_ptr<Program>     skyboxLod;
    std::unique_ptr<Program>     equirectangular;
    std::unique_ptr<Program>     diffuseIBL;
    std::unique_ptr<Program>     prefilteredIBL;
    std::unique_ptr<Program>     environmentBRDF;

    static const float  skybox_vertices[6*6*3];
    static const float3 front[6];
    static float3       up[6];

public:
    CubemapUtils();

    void         RenderCubemap          (const TextureCube* cubemap, const float4x4& proj, const float4x4& view) ;
    void         RenderCubemapLod       (const TextureCube* cubemap, const float4x4& proj, const float4x4& view, float lod) ;

    TextureCube* LocalIBL               (const float3& position, const Quat& rotation, float farPlane, uint resolution);
    TextureCube* DiffuseIBL             (TextureCube* texture, uint cubemapSize, uint resolution, uint numSamples, uint lodBias);
    TextureCube* PrefilteredSpecular    (TextureCube* texture, uint cubemapSize, uint resolutioni, uint numSamples, uint prefilteredLevels, uint lodBias);
    Texture2D*   EnvironmentBRDF        (uint width, uint height);

    TextureCube* ConvertToCubemap   (Texture2D* texture, uint width, uint height);

private:
    void Init();

};

#endif // __CUBEMAPUTILS_H__ //


