#include "Globals.h"

#include "DirLight.h"

#include "Config.h"

#include "Leaks.h"

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
	config.AddFloat("intensity", intensity);
	config.AddFloat("anisotropy", anisotropy);
}

// ---------------------------------------------------------
void DirLight::Load(Config& config) 
{
    polar = config.GetFloat("polar", PI/2.0f);
    azimuthal = config.GetFloat("azimuthal", PI/2.0f);
    color = config.GetFloat3("color", float3(1.0f, 1.0f, 1.0f));
    intensity = config.GetFloat("intensity", 1.0f);
    anisotropy = config.GetFloat("anisotropy", 0.0f);
}

// ---------------------------------------------------------
float3 DirLight::GetDir() const
{
    return float3(-sin(polar)*cos(azimuthal), -cos(polar), -sin(polar)*sin(azimuthal));
}

// ---------------------------------------------------------
float3 DirLight::GetUp() const
{
    return float3(-sin(polar+PI*0.5f)*cos(azimuthal), -cos(polar+PI*0.5f), -sin(polar+PI*0.5f)*sin(azimuthal));
}
