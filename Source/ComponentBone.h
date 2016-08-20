#ifndef __COMPONENT_BONE_H__
#define __COMPONENT_BONE_H__

#include "Component.h"
#include "ComponentWithResource.h"
#include "Math.h"

class ComponentMesh;

class ComponentBone : public Component, public ComponentWithResource
{
public:
	ALIGN_CLASS_TO_16

	ComponentBone(GameObject* container);
	~ComponentBone() override;

	void OnSave(Config& config) const override;
	void OnLoad(Config* config) override;


	bool SetResource(UID Resource);

	void OnDebugDraw() const override;

public:
	float4x4 anim_transform = float4x4::identity;
	ComponentMesh* attached_mesh = nullptr;
	bool translation_locked = false;
};

#endif // __COMPONENT_BONE_H__