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
	config.AddBool("Enabled", enabled);
}

void LocalIBLLight::Load(Config& config) 
{
    position = config.GetFloat3("Position", float3::zero);
    enabled  = config.GetBool("Enabled", true);
}

void LocalIBLLight::generate()
{
    iblData.generateEnvironment(position);
}

