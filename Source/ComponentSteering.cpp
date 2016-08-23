#include "Globals.h"
#include "Application.h"
#include "ComponentSteering.h"
#include "GameObject.h"
#include "Component.h"
#include "ModuleAI.h"
#include "DebugDraw.h"
#include "Imgui/imgui.h"

using namespace std;

// ---------------------------------------------------------
ComponentSteering::ComponentSteering(GameObject* container) : Component(container, Types::RigidBody)
{
}

// ---------------------------------------------------------
ComponentSteering::~ComponentSteering()
{
}

// ---------------------------------------------------------
void ComponentSteering::OnSave(Config& config) const
{
}

// ---------------------------------------------------------
void ComponentSteering::OnLoad(Config * config)
{
}

// ---------------------------------------------------------
void ComponentSteering::OnPlay()
{
	LOG("Steering update STARTS");
}

// ---------------------------------------------------------
void ComponentSteering::OnUpdate(float dt)
{
	LOG("Steering update %f", dt);
}

// ---------------------------------------------------------
void ComponentSteering::OnStop()
{
	LOG("Steering update STOPS");
}

// ---------------------------------------------------------
void ComponentSteering::OnDebugDraw() const
{
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
void ComponentSteering::DrawEditor()
{
	static const char* behaviours[] = { "Seek", "Flee", "Unknown" };

	int behaviour_type = behaviour;
	if (ImGui::Combo("Behaviour", &behaviour_type, behaviours, 3))
		SetBehaviour((Behaviour) behaviour_type);
}
