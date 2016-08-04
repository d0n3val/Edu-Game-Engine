#include "Component.h"
#include "Globals.h"

// ---------------------------------------------------------
Component::Component(GameObject* container) : game_object(container)
{
	if (game_object != nullptr)
		SetActive(true);
	else
		LOG("Component orphaned	since it's container Game Object is null");
}

// ---------------------------------------------------------
Component::~Component()
{}

// ---------------------------------------------------------
void Component::SetActive(bool active)
{
	if (this->active != active)
	{
		this->active = active;
		if (active)
			OnActivate();
		else
			OnDeActivate();
	}
}

// ---------------------------------------------------------
bool Component::IsActive() const
{
	return active;
}

// ---------------------------------------------------------
ComponentTypes Component::GetType() const
{
	return type;
}

const GameObject * Component::GetGameObject() const
{
	return game_object;
}
