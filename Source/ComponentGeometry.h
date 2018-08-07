#ifndef __COMPONENT_GEOMETRY_H__
#define __COMPONENT_GEOMETRY_H__

#include "Component.h"

class ComponentGeometry : public Component
{
public:
    ComponentGeometry();
    ~ComponentGeometry();

    void Initialize(const std::vector<UID>& m);

    virtual void OnSave(Config& config) const override;
    virtual void OnLoad(Config* config) override;

private:
    std::vector<UID> meshes;
};

#endif /* __COMPONENT_GEOMETRY_H__ */
