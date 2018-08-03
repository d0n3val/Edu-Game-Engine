#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

// Component to hold a 3D Mesh

#include "Component.h"
#include "ComponentWithResource.h"
#include "Math.h"
#include "Color.h"

class ComponentBone;
class ComponentMaterial;
class ResourceMesh;

class ComponentMesh : public Component, public ComponentWithResource
{
public:
	ComponentMesh (GameObject* container);
	~ComponentMesh() override;

	void OnSave(Config& config) const override;
	void OnLoad(Config* config) override;
	bool SetResource(UID resource) override;
	void OnStart() override;
	void OnGoDestroyed() override;

	void GetBoundingBox(AABB& box) const override;

    uint GetMaterialIndex() const;
	uint CountPotentialBones() const;
	void AttachBones(const GameObject* go);
	void DetachBones();
	uint CountAttachedBones() const;
	void ResetDeformableMesh();

private:
	void RecursiveFindBones(const GameObject* go, std::vector<ComponentBone*>& found) const;

public:
    uint material_index;
	std::vector<ComponentBone*> attached_bones;
	ResourceMesh* deformable = nullptr;
	uint root_bones_uid = 0;
	const GameObject* root_bones = nullptr;
	Color tint = White;
};

// ---------------------------------------------------------
inline uint ComponentMesh::GetMaterialIndex() const
{
    return material_index;
}

#endif // __COMPONENT_MESH_H__
