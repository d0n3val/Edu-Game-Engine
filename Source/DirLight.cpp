#include "Globals.h"

#include "DirLight.h"

// ---------------------------------------------------------
DirLight::DirLight() 
{
}

// ---------------------------------------------------------
DirLight::~DirLight()
{
}

// ---------------------------------------------------------
void DirLight::OnSave(Config& config) const
{
	//config.AddUInt("type", uint(type));
	//config.AddFloat4("color", color);
}

// ---------------------------------------------------------
void DirLight::OnLoad(Config* config) 
{
	//type  = Type(config->GetUInt("type", uint(POINT)));
    //color = config->GetFloat4("color", float4(1.0f, 1.0f, 1.0f, 1.0f));
}

float3 DirLight::GetDir() const
{
    return float3(-std::sin(polar)*std::cos(azimuthal), -std::cos(polar), -std::sin(polar)*std::sin(azimuthal));
}
