#include <list>
#include "Globals.h"
#include "GameObject.h"

using namespace std;

// ---------------------------------------------------------
GameObject::GameObject()
{
	transform = IdentityMatrix;
}

// ---------------------------------------------------------
GameObject::~GameObject()
{
	for(list<GameObject*>::iterator it = childs.begin(); it != childs.end(); ++it)
		RELEASE(*it);
}
