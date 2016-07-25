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

public:
	std::string name;
	mat4x4 transform;
	std::list<Component*> components;
	std::list<GameObject*> childs;
};

#endif // __GAMEOBJECT_H__