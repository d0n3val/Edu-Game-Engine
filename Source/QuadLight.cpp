#include "Globals.h"

#include "QuadLight.h"

#include "Config.h"

QuadLight::QuadLight()
{
}

QuadLight::~QuadLight()
{
}

void QuadLight::Save(Config &config) const
{
	config.AddFloat3("position", position);
	config.AddFloat3("color", color);
	config.AddFloat("intensity", intensity);
	config.AddBool("Enabled", enabled);
	config.AddFloat3("up", up);
	config.AddFloat3("right", right);
	config.AddFloat2("size", size);
}

void QuadLight::Load(Config &config)
{
	position  = config.GetFloat3("position", float3::zero);
	color     = config.GetFloat3("color", float3::one);
    intensity = config.GetFloat("intensity", 1.0f);
    enabled   = config.GetBool("Enabled", true);
	up        = config.GetFloat3("up", float3::unitY);
	right     = config.GetFloat3("right", float3::unitX);
	size      = config.GetFloat2("size", float2::one);
}
