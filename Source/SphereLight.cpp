#include "Globals.h"

#include "SphereLight.h"

#include "Config.h"


SphereLight::SphereLight()
{
}

SphereLight::~SphereLight()
{
}

void SphereLight::Save(Config& config) const
{
	config.AddFloat3("position", position);
	config.AddFloat3("color", color);
	config.AddFloat("radius", radius);
	config.AddFloat("intensity", intensity);
	config.AddBool("Enabled", enabled);
}

void SphereLight::Load(Config& config)
{
	position  = config.GetFloat3("position", float3::zero);
	color     = config.GetFloat3("color", float3::one);
	radius 	  = config.GetFloat("radius", 1.0f);
    intensity = config.GetFloat("intensity", 1.0f);
    enabled   = config.GetBool("Enabled", true);
}