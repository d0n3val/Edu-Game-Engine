#include "Globals.h"

#include "DirLight.h"

#include "Config.h"

// ---------------------------------------------------------
DirLight::DirLight() 
{
}

// ---------------------------------------------------------
DirLight::~DirLight()
{
}

// ---------------------------------------------------------
void DirLight::Save(Config& config) const
{
    config.AddFloat("polar", polar);
    config.AddFloat("azimuthal", azimuthal);
	config.AddFloat3("color", color);
}

// ---------------------------------------------------------
void DirLight::Load(Config& config) 
{
    polar = config.GetFloat("polar", PI/2.0f);
    azimuthal = config.GetFloat("azimuthal", PI/2.0f);
    color = config.GetFloat3("color", float3(1.0f, 1.0f, 1.0f));
}

// ---------------------------------------------------------
float3 DirLight::GetDir() const
{
    return float3(-sin(polar)*cos(azimuthal), -cos(polar), -sin(polar)*sin(azimuthal));
}
