#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

// Component to hold a material data (diffuse, etc...)

#include "Component.h"

struct TextureInfo;
class ResourceTexture;

class ComponentMaterial : public Component
{
public:
	ComponentMaterial (GameObject* container);
	~ComponentMaterial ();

	void OnSave(Config& config) const override;
	void OnLoad(Config* config) override;
	bool SetResource(UID resource);
	const ResourceTexture* GetResource() const;

public:
	UID resource = 0;
	const TextureInfo* texture = nullptr;
};

#endif // __COMPONENT_MESH_H__