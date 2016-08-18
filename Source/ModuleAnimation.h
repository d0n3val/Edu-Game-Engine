#ifndef __MODULEANIMATION_H__
#define __MODULEANIMATION_H__

#include "Module.h"

class GameObject;
class ComponentBone;
class ComponentMesh;
class ComponentAnimation;

class ModuleAnimation : public Module
{
public:

	ModuleAnimation(bool start_active = true);
	~ModuleAnimation();

	bool Init(Config* config) override;
	bool CleanUp() override;

	update_status Update(float dt) override;
	void RecursiveUpdateAnimation(GameObject* go, float dt);
	void RecursiveDeformMeshes(GameObject * go);
	void RecursiveResetMeshes(GameObject * go);

private:
	void UpdateAnimation(ComponentAnimation* anim, float dt);
	bool AdvanceAnimation(ComponentAnimation* anim, ComponentMesh* mesh, float dt);
	void DeformMesh(const ComponentBone* bone);
};

#endif // __MODULEANIMATION_H__