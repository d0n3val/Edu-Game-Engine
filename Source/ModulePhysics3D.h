#ifndef __MODULE_PHYSICS_3D_H__
#define __MODULE_PHYSICS_3D_H__

#include "Module.h"
#include "Globals.h"
#include <list>
#include "Primitive.h"
#include "Bullet/include/btBulletDynamicsCommon.h"

// Recommended scale is 1.0f == 1 meter, no less than 0.2 objects
#define GRAVITY btVector3(0.0f, -10.0f, 0.0f) 

class DebugDrawer;
struct PhysBody3D;
struct PhysVehicle3D;
struct VehicleInfo;

class ModulePhysics3D : public Module
{
public:
	ModulePhysics3D(bool start_enabled = true);
	~ModulePhysics3D();

	bool Init(Config* config = nullptr) override;
	bool Start(Config* config = nullptr) override;
	update_status PreUpdate(float dt) override;
	update_status Update(float dt) override;
	update_status PostUpdate(float dt) override;
	bool CleanUp();

	PhysBody3D*		AddBody(const Cube& cube, float mass = 1.0f);
	PhysBody3D*		AddBody(const PSphere& sphere, float mass = 1.0f);
	PhysBody3D*		AddBody(const PCylinder& cylinder, float mass = 1.0f);
	PhysBody3D*		AddBody(const PPlane& plane);
	PhysBody3D*		AddHeighField(const char* filename, int width, int height);
	PhysVehicle3D*	AddVehicle(const VehicleInfo& info);

	void DeleteBody(PhysBody3D* body);

private:

	bool debug = false;

	btDefaultCollisionConfiguration*	collision_conf = nullptr;
	btCollisionDispatcher*				dispatcher = nullptr;
	btBroadphaseInterface*				broad_phase = nullptr;
	btSequentialImpulseConstraintSolver* solver = nullptr;
	btDiscreteDynamicsWorld*			world = nullptr;
	btDefaultVehicleRaycaster*			vehicle_raycaster = nullptr;
	DebugDrawer*						debug_draw = nullptr;

	std::list<btCollisionShape*> shapes;
	std::list<PhysBody3D*> bodies;
	std::list<PhysVehicle3D*> vehicles;
};

class DebugDrawer : public btIDebugDraw
{
public:
	DebugDrawer() : line(0,0,0)
	{}

	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	void reportErrorWarning(const char* warningString);
	void draw3dText(const btVector3& location, const char* textString);
	void setDebugMode(int debugMode);
	int	 getDebugMode() const;

	DebugDrawModes mode;
	Line line;
	Primitive point;
};

#endif __MODULE_PHYSICS_3D_H__