#ifndef __COMPONENT_STEERING_H__
#define __COMPONENT_STEERING_H__

// Component to process kinetic movement, base for AI

#include "Component.h"
#include "Math.h"

class ComponentPath;

class ComponentSteering : public Component
{
	friend class ModuleAI;
public:

	enum Behaviour
	{
		seek,
		flee,
		arrive,
		align,
		unalign,
		match_velocity,
		pursue,
		evade,
		face,
		look_ahead,
		wander,
		follow_path,
		unknown
	};

public:
	ComponentSteering (GameObject* container);
	~ComponentSteering () override;

	void OnSave(Config& config) const override;
	void OnLoad(Config* config) override;

	void OnStart() override;
	void OnPlay() override;
	void OnUpdate(float dt) override;
	void OnStop() override;
	void OnDebugDraw(bool selected) const override;
	void OnGoDestroyed() override;

	void SetBehaviour(Behaviour new_behaviour);
	Behaviour GetBehaviour() const;

	void DrawEditor();

private:
	float3 Seek(const float3 & target) const;
	float3 Flee(const float3 & target) const;
	float3 Arrive(const float3 & target) const;
	float Align(const float3 & target_dir) const;
	float3 MatchVelocity(const float3 & target_velocity) const;
	float3 Pursue(const float3& position, const float3& velocity) const;
	float3 Face(const float3& position) const;
	float3 LookAhead() const;
	float3 Wander();
	float3 FollowPath() const;

private:
	uint goal_uid = 0;
	const GameObject* goal = nullptr;

	// Movement ---
	float3 mov_velocity = float3::zero;

	float mov_acceleration = 0.1f;
	float max_mov_speed = 1.0f;
	float max_distance = 50.0f;
	float min_distance = 1.0f;
	float slow_distance = 5.0f;

	// Rotation ---
	float rot_velocity = 0.f;

	float rot_acceleration = 0.01f;
	float max_rot_speed = 0.3f;
	float min_angle = 0.01f;
	float slow_angle = 0.2f;

	// Others ---
	float max_mov_prediction = 10.0f;

	float3 wander_offset = float3::unitZ;
	float wander_radius = 3.0f;
	float wander_change_rate = 0.5f;
	float3 last_wander_target = float3::zero;

	uint path_uid = 0;
	const ComponentPath* path = nullptr;

private:

	Behaviour behaviour = Behaviour::seek;
};

#endif // __COMPONENT_STEERING_H__