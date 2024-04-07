#ifndef __POINTLIGHT_H__
#define __POINTLIGHT_H__

#include "Math.h"

class Config;

class PointLight
{
public:
    PointLight();
    ~PointLight();

    void            Save            (Config& config) const;
    void            Load            (Config& config);

    const float3&   GetColor        () const { return color; }
    void            SetColor        (const float3& c) { color = c; }

    float3          GetPosition     () const { return position; }
    void            SetPosition     (const float3& p) { position = p; }

    float           GetRadius  () const  { return radius; }
    void            SetRadius  (float r) { radius = r; }

    float           GetIntensity () const { return intensity; }
    void            SetIntensity (float i) { intensity = i;}

    bool            GetEnabled      () const { return enabled; }
    void            SetEnabled      (bool e) { enabled = e; }

    float           GetAnisotropy () const {return anisotropy;}
    void           SetAnisotropy (float value) {anisotropy = value;}

private:

    float3 color     = float3::one;
    float3 position  = float3::zero;
    float  radius    = 1.0f;
    float  intensity = 1.0f;
    float  anisotropy = 0.0f;
    bool  enabled    = true;
};

#endif /* __POINTLIGHT_H__ */
