#include "Globals.h"
#include "SpotLight.h"

#include "Config.h"

#include "Leaks.h"

SpotLight::SpotLight()
{
}

SpotLight::~SpotLight()
{
}

void SpotLight::Save(Config& config) const
{
	config.AddFloat3("position", position);
	config.AddFloat3("direction", direction);
	config.AddFloat3("color", color);
    config.AddFloat("inner", inner);
    config.AddFloat("outter", outter);
	config.AddFloat("distance", distance);
	config.AddFloat("intensity", intensity);
	config.AddFloat("anisotropy", anisotropy);
	config.AddBool("Enabled", enabled);
}

void SpotLight::Load(Config& config)
{
	position  = config.GetFloat3("position", float3::zero);
	direction = config.GetFloat3("direction", -float3::unitY);
	color     = config.GetFloat3("color", float3::one);
    inner     = config.GetFloat("inner", 0.0f);
    outter    = config.GetFloat("outter", 0.0f);
	distance  = config.GetFloat("distance", 1.0f);
    intensity = config.GetFloat("intensity", 1.0f);
    anisotropy = config.GetFloat("anisotropy", 0.0f);
    enabled   = config.GetBool("Enabled", true);
}


