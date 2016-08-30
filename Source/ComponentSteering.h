#ifndef __COMPONENT_STEERING_H__
#define __COMPONENT_STEERING_H__

// Component to process kinetic movement, base for AI

#include "Component.h"
#include "Math.h"

class ComponentSteering : public Component
{
	friend class ModuleAI;
public:

	enum Behaviour
	{
		seek,
		flee,
		arrive,
		wander,
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

	void SetBehaviour(Behaviour new_behaviour);
	Behaviour GetBehaviour() const;


	void DrawEditor();

private:
	float3 Seek(const float3 & target) const;
	float3 Flee(const float3 & target) const;
	float3 Arrive(const float3 & target) const;

private:
	uint goal_uid = 0;
	const GameObject* goal = nullptr;
	float mov_acceleration = 0.1f;
	float max_mov_speed = 1.0f;
	float max_rot_speed = 0.1f;
	float max_distance = 50.0f;
	float min_distance = 1.0f;
	float slow_distance = 5.0f;
	float3 velocity = float3::zero;

private:

	Behaviour behaviour = Behaviour::seek;
};

#endif // __COMPONENT_STEERING_H__