#ifndef __DIRLIGHT_H__
#define __DIRLIGHT_H__

#include "Math.h"
#include "Config.h"

class DirLight 
{
public:

public:

    DirLight();
    ~DirLight();

    void OnSave(Config& config) const;
    void OnLoad(Config* config);

    const float3&   GetColor    () const { return color; }
    void            SetColor    (const float3& c) { color = c; }

    float           GetPolar    () const { return polar; }
    void            SetPolar    (float p) { polar = p; }
    float           GetAzimuthal() const { return azimuthal; }
    void            SetAzimuthal(float a) { azimuthal = a; }

    float3          GetDir      () const;

public:

    float3 color     = math::float3::one;
    float  polar     = 0.0f;
    float  azimuthal = 0.0f;
};

#endif /* __DIRLIGHT_H__ */


