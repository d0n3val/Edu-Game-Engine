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
class TubeLight;
class LocalIBLLight;

class LightManager
{
    struct SphereLightData
    {
        float4 position;
        float4 colour;
        float anisotropy;
        int pad0, pad1, pad2;
    };

    struct QuadLightData
    {
        float4 position;
        float4 up;
        float4 right;
        float4 colour;
        float4 size;
    };

    struct TubeLightData
    {
        float4 pos0; // position + radius
        float4 pos1; // position + anisotropy
        float4 colour; // color + attenuation radius
    };

    struct DirLightData
    {
        float4 dir; // dir + anisotropy
        float4 color; // color + intensity
    };

    struct PointLightData
    {
        float4 position; // position+radius
        float4 color;    // color+intensity
        float  anisotropy; 
        int pad0, pad1, pad2;
    };

    struct IBLLightData
    {
        uint64_t diffuse;
        uint64_t prefiltered;
        float4x4 toLocal;
        float4   position;
        float4   minParallax;
        float4   maxParallax;
        float4   minInfluence;
        float4   maxInfluence;
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
        float4x4 transform; // position + anisotropy
        float4 color; // colour+ anisotropy
        float  distance;
        float  inner;
        float  outer;
        float  radius; // base radius
        int    pad0, pad1, pad2;
        int    hasShadow;
        uint64_t shadowDepth;
        uint64_t shadowVariance;
        float4x4 shadowViewProj;
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

    struct TubeLightSet
    {
        int count = 0;
        int padding0;
        int padding1;
        int padding2;
        TubeLightData tubes[1];
    };

    struct SphereLightSet
    {
        int count = 0;
        int padding0;
        int padding1;
        int padding2;
        SphereLightData spheres[1];
    };

    struct IBLLightSet
    {
        int count = 0;
        int padding0;
        int padding1;
        int padding2;
        IBLLightData ibls[1];
    };

    typedef std::vector<std::unique_ptr<PointLight> > PointLightList;
    typedef std::vector<std::unique_ptr<SpotLight> > SpotLightList;
    typedef std::vector<std::unique_ptr<QuadLight> > QuadLightList;
    typedef std::vector<std::unique_ptr<SphereLight> > SphereLightList;
    typedef std::vector<std::unique_ptr<TubeLight> > TubeLightList;
    typedef std::vector<std::unique_ptr<LocalIBLLight> > LocalIBLLightList;

    std::unique_ptr<DirLight>   directional;
    PointLightList              points;
    SpotLightList               spots;
    QuadLightList               quads;
    SphereLightList             spheres;
    TubeLightList               tubes;
    LocalIBLLightList           ibls;

    std::unique_ptr<Buffer>     directionalSSBO[2];
    std::unique_ptr<Buffer>     spotLightSSBO[2];
    std::unique_ptr<Buffer>     pointLightSSBO[2];
    std::unique_ptr<Buffer>     sphereLightSSBO[2];
    std::unique_ptr<Buffer>     quadLightSSBO[2];
    std::unique_ptr<Buffer>     tubeLightSSBO[2];
    std::unique_ptr<Buffer>     iblLightSSBO[2];
   
    DirLightData*               directionalData[2];
    PointLightSet*              pointLightData[2];
    SpotLightSet*               spotLightData[2];
    QuadLightSet*               quadLightData[2];
    SphereLightSet*             sphereLightData[2];
    TubeLightSet*               tubeLightData[2];
    IBLLightSet*                iblLightData[2];

    uint                        pointBufferSize[2] = { 0, 0 };
    uint                        spotBufferSize[2] = { 0, 0 };
    uint                        quadBufferSize[2] = { 0, 0 };
    uint                        sphereBufferSize[2] = { 0, 0 };
    uint                        tubeBufferSize[2] = { 0, 0 };
    uint                        iblBufferSize[2] = { 0, 0 };
    uint                        frameCount = 0;
    uint                        enabledPointSize = 0;
    uint                        enabledSpotSize = 0;
    uint                        enabledQuadSize = 0;
    uint                        enabledSphereSize = 0;
    uint                        enabledTubeSize = 0;
    uint                        enablediblSize  = 0;
public:

    LightManager();
    ~LightManager();

    void LoadLights(const Config &config);
    void SaveLights(Config& config) const;
    void RemoveLights();

    void UpdateGPUBuffers(bool disableIBL = false);
    void Bind();

    const Buffer*       GetDirectionalBuffer    () const { return directionalSSBO[frameCount].get(); }
    const Buffer*       GetSpotBuffer           () const { return spotLightSSBO[frameCount].get(); }
    const Buffer*       GetPointBuffer          () const { return pointLightSSBO[frameCount].get(); }

    const DirLight*     GetDirLight             () const { return directional.get(); }
    DirLight*           GetDirLight             () { return directional.get(); }

    uint                AddPointLight           ();
    void                RemovePointLight        (uint index);
    uint                GetNumPointLights       () const { return uint(points.size()); }
    uint                GetEnabledPointLights   () const {return enabledPointSize;}
    const PointLight*   GetPointLight           (uint index) const { return points[index].get(); }
    PointLight*         GetPointLight           (uint index) { return points[index].get(); }

    uint                AddSpotLight            ();
    void                RemoveSpotLight         (uint index);
    uint                GetNumSpotLights        () const { return uint(spots.size()); }
    uint                GetEnabledSpotLights   () const {return enabledSpotSize;}
    const SpotLight*    GetSpotLight            (uint index) const { return spots[index].get(); }
    SpotLight*          GetSpotLight            (uint index) { return spots[index].get(); }

    uint                AddQuadLight            ();
    void                RemoveQuadLight         (uint index);
    uint                GetNumQuadLights        () const { return uint(quads.size()); }
    uint                GetEnabledQuadLights   () const {return enabledQuadSize;}
    const QuadLight*    GetQuadLight            (uint index) const { return quads[index].get(); }
    QuadLight*          GetQuadLight            (uint index) { return quads[index].get(); }

    uint                AddSphereLight            ();
    void                RemoveSphereLight         (uint index);
    uint                GetNumSphereLights        () const { return uint(spheres.size()); }
    uint                GetEnabledSphereLights   () const {return enabledSphereSize;}
    const SphereLight*  GetSphereLight            (uint index) const { return spheres[index].get(); }
    SphereLight*        GetSphereLight            (uint index) { return spheres[index].get(); }

    uint                AddTubeLight            ();
    void                RemoveTubeLight         (uint index);
    uint                GetNumTubeLights        () const { return uint(tubes.size()); }
    uint                GetEnabledTubeLights    () const {return enabledTubeSize;}
    const TubeLight*    GetTubeLight            (uint index) const { return tubes[index].get(); }
    TubeLight*          GetTubeLight            (uint index) { return tubes[index].get(); }


    uint                 AddLocalIBLLight        ();
    void                 RemoveLocalIBLLight     (uint index);
    uint                 GetNumLocalIBLLights    () const {return uint(ibls.size());}
    uint                 GetEnabledLocalIBLLists () const {return enablediblSize; }
    const LocalIBLLight* GetLocalIBLLight        (uint index) const {return ibls[index].get(); }
    LocalIBLLight*       GetLocalIBLLight        (uint index) {return ibls[index].get(); }

    void                 generateIBLs            ();
};