#include "Globals.h"
#include "Application.h"
#include "ComponentSteering.h"
#include "ComponentPath.h"
#include "GameObject.h"
#include "Component.h"
#include "ModuleAI.h"
#include "ModuleLevelManager.h"
#include "DebugDraw.h"
#include "ModuleEditor.h"
#include "PanelProperties.h"
#include "Imgui/imgui.h"

#include "mmgr/mmgr.h"

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
	config.AddUInt("Goal", (goal) ? goal->GetUID() : 0);

	config.AddFloat("Mov Acceleration", mov_acceleration);
	config.AddFloat("Mov Speed", max_mov_speed);
	config.AddFloat("Max Distance", max_distance);
	config.AddFloat("Min Distance", min_distance);
	config.AddFloat("Slow Distance", slow_distance);

	config.AddFloat("Rot Acceleration", rot_acceleration);
	config.AddFloat("Rot Speed", max_rot_speed);
	config.AddFloat("Min Angle", min_angle);
	config.AddFloat("Slow Angle", slow_angle);

	config.AddFloat("Max Prediction", max_mov_prediction);
	config.AddFloat3("Wander Offset", wander_offset);
	config.AddFloat("Wander Radius", wander_radius);
	config.AddFloat("Wander Rate", wander_change_rate);

	config.AddUInt("Path", (path) ? path->GetGameObject()->GetUID() : 0);
	config.AddFloat("Path Offset", path_offset);
	config.AddFloat("Path Prediction", path_prediction);

	config.AddFloat("Separation Radius", separation_radius);

	config.AddFloat("Obstacle Distance", obstacle_avoid_distance);
	config.AddFloat("Obstacle Detector", obstacle_detector_len);
}

// ---------------------------------------------------------
void ComponentSteering::OnLoad(Config * config)
{
	behaviour = (Behaviour) config->GetInt("Behaviour", Behaviour::seek);
	goal_uid = config->GetUInt("Goal");
	mov_acceleration = config->GetFloat("Mov Acceleration", 0.1f);
	max_mov_speed = config->GetFloat("Mov Speed", 1.0f);
	max_distance = config->GetFloat("Max Distance", 50.0f);
	min_distance = config->GetFloat("Min Distance", 1.0f);
	slow_distance = config->GetFloat("Slow Distance", 5.0f);

	rot_acceleration = config->GetFloat("Rot Acceleration", 0.01f);
	max_rot_speed = config->GetFloat("Rot Speed", 0.1f);
	min_angle = config->GetFloat("Min Angle", 0.01f);
	slow_angle = config->GetFloat("Slow Angle", 0.2f);
	wander_offset = config->GetFloat3("Wander Offset");
	wander_radius = config->GetFloat("Wander Radius", 1.0f);
	wander_change_rate = config->GetFloat("Wander Rate", 0.1f);
	path_uid = config->GetUInt("Path");

	path_offset = config->GetFloat("Path Offset", 0.1f);
	path_prediction = config->GetFloat("Path Prediction", 0.0f);
	separation_radius = config->GetFloat("Separation Radius", 10.0f);
	obstacle_avoid_distance = config->GetFloat("Obstacle Distance", 3.0f);
	obstacle_detector_len = config->GetFloat("Obstacle Detector", 3.0f);
}

