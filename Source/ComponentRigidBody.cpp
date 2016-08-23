#include "Globals.h"
#include "Application.h"
#include "ComponentRigidBody.h"
#include "GameObject.h"
#include "Component.h"
#include "ModulePhysics3D.h"
#include "DebugDraw.h"
#include "Bullet/include/btBulletDynamicsCommon.h"
#include "Imgui/imgui.h"

using namespace std;

// ---------------------------------------------------------
ComponentRigidBody::ComponentRigidBody(GameObject* container) : Component(container, Types::RigidBody)
{
	sphere.r = 1.0f;
	sphere.pos = float3::zero;

	box.r = float3::one;
	box.pos = float3::zero;
	box.axis[0] = float3::unitX;
	box.axis[1] = float3::unitY;
	box.axis[2] = float3::unitZ;

	capsule.r = 1.0f;
	capsule.l = LineSegment(float3::zero, float3::one);
}

// ---------------------------------------------------------
ComponentRigidBody::~ComponentRigidBody()
{
	if (body != nullptr && App->physics3D != nullptr)
		App->physics3D->DeleteBody(body);
}

// ---------------------------------------------------------
void ComponentRigidBody::GetBoundingBox(AABB & box) const
{
	switch (body_type)
	{
		case body_sphere:
			box.Enclose(sphere);
		break;
		case body_box:
			box.Enclose(this->box);
		break;
		case body_capsule:
			box.Enclose(capsule);
		break;
	}
}

// ---------------------------------------------------------
void ComponentRigidBody::OnSave(Config& config) const
{
	config.AddInt("Behaviour", behaviour);
	config.AddInt("Body Type", body_type);
	config.AddFloat("Mass", mass);
	config.AddArrayFloat("Sphere", &sphere.pos.x, 4);
	config.AddArrayFloat("Box", &box.pos.x, 6);
	config.AddArrayFloat("Capsule", &capsule.l.a.x, 7);
	config.AddArrayFloat("Linear Factor", &linear_factor.x, 3);
	config.AddArrayFloat("Angular Factor", &angular_factor.x, 3);
	config.AddFloat("Restitution", restitution);
}

// ---------------------------------------------------------
void ComponentRigidBody::OnLoad(Config * config)
{
	behaviour = (BodyBehaviour) config->GetInt("Behaviour", BodyBehaviour::fixed);
	body_type = (BodyType) config->GetInt("Body Type", BodyType::body_sphere);
	mass = config->GetFloat("Mass", 1.0f);

	sphere.pos.x = config->GetFloat("Sphere", 0.f, 0);
	sphere.pos.y = config->GetFloat("Sphere", 0.f, 1);
	sphere.pos.z = config->GetFloat("Sphere", 0.f, 2);
	sphere.r = config->GetFloat("Sphere", 1.f, 3);

	capsule.l.a.x = config->GetFloat("Capsule", 0.f, 0);
	capsule.l.a.y = config->GetFloat("Capsule", 0.f, 1);
	capsule.l.a.z = config->GetFloat("Capsule", 0.f, 2);
	capsule.l.b.x = config->GetFloat("Capsule", 0.f, 3);
	capsule.l.b.y = config->GetFloat("Capsule", 1.f, 4);
	capsule.l.b.z = config->GetFloat("Capsule", 0.f, 5);
	capsule.r = config->GetFloat("Capsule", 1.f, 6);

	box.pos.x = config->GetFloat("Box", 0.f, 0);
	box.pos.y = config->GetFloat("Box", 0.f, 1);
	box.pos.z = config->GetFloat("Box", 0.f, 2);
	box.r.x = config->GetFloat("Box", 1.f, 3);
	box.r.y = config->GetFloat("Box", 1.f, 4);
	box.r.z = config->GetFloat("Box", 1.f, 5);

	restitution = config->GetFloat("Restitution", 1.0f);

	linear_factor.x = config->GetFloat("Linear Factor", 1.f, 0);
	linear_factor.y = config->GetFloat("Linear Factor", 1.f, 1);
	linear_factor.z = config->GetFloat("Linear Factor", 1.f, 2);

	angular_factor.x = config->GetFloat("Angular Factor", 1.f, 0);
	angular_factor.y = config->GetFloat("Angular Factor", 1.f, 1);
	angular_factor.z = config->GetFloat("Angular Factor", 1.f, 2);
}

// ---------------------------------------------------------
void ComponentRigidBody::OnPlay()
{
	CreateBody();
}

// ---------------------------------------------------------
void ComponentRigidBody::OnStop()
{
	if (body != nullptr)
		App->physics3D->DeleteBody(body);
}

// ---------------------------------------------------------
void ComponentRigidBody::OnDebugDraw() const
{
	switch (body_type)
	{
		case body_sphere:
			DebugDraw(sphere, Green, game_object->GetGlobalTransformation());
		break;
		case body_box:
			DebugDraw(box, Green, game_object->GetGlobalTransformation());
		break;
		case body_capsule:
			DebugDraw(capsule, Green, game_object->GetGlobalTransformation());
		break;
	}
}

// ---------------------------------------------------------
void ComponentRigidBody::getWorldTransform(btTransform & worldTrans) const
{
	worldTrans.setOrigin(game_object->GetGlobalTransformation().TranslatePart());
	worldTrans.setRotation(game_object->GetGlobalTransformation().RotatePart().ToQuat());
}

