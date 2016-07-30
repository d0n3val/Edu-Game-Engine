#include "Globals.h"
#include "GameObject.h"
#include "Component.h"

using namespace std;

// ---------------------------------------------------------
GameObject::GameObject() : name("Unnamed"), transform(IdentityMatrix)
{
}

// ---------------------------------------------------------
GameObject::GameObject(const char* name) : name(name), transform(IdentityMatrix)
{
}

// ---------------------------------------------------------
GameObject::~GameObject()
{
	for(list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		RELEASE(*it);

	for(list<GameObject*>::iterator it = childs.begin(); it != childs.end(); ++it)
		RELEASE(*it);
}
