#pragma once

#include "Component.h"

class ResourceMesh;

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


private:

    void setCone(UID uid);
    void reCreateMesh();

private:

    UID cone = 0; 
    float height = 1.0f;
    float radius = 0.2f;

};