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
	config.AddFloat("Max Distance", max_distance);
	config.AddFloat("Min Distance", min_distance);
	config.AddFloat("Slow Distance", slow_distance);

	config.AddFloat("Rot Acceleration", rot_acceleration);
	config.AddFloat("Rot Speed", max_rot_speed);
	config.AddFloat("Min Angle", min_angle);
	config.AddFloat("Slow Angle", slow_angle);
}

// ---------------------------------------------------------
void ComponentSteering::OnLoad(Config * config)
{
	behaviour = (Behaviour) config->GetInt("Behaviour", Behaviour::seek);
	goal_uid = config->GetInt("Goal");
	mov_acceleration = config->GetFloat("Mov Acceleration", 0.1f);
	max_mov_speed = config->GetFloat("Mov Speed", 1.0f);
	max_distance = config->GetFloat("Max Distance", 50.0f);
	min_distance = config->GetFloat("Min Distance", 1.0f);
	slow_distance = config->GetFloat("Slow Distance", 5.0f);

	rot_acceleration = config->GetFloat("Rot Acceleration", 0.01f);
	max_rot_speed = config->GetFloat("Rot Speed", 0.1f);
	min_angle = config->GetFloat("Min Angle", 0.01f);
	slow_angle = config->GetFloat("Slow Angle", 0.2f);
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
			mov_velocity += Seek(goal->GetGlobalPosition());
		break;
	case ComponentSteering::flee:
		if (goal != nullptr)
			mov_velocity += Flee(goal->GetGlobalPosition());
		break;
	case ComponentSteering::arrive:
		if (goal != nullptr)
			mov_velocity = Arrive(goal->GetGlobalPosition());
		break;
	case ComponentSteering::align:
		if (goal != nullptr)
			rot_velocity = Align(goal->GetGlobalTransformation().WorldZ());
		break;
	case ComponentSteering::unalign:
		if (goal != nullptr)
			rot_velocity = Align(-goal->GetGlobalTransformation().WorldZ());
		break;
	case ComponentSteering::match_velocity:
		if (goal != nullptr)
			mov_velocity += MatchVelocity(goal->GetVelocity());
		break;
	case ComponentSteering::pursue:
		if (goal != nullptr)
			mov_velocity += Arrive(Pursue(goal->GetGlobalPosition(), goal->GetVelocity()));
		break;
	case ComponentSteering::evade:
		if (goal != nullptr)
			mov_velocity += Flee(Pursue(goal->GetGlobalPosition(), goal->GetVelocity()));
		break;
	case ComponentSteering::face:
		if (goal != nullptr)
			rot_velocity = Align(Face(goal->GetGlobalPosition()));
		break;
	case ComponentSteering::look_ahead:
		rot_velocity = Align(LookAhead());
		break;
	case ComponentSteering::wander:
		mov_velocity += Seek(Wander());
		rot_velocity = Align(LookAhead());
		break;
	case ComponentSteering::unknown:
		break;
	default:
		break;
	}

	// Trim down movement velocity
	if (mov_velocity.Length() > max_mov_speed)
	{
		mov_velocity.Normalize();
		mov_velocity *= max_mov_speed;
	}

	// Trim down roation velocity
	if (rot_velocity > max_rot_speed)
		rot_velocity = max_rot_speed;
	else if (rot_velocity < -max_rot_speed)
		rot_velocity = -max_rot_speed;

	// Finally update position & rotation
	game_object->Move(mov_velocity * dt);
	game_object->Rotate(rot_velocity * dt);
}

// ---------------------------------------------------------
void ComponentSteering::OnStop()
{
	mov_velocity = float3::zero;
	rot_velocity = 0.f;
}

