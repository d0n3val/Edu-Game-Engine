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
		wander,
		unknown
	};

public:
	ComponentSteering (GameObject* container);
	~ComponentSteering () override;

	void OnSave(Config& config) const override;
	void OnLoad(Config* config) override;

	void OnPlay() override;
	void OnUpdate(float dt) override;
	void OnStop() override;
	void OnDebugDraw() const override;

	void SetBehaviour(Behaviour new_behaviour);
	Behaviour GetBehaviour() const;

	void DrawEditor();

private:
	const GameObject* goal = nullptr;
	float max_mov_speed = 1.0f;
	float max_rot_speed = 0.1f;
	float max_distance = 50.0f;
	float min_distance = 0.5f;

private:

	Behaviour behaviour = Behaviour::seek;
};

#endif // __COMPONENT_STEERING_H__