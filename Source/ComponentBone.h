#ifndef __COMPONENT_BONE_H__
#define __COMPONENT_BONE_H__

#include "Component.h"
#include "Math.h"

class ComponentBone : public Component
{
public:
	ComponentBone(GameObject* container);
	virtual ~ComponentBone();

	void OnSave(Config& config) const;
	void OnLoad(Config* config);

	void OnDebugDraw() const override;

public:
	GameObject* attached_mesh = nullptr;
	uint num_weigths = 0;
	uint* weigth_indices = nullptr;
	float* weigths = nullptr;
	float4x4 offset;
};

#endif // __COMPONENT_BONE_H__