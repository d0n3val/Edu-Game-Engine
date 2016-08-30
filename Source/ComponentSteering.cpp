#include "Globals.h"
#include "Application.h"
#include "ComponentSteering.h"
#include "GameObject.h"
#include "Component.h"
#include "ModuleAI.h"
#include "ModuleLevelManager.h"
#include "DebugDraw.h"
#include "ModuleEditor.h"
#include "PanelProperties.h"
#include "Imgui/imgui.h"

using namespace std;

// ---------------------------------------------------------
ComponentSteering::ComponentSteering(GameObject* container) : Component(container, Types::Steering)
{
}

// ---------------------------------------------------------
ComponentSteering::~ComponentSteering()
{
}

// ---------------------------------------------------------
void ComponentSteering::OnSave(Config& config) const
{
	config.AddInt("Behaviour", behaviour);
	config.AddUID("Goal", (goal) ? goal->GetUID() : 0);
	config.AddFloat("Mov Acceleration", mov_acceleration);
	config.AddFloat("Mov Speed", max_mov_speed);
	config.AddFloat("Rot Speed", max_rot_speed);
	config.AddFloat("Max Distance", max_distance);
	config.AddFloat("Min Distance", min_distance);
}

// ---------------------------------------------------------
void ComponentSteering::OnLoad(Config * config)
{
	behaviour = (Behaviour) config->GetInt("Behaviour", Behaviour::seek);
	goal_uid = config->GetInt("Goal");
	mov_acceleration = config->GetFloat("Mov Acceleration", 0.1f);
	max_mov_speed = config->GetFloat("Mov Speed", 1.0f);
	max_rot_speed = config->GetFloat("Rot Speed", 0.1f);
	max_distance = config->GetFloat("Max Distance", 50.0f);
	min_distance = config->GetFloat("Min Distance", 1.0f);
}

// ---------------------------------------------------------
void ComponentSteering::OnStart()
{
	if (goal_uid != 0)
		goal = App->level->Find(goal_uid);
}

// ---------------------------------------------------------
void ComponentSteering::OnPlay()
{
}

// ---------------------------------------------------------
float RandomBinomial()
{
	return ((float)rand() / (float)RAND_MAX) - ((float)rand() / (float)RAND_MAX);
}

// ---------------------------------------------------------
void ComponentSteering::OnUpdate(float dt)
{
	switch (behaviour)
	{
	case ComponentSteering::seek:
		if (goal != nullptr)
			velocity += Seek(goal->GetGlobalPosition());
		break;
	case ComponentSteering::flee:
		if (goal != nullptr)
			velocity += Flee(goal->GetGlobalPosition());
		break;
	case ComponentSteering::arrive:
		if (goal != nullptr)
			velocity = Arrive(goal->GetGlobalPosition());
		break;
	case ComponentSteering::wander:
		break;
	case ComponentSteering::unknown:
		break;
	default:
		break;
	}

	// Trim down velocity
	if (velocity.Length() > max_mov_speed)
	{
		velocity.Normalize();
		velocity *= max_mov_speed;
	}

	// Finally update position
	game_object->Move(velocity * dt);

	/*
	if (goal != nullptr)
	{
		float3 my_pos = game_object->GetGlobalPosition();
		float3 goal_pos = goal->GetGlobalPosition();
		float distance = goal_pos.Distance(my_pos);
		
		float3 direction = goal_pos - my_pos;
		direction.Normalize();
		Quat my_rot = game_object->GetLocalRotationQ();
		float3 my_front = game_object->GetGlobalTransformation().WorldZ();
		float3 my_right = game_object->GetGlobalTransformation().WorldX();

		float speed_dt = max_mov_speed * dt;

		switch (behaviour)
		{
			case Behaviour::seek:
			{
				if (distance < max_distance && distance > min_distance) 
				{
					game_object->SetLocalPosition(my_pos + (direction * speed_dt));
				}
			} break;

			case Behaviour::flee:
			{
				if (distance < max_distance) 
				{
					direction = my_pos - goal_pos;
					direction.Normalize();
					game_object->SetLocalPosition(my_pos + (direction * speed_dt));
				}
			} break;

			case Behaviour::wander:
			{
				float r = RandomBinomial();
				direction = Quat::RotateY(r).Transform(my_front);
				game_object->SetLocalPosition(my_pos + (direction * speed_dt));
			} break;
		}

		// rotate to look in our current movement direction (Z)
		float angle = direction.AngleBetweenNorm(my_right);

		if (angle > 0.01f)
		{
			bool left = direction.Dot(my_right) >= 0.f;
			float rotation = MIN(angle*dt, max_rot_speed*dt);
			if (!left)
				rotation = -rotation;
			game_object->SetLocalRotation(my_rot * Quat::RotateY(rotation));
		}
	}
	*/
}

