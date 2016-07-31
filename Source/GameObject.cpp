#include "Globals.h"
#include "GameObject.h"
#include "Component.h"

using namespace std;

// ---------------------------------------------------------
GameObject::GameObject() : name("Unnamed"), transform(IdentityMatrix), global_transform(IdentityMatrix)
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

// ---------------------------------------------------------
vec3 GameObject::GetLocalForwardVec() const
{
	return vec3(transform.M[2], transform.M[6], transform.M[10]);
}

// ---------------------------------------------------------
vec3 GameObject::GetGlobalForwardVec() const
{
	return vec3(global_transform.M[2], global_transform.M[6], global_transform.M[10]);
}

// ---------------------------------------------------------
vec3 GameObject::GetLocalRightVec() const
{
	return vec3(transform.M[0], transform.M[4], transform.M[8]);
}

// ---------------------------------------------------------
vec3 GameObject::GetGlobalRightVec() const
{
	return vec3(global_transform.M[0], global_transform.M[4], global_transform.M[8]);
}

// ---------------------------------------------------------
vec3 GameObject::GetLocalUpVec() const
{
	return vec3(transform.M[1], transform.M[5], transform.M[9]);
}

// ---------------------------------------------------------
vec3 GameObject::GetGlobalUpVec() const
{
	return vec3(global_transform.M[1], global_transform.M[5], global_transform.M[9]);
}

// ---------------------------------------------------------
vec3 GameObject::GetLocalPosition() const
{
	return transform.translation();
}

// ---------------------------------------------------------
vec3 GameObject::GetGlobalPosition() const
{
	return global_transform.translation();
}

// ---------------------------------------------------------
const float* GameObject::GetGlobalTranform() const
{
	return &global_transform;
}

// ---------------------------------------------------------
void GameObject::RecursiveCalcGlobalTransform(const mat4x4& parent)
{
	global_transform = parent * transform;

	for(list<GameObject*>::iterator it = childs.begin(); it != childs.end(); ++it)
		(*it)->RecursiveCalcGlobalTransform(global_transform);
}