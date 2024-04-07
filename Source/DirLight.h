#ifndef __DIRLIGHT_H__
#define __DIRLIGHT_H__

#include "Math.h"

class Config;

class DirLight 
{
public:

    DirLight();
    ~DirLight();

    void            Save        (Config& config) const;
    void            Load        (Config& config);

    const float3&   GetColor    () const { return color; }
    void            SetColor    (const float3& c) { color = c; }

    float           GetPolar    () const { return polar; }
    void            SetPolar    (float p) { polar = p; }
    float           GetAzimuthal() const { return azimuthal; }
    void            SetAzimuthal(float a) { azimuthal = a; }

    float3          GetDir      () const;
    float3          GetUp       () const;

    float           GetIntensity     () const { return intensity; }
    void            SetIntensity     (float i) { intensity = i; }

    float           GetAnisotropy () const {return anisotropy;}
    void            SetAnisotropy (float value) {anisotropy = value;}

private:

    float3 color     = math::float3::one;
    float  polar     = PI/2.0f;
    float  azimuthal = PI/2.0f;
    float  intensity = 1.0f;
    float  anisotropy = 0.0f;
};

#endif /* __DIRLIGHT_H__ */


