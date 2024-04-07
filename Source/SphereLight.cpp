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
	config.AddFloat("lightRadius", lightRadius);
	config.AddFloat("intensity", intensity);
	config.AddFloat("anisotropy", anisotropy);
	config.AddBool("Enabled", enabled);
}

void SphereLight::Load(Config& config)
{
	position  	= config.GetFloat3("position", float3::zero);
	color       = config.GetFloat3("color", float3::one);
	radius 	  	= config.GetFloat("radius", 1.0f);
	lightRadius = config.GetFloat("lightRadius", 1.0f);
    intensity   = config.GetFloat("intensity", 1.0f);
    anisotropy  = config.GetFloat("anisotropy", 0.0f);
    enabled   	= config.GetBool("Enabled", true);
}