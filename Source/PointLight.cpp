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
	config.AddFloat("linear", liner);
	config.AddFloat("quadric", quadric);
}

void PointLight::Load(Config& config)
{
	position = config.GetFloat3("position", float3::zero);
	color    = config.AddFloat3("color", float3::one);
	constant = config.AddFloat("constant", 1.0f);
	linear   = config.AddFloat("linear", 0.1f);
	quadric  = config.AddFloat("quadric", 0.05f);
}

