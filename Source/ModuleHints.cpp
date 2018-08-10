#include "ModuleHints.h"
#include "DemoMgr.h"

ModuleHints::ModuleHints() : Module("hints")
{
    hints[unsigned(ENABLE_NORMAL_MAPPING)].type                = TYPE_BOOL; 
    hints[unsigned(ENABLE_NORMAL_MAPPING)].value.bvalue        = true;

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
}

ModuleHints::~ModuleHints()
{
}

