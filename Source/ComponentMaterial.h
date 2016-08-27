#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

// Component to hold a material data (diffuse, etc...)

#include "Component.h"
#include "ComponentWithResource.h"

struct TextureInfo;
class ResourceTexture;

class ComponentMaterial : public Component, public ComponentWithResource
{
public:
	ALIGN_CLASS_TO_16

	ComponentMaterial (GameObject* container);
	~ComponentMaterial ();

	void OnSave(Config& config) const override;
	void OnLoad(Config* config) override;
	bool SetResource(UID resource) override;

public:
	float alpha_test = 0.5f;
	float4x4 tex_transform = float4x4::identity;

};

#endif // __COMPONENT_MESH_H__