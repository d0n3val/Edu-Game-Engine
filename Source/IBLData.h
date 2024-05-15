#pragma once 

#include "CubemapUtils.h"
#include "ResHandle.h"
#include <memory>

class ResourceTexture;
class Config;
class TextureCube;
class Texture2D;

class IBLData
{
    typedef std::unique_ptr<TextureCube> TextureCubePtr;
    typedef std::unique_ptr<Texture2D> Texture2DPtr;

    CubemapUtils        utils;
    ResHandle           envRes;
	TextureCube*        environment = nullptr;
    TextureCubePtr      diffuseIBL;    
    TextureCubePtr      prefilteredIBL;
    uint                prefilteredLevels = 0;
    float               intensity = 1.0f;
    static Texture2DPtr environmentBRDF;
    static uint          refCount;

public:

    IBLData();
    ~IBLData();

    bool    DrawEnvironment     (const float4x4& proj, const float4x4& view);
    void    DrawDiffuseIBL      (const float4x4& proj, const float4x4& view);
    void    DrawPrefilteredIBL  (const float4x4& proj, const float4x4& view, float roughness);
    void    Load                (const Config& config);
    void    Save                (Config& config) const;


    void    Bind                ();

    void               generateEnvironment(const float3& position, const Quat& rotation, float farPlane, uint resolution, uint numSamples, uint roughnessLevels);

    void               SetEnvironmentRes    (UID uid);
    ResHandle          GetEnvironmentRes() const { return envRes;  }
    
    void               SetEnvironment       (TextureCube* env, uint cubemapSize, uint resolution = 256, uint numSamples = 2048, uint roughnessLevels = 9);
    TextureCube*       GetEnvironment       ()        { return environment; }
    const TextureCube* GetEnvironment       () const  { return environment; } 

    const TextureCube* GetDiffuseIBL        () const { return diffuseIBL.get(); }
    TextureCube*        GetDiffuseIBL       () { return diffuseIBL.get(); }

    const TextureCube* GetPrefilterdIBL     () const { return prefilteredIBL.get(); }
    TextureCube*       GetPrefilterdIBL     () { return prefilteredIBL.get(); }

    uint               GetPrefilterdLevels  () const { return prefilteredLevels; }
    const Texture2D*   GetEnvironmentBRDF   () const { return environmentBRDF.get(); }

    float              GetIntensity() const { return intensity;  }
    void               SetIntensity(float value) { intensity = value; }
};
