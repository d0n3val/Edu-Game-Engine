#ifndef __PHYSBODY3D_H__
#define __PHYSBODY3D_H__

#include <list>

class btRigidBody;
class Module;

struct PhysBody3D
{
public:
	PhysBody3D(btRigidBody* body);
	~PhysBody3D();

	void GetTransform(float* matrix) const;
	void SetTransform(float* matrix) const;
	void SetPos(float x, float y, float z);

private:
	btRigidBody* body;

public:
	std::list<Module*> collision_listeners;
};
#endif // __PHYSBODY3D_H__
