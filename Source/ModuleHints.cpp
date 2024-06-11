#include "Globals.h"
#include "ModuleHints.h"

#include "Config.h"
#include "visit_variant.h"
#include "parson.h"

#include "Leaks.h"

namespace
{
    ModuleHints::DValue ParseDValue(JSON_Value* value)
    {
        ModuleHints::DValue dvalue;

        switch (json_value_get_type(value))
        {
        case JSONNumber:
            dvalue = (float)json_value_get_number(value);
            break;
        case JSONArray:
        {
            JSON_Array* array = json_value_get_array(value);
            if (json_array_get_count(array) == 2 && json_value_get_type(json_array_get_value(array, 0)) == JSONNumber &&
                json_value_get_type(json_array_get_value(array, 1)) == JSONNumber)
            {
                dvalue = float2((float)json_value_get_number(json_array_get_value(array, 0)), (float)json_value_get_number(json_array_get_value(array, 1)));
            }
            else if (json_array_get_count(array) == 3 && json_value_get_type(json_array_get_value(array, 0)) == JSONNumber &&
                json_value_get_type(json_array_get_value(array, 1)) == JSONNumber && json_value_get_type(json_array_get_value(array, 2)) == JSONNumber)
            {
                dvalue = float3((float)json_value_get_number(json_array_get_value(array, 0)), (float)json_value_get_number(json_array_get_value(array, 1)),
                                (float)json_value_get_number(json_array_get_value(array, 2)));
            }
            

            break;
        }
        case JSONBoolean:
            dvalue = json_value_get_boolean(value) ? true : false;
            break;
        default:
            break;
        }

        return dvalue;
    }
}

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

    hints[ENABLE_CASCADE_SHADOW].type             = TYPE_BOOL; 
    hints[ENABLE_CASCADE_SHADOW].value.bvalue     = true;

    hints[UPDATE_SHADOW_VOLUME].type              = TYPE_BOOL; 
    hints[UPDATE_SHADOW_VOLUME].value.bvalue      = true;

    hints[SHOW_SHADOW_MAP].type                   = TYPE_BOOL; 
    hints[SHOW_SHADOW_MAP].value.bvalue           = false;

    hints[SHADOW_BIAS].type                       = TYPE_FLOAT;
    hints[SHADOW_BIAS].value.fvalue               = 0.05f;

    hints[SHADOW_SLOPEBIAS].type                  = TYPE_FLOAT;
    hints[SHADOW_SLOPEBIAS].value.fvalue          = 0.05f;

    hints[SHADOW_CASCADE_0_DEPTH].type            = TYPE_FLOAT2;
    SetFloat2Value(SHADOW_CASCADE_0_DEPTH, float2(0.01f, 15.0f));

    hints[SHADOW_CASCADE_0_RES].type              = TYPE_FLOAT2;
    SetFloat2Value(SHADOW_CASCADE_0_RES, float2(640.0f, 640.0f));

    hints[SHADOW_CASCADE_0_UPDATE].type           = TYPE_INT;
    SetIntValue(SHADOW_CASCADE_0_UPDATE, 1);

    hints[SHADOW_CASCADE_1_DEPTH].type            = TYPE_FLOAT;
    hints[SHADOW_CASCADE_1_DEPTH].value.fvalue    = 10.0f;

    hints[SHADOW_CASCADE_1_RES].type              = TYPE_FLOAT2;
    SetFloat2Value(SHADOW_CASCADE_1_RES, float2(640.0f, 640.0f));

    hints[SHADOW_CASCADE_1_UPDATE].type           = TYPE_INT;
    SetIntValue(SHADOW_CASCADE_1_UPDATE, 1);

    hints[SHADOW_CASCADE_2_DEPTH].type            = TYPE_FLOAT;
    hints[SHADOW_CASCADE_2_DEPTH].value.fvalue    = 50.0f;

    hints[SHADOW_CASCADE_2_RES].type              = TYPE_FLOAT2;
    SetFloat2Value(SHADOW_CASCADE_2_RES, float2(640.0f, 642.0f)); 

    hints[SHADOW_CASCADE_2_UPDATE].type           = TYPE_INT;
    SetIntValue(SHADOW_CASCADE_2_UPDATE, 1);

    hints[SHADOW_SHOW_CASCADES].type              = TYPE_BOOL;
    SetBoolValue(SHADOW_SHOW_CASCADES, false);

    hints[SHADOW_ENABLE_SOFT].type = TYPE_BOOL;
    SetBoolValue(SHADOW_ENABLE_SOFT, true);

    hints[SHADOW_PCF_SIZE].type = TYPE_INT;
    SetIntValue(SHADOW_PCF_SIZE, 1);


	hints[METRIC_PROPORTION].type				  = TYPE_FLOAT;
	hints[METRIC_PROPORTION].value.fvalue	      = 1.0f;

    hints[ENABLE_FRESNEL].type                    = TYPE_BOOL; 
    hints[ENABLE_FRESNEL].value.bvalue            = true;

    hints[TONEMAPPING].type                       = TYPE_INT; 
    hints[TONEMAPPING].value.bvalue               = 0;

    hints[ENABLE_MSAA].type                       = TYPE_BOOL; 
    hints[ENABLE_MSAA].value.bvalue               = true;

    hints[ENABLE_GAMMA].type                      = TYPE_BOOL; 
    hints[ENABLE_GAMMA].value.bvalue              = true;

    hints[ENABLE_BLOOM].type                      = TYPE_BOOL; 
    hints[ENABLE_BLOOM].value.bvalue              = true;

    hints[ENABLE_LUT].type                        = TYPE_BOOL; 
    hints[ENABLE_LUT].value.bvalue                = true;

    hints[BLOOM_INTENSITY].type                   = TYPE_FLOAT; 
    hints[BLOOM_INTENSITY].value.fvalue           = 1.0f;

    hints[SHOW_PARTICLE_BILLBOARDS].type          = TYPE_BOOL;
    hints[SHOW_PARTICLE_BILLBOARDS].value.bvalue  = false;

    hints[FOG_TYPE].type          = TYPE_INT;
    hints[FOG_TYPE].value.ivalue  = 0;

    hints[DIST_FOG_COLOUR].type          = TYPE_FLOAT3;
    hints[DIST_FOG_COLOUR].value.f3value[0] = hints[DIST_FOG_COLOUR].value.f3value[1] = hints[DIST_FOG_COLOUR].value.f3value[2] = 1;

    hints[DIST_FOG_MIN].type = TYPE_FLOAT;
    hints[DIST_FOG_MIN].value.fvalue = 0.0;
                
    hints[DIST_FOG_MAX].type = TYPE_FLOAT;
    hints[DIST_FOG_MAX].value.fvalue = 1000.0;

    hints[DIST_FOG_CURVE].type = TYPE_FLOAT4;
    hints[DIST_FOG_CURVE].value.f4value[0] = hints[DIST_FOG_CURVE].value.f4value[1] = hints[DIST_FOG_CURVE].value.f4value[2] = hints[DIST_FOG_CURVE].value.f4value[3] = 1;

    hints[DIST_FOG_BLUR].type = TYPE_BOOL;
    hints[DIST_FOG_BLUR].value.bvalue = true;

    hints[HEIGHT_FOG_GLOBAL_DENSITY].type = TYPE_FLOAT;
    hints[HEIGHT_FOG_GLOBAL_DENSITY].value.fvalue = 0.1f;
    hints[HEIGHT_FOG_DENSITY_FALLOFF].type = TYPE_FLOAT;
    hints[HEIGHT_FOG_DENSITY_FALLOFF].value.fvalue = 0.1f;
    
    hints[HEIGHT_FOG_COLOR].type = TYPE_FLOAT3;
    hints[HEIGHT_FOG_COLOR].value.f3value[0] = 0.5f;
    hints[HEIGHT_FOG_COLOR].value.f3value[1] = 0.6f;
    hints[HEIGHT_FOG_COLOR].value.f3value[2] = 0.7f;

    hints[HEIGHT_FOG_SUN_COLOR].type = TYPE_FLOAT3;
    hints[HEIGHT_FOG_SUN_COLOR].value.f3value[0] = 1.0f;
    hints[HEIGHT_FOG_SUN_COLOR].value.f3value[1] = 0.9f;
    hints[HEIGHT_FOG_SUN_COLOR].value.f3value[2] = 0.7f;


    hints[RAYMARCHING_BLUR].type = TYPE_BOOL;
    hints[RAYMARCHING_BLUR].value.bvalue = true;

    hints[RAYMARCHING_AMBIENT_COLOUR].type = TYPE_FLOAT3;
    hints[RAYMARCHING_AMBIENT_COLOUR].value.f3value[0] = hints[RAYMARCHING_AMBIENT_COLOUR].value.f3value[1] = hints[RAYMARCHING_AMBIENT_COLOUR].value.f3value[2] = 0.0f;

    hints[RAYMARCHING_EXTINCTION_COEFF].type = TYPE_FLOAT;
    hints[RAYMARCHING_EXTINCTION_COEFF].value.fvalue = 0.0f;

    hints[RAYMARCHING_FOG_INTENSITY].type = TYPE_FLOAT;
    hints[RAYMARCHING_FOG_INTENSITY].value.fvalue = 1.0f;
    
    hints[RAYMARCHING_STEP_SIZE].type = TYPE_FLOAT;
    hints[RAYMARCHING_STEP_SIZE].value.fvalue = 0.05f;

    hints[RAYMARCHING_ATT_CORRECTION].type = TYPE_FLOAT;
    hints[RAYMARCHING_ATT_CORRECTION].value.fvalue = 1.0f;
    
    hints[RAYMARCHING_NOISE_SCALE].type = TYPE_FLOAT;
    hints[RAYMARCHING_NOISE_SCALE].value.fvalue = 0.0f;
    
    hints[RAYMARCHING_NOISE_SPEED].type = TYPE_FLOAT;
    hints[RAYMARCHING_NOISE_SPEED].value.fvalue = 1.0f;

    hints[EXPOSURE].type = TYPE_FLOAT;
    hints[EXPOSURE].value.fvalue = 0.0f;

    hints[ABERRATION_OFFSET].type = TYPE_FLOAT3;
    hints[ABERRATION_OFFSET].value.f3value[0] = 0.015f;
    hints[ABERRATION_OFFSET].value.f3value[1] = 0.008f;
    hints[ABERRATION_OFFSET].value.f3value[2] = -0.008f;

    hints[ABERRATION_BLOOM_OFFSET].type = TYPE_FLOAT3;
    hints[ABERRATION_BLOOM_OFFSET].value.f3value[0] = 0.025f;
    hints[ABERRATION_BLOOM_OFFSET].value.f3value[1] = 0.01f;
    hints[ABERRATION_BLOOM_OFFSET].value.f3value[2] = -0.01f;

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
    config->AddBool("Enable shadow front culling", hints[ENABLE_CASCADE_SHADOW].value.bvalue);
    config->AddBool("Update shadow volume", hints[UPDATE_SHADOW_VOLUME].value.bvalue);
    config->AddBool("Show shadow map", hints[SHOW_SHADOW_MAP].value.bvalue);

    config->AddFloat2("Shadow cascade 0 depth", GetFloat2Value(SHADOW_CASCADE_0_DEPTH));
    config->AddFloat2("Shadow cascade 0 res", GetFloat2Value(SHADOW_CASCADE_0_RES));
    config->AddInt("Shadow cascade 0 update", GetIntValue(SHADOW_CASCADE_0_UPDATE));

    config->AddFloat("Shadow cascade 1 depth", hints[SHADOW_CASCADE_1_DEPTH].value.fvalue);
    config->AddFloat2("Shadow cascade 1 res", GetFloat2Value(SHADOW_CASCADE_1_RES));
    config->AddInt("Shadow cascade 1 update", GetIntValue(SHADOW_CASCADE_1_UPDATE));

    config->AddFloat("Shadow cascade 2 depth", hints[SHADOW_CASCADE_2_DEPTH].value.fvalue);
    config->AddFloat2("Shadow cascade 2 res", GetFloat2Value(SHADOW_CASCADE_2_RES));
    config->AddInt("Shadow cascade 2 update", GetIntValue(SHADOW_CASCADE_2_UPDATE));
    config->AddBool("Shadow show cascades", GetBoolValue(SHADOW_SHOW_CASCADES));

    config->AddBool("Shadow enable soft", GetBoolValue(SHADOW_ENABLE_SOFT));
    config->AddInt("Shadow pcf size", GetIntValue(SHADOW_PCF_SIZE));

    config->AddFloat("Shadow bias", hints[SHADOW_BIAS].value.fvalue);
    config->AddFloat("Shadow slope bias", hints[SHADOW_SLOPEBIAS].value.fvalue);
    config->AddFloat("Metric proprotion", hints[METRIC_PROPORTION].value.fvalue);
    config->AddFloat("Fresnel", hints[ENABLE_FRESNEL].value.bvalue);

    config->AddInt("Tonemapping", hints[TONEMAPPING].value.ivalue);
    config->AddBool("Enable msaa", hints[ENABLE_MSAA].value.bvalue);
    config->AddBool("Enable gamma", hints[ENABLE_GAMMA].value.bvalue);
    config->AddBool("Enable bloom", hints[ENABLE_BLOOM].value.bvalue);
    config->AddBool("Enable LUT", hints[ENABLE_LUT].value.bvalue);
    config->AddFloat("Bloom intensity", hints[BLOOM_INTENSITY].value.fvalue);

    config->AddBool("Show billboards", hints[SHOW_PARTICLE_BILLBOARDS].value.bvalue);

    config->AddInt("Fog Type", hints[FOG_TYPE].value.ivalue);
    config->AddFloat3("Distance Fog Colour", float3(hints[DIST_FOG_COLOUR].value.f3value));
    config->AddFloat("Distance Fog Min", hints[DIST_FOG_MIN].value.fvalue);
    config->AddFloat("Distance Fog Max", hints[DIST_FOG_MAX].value.fvalue);
    config->AddFloat4("Distance Fog Curve", float4(hints[DIST_FOG_CURVE].value.f4value));
    config->AddBool("Distance Fog Blur", hints[DIST_FOG_BLUR].value.bvalue);

    config->AddFloat("Height Fog global density", hints[HEIGHT_FOG_GLOBAL_DENSITY].value.fvalue);
    config->AddFloat("Height Fog density falloff", hints[HEIGHT_FOG_DENSITY_FALLOFF].value.fvalue);
    config->AddFloat3("Height Fog color", float3(hints[HEIGHT_FOG_COLOR].value.f3value));
    config->AddFloat3("Height Fog sun color", float3(hints[HEIGHT_FOG_SUN_COLOR].value.f3value));

    config->AddBool("RayMarching Blur", hints[RAYMARCHING_BLUR].value.bvalue);
    config->AddFloat3("RayMarching Ambient", float3(hints[RAYMARCHING_AMBIENT_COLOUR].value.f3value));
    config->AddFloat("RayMarching Extinction", hints[RAYMARCHING_EXTINCTION_COEFF].value.fvalue);
    config->AddFloat("RayMarching Fog Intensity", hints[RAYMARCHING_FOG_INTENSITY].value.fvalue);
    config->AddFloat("RayMarching Step Size", hints[RAYMARCHING_STEP_SIZE].value.fvalue);
    config->AddFloat("RayMarching Att Correction", hints[RAYMARCHING_ATT_CORRECTION].value.fvalue);
    config->AddFloat("RayMarching Noise Scale", hints[RAYMARCHING_NOISE_SCALE].value.fvalue);
    config->AddFloat("RayMarching Noise Speed", hints[RAYMARCHING_NOISE_SPEED].value.fvalue);
    config->AddFloat("Exposure", hints[EXPOSURE].value.fvalue);

    config->AddFloat3("Aberration offset", float3(hints[ABERRATION_OFFSET].value.f3value));
    config->AddFloat3("Aberration bloom offset", float3(hints[ABERRATION_BLOOM_OFFSET].value.f3value));

    Config dHintsCfg = config->AddSection("DHits");

    for(auto it = dhints.begin(); it != dhints.end(); ++it)
    {
        const std::string& name = it->first;
        std::visit(overload {
            [name, &dHintsCfg](float value) { dHintsCfg.AddFloat(name.c_str(), value);},
            [name, &dHintsCfg](bool value)  { dHintsCfg.AddBool(name.c_str(), value);},
            [name, &dHintsCfg](int value) { dHintsCfg.AddInt(name.c_str(), value); },
            [name, &dHintsCfg](float2 value)  { dHintsCfg.AddFloat2(name.c_str(), value); },
            [name, &dHintsCfg](float3 value) { dHintsCfg.AddFloat3(name.c_str(), value);  },            
            }, it->second);
    }
}

