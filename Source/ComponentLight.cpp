#include "Globals.h"

#include "ComponentLight.h"

// ---------------------------------------------------------
ComponentLight::ComponentLight()
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
	config.AddArrayFloat("position", reinterpret_cast<float*>(&position), 3);
	config.AddArrayFloat("direction", reinterpret_cast<float*>(&direction), 3);
	config.AddArrayFloat("up", reinterpret_cast<float*>(&up), 3);
}

// ---------------------------------------------------------
void ComponentLight::OnLoad(Config* config) 
{
	config.AddUInt("type", uint(type));
	config.AddArrayFloat("position", reinterpret_cast<float*>(&position), 3);
	config.AddArrayFloat("direction", reinterpret_cast<float*>(&direction), 3);
	config.AddArrayFloat("up", reinterpret_cast<float*>(&up), 3);
}

