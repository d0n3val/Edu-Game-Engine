#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include <list>
#include "glmath.h"

class Component;

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

public:
	mat4x4 transform;
	std::list<Component*> components;
	std::list<GameObject*> childs;
};

#endif // __GAMEOBJECT_H__