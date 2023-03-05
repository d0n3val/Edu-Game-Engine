#pragma once

#include "Math.h"

class Config;

class QuadLight
{
public:

    QuadLight();
    ~QuadLight();

    void Save  (Config& config) const;
    void Load  (Config& config);

    const float3&   GetColor        () const { return color; }
    void            SetColor        (const float3& c) { color = c; }

    float3          GetPosition     () const { return position; }
    void            SetPosition     (const float3& p) { position = p; }

    float3          GetUp           () const { return up; }
    void            SetUp           (const float3& p) { up = p; }

    float3          GetRight        () const { return right; }
    void            SetRight        (const float3& p) { right = p; }

    float2          GetSize         () const { return size; }
    void            SetSize         (const float2& p) { size = p; }

    float           GetIntensity () const { return intensity; }
    void            SetIntensity (float i) { intensity = i;}

    bool            GetEnabled      () const { return enabled; }
    void            SetEnabled      (bool e) { enabled = e; }

private:

    float3 color     = float3::one;
    float3 position  = float3::zero;
    float3 up        = float3::unitY;
    float3 right     = float3::unitX;
    float2 size      = float2::one;

    float  intensity = 1.0f;
    bool  enabled    = true;
};