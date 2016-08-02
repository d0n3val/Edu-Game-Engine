#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include <list>
#include "Math.h"

enum ComponentTypes;
class Component;

class GameObject
{
public:
	GameObject(const aiMatrix4x4& transformation);
	GameObject(const char* name, const aiMatrix4x4& transformation);
	virtual ~GameObject();

	void AddChild(GameObject* go);
	Component* CreateComponent(ComponentTypes type);

	aiVector3D GetLocalForwardVec() const;
	aiVector3D GetGlobalForwardVec() const;

	aiVector3D GetLocalRightVec() const;
	aiVector3D GetGlobalRightVec() const;

	aiVector3D GetLocalUpVec() const;
	aiVector3D GetGlobalUpVec() const;

	aiVector3D GetLocalPosition() const;
	aiVector3D GetGlobalPosition() const;

	const aiMatrix4x4 GetLocalTransformation() const;
	const aiMatrix4x4 GetGlobalTransformation() const;

	const float* GetOpenGLGlobalTranform() const;

	void RecursiveCalcGlobalTransform(const aiMatrix4x4& parent);

	bool IsActive() const;
	void SetActive(bool active);

public:
	std::string name;
	aiVector3D extra_translation;
	aiVector3D extra_rotation;
	aiVector3D extra_scale;
	std::list<GameObject*> childs;
	std::list<Component*> components;

private:
	aiMatrix4x4 local_trans;
	aiMatrix4x4 global_trans;
	bool active = true;
};

#endif // __GAMEOBJECT_H__