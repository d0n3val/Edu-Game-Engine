#ifndef __COMPONENT_GEOMETRY_H__
#define __COMPONENT_GEOMETRY_H__

#include "Component.h"

class ComponentGeometry : public Component
{
public:
    ComponentGeometry();
    ~ComponentGeometry();

    void Initialize(const UID* ids, const unsigned* mesh_indices, unsigned count);

    virtual void OnSave(Config& config) const override;
    virtual void OnLoad(Config* config) override;
	
public:
    std::vector<UID> meshes;
};

#endif /* __COMPONENT_GEOMETRY_H__ */
