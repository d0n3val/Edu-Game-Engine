#ifndef __COMPONENT_RIGID_BODY_H__
#define __COMPONENT_RIGID_BODY_H__

// Component to allow another mesh to deform based on a skeleton 

#include "Component.h"
#include "Math.h"
#include "Bullet/include/LinearMath/btMotionState.h"
#include "Bullet/include/LinearMath/btTransform.h"

class btRigidBody;

class ComponentRigidBody : public Component, public btMotionState
{
	friend class ModulePhysics3D;
public:
	enum BodyType
	{
		body_sphere,
		body_box,
		body_capsule,
		body_unknown
	};

	enum BodyBehaviour
	{
		fixed,
		dynamic,
		kinematic
	};

public:
	ComponentRigidBody (GameObject* container);
	~ComponentRigidBody () override;

	void OnSave(Config& config) const override;
	void OnLoad(Config* config) override;

	void OnStart() override;
	void OnDebugDraw() const override;

	// from btMotionState
	void getWorldTransform(btTransform& worldTrans ) const override;
	void setWorldTransform(const btTransform& worldTrans) override;

	void SetBodyType(BodyType new_type);
	BodyType GetBodyType() const;

	void SetBehaviour(BodyBehaviour new_behaviour);
	BodyBehaviour GetBehaviour() const;

	void DrawEditor();

private:
	void ResetShapes();
	void CreateBody();

private:

	BodyBehaviour behaviour = BodyBehaviour::dynamic;
	BodyType body_type = BodyType::body_sphere;

	Sphere sphere;
	OBB box;
	Capsule capsule;

	btRigidBody* body = nullptr;
	float mass = 1.0f;
};

#endif // __COMPONENT_RIGID_BODY_H__