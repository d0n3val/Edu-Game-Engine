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
    config.AddFloat("Radius", radius);
    config.AddFloat3("Box-Min", box.minPoint);
    config.AddFloat3("Box-Max", box.maxPoint);
	config.AddBool("Enabled", enabled);
}

void LocalIBLLight::Load(Config& config) 
{
    position = config.GetFloat3("Position", float3::zero);
    radius = config.GetFloat("Radius", 0.0f);
    box.minPoint = config.GetFloat3("Box-Min", float3::zero);
    box.maxPoint = config.GetFloat3("Box-Max", float3::zero);
    enabled  = config.GetBool("Enabled", true);
}

void LocalIBLLight::generate()
{
    iblData.generateEnvironment(position);
}

