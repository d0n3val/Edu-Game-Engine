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

    float           GetConstantAtt  () const { return constant; }
    void            SetConstantAtt  (float att) { constant = att; }

    float           GetLinearAtt    () const { return linear; }
    void            SetLinearAtt    (float att) { linear = att; }

    float           GetQuadricAtt   () const { return quadric; }
    void            SetQuadricAtt   (float att) { quadric = att; }

    bool            GetEnabled      () const { return enabled; }
    void            SetEnabled      (bool e) { enabled = e; }

private:

    float3 color     = float3::one;
    float3 position  = float3::zero;
    float3 direction = -float3::unitY;
    float  inner     = 0.0f;
    float  outter    = 0.0f;
    float  constant  = 1.0f;
    float  linear    = 0.0f;
    float  quadric   = 0.0f;
    bool   enabled   = true;
};

#endif /* __SPOTLIGHT_H__ */
