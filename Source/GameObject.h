#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include <list>
#include "glmath.h"

class Component;

class GameObject
{
public:
	GameObject();
	GameObject(const char* name);
	virtual ~GameObject();

	vec3 GetLocalForwardVec() const;
	vec3 GetGlobalForwardVec() const;

	vec3 GetLocalRightVec() const;
	vec3 GetGlobalRightVec() const;

	vec3 GetLocalUpVec() const;
	vec3 GetGlobalUpVec() const;

	vec3 GetLocalPosition() const;
	vec3 GetGlobalPosition() const;

	const float* GetGlobalTranform() const;
	void RecursiveCalcGlobalTransform(const mat4x4& parent = IdentityMatrix);

public:
	std::string name;
	mat4x4 transform;
	std::list<Component*> components;
	std::list<GameObject*> childs;

private:
	mat4x4 global_transform;
};

#endif // __GAMEOBJECT_H__