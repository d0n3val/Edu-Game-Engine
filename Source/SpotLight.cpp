#include "Globals.h"
#include "SpotLight.h"

#include "Config.h"

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
	config.AddFloat("constant", constant);
	config.AddFloat("linear", linear);
	config.AddFloat("quadric", quadric*1000.0f);
}

void SpotLight::Load(Config& config)
{
	position  = config.GetFloat3("position", float3::zero);
	direction = config.GetFloat3("direction", -float3::unitY);
	color     = config.GetFloat3("color", float3::one);
    inner     = config.GetFloat("inner", 0.0f);
    outter    = config.GetFloat("outter", 0.0f);
	constant  = config.GetFloat("constant", 1.0f);
	linear    = config.GetFloat("linear", 0.0f);
	quadric   = config.GetFloat("quadric", 0.0f)*0.001f;
}


