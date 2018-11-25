#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

#include "ComponentWithResource.h"
#include "Component.h"

class ComponentMesh : public Component, public ComponentWithResource
{
public:
    explicit ComponentMesh(GameObject* go);
    ~ComponentMesh();

	virtual bool SetResource    (UID uid) override;
    virtual void OnSave         (Config& config) const override;
    virtual void OnLoad         (Config* config) override;

	void         GetBoundingBox (AABB& box) const override;

};

#endif /* __COMPONENT_MESH_H__ */
