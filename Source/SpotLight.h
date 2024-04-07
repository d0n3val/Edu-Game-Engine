#ifndef __SPOTLIGHT_H__
#define __SPOTLIGHT_H__

#include "Math.h"

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

    float3          GetPosition     () const { return position; }
    void            SetPosition     (const float3& p) { position = p; }

    float3          GetDirection     () const { return direction; }
	void            SetDirection(const float3& d) { direction = d.Normalized();; }

    float           GetInnerCutoff   () const { return inner; }
    void            SetInnerCutoff   (float angle) { inner = angle; }

    float           GetOutterCutoff  () const { return outter; }
    void            SetOutterCutoff  (float angle) { outter = angle; }

    float           GetDistance      () const { return distance; }
    void            SetDistance      (float r) { distance = r; }

    float           GetIntensity     () const { return intensity; }
    void            SetIntensity     (float i) { intensity = i; }

    bool            GetEnabled      () const { return enabled; }
    void            SetEnabled      (bool e) { enabled = e; }

    float           GetAnisotropy () const {return anisotropy;}
    void            SetAnisotropy (float value) {anisotropy = value;}

private:

    float3 color     = float3::one;
    float3 position  = float3::zero;
    float3 direction = -float3::unitY;
    float  inner     = 0.0f;
    float  outter    = 0.0f;
    float  distance  = 1.0f;
    float  intensity = 1.0f;
    float  anisotropy = 0.0f;
    bool   enabled   = true;
};

#endif /* __SPOTLIGHT_H__ */
