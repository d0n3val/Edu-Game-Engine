#ifndef __SPOTLIGHT_H__
#define __SPOTLIGHT_H__

#include "Math.h"
#include "OGL.h"

#include <memory>

class Config;

class SpotLight
{
public:
    SpotLight();
    ~SpotLight();

    void            Save            (Config& config) const;
    void            Load            (Config& config);

    const float3&   GetColor        () const { return color; }
    void            SetColor        (const float3& c) { color = c; }

    float3          GetDirection     () const { return transform.Row3(1); }
    void            SetTransform     (const float4x4& t) { transform = t; }
    const float4x4& GetTransform    () const { return transform; }

    float           GetInnerCutoff   () const { return inner; }
    void            SetInnerCutoff   (float angle) { inner = angle; }

    float           GetOutterCutoff  () const { return outter; }
    void            SetOutterCutoff  (float angle) { outter = angle; }

    float           GetMaxDistance      () const { return maxDist; }
    void            SetMaxDistance      (float r) { maxDist = r; }

    float           GetMinDistance      () const { return minDist; }
    void            SetMinDistance      (float r) { minDist = r; }

    float           GetIntensity     () const { return intensity; }
    void            SetIntensity     (float i) { intensity = i; }

    bool            GetEnabled       () const { return enabled; }
    void            SetEnabled       (bool e) { enabled = e; }

    float           GetAnisotropy    () const {return anisotropy;}
    void            SetAnisotropy    (float value) {anisotropy = value;}

    const Texture*  GetShadowTex () const {return shadowTex;}
    void            SetShadowTex (const Texture2D* texture) { shadowTex = texture;}

    const float4x4& GetShadowViewProj() const {return shadowViewProj;}
    void            SetShadowViewProj(const float4x4& viewProj) {shadowViewProj = viewProj;}

    uint            GetShadowSize() const { return shadowSize;}
    void            SetShadowSize(uint size);

private:

    float3 color     = float3::one;
    float4x4 transform = float3x3::identity;
    float  inner     = 0.0f;
    float  outter    = 0.0f;
    float  maxDist  = 1.0f;
    float  minDist  = 0.1f;
    float  intensity = 1.0f;
    float  anisotropy = 0.0f;
    float4x4 shadowViewProj = float4x4::identity;
    uint shadowSize = 256;
    bool   enabled   = true;
    const Texture2D* shadowTex = nullptr;
};

#endif /* __SPOTLIGHT_H__ */