// ---------------------------------------------------------
void ComponentSteering::OnDebugDraw(bool selected) const
{
	float4x4 m;
	m.Translate(game_object->GetGlobalPosition());


	if (selected == true && goal != nullptr)
	{
		switch (behaviour)
		{
			case seek:
			case flee:
			case arrive:
			{
				DebugDrawCircle(goal->GetGlobalPosition(), min_distance, Green);
				DebugDrawCircle(goal->GetGlobalPosition(), slow_distance, Red);
				DebugDrawRing(goal->GetGlobalPosition(), max_distance, min_distance, Blue);
			} break;
			case align:
			case unalign:
			{
				float3 orientation = float3::unitZ;
				if (behaviour == unalign)
					orientation = -orientation;

				float3 offset(0.f, goal->GetLocalBBox().HalfSize().y, 0.f);
				float inner_radius = MAX(goal->GetLocalBBox().HalfSize().x, goal->GetLocalBBox().HalfSize().z);
				DebugDrawArrow(orientation, offset, Red, goal->GetGlobalTransformation());
				DebugDrawArc(offset, inner_radius * 2.0f, min_angle*0.5f, -min_angle*0.5f, inner_radius, Green, goal->GetGlobalTransformation());
				DebugDrawArc(offset, inner_radius * 3.0f, slow_angle*0.5f, -slow_angle*0.5f, inner_radius*2.0f, Red, goal->GetGlobalTransformation());
			} break;
			case pursue:
			case evade:
			{
				float3 target = Pursue(goal->GetGlobalPosition(), goal->GetVelocity());
				DebugDraw(Sphere(target, 1.0f), Green, float4x4::Translate(target));
			} break;
			case wander:
			{
				DebugDraw(Sphere(float3::zero, 1.0f), Green, float4x4::Translate(last_wander_target));
			} break;

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
		entrance_velocity = mov_velocity.Length();
		return mov_velocity + (dir.Normalized() * mov_acceleration);
	}
	else 
	{
		float3 max_velocity = (dir.Normalized() * entrance_velocity);
		return max_velocity * (distance / slow_distance);
	}
}

// ---------------------------------------------------------
float ComponentSteering::Align(const float3 & target_dir) const
{
	float3 dir = game_object->GetGlobalTransformation().WorldZ();
	float my_rotation = atan2f(dir.x, dir.z);
	float target_rotation = atan2f(target_dir.x, target_dir.z);
	float diff = my_rotation - target_rotation;

	// wrap diff around [-pi, pi]
	diff += PI;
	diff -= floorf( diff * INV_TWO_PI ) * TWO_PI;
	diff -= PI;
	// ---

	float absolute_diff = fabsf(diff);

	// Are we there yet ?
	if(absolute_diff < min_angle)
		return 0.0f;

	static float entrance_velocity = 0.2f;
	if (absolute_diff > slow_angle)
	{
		entrance_velocity = rot_velocity;
		if (diff < 0.0f || diff > PI)
			return rot_velocity + rot_acceleration;
		else
			return rot_velocity - rot_acceleration;
	}
	else
	{
		return entrance_velocity * (absolute_diff / slow_angle);
	}
}

// ---------------------------------------------------------
float3 ComponentSteering::MatchVelocity(const float3 & target_velocity) const
{
	float3 ret = target_velocity - mov_velocity;

	if (target_velocity.LengthSq() > mov_velocity.LengthSq())
		ret = -ret;

	// Trim down movement velocity
	if (ret.Length() > mov_acceleration)
	{
		ret.Normalize();
		ret *= mov_acceleration;
	}

	return ret;
}

// ---------------------------------------------------------
float3 ComponentSteering::Pursue(const float3 & position, const float3 & velocity) const
{
	float3 dir = position - game_object->GetGlobalPosition();
	float distance = dir.Length();
	float my_velocity = mov_velocity.Length();
	float prediction;
	
	if (my_velocity < distance / max_mov_prediction)
		prediction = max_mov_prediction;
	else
		prediction = distance / my_velocity;

	return position + (velocity * -prediction);
}

// ---------------------------------------------------------
float3 ComponentSteering::Face(const float3 & position) const
{
	float3 dir = position - game_object->GetGlobalPosition();
	return dir.Normalized();
}

// ---------------------------------------------------------
float3 ComponentSteering::LookAhead() const
{
	return mov_velocity.Normalized();
}

// ---------------------------------------------------------
float3 ComponentSteering::Wander() const
{
	// Calculate a dummy position ahead with some randomness
	return float3();
}

// ---------------------------------------------------------
void ComponentSteering::DrawEditor()
{
	static_assert(Behaviour::unknown == 11, "code needs update");

	static const char* behaviours[] = { 
		"Seek", "Flee", "Arrive", "Align", "UnAlign", "Match Velocity", 
		"Pursue", "Evade", "Face", "Look Ahead", "Wander", "Unknown" };

	int behaviour_type = behaviour;
	if (ImGui::Combo("Behaviour", &behaviour_type, behaviours, (int) Behaviour::unknown))
		SetBehaviour((Behaviour) behaviour_type);

	ImGui::Text("Target:");
	const GameObject* selected = App->editor->props->PickGameObject(goal);
	if (selected != nullptr)
		goal = selected;

	// Editor for moving stuff -------------
	ImGui::DragFloat("Mov Acceleration", &mov_acceleration, 0.01f);
	ImGui::DragFloat("Mov Velocity", &max_mov_speed, 0.1f);
	ImGui::DragFloatRange2("Range", &min_distance, &max_distance, 0.1f, 0.1f);
	ImGui::DragFloat("Slow Radius", &slow_distance, 0.1f, 0.1f);

	float dist = 0.0f;
	if(goal)
		dist = (goal->GetGlobalPosition() - game_object->GetGlobalPosition()).Length();
	dist -= min_distance;
	ImGui::Text("Dist: %0.3f ETA: %0.3f", dist, dist / mov_velocity.Length());

	static vector<float> velocities(50);
	if (velocities.size() == 50)
	{
		for (uint i = 0; i < 50 - 1; ++i)
			velocities[i] = velocities[i + 1];

		velocities[49] = mov_velocity.Length();
	}
	else
		velocities.push_back(mov_velocity.Length());
	
	ImGui::PlotHistogram("##velocity", &velocities[0], velocities.size(), 0, "Movement Velocity", 0.0f, max_mov_speed, ImVec2(310,75));

	// Editor for rotation stuff --------------------
	ImGui::Separator();
	ImGui::SliderAngle("Rot Acceleration", &rot_acceleration, 0.01f);
	ImGui::SliderAngle("Rot Velocity", &max_rot_speed, 0.01f);
	ImGui::SliderAngle("Min Angle", &min_angle, 0.01f);
	ImGui::SliderAngle("Slow Angle", &slow_angle, 0.01f);

	dist = 0.0f;
	if (goal != nullptr) {
		float3 target_dir = goal->GetGlobalTransformation().WorldZ();
		float3 dir = game_object->GetGlobalTransformation().WorldZ();
		float my_rotation = atan2f(dir.x, dir.z);
		float target_rotation = atan2f(target_dir.x, target_dir.z);
		float diff = my_rotation - target_rotation;
		dist = fabsf(diff);
	}
	dist -= min_angle;
	ImGui::Text("Dist: %0.3f ETA: %0.3f", dist, dist / rot_velocity);

	static vector<float> rot_velocities(50);
	if (rot_velocities.size() == 50)
	{
		for (uint i = 0; i < 50 - 1; ++i)
			rot_velocities[i] = rot_velocities[i + 1];

		rot_velocities[49] = fabsf(rot_velocity);
	}
	else
		rot_velocities.push_back(fabsf(rot_velocity));
	
	ImGui::PlotHistogram("##rot_velocity", &rot_velocities[0], rot_velocities.size(), 0, "Angular Velocity", 0.0f, max_rot_speed, ImVec2(310,75));

	// Others stuff --------------------
	ImGui::Separator();
	/*
	static ImVec2 foo[10] = { {-1,0} };
	//foo[0].x = -1; // init data so editor knows to take it from here
	if (ImGui::Curve("Curve Editor", ImGui::GetContentRegionAvail(), 10, foo))
	{
		// curve changed
		LOG("At 0.5 we have a %f", ImGui::CurveValue(0.5f, 10, foo));
	}*/
}