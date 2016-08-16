#ifndef __COMPONENT_SKELETON_H__
#define __COMPONENT_SKELETON_H__

// Component to allow another mesh to deform based on a skeleton 

#include "Component.h"
#include "Math.h"

class ComponentMesh;

class ComponentSkeleton : public Component
{
public:
	ComponentSkeleton (GameObject* container);

	void OnSave(Config& config) const override;
	void OnLoad(Config* config) override;

	ComponentMesh* FindMesh() const;

private:
};

#endif // __COMPONENT_SKELETON_H__