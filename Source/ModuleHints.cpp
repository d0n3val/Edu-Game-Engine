#include "Globals.h"
#include "ModuleHints.h"

#include "Config.h"

ModuleHints::ModuleHints() : Module("Render Hints")
{
    hints[ENABLE_NORMAL_MAPPING].type             = TYPE_BOOL; 
    hints[ENABLE_NORMAL_MAPPING].value.bvalue     = true;

    hints[SHOW_TANGENT_SPACE].type                = TYPE_BOOL; 
    hints[SHOW_TANGENT_SPACE].value.bvalue        = false;

    hints[ENABLE_SPECULAR_MAPPING].type           = TYPE_BOOL; 
    hints[ENABLE_SPECULAR_MAPPING].value.bvalue   = true;

    hints[ENABLE_SHADOW_MAPPING].type             = TYPE_BOOL; 
    hints[ENABLE_SHADOW_MAPPING].value.bvalue     = true;

    hints[SHOW_SHADOW_CLIPPING].type              = TYPE_BOOL; 
    hints[SHOW_SHADOW_CLIPPING].value.bvalue      = false;

    hints[ENABLE_SHADOW_FRONT_CULLING].type          = TYPE_BOOL; 
    hints[ENABLE_SHADOW_FRONT_CULLING].value.bvalue  = true;

    hints[SHADOW_BIAS].type                       = TYPE_FLOAT;
    hints[SHADOW_BIAS].value.fvalue               = 0.05f;

    hints[SHADOW_RESOLUTION].type                 = TYPE_FLOAT;
    hints[SHADOW_RESOLUTION].value.fvalue         = 1.0f;

	hints[METRIC_PROPORTION].type				  = TYPE_FLOAT;
	hints[METRIC_PROPORTION].value.fvalue	      = 1.0f;

    hints[ENABLE_FRESNEL].type                    = TYPE_BOOL; 
    hints[ENABLE_FRESNEL].value.bvalue            = true;

    hints[TONEMAPPING].type                       = TYPE_INT; 
    hints[TONEMAPPING].value.bvalue               = 0;

    hints[ENABLE_MSAA].type                       = TYPE_BOOL; 
    hints[ENABLE_MSAA].value.bvalue               = true;

}

ModuleHints::~ModuleHints()
{
}

void ModuleHints::Save(Config* config) const 
{
    config->AddBool("Normal mapping", hints[ENABLE_NORMAL_MAPPING].value.bvalue);
    config->AddBool("Show tangent space", hints[SHOW_TANGENT_SPACE].value.bvalue);
    config->AddBool("Specular mapping", hints[ENABLE_SPECULAR_MAPPING].value.bvalue);
    config->AddBool("Shadow mapping", hints[ENABLE_SHADOW_MAPPING].value.bvalue);
    config->AddBool("Show shadow clipping", hints[SHOW_SHADOW_CLIPPING].value.bvalue);
    config->AddBool("Enable shadow front culling", hints[ENABLE_SHADOW_FRONT_CULLING].value.bvalue);

    config->AddFloat("Shadow bias", hints[SHADOW_BIAS].value.fvalue);
    config->AddFloat("Shadow resolution", hints[SHADOW_RESOLUTION].value.fvalue);
    config->AddFloat("Metric proprotion", hints[METRIC_PROPORTION].value.fvalue);
    config->AddFloat("Fresnel", hints[ENABLE_FRESNEL].value.bvalue);

    config->AddInt("Tonemapping", hints[TONEMAPPING].value.ivalue);
    config->AddBool("Enable msaa", hints[ENABLE_MSAA].value.bvalue);
}

bool ModuleHints::Init(Config* config) 
{
    hints[ENABLE_NORMAL_MAPPING].value.bvalue = config->GetBool("Normal mapping", true);
    hints[SHOW_TANGENT_SPACE].value.bvalue = config->GetBool("Show tangent space", false);
    hints[ENABLE_SPECULAR_MAPPING].value.bvalue = config->GetBool("Specular mapping", true);
    hints[ENABLE_SHADOW_MAPPING].value.bvalue = config->GetBool("Shadow mapping", true);
    hints[SHOW_SHADOW_CLIPPING].value.bvalue = config->GetBool("Show shadow clipping", false);
    hints[ENABLE_SHADOW_FRONT_CULLING].value.bvalue = config->GetBool("Enable shadow front culling", true);

    hints[SHADOW_BIAS].value.fvalue = config->GetFloat("Shadow bias", 0.05f);
    hints[SHADOW_RESOLUTION].value.fvalue = config->GetFloat("Shadow resolution", 1.0f);
    hints[METRIC_PROPORTION].value.fvalue = config->GetFloat("Metric proprotion", 1.0f);
    hints[ENABLE_FRESNEL].value.bvalue = config->GetBool("Fresnel", true);

    hints[TONEMAPPING].value.ivalue = config->GetInt("Tonemapping", 0);
    hints[ENABLE_MSAA].value.bvalue = config->GetBool("Enable msaa", true);

    return true;
}

