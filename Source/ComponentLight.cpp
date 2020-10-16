#include "Globals.h"

#include "ComponentLight.h"


#include "Leaks.h"

// ---------------------------------------------------------
ComponentLight::ComponentLight(GameObject* container) : Component(container, Types::Light)
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
	config.AddFloat4("color", color);
}

// ---------------------------------------------------------
void ComponentLight::OnLoad(Config* config) 
{
	type  = Type(config->GetUInt("type", uint(POINT)));
    color = config->GetFloat4("color", float4(1.0f, 1.0f, 1.0f, 1.0f));
}

