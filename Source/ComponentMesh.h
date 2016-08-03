#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

// Component to hold a 3D Mesh

#include "Component.h"
#include "Math.h"

struct Mesh;

class ComponentMesh : public Component
{
public:
	ComponentMesh (GameObject* container);

	void SetMesh(const Mesh* data);
	const Mesh* GetMesh() const;
	const AABB* GetBoundingBox() const;

private:
	const Mesh* mesh_data = nullptr;
	math::AABB	bounding_box;
};

#endif // __COMPONENT_MESH_H__