#ifndef __COMPONENT_MESH_H__
#define __COMPONENT_MESH_H__

// Component to hold a 3D Mesh

#include "Component.h"

struct mesh;

class ComponentMesh : public Component
{
public:
	ComponentMesh (GameObject* container);
	~ComponentMesh ();

	void OnActivate() override;
	void OnDeActivate() override;

	void OnStart() override;
	void OnUpdate() override;
	void OnFinish() override;

public:
	const mesh* mesh_data = nullptr;
};

#endif // __COMPONENT_MESH_H__