bool ModuleHints::Init(Config* config) 
{
    hints[ENABLE_NORMAL_MAPPING].value.bvalue = config->GetBool("Normal mapping", true);
    hints[SHOW_TANGENT_SPACE].value.bvalue = config->GetBool("Show tangent space", false);
    hints[ENABLE_SPECULAR_MAPPING].value.bvalue = config->GetBool("Specular mapping", true);
    hints[ENABLE_SHADOW_MAPPING].value.bvalue = config->GetBool("Shadow mapping", true);
    hints[SHOW_SHADOW_CLIPPING].value.bvalue = config->GetBool("Show shadow clipping", false);
    hints[ENABLE_CASCADE_SHADOW].value.bvalue = config->GetBool("Enable shadow front culling", true);
    hints[UPDATE_SHADOW_VOLUME].value.bvalue = config->GetBool("Update shadow volume", true);
    hints[SHOW_SHADOW_MAP].value.bvalue = config->GetBool("Show shadow map", true);

    hints[SHADOW_BIAS].value.fvalue = config->GetFloat("Shadow bias", 0.05f);
    hints[SHADOW_SLOPEBIAS].value.fvalue = config->GetFloat("Shadow slope bias", 0.05f);
    hints[METRIC_PROPORTION].value.fvalue = config->GetFloat("Metric proprotion", 1.0f);
    hints[ENABLE_FRESNEL].value.bvalue = config->GetBool("Fresnel", true);

    SetFloat2Value(SHADOW_CASCADE_0_DEPTH, config->GetFloat2("Shadow cascade 0 depth", float2(0.01f, 15.0f)));
    SetFloat2Value(SHADOW_CASCADE_0_RES, config->GetFloat2("Shadow cascade 0 res", float2(640.0f, 640.0f)));
    SetIntValue(SHADOW_CASCADE_0_UPDATE, config->GetInt("Shadow cascade 0 update", 1));

    hints[SHADOW_CASCADE_1_DEPTH].value.fvalue = config->GetFloat("Shadow cascade 1 depth", 25.0f);
    SetFloat2Value(SHADOW_CASCADE_1_RES, config->GetFloat2("Shadow cascade 1 res", float2(640.0f, 640.0f)));
    SetIntValue(SHADOW_CASCADE_1_UPDATE, config->GetInt("Shadow cascade 1 update", 1));

    hints[SHADOW_CASCADE_2_DEPTH].value.fvalue = config->GetFloat("Shadow cascade 2 depth", 100.0f);
    SetFloat2Value(SHADOW_CASCADE_2_RES, config->GetFloat2("Shadow cascade 2 res", float2(640.0f, 640.0f)));
    SetIntValue(SHADOW_CASCADE_2_UPDATE, config->GetInt("Shadow cascade 2 update", 1));

    SetBoolValue(SHADOW_SHOW_CASCADES, config->GetBool("Shadow show cascades", false));

    SetBoolValue(SHADOW_ENABLE_SOFT, config->GetBool("Shadow enable soft", false));
    SetIntValue(SHADOW_PCF_SIZE, config->GetInt("Shadow pcf size", 1));

    hints[TONEMAPPING].value.ivalue = config->GetInt("Tonemapping", 0);
    hints[ENABLE_MSAA].value.bvalue = config->GetBool("Enable msaa", true);
    hints[ENABLE_GAMMA].value.bvalue = config->GetBool("Enable gamma", true);
    hints[ENABLE_BLOOM].value.bvalue = config->GetBool("Enable bloom", true);
    hints[ENABLE_LUT].value.bvalue = config->GetBool("Enable LUT", true);
    hints[BLOOM_INTENSITY].value.fvalue = config->GetFloat("Bloom intensity", 1.0f);

    hints[SHOW_PARTICLE_BILLBOARDS].value.bvalue = config->GetBool("Show billboards", false);
    
    hints[FOG_TYPE].value.ivalue = config->GetInt("Fog Type", 0);
    float3 colour = config->GetFloat3("Distance Fog Colour");
    hints[DIST_FOG_COLOUR].value.f3value[0] = colour[0];
    hints[DIST_FOG_COLOUR].value.f3value[1] = colour[1];
    hints[DIST_FOG_COLOUR].value.f3value[2] = colour[2];

    hints[DIST_FOG_MIN].value.fvalue = config->GetFloat("Distance Fog Min");
    hints[DIST_FOG_MAX].value.fvalue = config->GetFloat("Distance Fog Max");

    float4 curve = config->GetFloat4("Distance Fog Curve");
    hints[DIST_FOG_CURVE].value.f3value[0] = curve[0];
    hints[DIST_FOG_CURVE].value.f3value[1] = curve[1];
    hints[DIST_FOG_CURVE].value.f3value[2] = curve[2];
    hints[DIST_FOG_CURVE].value.f3value[3] = curve[3];

    hints[DIST_FOG_BLUR].value.bvalue = config->GetBool("Distance Fog Blur");

    hints[HEIGHT_FOG_GLOBAL_DENSITY].value.fvalue = config->GetFloat("Height Fog global density");
    hints[HEIGHT_FOG_DENSITY_FALLOFF].value.fvalue = config->GetFloat("Height Fog density falloff");
    float3 color = config->GetFloat3("Height Fog color");
    hints[HEIGHT_FOG_COLOR].value.f3value[0] = color[0];
    hints[HEIGHT_FOG_COLOR].value.f3value[1] = color[1];
    hints[HEIGHT_FOG_COLOR].value.f3value[2] = color[2];
    color = config->GetFloat3("Height Fog sun color");
    hints[HEIGHT_FOG_SUN_COLOR].value.f3value[0] = color[0];
    hints[HEIGHT_FOG_SUN_COLOR].value.f3value[1] = color[1];
    hints[HEIGHT_FOG_SUN_COLOR].value.f3value[2] = color[2];

    hints[RAYMARCHING_BLUR].value.bvalue = config->GetBool("RayMarching Blur");

    float3 ambient = config->GetFloat3("RayMarching Ambient");
    hints[RAYMARCHING_AMBIENT_COLOUR].value.f3value[0] = ambient[0];
    hints[RAYMARCHING_AMBIENT_COLOUR].value.f3value[1] = ambient[1];
    hints[RAYMARCHING_AMBIENT_COLOUR].value.f3value[2] = ambient[2];

    hints[RAYMARCHING_EXTINCTION_COEFF].value.fvalue = config->GetFloat("RayMarching Extinction");
    hints[RAYMARCHING_FOG_INTENSITY].value.fvalue = config->GetFloat("RayMarching Fog Intensity");
    
    hints[RAYMARCHING_STEP_SIZE].value.fvalue = config->GetFloat("RayMarching Step Size");
    hints[RAYMARCHING_ATT_CORRECTION].value.fvalue = config->GetFloat("RayMarching Att Correction");
    hints[RAYMARCHING_NOISE_SCALE].value.fvalue = config->GetFloat("RayMarching Noise Scale");
    hints[RAYMARCHING_NOISE_SPEED].value.fvalue = config->GetFloat("RayMarching Noise Speed");
    hints[EXPOSURE].value.fvalue = config->GetFloat("Exposure");

    float3 offset = config->GetFloat3("Aberration offset", float3(0.015f, 0.008f, -0.008f));
    hints[ABERRATION_OFFSET].value.f3value[0] = offset.x;
    hints[ABERRATION_OFFSET].value.f3value[1] = offset.y;
    hints[ABERRATION_OFFSET].value.f3value[2] = offset.z;

    offset = config->GetFloat3("Aberration bloom offset", float3(0.025f, 0.1f, -0.1f));
    hints[ABERRATION_BLOOM_OFFSET].value.f3value[0] = offset.x;
    hints[ABERRATION_BLOOM_OFFSET].value.f3value[1] = offset.y;
    hints[ABERRATION_BLOOM_OFFSET].value.f3value[2] = offset.z;

    Config dHintsCfg = config->GetSection("DHits");
    json_object_t* dHintsRoot = dHintsCfg.GetRoot();

    for(int i=0, count = int(json_object_get_count(dHintsRoot)) ; i< count; ++i)
    {
        const char *name = json_object_get_name(dHintsRoot, i);
        dhints[std::string(name)] = ParseDValue(json_object_get_value_at(dHintsRoot, i));
    }

    return true;
}

void ModuleHints::SetDHint(const std::string& name, const DValue& value)
{
    dhints[name] = value;
}

const ModuleHints::DValue& ModuleHints::GetDHint(const std::string& name, const DValue& defaultDValue) 
{
    auto it = dhints.find(name);

    if(it != dhints.end())
    {
        return it->second;
    }

    dhints[name] = defaultDValue;

    return defaultDValue;
}
