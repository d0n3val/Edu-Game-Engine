#include "Globals.h"

#include "AmbientLight.h"

#include "Config.h"

#include "Leaks.h"

// ---------------------------------------------------------
AmbientLight::AmbientLight()
{
}

// ---------------------------------------------------------
AmbientLight::~AmbientLight()
{
}

// ---------------------------------------------------------
void AmbientLight::Save(Config& config) const
{
	config.AddFloat3("color", color);
}

// ---------------------------------------------------------
void AmbientLight::Load(Config& config)
{
    color = config.GetFloat3("color", float3(0.0f, 0.0f, 0.0f));
}

