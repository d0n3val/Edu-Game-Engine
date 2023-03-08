#pragma once

#include "Math.h"

class Config;


class TubeLight
{
public:
    TubeLight();
    ~TubeLight();
    
    void Save  (Config& config) const;
    void Load  (Config& config);


    const float3&   GetColor        () const { return colour; }
    void            SetColor        (const float3& c) { colour = c; }

    float           GetIntensity    () const { return intensity; }
    void            SetIntensity    (float i) { intensity = i;}

    float3          GetPosition0     () const { return position0; }
    void            SetPosition0     (const float3& p) { position0 = p; }

    float3          GetPosition1     () const { return position1; }
    void            SetPosition1     (const float3& p) { position1 = p; }

    float           GetRadius       () const { return radius; }
    void            SetRadius       (float r)  { radius = r; }

    bool            GetEnabled      () const { return enabled; }
    void            SetEnabled      (bool e) { enabled = e; }

private:
    float3 position0 = float3::zero;
    float3 position1 = float3::zero;
    float3 colour = float3::zero;
    float  intensity = 0.0f;
    float  radius = 0.0f;
    bool  enabled       = true;
};
