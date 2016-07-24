#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include <list>
#include "glmath.h"

class Component;

class GameObject
{
public:
	GameObject();
	~GameObject();

public:
	mat4x4 transform;
	std::list<Component*> components;
};

#endif // __GAMEOBJECT_H__