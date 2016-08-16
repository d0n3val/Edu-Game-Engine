#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

// Component to hold a 3D Mesh

#include "Component.h"
#include "ComponentWithResource.h"
#include "Math.h"

class ComponentBone;
class ResourceMesh;

class ComponentMesh : public Component, public ComponentWithResource
{
public:
	ComponentMesh (GameObject* container);
	~ComponentMesh();

	void OnSave(Config& config) const override;
	void OnLoad(Config* config) override;
	bool SetResource(UID resource) override;

	const AABB& GetBoundingBox() const;
	uint CountPotentialBones() const;
	void AttachBones();
	void DetachBones();
	uint CountAttachedBones() const;
	void ResetDeformableMesh();

private:
	void RecursiveFindBones(const GameObject* go, std::vector<ComponentBone*>& found) const;

public:
	std::vector<ComponentBone*> attached_bones;
	ResourceMesh* deformable = nullptr;
};

#endif // __COMPONENT_MESH_H__