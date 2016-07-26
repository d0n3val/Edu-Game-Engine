#include "Globals.h"
#include "Application.h"
#include "ModulePhysics3D.h"
#include "Primitive.h"
#include "PhysBody3D.h"
#include "PhysVehicle3D.h"
#include "ModuleInput.h"
#include "ModuleCamera3D.h"
#include "Bullet/src/btBulletDynamicsCommon.h"
#include "Bullet\src\BulletCollision\CollisionShapes\btHeightfieldTerrainShape.h"

using namespace std;

#ifdef _DEBUG
	#pragma comment (lib, "Bullet/bin/BulletDynamics_vs2015_debug.lib")
	#pragma comment (lib, "Bullet/bin/BulletCollision_vs2015_debug.lib")
	#pragma comment (lib, "Bullet/bin/LinearMath_vs2015_debug.lib")
#else
	#pragma comment (lib, "Bullet/bin/BulletDynamics_vs2015.lib")
	#pragma comment (lib, "Bullet/bin/BulletCollision_vs2015.lib")
	#pragma comment (lib, "Bullet/bin/LinearMath_vs2015.lib")
#endif

ModulePhysics3D::ModulePhysics3D(bool start_enabled) : Module("Physics", start_enabled)
{
}

// Destructor
ModulePhysics3D::~ModulePhysics3D()
{
	RELEASE(debug_draw);
	RELEASE(solver);
	RELEASE(broad_phase);
	RELEASE(dispatcher);
	RELEASE(collision_conf);
}

// Render not available yet----------------------------------
bool ModulePhysics3D::Init(Config* config)
{
	LOG("Creating 3D Physics simulation");
	bool ret = true;

	collision_conf = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collision_conf);
	broad_phase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver;
	debug_draw = new DebugDrawer();

	return ret;
}

// ---------------------------------------------------------
bool ModulePhysics3D::Start()
{
	LOG("Creating Physics environment");

	world = new btDiscreteDynamicsWorld(dispatcher, broad_phase, solver, collision_conf);
	world->setDebugDrawer(debug_draw);
	world->setGravity(GRAVITY);
	vehicle_raycaster = new btDefaultVehicleRaycaster(world);

	return true;
}

// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody(const Cube& cube, float mass)
{
	btCollisionShape* colShape = new btBoxShape(btVector3(cube.size.x*0.5f, cube.size.y*0.5f, cube.size.z*0.5f));

	shapes.push_back(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(&cube.transform);

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);
	
	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}

// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody(const Sphere& sphere, float mass)
{
	btCollisionShape* colShape = new btSphereShape(sphere.radius);
	shapes.push_back(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(&sphere.transform);

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);

	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}

// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody(const Cylinder& cylinder, float mass)
{
	btCollisionShape* colShape = new btCylinderShapeX(btVector3(cylinder.height*0.5f, cylinder.radius*2, 0.0f));
	shapes.push_back(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(&cylinder.transform);

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);

	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}

// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody(const Plane& plane)
{
	btCollisionShape* colShape = new btStaticPlaneShape(btVector3(plane.normal.x, plane.normal.y, plane.normal.z), plane.constant);
	shapes.push_back(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(&plane.transform);

	btVector3 localInertia(0, 0, 0);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);

	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}

