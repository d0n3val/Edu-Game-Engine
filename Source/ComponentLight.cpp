#include "Globals.h"

#include "ComponentLight.h"

// ---------------------------------------------------------
ComponentLight::ComponentLight(GameObject* container) : Component(container, Type::Light)
{
}

// ---------------------------------------------------------
ComponentLight::~ComponentLight()
{
}

// ---------------------------------------------------------
void ComponentLight::OnSave(Config& config) const
{
	config.AddUInt("type", uint(type));
	config.AddFloat3("color", color);
}

// ---------------------------------------------------------
void ComponentLight::OnLoad(Config* config) 
{
	type  = Type(config.GetUInt("type", uint(POINT)));
    color = config.GetFloat3("color", float3(1.0f, 1.0f, 1.0f));
}

