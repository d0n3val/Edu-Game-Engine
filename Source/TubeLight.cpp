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
	config.AddFloat3("position", position);
	config.AddFloat4("rotation", float4(rotation.x, rotation.y, rotation.z, rotation.w));
	config.AddFloat("height", height);
	config.AddFloat3("colour", colour);
	config.AddFloat("intensity", intensity);
	config.AddFloat("radius", radius);
	config.AddFloat("attenuation_radius", attRadius);
	config.AddBool("Enabled", enabled);
}

void TubeLight::Load(Config &config)
{
	position = config.GetFloat3("position", float3::zero);
	float4 tmp = config.GetFloat4("rotation", float4(0,0,0,1));
	rotation = Quat(tmp.x, tmp.y, tmp.z, tmp.w);

	height = config.GetFloat("height", 1.0f);
	colour     = config.GetFloat3("colour", float3::zero);
    intensity  = config.GetFloat("intensity", 0.0f);
    radius     = config.GetFloat("radius", radius);
    attRadius  = config.GetFloat("attenuation_radius", attRadius);
    enabled    = config.GetBool("Enabled", true);
}

float3 TubeLight::GetPosition0() const
{
	return rotation*(float3(0.0f, 0.5f, 0.0f)*height)+position;
}

float3 TubeLight::GetPosition1() const
{
	return rotation*(float3(0.0f, -0.5f, 0.0f)*height)+position;
}