// ---------------------------------------------------------
void ComponentSteering::OnStart()
{
	if (goal_uid != 0)
		goal = App->level->Find(goal_uid);

	if (path_uid != 0)
	{
		GameObject* path_go = App->level->Find(path_uid);
		vector<Component*> results;
		path_go->FindComponents(Component::Path, results);
		if (results.size() > 0)
			path = (ComponentPath*) results[0];
	}
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
	case ComponentSteering::follow_path:
		mov_velocity += Seek(FollowPath());
		rot_velocity = Align(LookAhead());
		break;
	case ComponentSteering::separation:
		mov_velocity += Separation();
		break;
	case ComponentSteering::collision_avoidance:
		mov_velocity += CollisionAvoidance();
		break;
	case ComponentSteering::obstacle_avoidance:
		mov_velocity += ObstacleAvoidance();
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
				dd::circle(goal->GetGlobalPosition(), float3::unitY, dd::colors::Green, min_distance, 10);
				dd::circle(goal->GetGlobalPosition(), float3::unitY, dd::colors::Red, slow_distance, 10);
				dd::circle(goal->GetGlobalPosition(), float3::unitY, dd::colors::Blue, max_distance, 10);
			} break;
			case align:
			case unalign:
			{
				float3 orientation = float3::unitZ;
				if (behaviour == unalign)
					orientation = -orientation;

				float3 offset(0.f, goal->GetLocalBBox().HalfSize().y, 0.f);
				float inner_radius = MAX(goal->GetLocalBBox().HalfSize().x, goal->GetLocalBBox().HalfSize().z);
				
				dd::arrow(goal->GetGlobalTransformation().TransformPos(offset), goal->GetGlobalTransformation().TransformPos(offset + orientation), dd::colors::Red, Length(orientation)*0.1f);
				dd::arc(goal->GetGlobalTransformation().TransformPos(offset), goal->GetGlobalTransformation().TransformDir(float3::unitY), dd::colors::Green, inner_radius * 2.0f, 10, min_angle*0.5f, -min_angle*0.5f);
				dd::arc(goal->GetGlobalTransformation().TransformPos(offset), goal->GetGlobalTransformation().TransformDir(float3::unitY), dd::colors::Red, inner_radius * 3.0f, 10, slow_angle*0.5f, -slow_angle*0.5f);
			} break;
			case pursue:
			case evade:
			{
				float3 target = Pursue(goal->GetGlobalPosition(), goal->GetVelocity());
				dd::sphere(float4x4(float4x4::Translate(target)).TransformPos(target), dd::colors::Green, 1.0f);

			} break;
			case wander:
			{
				// Circle to generate the wander position
				float3 center = game_object->GetGlobalTransformation().TransformPos(wander_offset);
				dd::circle(center, float3::unitY, dd::colors::White, wander_radius, 10);

				// Last generated position
				dd::circle(last_wander_target, float3::unitY, dd::colors::Red, 0.5f, 10);
			} break;
			case follow_path:
			{
				if (path != nullptr)
				{
					path->OnDebugDraw(true);

					// Last generated closest position to path
					dd::circle(path->GetPos(last_closest), float3::unitY, dd::colors::Red, 0.5f, 10);
					dd::circle(path->GetPos(last_closest + path_offset), float3::unitY, dd::colors::Yellow, 0.5f, 10);


					float3 future_pos = game_object->GetGlobalPosition() + mov_velocity * path_prediction;
					dd::circle(future_pos, float3::unitY, dd::colors::Green, 0.5f, 10);
				}
			} break;
			case separation:
			{
				vector<GameObject*> results;
				App->level->FindNear(game_object->GetGlobalPosition(), separation_radius, results);

				for (vector<GameObject*>::const_iterator it = results.begin(); it != results.end(); ++it)
				{
					if ((*it)->HasComponent(Component::Steering) == true && *it != game_object)
					{
						dd::sphere((*it)->GetGlobalTransformation().TransformPos(float3::zero), dd::colors::White, 2.0f);
						float3 direction =  game_object->GetGlobalPosition() - (*it)->GetGlobalPosition();
						// Linear
						float force = mov_acceleration * ((separation_radius - direction.Length()) / separation_radius);
						float3 offset(0.f, goal->GetLocalBBox().HalfSize().y, 0.f);
						dd::arrow(game_object->GetGlobalTransformation().TransformPos(offset), game_object->GetGlobalTransformation().TransformPos(offset + direction.Normalized() * force), dd::colors::Green, force*0.1f);
					}
				}
			} // fallback
			case collision_avoidance:
			{
				dd::circle(game_object->GetGlobalPosition(), float3::unitY, dd::colors::White, separation_radius, 10);
			} break;
			case obstacle_avoidance:
			{
				float3 a = game_object->GetGlobalTransformation().TransformPos(float3::unitZ * game_object->GetRadius());
				float3 b = game_object->GetGlobalTransformation().TransformPos(float3::unitZ * (game_object->GetRadius() + obstacle_detector_len));
				
				LineSegment detector(a, b);
				dd::line(a, b, dd::colors::Yellow);

				float dist;
				float3 normal;
				const GameObject* go = App->level->CastRayOnBoundingBoxes(detector, dist, normal);
				
				if (go != nullptr)
				{
					float3 hit_pos = detector.GetPoint(dist);
					float3 ret = hit_pos + normal * obstacle_avoid_distance;
					dd::line(hit_pos, ret, dd::colors::Yellow);
					dd::circle(ret, float3::unitY, dd::colors::Red, 2.0f, 10);
				}
			} break;
		}
	}
}

