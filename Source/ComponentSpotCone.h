#pragma once

#include "Component.h"
#include "Timer.h"

class ResourceMesh;
class ResourceTexture;

class ComponentSpotCone : public Component
{
public:
    ComponentSpotCone(GameObject* go);
    ~ComponentSpotCone();

	void OnPlay   () override;
	void OnStop   () override;
	void OnUpdate (float dt) override;

    void OnSave   (Config& config) const override;
    void OnLoad   (Config* config) override;

	void GetBoundingBox(AABB& box) const override;

    const ResourceMesh* getMeshRes () const;

    float getHeight() const {return height;}
    float getRadius() const {return radius;}

    void setHeight(float newHeight);
    void setRadius(float newRadius);

    void setFog0(UID texture);
    void setFog1(UID texture);

    UID getFog0() const {return fog0; }
    UID getFog1() const {return fog1; }

    const ResourceTexture* getFog0Res() const;
    const ResourceTexture* getFog1Res() const;

    ResourceTexture* getFog0Res();
    ResourceTexture* getFog1Res();

    void setFog0Tiling(const float2& tiling) { fog0Tiling = tiling; }
    void setFog0Offset(const float2& offset) { fog0Offset = offset; }
    void setFog0Speed(const float2& speed) { fog0Speed = speed; }

    const float2& getFog0Tiling() const {return fog0Tiling; }
    const float2& getFog0Offset() const {return fog0Offset; }
    const float2& getFog0Speed() const {return fog0Speed; }

    void setFog1Tiling(const float2& tiling) { fog1Tiling = tiling; }
    void setFog1Offset(const float2& offset) { fog1Offset = offset; }
    void setFog1Speed(const float2& speed) { fog1Speed = speed; }

    const float2& getFog1Tiling() const {return fog1Tiling; }
    const float2& getFog1Offset() const {return fog1Offset; }
    const float2& getFog1Speed() const {return fog1Speed; }

    float getTime() const {return float(timer.Read())*0.001f;}

    float4 getColour() const {return colour;}
    void setColour(const float4 colourIntensity) { colour = colourIntensity; }

    float getTransparency() const {return transparency;}
    float getSmoothAmount() const {return smoothAmount;}
    float getFresnelAmount() const {return fresnelAmount;}

    void setTransparency(float value) { transparency = value;}
    void setSmoothAmount(float value) { smoothAmount = value;}
    void setFresnelAmount(float value) { fresnelAmount = value;}

private:

    void setCone(UID uid);
    void reCreateMesh();

private:

    UID cone = 0; 
    UID fog0 = 0;
    UID fog1 = 0;

    float4 colour = float4::one;

    float2 fog0Tiling = float2::one;
    float2 fog0Offset = float2::zero;
    float2 fog0Speed = float2::zero;

    float2 fog1Tiling = float2::one;
    float2 fog1Offset = float2::zero;
    float2 fog1Speed = float2::zero;

    float transparency = 1.0f;
    float smoothAmount = 1.0f;
    float fresnelAmount = 1.0f;

    float height = 1.0f;
    float radius = 0.2f;
    Timer timer;

};