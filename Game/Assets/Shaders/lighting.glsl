#ifndef _LIGHTING_GLSL_
#define _LIGHTING_GLSL_

#include "/shaders/common.glsl"
#include "/shaders/pbrDefs.glsl"

layout(binding = AO_TEX_BINDING) uniform sampler2D ambientOcclusion;
layout(binding = DIFFUSE_IBL_TEX_BINDING) uniform samplerCube     diffuseIBL;
layout(binding = PREFILTERED_IBL_TEX_BINDING) uniform samplerCube prefilteredIBL;
layout(binding = ENVIRONMENT_BRDF_TEX_BINDING) uniform sampler2D  environmentBRDF;
layout(location = PREFILTERED_LOD_LEVELS_LOCATION) uniform int    prefilteredLevels;

struct DirLight
{
    vec4 dir;
    vec4 color;
};

struct PointLight
{
    vec4 position; // position+radius
    vec4 color;
};

struct SpotLight
{
    vec4  position;
    vec4  direction;
    vec4  color;
    float dist;
    float inner;
    float outer;
    float intensity;
};

readonly layout(std430, binding = DIRLIGHT_SSBO_BINDING) buffer DirLightBuffer
{
    DirLight directional;
};

readonly layout(std430, binding = POINTLIGHT_SSBO_BINDING) buffer PointLights
{
    uint       num_point;
    PointLight points[];
};

readonly layout(std430, binding = SPOTLIGHT_SSBO_BINDING) buffer SpotLights
{
    uint      num_spot;
    SpotLight spots[];
};

float GetCone(const vec3 light_dir, const vec3 cone_dir, float inner, float outer)
{
    float cos_a    = dot(light_dir, cone_dir);
    float len      = max(0.0001, inner-outer);

    return min(1.0, max(0.0, (cos_a-outer)/len));
}

vec3 Directional(const vec3 normal, const vec3 view_dir, const DirLight light, const vec3 diffuseColor, const vec3 specularColor, float roughness)
{
    return GGXShading(normal, view_dir, -light.dir.xyz, light.color.rgb*light.color.a, diffuseColor, specularColor, roughness, 1.0);
}

vec3 Point(const vec3 pos, const vec3 normal, const vec3 view_dir, const PointLight light, 
           const vec3 diffuseColor, const vec3 specularColor, float roughness)
{
    vec3 light_dir    = light.position.xyz-pos;
    float distance    = length(light_dir);
    light_dir         = light_dir/distance;
    float radius      = light.position.w;
    float intensity   = light.color.a;

    // epic falloff
    float att         = Sq(max(1.0-Sq(Sq(distance/radius)), 0.0))/(Sq(distance)+1);

    return GGXShading(normal, view_dir, light_dir, light.color.rgb*intensity, diffuseColor, specularColor, roughness, att);
}

vec3 Spot(const vec3 pos, const vec3 normal, const vec3 view_dir, const SpotLight light, 
                   const vec3 diffuseColor, const vec3 specularColor, float roughness)
{
    vec3 light_dir    = light.position.xyz-pos;
    float distance    = length(light_dir);
    float projDist    = dot(light.direction.xyz, -light_dir);
    light_dir         = light_dir/distance;
    float lightDist   = light.dist;
    float inner       = light.inner;
    float outer       = light.outer;
    float intensity   = light.intensity;

    float cone        = GetCone(-light_dir, light.direction.xyz, inner, outer);

    // epic falloff
    float att         = Sq(max(1.0-Sq(Sq(projDist/lightDist)), 0.0))/(Sq(projDist)+1);

    return GGXShading(normal, view_dir, light_dir, light.color.rgb*intensity, diffuseColor, specularColor, roughness, att*cone);
}

//vec4 Shading(const in vec3 pos, const in vec3 normal, vec4 diffuse, vec3 specular, float smoothness, vec3 occlusion, vec3 emissive)
vec4 Shading(in PBR pbr)
{
    vec3 V           = normalize(view_pos.xyz-pbr.position);
    vec3 R           = reflect(-V, pbr.normal);
    float NdotV      = dot(pbr.normal, V);
    float roughness  = Sq(1.0-pbr.smoothness); 

    // Add Indirect lighting 
    vec3 indirect = vec3(0.0);

    if(NdotV >= 0.0)
    {
        vec3 irradiance = texture(diffuseIBL, pbr.normal).rgb;
        vec3 radiance   = textureLod(prefilteredIBL, R, roughness*(prefilteredLevels-1)).rgb;
        vec2 fab        = texture(environmentBRDF, vec2(NdotV, roughness)).rg;
        indirect        = (pbr.diffuse*(1-pbr.specular))*irradiance+radiance*(pbr.specular*fab.x+fab.y);
    }
    
    vec3 color = Directional(pbr.normal, V, directional, pbr.diffuse, pbr.specular, roughness);

    for(uint i=0; i< num_point; ++i)
    {
        color += Point(pbr.position, pbr.normal, V, points[i], pbr.diffuse, pbr.specular, roughness);
    }

    for(uint i=0; i< num_spot; ++i)
    {
        color += Spot(pbr.position, pbr.normal, V, spots[i], pbr.diffuse, pbr.specular, roughness);
    }

#if 0
    // Compute ambient occlusion
    vec4 projectedPos = proj*view*vec4(pbr.position, 1.0);
    vec2 occlusionUV  = (projectedPos.xy/projectedPos.w)*0.5+0.5;

    vec3 occlusionFactor = vec3(texture(ambientOcclusion, occlusionUV).r);
#else
    vec3 occlusionFactor = vec3(1.0);

#endif

    color += indirect*occlusionFactor;
    color += pbr.emissive;

    return vec4(color, 1.0); 
}


#endif /* _LIGHTING_GLSL_ */