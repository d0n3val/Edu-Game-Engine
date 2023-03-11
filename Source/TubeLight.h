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

    float3          GetPosition0    () const;
    float3          GetPosition1    () const;

    float3          GetPosition     () const {return position;}
    void            SetPosition     (const float3& pos) { position = pos; }

    float           GetHeight       () const {return height;}
    void            SetHeight       (float h) {height = h;}

    float           GetRadius       () const { return radius; }
    void            SetRadius       (float r)  { radius = r; }

    float           GetAttRadius () const {return attRadius; }
    void            SetAttRadius (float attR) { attRadius = attR; }


    bool            GetEnabled      () const { return enabled; }
    void            SetEnabled      (bool e) { enabled = e; }

    const Quat&     GetRotation     () const { return rotation;}
    void            SetRotation     (const Quat& q) { rotation = q;}


private:
    float3 position = float3::zero;
    Quat rotation = Quat::identity;
    float height = 1.0f;
    float3 colour = float3::zero;
    float intensity = 0.0f;
    float radius = 0.0f;
    float attRadius = 0.0f;
    bool enabled = true;
};