// ---------------------------------------------------------
PhysBody3D*	ModulePhysics3D::AddHeighField(const char* filename, int width, int length)
{
	unsigned char* heightfieldData = new unsigned char[width*length];
	{
		for(int i = 0; i<width*length; i++)
			heightfieldData[i] = 0;
	}

	FILE* heightfieldFile;
	fopen_s(&heightfieldFile, filename, "r");
	if(heightfieldFile)
	{
		int numBytes = fread(heightfieldData, 1, width*length, heightfieldFile);
		//btAssert(numBytes);
		if(!numBytes)
		{
			printf("couldn't read heightfield at %s\n", filename);
		}
		fclose(heightfieldFile);
	}

	//btScalar maxHeight = 20000.f;//exposes a bug
	btScalar maxHeight = 100;

	bool useFloatDatam = false;
	bool flipQuadEdges = false;

	int upIndex = 1;

	btHeightfieldTerrainShape* heightFieldShape = new btHeightfieldTerrainShape(width, length, heightfieldData, maxHeight, upIndex, useFloatDatam, flipQuadEdges);
	btVector3 mmin, mmax;
	heightFieldShape->getAabb(btTransform::getIdentity(), mmin, mmax);

	btCollisionShape* groundShape = heightFieldShape;

	heightFieldShape->setUseDiamondSubdivision(true);

	btVector3 localScaling(10, 1, 10);
	localScaling[upIndex] = 1.f;
	groundShape->setLocalScaling(localScaling);
	shapes.push_back(groundShape);

	//create ground object

	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(btVector3(0.f, 49.4f, 0.f));

	btVector3 localInertia(0, 0, 0);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, groundShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);

	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.push_back(pbody);

	return pbody;
}