// ---------------------------------------------------------
void ComponentRigidBody::setWorldTransform(const btTransform & worldTrans)
{
	btQuaternion rot = worldTrans.getRotation();
    btVector3 pos = worldTrans.getOrigin();

	float4x4 new_global(rot, pos);

	// now find out our new local transformation in order to meet the global one from physics
	float4x4 new_local = new_global * game_object->GetParent()->GetLocalTransform().Inverted();
	float3 translation, scale;
	Quat rotation;

	new_local.Decompose(translation, rotation, scale);
	game_object->SetLocalPosition(translation);
	game_object->SetLocalRotation(rotation);
	game_object->SetLocalScale(scale);
}

// ---------------------------------------------------------
void ComponentRigidBody::CreateBody()
{
	if (body != nullptr)
		App->physics3D->DeleteBody(body);

	switch (body_type)
	{
		case body_sphere:
			body = App->physics3D->AddBody(sphere, this);
		break;
		case body_box:
			body = App->physics3D->AddBody(box, this);
		break;
		case body_capsule:
			body = App->physics3D->AddBody(capsule, this);
		break;
	}

	if (body != nullptr)
	{
		body->setLinearFactor(linear_factor);
		body->setAngularFactor(angular_factor);
		body->setRestitution(restitution);
	}
}

// ---------------------------------------------------------
void ComponentRigidBody::SetBodyType(BodyType new_type)
{
	if (new_type != body_unknown && new_type != body_type)
	{
		body_type = new_type;
	}
}

// ---------------------------------------------------------
ComponentRigidBody::BodyType ComponentRigidBody::GetBodyType() const
{
	return body_type;
}

// ---------------------------------------------------------
void ComponentRigidBody::SetBehaviour(BodyBehaviour new_behaviour)
{
	if (new_behaviour != behaviour)
	{
		behaviour = new_behaviour;
	}
}

// ---------------------------------------------------------
ComponentRigidBody::BodyBehaviour ComponentRigidBody::GetBehaviour() const
{
	return behaviour;
}

// ---------------------------------------------------------
void ComponentRigidBody::DrawEditor()
{
	static const char* behaviours[] = { "Fixed", "Dynamic", "Kinematic" };

	int behaviour_type = behaviour;
	if (ImGui::Combo("Behaviour", &behaviour_type, behaviours, 3))
		SetBehaviour((BodyBehaviour) behaviour_type);

	static const char* types[] = { "Sphere", "Box", "Capsule" };

	int type = body_type;
	if (ImGui::Combo("Type", &type, types, 3))
		SetBodyType((BodyType) type);

	switch (type)
	{
		case BodyType::body_sphere:
			ImGui::DragFloat("Radius", &sphere.r, 0.1f, 0.1f);
		break;
		case BodyType::body_box:
			ImGui::DragFloat3("Box", &box.r.x, 0.1f, 0.1f);
		break;
		case BodyType::body_capsule:
			ImGui::DragFloat3("Top", &capsule.l.a.x, 0.1f, 0.1f);
			ImGui::DragFloat3("Bottom", &capsule.l.b.x, 0.1f, 0.1f);
			ImGui::DragFloat("Radius", &capsule.r, 0.1f, 0.1f);
		break;
	}
	ImGui::DragFloat("Mass", &mass, 0.1f, 0.1f, 100000.f);

	if (ImGui::Button("Commit Changes to Physics Engine"))
		CreateBody();

	if (ImGui::DragFloat3("Linear Factor", &linear_factor.x, 0.05f, 0.f, 1.f) && body)
		body->setLinearFactor(linear_factor);

	if (ImGui::DragFloat3("Angular Factor", &angular_factor.x, 0.05f, 0.f, 1.f) && body)
		body->setAngularFactor(angular_factor);

	if(ImGui::DragFloat("Restitution", &restitution, 0.1f) && body)
		body->setRestitution(restitution);

	if (body != nullptr)
	{
		float3 data = body->getLinearVelocity();
		IMGUI_PRINT("Linear Velocity: ", "%.2f %.2f %.2f", data.x, data.y, data.z);
		data = body->getAngularVelocity();
		IMGUI_PRINT("Angular Velocity: ", "%.2f %.2f %.2f", data.x, data.y, data.z);
		data = body->getCenterOfMassPosition();
		IMGUI_PRINT("Center of Mass: ", "%.2f %.2f %.2f", data.x, data.y, data.z);
		data = body->getLocalInertia();
		IMGUI_PRINT("Local Inertia: ", "%.2f %.2f %.2f", data.x, data.y, data.z);
		data = body->getTotalForce();
		IMGUI_PRINT("Total Force: ", "%.2f %.2f %.2f", data.x, data.y, data.z);
		data = body->getTotalTorque();
		IMGUI_PRINT("Total Torque: ", "%.2f %.2f %.2f", data.x, data.y, data.z);
		data.x = body->getFriction();
		data.y = body->getHitFraction();
		data.z = body->getRollingFriction();
		IMGUI_PRINT("Friction/Hit/Rolling: ", "%.2f %.2f %.2f", data.x, data.y, data.z);
	}
}
