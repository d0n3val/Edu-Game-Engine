#ifndef _PBR_DEFS_GLSL_
#define _PBR_DEFS_GLSL_

#include "/shaders/common.glsl"

struct PBR
{
    vec3  diffuse;
    vec3  specular;
    vec3  emissive;
    vec3  normal;
    vec3  position;
    float smoothness;
    float occlusion;
    float alpha;
    float shadow;
    int   planarReflections;
};

vec3 GetFresnel(vec3 dir0, vec3 dir1, const vec3 f0)
{
    float cos_theta = max(dot(dir0, dir1), 0.0); 

    return f0+(1-f0)*pow(1.0-cos_theta, 5.0);
}

// from filament
float GGXNDF(float roughness, float NdotH)
{
    float a = NdotH*roughness;
    float k = roughness/max(1.0-NdotH*NdotH+a*a, 0.001);
    return k*k*(1.0/PI);
}

float SMITHVSF(float NdotL, float NdotV, float roughness)
{
    // optimized version
    return 0.5/mix(2.0*NdotL*NdotV, NdotL+NdotV, roughness);
}

vec3 GGXShadingSpec(const vec3 normal, const vec3 view_dir, const vec3 light_dir, 
                    const vec3 light_color, const vec3 specularColor, float roughness, 
                    float att)
{
    float dotNL      = max(dot(normal, light_dir), 0.001);
    float dotNV      = max(dot(normal, view_dir), 0.001);

    vec3 half_dir    = normalize(view_dir+light_dir);
    vec3 fresnel     = GetFresnel(light_dir, half_dir, specularColor);
    float ndf        = GGXNDF(roughness, max(0.001, dot(half_dir, normal)));
    float vsf        = SMITHVSF(dotNL, dotNV, roughness);

    return 0.25*fresnel*ndf*vsf*light_color*dotNL*att;
}

vec3 Lambert(const vec3 normal, const vec3 light_dir, const vec3 light_color, const vec3 diffuseColor, const vec3 specularColor, float att)
{
    float dotNL = max(dot(normal, light_dir), 0.001);

    return diffuseColor*(1-specularColor)*light_color*dotNL*att;
}

vec3 GGXShading(const vec3 normal, const vec3 view_dir, const vec3 light_dir, const vec3 light_color, 
                const vec3 diffuseColor, const vec3 specularColor, float roughness, float att)
{
    float dotNL      = max(dot(normal, light_dir), 0.001);
    float dotNV      = max(dot(normal, view_dir), 0.001);

    vec3 half_dir    = normalize(view_dir+light_dir);
    vec3 fresnel     = GetFresnel(light_dir, half_dir, specularColor);
    float ndf        = GGXNDF(roughness, max(0.001, dot(half_dir, normal)));
    float vsf        = SMITHVSF(dotNL, dotNV, roughness);

    return (diffuseColor*(1-specularColor)+(0.25*fresnel*ndf*vsf))*light_color*dotNL*att;
}

#endif /* _PBR_DEFS_GLSL_ */