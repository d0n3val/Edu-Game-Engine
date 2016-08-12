#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

// Component to hold a 3D Mesh

#include "Component.h"
#include "Math.h"

class ResourceMesh;

class ComponentMesh : public Component
{
public:
	ComponentMesh (GameObject* container);

	void OnSave(Config& config) const override;
	void OnLoad(Config* config) override;

	bool SetResource(UID resource);
	const ResourceMesh* GetResource() const;
	const AABB& GetBoundingBox() const;

private:
	UID resource;
	AABB bbox;
};

#endif // __COMPONENT_MESH_H__