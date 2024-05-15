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
	config.AddFloat4("Transform0", transform.Col(0));
	config.AddFloat4("Transform1", transform.Col(1));
	config.AddFloat4("Transform2", transform.Col(2));
	config.AddFloat4("Transform3", transform.Col(3));
	config.AddFloat3("color", color);
    config.AddFloat("inner", inner);
    config.AddFloat("outter", outter);
	config.AddFloat("distance", maxDist);
	config.AddFloat("minDist", minDist);
	config.AddFloat("intensity", intensity);
	config.AddFloat("anisotropy", anisotropy);
	config.AddBool("Enabled", enabled);
}

void SpotLight::Load(Config& config)
{
	transform.SetCol(0, config.GetFloat4("Transform0", transform.Col(0)));
	transform.SetCol(1, config.GetFloat4("Transform1", transform.Col(1)));
	transform.SetCol(2, config.GetFloat4("Transform2", transform.Col(2)));
	transform.SetCol(3, config.GetFloat4("Transform3", transform.Col(3)));

	color      = config.GetFloat3("color", float3::one);
    inner      = config.GetFloat("inner", 0.0f);
    outter     = config.GetFloat("outter", 0.0f);
	maxDist    = config.GetFloat("distance", 1.0f);
	minDist    = config.GetFloat("minDist", 1.0f);
    intensity  = config.GetFloat("intensity", 1.0f);
    anisotropy = config.GetFloat("anisotropy", 0.0f);
    enabled    = config.GetBool("Enabled", true);
}