// ---------------------------------------------------------
void ComponentSteering::OnGoDestroyed()
{
	if(goal != nullptr)
		goal = App->level->Validate(goal);

	if (path != nullptr)
	{
		if(App->level->Validate(path->GetGameObject()) == nullptr)
			path = nullptr;
	}
}
// ---------------------------------------------------------
void ComponentSteering::SetBehaviour(Behaviour new_behaviour)
{
	if (new_behaviour != behaviour)
	{
		behaviour = new_behaviour;

		// Add here initial settings when we switch behaviours
		switch (behaviour)
		{
			case follow_path:
				last_closest = 0.0f;
			break;
		}
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
float3 ComponentSteering::Wander() 
{
	// Calculate a dummy position ahead with some randomness
	if (App->Random().Float() < wander_change_rate)
	{
		float3 center = game_object->GetGlobalTransformation().TransformPos(wander_offset);
		Circle circle(center, float3::unitY, wander_radius);
		float3 rnd = circle.RandomPointInside(App->Random());
		last_wander_target = float3(rnd.x, game_object->GetGlobalPosition().y, rnd.z);
	}

	return last_wander_target;
}

// ---------------------------------------------------------
float3 ComponentSteering::FollowPath() const
{
	float3 ret = float3::zero;

	if (path != nullptr)
	{
		float3 pos_to_evaluate = game_object->GetGlobalPosition() + mov_velocity * path_prediction;
		last_closest = path->GetClosestPoint(pos_to_evaluate, &last_closest);
		return path->GetPos(last_closest + path_offset);
	}

	return ret;
}

// ---------------------------------------------------------
float3 ComponentSteering::Separation() const
{
	float3 ret = float3::zero;

	vector<GameObject*> results;
	App->level->FindNear(game_object->GetGlobalPosition(), separation_radius, results);

	for (vector<GameObject*>::const_iterator it = results.begin(); it != results.end(); ++it)
	{
		if ((*it)->HasComponent(Component::Steering) == true && *it != game_object)
		{
			float3 direction =  game_object->GetGlobalPosition() - (*it)->GetGlobalPosition();
			// Linear
			float force = mov_acceleration * ((separation_radius - direction.Length()) / separation_radius);
			ret += direction.Normalized() * force;
		}
	}

	return ret;
}

// ---------------------------------------------------------
float3 ComponentSteering::CollisionAvoidance() const
{
	float3 ret = float3::zero;

	vector<GameObject*> results;
	App->level->FindNear(game_object->GetGlobalPosition(), separation_radius, results);

	float smallest_collision_time = inf;
	const GameObject* go_to_avoid = nullptr;
	float3 smallest_relative_vel;
	float3 smallest_relative_pos;
	float smallest_min_separation;
	float smallest_distance;

	// find the gameobject that has a shortest time to collision and focus only on this one
	for (vector<GameObject*>::const_iterator it = results.begin(); it != results.end(); ++it)
	{
		const GameObject* go = *it;
		if (go->HasComponent(Component::Steering) == true && go != game_object)
		{
			// Predict time of collision
			float3 relative_pos = go->GetGlobalPosition() - game_object->GetGlobalPosition();
			float3 relative_vel = go->GetVelocity() - game_object->GetVelocity();
			float relative_speed = relative_vel.Length();
			float time_to_collision = relative_pos.Dot(relative_vel) / (relative_speed * relative_speed);

			// Check if there will be collision at all
			float distance = relative_pos.Length();
			float min_separation = distance - relative_speed * time_to_collision;

			if (min_separation > separation_radius * 2.0f)
				continue; // no collision

			if (time_to_collision > 0 && time_to_collision < smallest_collision_time)
			{
				smallest_collision_time = time_to_collision;
				go_to_avoid = go;

				smallest_relative_vel = relative_vel;
				smallest_relative_pos = relative_pos;
				smallest_min_separation = min_separation;
				smallest_distance = distance;
			}
		}
	}

	// now avoid the selected gameobject
	if (go_to_avoid != nullptr)
	{
		LOG("Avoiding %s", go_to_avoid->name.c_str());

		// colliding or hitting exactly
		if (smallest_min_separation <= 0.0f || smallest_distance < separation_radius * 2.0f)
		{
			ret = go_to_avoid->GetGlobalPosition() - game_object->GetGlobalPosition();
		}
		else // calculate the future relative position
		{
			ret = smallest_relative_pos + smallest_relative_vel * smallest_collision_time;
		}

		ret.Normalize();
		ret *= mov_acceleration;
	}

	return ret;
}

// ---------------------------------------------------------
float3 ComponentSteering::ObstacleAvoidance() const
{
	float3 ret = float3::zero;

	float3 a = game_object->GetGlobalTransformation().TransformPos(float3::unitZ * game_object->GetRadius());
	float3 b = game_object->GetGlobalTransformation().TransformPos(float3::unitZ * (game_object->GetRadius() + obstacle_detector_len));
	
	LineSegment detector(a, b);

	float dist;
	float3 normal;
	const GameObject* go = App->level->CastRayOnBoundingBoxes(detector, dist, normal);
	
	if (go != nullptr)
	{
		float3 hit_pos = detector.GetPoint(dist);
		ret = hit_pos + normal * obstacle_avoid_distance;
	}

	return ret;
}

// ---------------------------------------------------------
void ComponentSteering::DrawEditor()
{
	static_assert(Behaviour::unknown == 15, "code needs update");

	static const char* behaviours[] = { 
		"Seek", "Flee", "Arrive", "Align", "UnAlign", "Match Velocity", 
		"Pursue", "Evade", "Face", "Look Ahead", "Wander", "Follow Path",
		"Separation", "Collision Avoidance", "Obstacle Avoidance", "Unknown" };

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

	switch (behaviour)
	{
		case wander:
		{
			if (ImGui::CollapsingHeader("Wander Behavior"))
			{
				ImGui::DragFloat3("Offset", &wander_offset.x, 0.1f);
				ImGui::DragFloat("Radius", &wander_radius, 0.1f);
				ImGui::SliderFloat("Rate", &wander_change_rate, 0.01f, 1.0f);
			}
		} break;
		case follow_path:
		{
			if (ImGui::CollapsingHeader("Follow Path"))
			{
				ImGui::PushID("PickPath");
				const GameObject* go = App->editor->props->PickGameObject((path) ? path->GetGameObject() : nullptr);
				if (go != nullptr)
				{
					vector<Component*> results;
					go->FindComponents(Component::Path, results);
					if (results.size() > 0)
						path = (ComponentPath*) results[0];
				}
				ImGui::PopID();
				ImGui::DragFloat("Offset", &path_offset, 0.01f, 0.01f, 1.0f);
				ImGui::DragFloat("Prediction", &path_prediction, 0.01f, 0.01f, 10.0f);
			}
		} break;

		case collision_avoidance:
		{

		} // fallback
		case separation:
		{
			ImGui::DragFloat("Radius", &separation_radius, 0.1f, 0.1f);
		} break; 

		case obstacle_avoidance:
		{
			ImGui::DragFloat("Detector Len", &obstacle_detector_len, 0.01f, 1.0f, 50.0f);
			ImGui::DragFloat("Distance", &obstacle_avoid_distance, 0.01f, 1.0f, 10.0f);
		}
	}

	// Curve editor! ---
	/*
	static ImVec2 foo[10] = { {-1,0} };
	//foo[0].x = -1; // init data so editor knows to take it from here
	if (ImGui::Curve("Curve Editor", ImGui::GetContentRegionAvail(), 10, foo))
	{
		// curve changed
		LOG("At 0.5 we have a %f", ImGui::CurveValue(0.5f, 10, foo));
	}*/
}
