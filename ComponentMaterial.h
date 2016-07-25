#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

// Component to hold a material data (diffuse, etc...)

#include "Component.h"

class ComponentMaterial : public Component
{
public:
	ComponentMaterial (GameObject* container);
	~ComponentMaterial ();

	void OnActivate() override;
	void OnDeActivate() override;

	void OnStart() override;
	void OnUpdate() override;
	void OnFinish() override;

public:
	uint material_id = 0;
};

#endif // __COMPONENT_MESH_H__