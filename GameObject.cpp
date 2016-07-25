#include <list>
#include "Globals.h"
#include "GameObject.h"

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
	for(list<GameObject*>::iterator it = childs.begin(); it != childs.end(); ++it)
		RELEASE(*it);
}
