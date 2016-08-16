#ifndef __MODULEANIMATION_H__
#define __MODULEANIMATION_H__

#include "Module.h"

class GameObject;
class ComponentBone;
class ComponentMesh;

class ModuleAnimation : public Module
{
public:

	ModuleAnimation(bool start_active = true);
	~ModuleAnimation();

	bool Init(Config* config) override;
	bool CleanUp() override;

	update_status Update(float dt) override;
	void RecursiveDeformMeshes(GameObject * go);
	void RecursiveResetMeshes(GameObject * go);

private:
	void DeformMesh(const ComponentBone* bone);
};

#endif // __MODULEANIMATION_H__