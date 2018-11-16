#ifndef __COMPONENT_GEOMETRY_H__
#define __COMPONENT_GEOMETRY_H__

#include "Component.h"

class ComponentGeometry : public Component
{
public:
    explicit ComponentGeometry(GameObject* go);
    ~ComponentGeometry();

    void         Initialize     (const UID* ids, const unsigned* mesh_indices, unsigned count);

    virtual void OnSave         (Config& config) const override;
    virtual void OnLoad         (Config* config) override;

	void         GetBoundingBox (AABB& box) const override;

public:
    std::vector<UID> meshes;
};

#endif /* __COMPONENT_GEOMETRY_H__ */
