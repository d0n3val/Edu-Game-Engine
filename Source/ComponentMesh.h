#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

#include "ComponentWithResource.h"
#include "Component.h"

class ResourceMesh;

class ComponentMesh : public Component
{
public:
    explicit ComponentMesh(GameObject* go);
    ~ComponentMesh();

	bool                SetResource     (UID uid);
    const ResourceMesh* GetResource     () const;
    virtual void        OnSave          (Config& config) const override;
    virtual void        OnLoad          (Config* config) override;

	void                GetBoundingBox  (AABB& box) const override;

    static Types        GetClassType    () { return Mesh; }

private:
	UID resource = 0;
};

#endif /* __COMPONENT_MESH_H__ */
