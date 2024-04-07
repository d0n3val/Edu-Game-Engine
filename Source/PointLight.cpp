#include "Globals.h"
#include "PointLight.h"

#include "Config.h"

#include "Leaks.h"

PointLight::PointLight()
{
}

PointLight::~PointLight()
{
}

void PointLight::Save(Config& config) const
{
	config.AddFloat3("position", position);
	config.AddFloat3("color", color);
	config.AddFloat("radius", radius);
	config.AddFloat("intensity", intensity);
	config.AddFloat("anisotropy", anisotropy);
	config.AddBool("Enabled", enabled);
}

void PointLight::Load(Config& config)
{
	position = config.GetFloat3("position", float3::zero);
	color    = config.GetFloat3("color", float3::one);
	radius 	 = config.GetFloat("radius", 1.0f);
    intensity = config.GetFloat("intensity", 1.0f);
    anisotropy = config.GetFloat("anisotropy", 0.0f);
    enabled  = config.GetBool("Enabled", true);
}

