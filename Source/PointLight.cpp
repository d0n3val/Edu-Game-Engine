#include "Globals.h"
#include "PointLight.h"

#include "Config.h"

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
	config.AddFloat("constant", constant);
	config.AddFloat("linear", linear);
	config.AddFloat("quadric", quadric*1000.0f);
}

void PointLight::Load(Config& config)
{
	position = config.GetFloat3("position", float3::zero);
	color    = config.GetFloat3("color", float3::one);
	constant = config.GetFloat("constant", 1.0f);
	linear   = config.GetFloat("linear", 0.0f);
	quadric  = config.GetFloat("quadric", 0.0f)/1000.0f;
}

