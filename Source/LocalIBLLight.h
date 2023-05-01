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

    const Quat& GetRotation() const {return rotation; }
    void SetRotation(const Quat& rot) { rotation = rot; }

    float4x4 GetTransform() const { return float4x4(rotation, position); }

    const AABB& GetParallaxAABB() const {return parallax;}
    void SetParallaxAABB(const AABB& bounding) {parallax = bounding; }

    const AABB& GetInfluenceAABB() const {return influence;}
    void SetInfluenceAABB(const AABB& bounding) {influence = bounding;}

    float GetFarPlane() const {return farPlane; }
    void SetFarPlane(float plane) {farPlane = plane; }

    bool GetEnabled() const {return enabled;}
    void SetEnabled(bool e) { enabled = e;}

    IBLData& getIBLData() { return iblData; }
    const IBLData& getIBLData() const { return iblData; }

    uint GetResolution() const {return resolution;}
    void SetResolution(uint res) { resolution = res;}

    uint GetNumSamples() const {return numSamples;}
    void SetNumSamples(uint samples) {numSamples = samples;}

    uint GetNumRoughnessLevels() const {return roughnessLevels;}
    void SetNumRoughnessLevels(uint levels){ roughnessLevels = levels;}

private:

    typedef std::unique_ptr<TextureCube> TextureCubePtr;

    float3 position = float3::zero;
    Quat   rotation = Quat::identity;
    AABB   parallax = AABB(float3::zero, float3::zero);
    AABB   influence = AABB(float3::zero, float3::zero);
    float  farPlane = 200.0f;
    uint   resolution = 512;
    uint   numSamples = 2048;
    uint   roughnessLevels = 8;

    IBLData        iblData;
    bool           enabled = true;
};