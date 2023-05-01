#include "Globals.h"
#include "LocalIBLLight.h"

#include "Config.h"
#include "OGL.h"
#include "OpenGL.h"
#include "CubemapUtils.h"


LocalIBLLight::LocalIBLLight()
{
}

LocalIBLLight::~LocalIBLLight()
{

}

void LocalIBLLight::Save(Config& config) const 
{
    config.AddFloat3("Position", position);
	config.AddFloat4("Rotation", float4(rotation.x, rotation.y, rotation.z, rotation.w));
    config.AddFloat3("Parallax-Min", parallax.minPoint);
    config.AddFloat3("Parallax-Max", parallax.maxPoint);
    config.AddFloat3("Influence-Min", influence.minPoint);
    config.AddFloat3("Influence-Max", influence.maxPoint);
    config.AddFloat("FarPlane", farPlane);
    config.AddUInt("Resolution", resolution);
    config.AddUInt("Samples", numSamples);
    config.AddUInt("RoughnessLevels", roughnessLevels);
	config.AddBool("Enabled", enabled);
}

void LocalIBLLight::Load(Config& config) 
{
    position = config.GetFloat3("Position", float3::zero);
	float4 tmp = config.GetFloat4("Rotation", float4(0,0,0,1));
	rotation = Quat(tmp.x, tmp.y, tmp.z, tmp.w);
    parallax.minPoint = config.GetFloat3("Parallax-Min", float3::zero);
    parallax.maxPoint = config.GetFloat3("Parallax-Max", float3::zero);
    influence.minPoint = config.GetFloat3("Influence-Min", float3::zero);
    influence.maxPoint = config.GetFloat3("Influence-Max", float3::zero);
    farPlane = config.GetFloat("FarPlane", 100.0f);
    resolution = config.GetUInt("Resolution", 512);
    numSamples = config.GetUInt("Samples", 2048);
    roughnessLevels = config.GetUInt("RoughnessLevels", 8);
    enabled = config.GetBool("Enabled", true);
}

void LocalIBLLight::generate()
{
    iblData.generateEnvironment(position, rotation, farPlane, resolution, numSamples, roughnessLevels);
}

