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

    hints[ENABLE_CASCADE_SHADOW].type          = TYPE_BOOL; 
    hints[ENABLE_CASCADE_SHADOW].value.bvalue  = true;

    hints[UPDATE_SHADOW_VOLUME].type                 = TYPE_BOOL; 
    hints[UPDATE_SHADOW_VOLUME].value.bvalue         = true;

    hints[SHOW_SHADOW_MAP].type                     = TYPE_BOOL; 
    hints[SHOW_SHADOW_MAP].value.bvalue             = false;

    hints[SHADOW_BIAS].type                       = TYPE_FLOAT;
    hints[SHADOW_BIAS].value.fvalue               = 0.05f;

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

    hints[SHOW_PARTICLE_BILLBOARDS].type          = TYPE_BOOL;
    hints[SHOW_PARTICLE_BILLBOARDS].value.bvalue  = false;
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
    config->AddFloat("Metric proprotion", hints[METRIC_PROPORTION].value.fvalue);
    config->AddFloat("Fresnel", hints[ENABLE_FRESNEL].value.bvalue);

    config->AddInt("Tonemapping", hints[TONEMAPPING].value.ivalue);
    config->AddBool("Enable msaa", hints[ENABLE_MSAA].value.bvalue);
    config->AddBool("Enable gamma", hints[ENABLE_GAMMA].value.bvalue);
    config->AddBool("Enable bloom", hints[ENABLE_BLOOM].value.bvalue);

    config->AddBool("Show billboards", hints[SHOW_PARTICLE_BILLBOARDS].value.bvalue);

    Config dHintsCfg = config->AddSection("DHits");

    for(auto it = dhints.begin(); it != dhints.end(); ++it)
    {
        const std::string& name = it->first;
        std::visit(overload {
            [name, &dHintsCfg](float value) { dHintsCfg.AddFloat(name.c_str(), value);},
            [name, &dHintsCfg](bool value)  { dHintsCfg.AddBool(name.c_str(), value);},
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

    hints[SHOW_PARTICLE_BILLBOARDS].value.bvalue = config->GetBool("Show billboards", false);

    Config dHintsCfg = config->GetSection("DHits");
    json_object_t* dHintsRoot = dHintsCfg.GetRoot();

    for(int i=0, count = json_object_get_count(dHintsRoot) ; i< count; ++i)
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
