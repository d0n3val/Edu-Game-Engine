#include "Component.h"
#include "Globals.h"

// ---------------------------------------------------------
Component::Component(GameObject* container) : game_object(container)
{
	if (game_object != nullptr)
		Activate();
	else
		LOG("Component orphaned	since it's container Game Object is null");
}

// ---------------------------------------------------------
Component::~Component()
{}

// ---------------------------------------------------------
void Component::Activate()
{
	if (active == false)
	{
		active = true;
		OnActivate();
	}
}

// ---------------------------------------------------------
void Component::DeActivate()
{
	if (active == true)
	{
		active = false;
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