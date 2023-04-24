#pragma once

#include "IBLData.h"

#include <memory>

class TextureCube;
class Config;

class LocalIBLLight 
{
public:

    LocalIBLLight();
    ~LocalIBLLight();

    void Save (Config& config) const ;
    void Load (Config& config);

    void generate();

    const float3& GetPosition() const {return position;}
    void SetPosition(const float3& pos) { position = pos; }

    float GetRadius() const {return radius;}
    void SetRadius(float rad) { radius = rad; }

    const AABB& GetAABB() const {return box;}
    void SetAABB(const AABB& bounding) {box = bounding; }

    bool GetEnabled() const {return enabled;}
    void SetEnabled(bool e) { enabled = e;}

    IBLData& getIBLData() { return iblData; }
    const IBLData& getIBLData() const { return iblData; }


private:

    typedef std::unique_ptr<TextureCube> TextureCubePtr;

    float3 position = float3::zero;
    Quat   rotation = Quat::identity;
    AABB   box      = AABB(float3::zero, float3::zero);
    float  radius   = 0.0f;

    IBLData        iblData;
    bool           enabled = true;
};