// ---------------------------------------------------------
void ComponentSteering::OnStop()
{
}

// ---------------------------------------------------------
void ComponentSteering::OnDebugDraw(bool selected) const
{
	DebugDrawArrow(velocity, float3(0.f, game_object->GetLocalBBox().HalfSize().y, 0.f), Blue, game_object->GetGlobalTransformation());

	if (selected == true)
	{
		if (goal != nullptr)
		{
			DebugDrawCircle(goal->GetGlobalPosition(), min_distance, Green);
			DebugDrawCircle(goal->GetGlobalPosition(), slow_distance, Red);
			DebugDrawRing(goal->GetGlobalPosition(), max_distance, min_distance, Blue);
		}
	}
}

// ---------------------------------------------------------
void ComponentSteering::SetBehaviour(Behaviour new_behaviour)
{
	if (new_behaviour != behaviour)
	{
		behaviour = new_behaviour;
	}
}

// ---------------------------------------------------------
ComponentSteering::Behaviour ComponentSteering::GetBehaviour() const
{
	return behaviour;
}

// ---------------------------------------------------------
float3 ComponentSteering::Seek(const float3& target) const
{
	float3 dir = target - game_object->GetGlobalPosition();
	return dir.Normalized() * mov_acceleration;
}

// ---------------------------------------------------------
float3 ComponentSteering::Flee(const float3& target) const
{
	float3 dir = game_object->GetGlobalPosition() - target;
	return dir.Normalized() * mov_acceleration;
}

// ---------------------------------------------------------
float3 ComponentSteering::Arrive(const float3 & target) const
{
	float3 dir = target - game_object->GetGlobalPosition();
	float distance = dir.Length();

	// Are we there yet ?
	if (distance < min_distance)
		return float3::zero;

	// Return new velocity, we could be slowing down if we are near
	static float entrance_velocity = 1.0f;
	if (distance > slow_distance)
	{
		entrance_velocity = velocity.Length();
		return velocity + (dir.Normalized() * mov_acceleration);
	}
	else 
	{
		float3 max_velocity = (dir.Normalized() * entrance_velocity);
		return max_velocity * (distance / slow_distance);
	}
}

// ---------------------------------------------------------
void ComponentSteering::DrawEditor()
{
	static_assert(Behaviour::unknown == 4, "code needs update");

	static const char* behaviours[] = { "Seek", "Flee", "Arrive", "Wander", "Unknown" };

	int behaviour_type = behaviour;
	if (ImGui::Combo("Behaviour", &behaviour_type, behaviours, 3))
		SetBehaviour((Behaviour) behaviour_type);

	ImGui::Text("Target:");
	const GameObject* selected = App->editor->props->PickGameObject(goal);
	if (selected != nullptr)
		goal = selected;

	ImGui::DragFloat("Mov Acceleration", &mov_acceleration, 0.01f);
	ImGui::DragFloat("Mov Velocity", &max_mov_speed, 0.1f);
	ImGui::SliderAngle("Rot Velocity", &max_rot_speed, 0.01f);
	ImGui::DragFloatRange2("Range", &min_distance, &max_distance, 0.1f, 0.1f);
	ImGui::DragFloat("Slow Radius", &slow_distance, 0.1f, 0.1f);

	float dist = (goal->GetGlobalPosition() - game_object->GetGlobalPosition()).Length();
	dist -= min_distance;
	ImGui::Text("Dist: %0.3f ETA: %0.3f", dist, dist / velocity.Length());

	static vector<float> velocities(50);
	if (velocities.size() == 50)
	{
		for (uint i = 0; i < 50 - 1; ++i)
			velocities[i] = velocities[i + 1];

		velocities[49] = velocity.Length();
	}
	else
		velocities.push_back(velocity.Length());
	
	ImGui::PlotHistogram("##velocity", &velocities[0], velocities.size(), 0, "Velocity", 0.0f, max_mov_speed, ImVec2(310,100));

	/*
	static ImVec2 foo[10] = { {-1,0} };
	//foo[0].x = -1; // init data so editor knows to take it from here
	if (ImGui::Curve("Curve Editor", ImGui::GetContentRegionAvail(), 10, foo))
	{
		// curve changed
		LOG("At 0.5 we have a %f", ImGui::CurveValue(0.5f, 10, foo));
	}*/
}