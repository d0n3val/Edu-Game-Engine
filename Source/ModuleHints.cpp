#include "Globals.h"
#include "ModuleHints.h"

#include "Config.h"

ModuleHints::ModuleHints() : Module("Render Hints")
{
    hints[unsigned(ENABLE_NORMAL_MAPPING)].type                = TYPE_BOOL; 
    hints[unsigned(ENABLE_NORMAL_MAPPING)].value.bvalue        = true;

    hints[unsigned(SHOW_TANGENT_SPACE)].type                   = TYPE_BOOL; 
    hints[unsigned(SHOW_TANGENT_SPACE)].value.bvalue           = false;

    hints[unsigned(ENABLE_SPECULAR_MAPPING)].type              = TYPE_BOOL; 
    hints[unsigned(ENABLE_SPECULAR_MAPPING)].value.bvalue      = true;

    hints[unsigned(ENABLE_SHADOW_MAPPING)].type                = TYPE_BOOL; 
    hints[unsigned(ENABLE_SHADOW_MAPPING)].value.bvalue        = true;

    hints[unsigned(SHOW_SHADOW_CLIPPING)].type                 = TYPE_BOOL; 
    hints[unsigned(SHOW_SHADOW_CLIPPING)].value.bvalue         = false;

    hints[unsigned(ENABLE_SHADOW_FRONT_CULLING)].type          = TYPE_BOOL; 
    hints[unsigned(ENABLE_SHADOW_FRONT_CULLING)].value.bvalue  = true;

    hints[unsigned(SHADOW_BIAS)].type                          = TYPE_FLOAT;
    hints[unsigned(SHADOW_BIAS)].value.fvalue                  = 0.05f;

    hints[unsigned(SHADOW_RESOLUTION)].type                    = TYPE_FLOAT;
    hints[unsigned(SHADOW_RESOLUTION)].value.fvalue            = 1.0f;

	hints[unsigned(METRIC_PROPORTION)].type					   = TYPE_FLOAT;
	hints[unsigned(METRIC_PROPORTION)].value.fvalue			   = 1.0f;
}

ModuleHints::~ModuleHints()
{
}

void ModuleHints::Save(Config* config) const 
{
    config->AddBool("Normal mapping", hints[unsigned(ENABLE_NORMAL_MAPPING)].value.bvalue);
    config->AddBool("Show tangent space", hints[unsigned(SHOW_TANGENT_SPACE)].value.bvalue);
    config->AddBool("Specular mapping", hints[unsigned(ENABLE_SPECULAR_MAPPING)].value.bvalue);
    config->AddBool("Shadow mapping", hints[unsigned(ENABLE_SHADOW_MAPPING)].value.bvalue);
    config->AddBool("Show shadow clipping", hints[unsigned(SHOW_SHADOW_CLIPPING)].value.bvalue);
    config->AddBool("Enable shadow front culling", hints[unsigned(ENABLE_SHADOW_FRONT_CULLING)].value.bvalue);

    config->AddFloat("Shadow bias", hints[unsigned(SHADOW_BIAS)].value.fvalue);
    config->AddFloat("Shadow resolution", hints[unsigned(SHADOW_RESOLUTION)].value.fvalue);
    config->AddFloat("Metric proprotion", hints[unsigned(METRIC_PROPORTION)].value.fvalue);
}

bool ModuleHints::Init(Config* config) 
{
    hints[unsigned(ENABLE_NORMAL_MAPPING)].value.bvalue = config->GetBool("Normal mapping", true);
    hints[unsigned(SHOW_TANGENT_SPACE)].value.bvalue = config->GetBool("Show tangent space", false);
    hints[unsigned(ENABLE_SPECULAR_MAPPING)].value.bvalue = config->GetBool("Specular mapping", true);
    hints[unsigned(ENABLE_SHADOW_MAPPING)].value.bvalue = config->GetBool("Shadow mapping", true);
    hints[unsigned(SHOW_SHADOW_CLIPPING)].value.bvalue = config->GetBool("Show shadow clipping", false);
    hints[unsigned(ENABLE_SHADOW_FRONT_CULLING)].value.bvalue = config->GetBool("Enable shadow front culling", true);

    hints[unsigned(SHADOW_BIAS)].value.fvalue = config->GetFloat("Shadow bias", 0.05f);
    hints[unsigned(SHADOW_RESOLUTION)].value.fvalue = config->GetFloat("Shadow resolution", 1.0f);
    hints[unsigned(METRIC_PROPORTION)].value.fvalue = config->GetFloat("Metric proprotion", 1.0f);

    return true;
}

