#include "Globals.h"

#include "AmbientLight.h"


AmbientLight::AmbientLight()
{
}

AmbientLight::~AmbientLight()
{
}

void AmbientLight::OnSave(Config& config) const
{
	//config.AddUInt("type", uint(type));
	//config.AddFloat4("color", color);
}

void AmbientLight::OnLoad(Config* config)
{
    //type  = Type(config->GetUInt("type", uint(POINT)));
    //color = config->GetFloat4("color", float4(1.0f, 1.0f, 1.0f, 1.0f));
}

