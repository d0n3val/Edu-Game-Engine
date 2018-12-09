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

    float           GetConstantAtt  () const { return constant; }
    void            SetConstantAtt  (float att) { constant = att; }

    float           GetLinearAtt    () const { return linear; }
    void            SetLinearAtt    (float att) { linear = att; }

    float           GetQuadricAtt   () const { return quadric; }
    void            SetQuadricAtt   (float att) { quadric = att; }
private:

    float3 color     = float3::one;
    float3 position  = float3::zero;
    float  constant  = 1.0f;
    float  linear    = 0.1f;
    float  quadric   = 0.05f;
};

#endif /* __POINTLIGHT_H__ */
