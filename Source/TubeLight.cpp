#include "Globals.h"

#include "TubeLight.h"
#include "Config.h"

TubeLight::TubeLight()
{

}

TubeLight::~TubeLight()
{
}

void TubeLight::Save(Config &config) const
{
	config.AddFloat3("position0", position0);
	config.AddFloat3("position1", position1);
	config.AddFloat3("colour", colour);
	config.AddFloat("intensity", intensity);
	config.AddFloat("radius", radius);
	config.AddBool("Enabled", enabled);
}

void TubeLight::Load(Config &config)
{
	position0  = config.GetFloat3("position0", float3::zero);
	position1  = config.GetFloat3("position1", float3::zero);
	colour     = config.GetFloat3("colour", float3::zero);
    intensity  = config.GetFloat("intensity", 0.0f);
    radius     = config.GetFloat("radius", radius);
    enabled    = config.GetBool("Enabled", true);
}
