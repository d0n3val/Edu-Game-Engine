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
	virtual ~ComponentBone();

	void OnSave(Config& config) const;
	void OnLoad(Config* config);

	bool SetResource(UID Resource);

	void OnDebugDraw() const override;

public:
	float4x4 anim_transform = float4x4::identity;
	ComponentMesh* attached_mesh = nullptr;
};

#endif // __COMPONENT_BONE_H__