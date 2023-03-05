#pragma once

#include "OGL.h"
#include <vector>
#include <memory>

class DirLight;
class PointLight;
class SpotLight;
class Config;
class QuadLight;
class SphereLight;

class LightManager
{
    struct SphereLightData
    {
        float4 position;
        float4 colour;
    };

    struct QuadLightData
    {
        float4 position;
        float4 up;
        float4 right;
        float4 colour;
        float2 size;
    };

    struct DirLightData
    {
        float4 dir;
        float4 color;
    };

    struct PointLightData
    {
        float3 position;
        float  radius;
        float4 color;
    };

    struct PointLightSet
    {
        int count = 0;
        int padding0;
        int padding1;
        int padding2;
        PointLightData points[1];
    };

    struct SpotLightData
    {
        float4 position;
        float4 direction;
        float4 color;
        float  distance;
        float  inner;
        float  outer;
        float  intensity;
    };

    struct SpotLightSet
    {
        int count = 0;
        int padding0;
        int padding1;
        int padding2;
        SpotLightData spots[1];
    };

    struct QuadLightSet
    {
        int count = 0;
        int padding0;
        int padding1;
        int padding2;
        QuadLightData quads[1];
    };

    struct SphereLightSet
    {
        int count = 0;
        int padding0;
        int padding1;
        int padding2;
        SphereLightData spheres[1];
    };

    typedef std::vector<std::unique_ptr<PointLight> > PointLightList;
    typedef std::vector<std::unique_ptr<SpotLight> > SpotLightList;
    typedef std::vector<std::unique_ptr<QuadLight> > QuadLightList;
    typedef std::vector<std::unique_ptr<SphereLight> > SphereLightList;

    std::unique_ptr<DirLight>   directional;
    PointLightList              points;
    SpotLightList               spots;
    QuadLightList               quads;
    SphereLightList             spheres;

    std::unique_ptr<Buffer>     directionalSSBO[2];
    std::unique_ptr<Buffer>     spotLightSSBO[2];
    std::unique_ptr<Buffer>     pointLightSSBO[2];
    std::unique_ptr<Buffer>     sphereLightSSBO[2];
    std::unique_ptr<Buffer>     quadLightSSBO[2];
   
    DirLightData*               directionalData[2];
    PointLightSet*              pointLightData[2];
    SpotLightSet*               spotLightData[2];
    QuadLightSet*               quadLightData[2];
    SphereLightSet*             sphereLightData[2];

    uint                        pointBufferSize = 0;
    uint                        spotBufferSize = 0;
    uint                        quadBufferSize = 0;
    uint                        sphereBufferSize = 0;
    uint                        frameCount = 0;
public:

    LightManager();
    ~LightManager();

    void LoadLights(const Config &config);
    void SaveLights(Config& config) const;
    void RemoveLights();

    void UpdateGPUBuffers();
    void Bind();

    const Buffer*       GetDirectionalBuffer    () const { return directionalSSBO[frameCount].get(); }
    const Buffer*       GetSpotBuffer           () const { return spotLightSSBO[frameCount].get(); }
    const Buffer*       GetPointBuffer          () const { return pointLightSSBO[frameCount].get(); }

    const DirLight*     GetDirLight             () const { return directional.get(); }
    DirLight*           GetDirLight             () { return directional.get(); }

    uint                AddPointLight           ();
    void                RemovePointLight        (uint index);
    uint                GetNumPointLights       () const { return uint(points.size()); }
    const PointLight*   GetPointLight           (uint index) const { return points[index].get(); }
    PointLight*         GetPointLight           (uint index) { return points[index].get(); }

    uint                AddSpotLight            ();
    void                RemoveSpotLight         (uint index);
    uint                GetNumSpotLights        () const { return uint(spots.size()); }
    const SpotLight*    GetSpotLight            (uint index) const { return spots[index].get(); }
    SpotLight*          GetSpotLight            (uint index) { return spots[index].get(); }

    uint                AddQuadLight            ();
    void                RemoveQuadLight         (uint index);
    uint                GetNumQuadLights        () const { return uint(quads.size()); }
    const QuadLight*    GetQuadLight            (uint index) const { return quads[index].get(); }
    QuadLight*          GetQuadLight            (uint index) { return quads[index].get(); }

    uint                AddSphereLight            ();
    void                RemoveSphereLight         (uint index);
    uint                GetNumSphereLights        () const { return uint(spheres.size()); }
    const SphereLight*  GetSphereLight            (uint index) const { return spheres[index].get(); }
    SphereLight*        GetSphereLight            (uint index) { return spheres[index].get(); }
};