// ---------------------------------------------------------
PhysVehicle3D* ModulePhysics3D::AddVehicle(const VehicleInfo& info)
{
	btCompoundShape* comShape = new btCompoundShape();
	shapes.push_back(comShape);

	btCollisionShape* colShape = new btBoxShape(btVector3(info.chassis_size.x*0.5f, info.chassis_size.y*0.5f, info.chassis_size.z*0.5f));
	shapes.push_back(colShape);

	btTransform trans;
	trans.setIdentity();
	trans.setOrigin(btVector3(info.chassis_offset.x, info.chassis_offset.y, info.chassis_offset.z));

	comShape->addChildShape(trans, colShape);

	btTransform startTransform;
	startTransform.setIdentity();

	btVector3 localInertia(0, 0, 0);
	comShape->calculateLocalInertia(info.mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(info.mass, myMotionState, comShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	body->setContactProcessingThreshold(BT_LARGE_FLOAT);
	body->setActivationState(DISABLE_DEACTIVATION);

	world->addRigidBody(body);
	
	btRaycastVehicle::btVehicleTuning tuning;
	tuning.m_frictionSlip = info.frictionSlip;
	tuning.m_maxSuspensionForce = info.maxSuspensionForce;
	tuning.m_maxSuspensionTravelCm = info.maxSuspensionTravelCm;
	tuning.m_suspensionCompression = info.suspensionCompression;
	tuning.m_suspensionDamping = info.suspensionDamping;
	tuning.m_suspensionStiffness = info.suspensionStiffness;

	btRaycastVehicle* vehicle = new btRaycastVehicle(tuning, body, vehicle_raycaster);

	vehicle->setCoordinateSystem(0, 1, 2);

	for(int i = 0; i < info.num_wheels; ++i)
	{
		btVector3 conn(info.wheels[i].connection.x, info.wheels[i].connection.y, info.wheels[i].connection.z);
		btVector3 dir(info.wheels[i].direction.x, info.wheels[i].direction.y, info.wheels[i].direction.z);
		btVector3 axis(info.wheels[i].axis.x, info.wheels[i].axis.y, info.wheels[i].axis.z);

		vehicle->addWheel(conn, dir, axis, info.wheels[i].suspensionRestLength, info.wheels[i].radius, tuning, info.wheels[i].front);
	}
	// ---------------------

	PhysVehicle3D* pvehicle = new PhysVehicle3D(body, vehicle, info);
	world->addVehicle(vehicle);
	vehicles.push_back(pvehicle);

	return pvehicle;
}

// ---------------------------------------------------------
void ModulePhysics3D::DeleteBody(PhysBody3D* pbody)
{
	/*
	if(pbody->body && pbody->body->getMotionState())
		RELEASE( pbody->body->getMotionState());

	world->removeCollisionObject(pbody->body);

	RELEASE( pbody->body);
	pbody->body = nullptr;

	RELEASE( pbody->collision_shape);
	pbody->collision_shape = nullptr;
	*/
	// TODO: remove from the array "bodies"
}

// ---------------------------------------------------------
update_status ModulePhysics3D::PreUpdate(float dt)
{
	// Step the physics world
	world->stepSimulation(dt, 15);

	// Detect collisions
	int numManifolds = world->getDispatcher()->getNumManifolds();
	for(int i = 0; i<numManifolds; i++)
	{
		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = (btCollisionObject*) (contactManifold->getBody0());
		btCollisionObject* obB = (btCollisionObject*) (contactManifold->getBody1());

		int numContacts = contactManifold->getNumContacts();
		if(numContacts > 0)
		{
			PhysBody3D* pbodyA = (PhysBody3D*)obA->getUserPointer();
			PhysBody3D* pbodyB = (PhysBody3D*)obB->getUserPointer();

			if(pbodyA && pbodyB)
			{
				for (list<Module*>::iterator it = pbodyA->collision_listeners.begin(); it != pbodyA->collision_listeners.end(); ++it)
					(*it)->OnCollision(pbodyA, pbodyB);

				for (list<Module*>::iterator it = pbodyB->collision_listeners.begin(); it != pbodyB->collision_listeners.end(); ++it)
					(*it)->OnCollision(pbodyB, pbodyA);
			}
		}
	}

	return UPDATE_CONTINUE;
}

// ---------------------------------------------------------
update_status ModulePhysics3D::Update(float dt)
{
	if(App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;

	if(debug == true)
	{
		world->debugDrawWorld();

		// Render vehicles
		for (list<PhysVehicle3D*>::iterator it = vehicles.begin(); it != vehicles.end(); ++it)
			(*it)->Render();

		// drop some primitives on 1,2,3
		if(App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
		{
			Sphere s(1);
			s.SetPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);
			App->physics3D->AddBody(s);
		}

		if(App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN)
		{
			Cube c(1, 1, 1);
			c.SetPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);
			App->physics3D->AddBody(c);
		}

		if(App->input->GetKey(SDL_SCANCODE_3) == KEY_DOWN)
		{
			Cylinder c(0.5, 1);
			c.SetPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);
			App->physics3D->AddBody(c);
		}
	}

	return UPDATE_CONTINUE;
}

// ---------------------------------------------------------
update_status ModulePhysics3D::PostUpdate(float dt)
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModulePhysics3D::CleanUp()
{
	LOG("Destroying 3D Physics simulation");

	// Free all the bodies ---
	for(int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		btMotionState* state;
		if(body && (state = body->getMotionState()) != nullptr)
		{
			RELEASE(state);
		}
		world->removeCollisionObject(obj);
		RELEASE(obj);
	}

	// Free all collision shapes
	for (list<btCollisionShape*>::iterator it = shapes.begin(); it != shapes.end(); ++it)
		RELEASE(*it);

	shapes.clear();
	
	for (list<PhysBody3D*>::iterator it = bodies.begin(); it != bodies.end(); ++it)
		RELEASE(*it);

	bodies.clear();

	for (list<PhysVehicle3D*>::iterator it = vehicles.begin(); it != vehicles.end(); ++it)
		RELEASE(*it);

	vehicles.clear();

	// Order matters !
	RELEASE(vehicle_raycaster);
	RELEASE(world);

	return true;
}

// =============================================
void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	line.origin.Set(from.getX(), from.getY(), from.getZ());
	line.destination.Set(to.getX(), to.getY(), to.getZ());
	line.color.Set(color.getX(), color.getY(), color.getZ());
	line.Render();
}

void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	point.transform.translate(PointOnB.getX(), PointOnB.getY(), PointOnB.getZ());
	point.color.Set(color.getX(), color.getY(), color.getZ());
	point.Render();
}

void DebugDrawer::reportErrorWarning(const char* warningString)
{
	LOG("Bullet warning: %s", warningString);
}

void DebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
	LOG("Bullet draw text: %s", textString);
}

void DebugDrawer::setDebugMode(int debugMode)
{
	mode = (DebugDrawModes) debugMode;
}

int	 DebugDrawer::getDebugMode() const
{
	return mode;
}
