#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

// Component to hold a material data (diffuse, etc...)

#include "Component.h"

class ComponentMaterial : public Component
{
public:
	ComponentMaterial (GameObject* container);
	~ComponentMaterial ();

	void OnSave(Config* config) const override;
	void OnLoad(Config* config) override;

public:
	uint material_id = 0;
};

#endif // __COMPONENT_MESH_H__