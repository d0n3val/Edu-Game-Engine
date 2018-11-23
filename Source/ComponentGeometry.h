#ifndef __COMPONENT_GEOMETRY_H__
#define __COMPONENT_GEOMETRY_H__

#include "ComponentWithResource.h"
#include "Component.h"

class ComponentGeometry : public Component, public ComponentWithResource
{
public:
    explicit ComponentGeometry(GameObject* go);
    ~ComponentGeometry();

	virtual bool SetResource    (UID uid) override;
    virtual void OnSave         (Config& config) const override;
    virtual void OnLoad         (Config* config) override;

	void         GetBoundingBox (AABB& box) const override;

public:
    std::vector<UID> meshes;
};

#endif /* __COMPONENT_GEOMETRY_H__ */
