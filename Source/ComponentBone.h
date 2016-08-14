#ifndef __COMPONENT_BONE_H__
#define __COMPONENT_BONE_H__

#include "Component.h"
#include "ComponentWithResource.h"
#include "Math.h"

class ComponentBone : public Component, public ComponentWithResource
{
public:
	ComponentBone(GameObject* container);
	virtual ~ComponentBone();

	void OnSave(Config& config) const;
	void OnLoad(Config* config);

	bool SetResource(UID Resource);

	void OnDebugDraw() const override;

public:
	GameObject* attached_mesh = nullptr;
};

#endif // __COMPONENT_BONE_H__