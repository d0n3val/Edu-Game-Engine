#ifndef _LIGHTING_GLSL_
#define _LIGHTING_GLSL_

#include "/shaders/common.glsl"
#include "/shaders/pbrDefs.glsl"
#include "/shaders/cameraDefs.glsl"

#define MIN_ROUGHNESS 0.001

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

struct SphereLight
{
    vec4 position; // position+radius
    vec4 color;    // rgb+light radius
};

struct QuadLight
{
    vec4 position;
    vec4 up;
    vec4 right;
    vec4 color;
    vec4 size;
};

struct TubeLight
{
    vec4 p0; // pos + radius
    vec4 p1;
    vec4 color;
};

struct IBLLight
{
    samplerCube diffuse;
    samplerCube prefiltered;
    vec4 pos;
    vec4 minPoint;
    vec4 maxPoint;
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

readonly layout(std430, binding = SPHERELIGHT_SSBO_BINDING) buffer SphereLights
{
    uint        num_sphere;
    SphereLight spheres[];
};

readonly layout(std430, binding = QUADLIGHT_SSBO_BINDING) buffer QuadLights
{
    uint      num_quad;
    QuadLight quads[];
};

readonly layout(std430, binding = TUBELIGHT_SSBO_BINDING) buffer TubeLights
{
    uint num_tube;
    TubeLight tubes[];
};

readonly layout(std430, binding = IBLLIGHT_SSBO_BINDING) buffer IBLLights
{
    uint num_ibl;
    IBLLight ibls[];
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
           const vec3 diffuseColor, const vec3 specularColor, float roughness) {
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

vec3 SphereSpec(const vec3 pos, const vec3 normal, const vec3 view_dir, const vec3 light_pos, float sphere_radius, 
                float attenuation_radius, const vec3 light_color, const vec3 specularColor, float roughness)
{
    vec3 reflect_dir  = normalize(reflect(-view_dir, normal));
    vec3 light_dir    = light_pos-pos;
    vec3 centerToRay  = pos+reflect_dir*dot(light_dir, reflect_dir)-light_pos;
    vec3 closestPoint = light_pos+(centerToRay)*min(sphere_radius/length(centerToRay), 1.0);

    float distance    = length(closestPoint-pos);
    light_dir         = (closestPoint-pos)/distance;
    // epic falloff
    float att         = Sq(max(1.0-Sq(Sq(distance/attenuation_radius)), 0.0))/(Sq(distance)+1);

    return GGXShadingSpec(normal, view_dir, light_dir, light_color, specularColor, roughness, att);
}

vec3 Sphere(const vec3 pos, const vec3 normal, vec3 view_dir, const vec3 light_pos, float sphere_radius, 
            float attenuation_radius, const vec3 light_color, const vec3 diffuseColor, const vec3 specularColor, float roughness)
{
    vec3 reflect_dir  = reflect(-view_dir, normal);
    vec3 light_dir    = light_pos-pos;
    vec3 centerToRay  = pos+reflect_dir*max(0.0, dot(light_dir, reflect_dir))-light_pos;
    vec3 closestPoint = light_pos+centerToRay*min(sphere_radius/length(centerToRay), 1.0);

    float distance    = length(closestPoint-pos);
    light_dir         = (closestPoint-pos)/distance;

    // epic falloff
    float att     = Sq(max(1.0-Sq(Sq(distance/attenuation_radius)), 0.0))/(Sq(distance)+1);

    vec3 specular = GGXShadingSpec(normal, view_dir, light_dir, light_color, specularColor, roughness, att);

    light_dir    = normalize(light_pos-pos);
    distance     = length(light_pos-pos);
    att          = Sq(max(1.0-Sq(Sq(distance/attenuation_radius)), 0.0))/(Sq(distance)+1);
    vec3 diffuse = Lambert(normal, light_dir, light_color, diffuseColor, specularColor, att);

    return diffuse+specular;
}

vec3 Sphere(const vec3 pos, const vec3 normal, const vec3 view_dir, const SphereLight light, 
            const vec3 diffuseColor, const vec3 specularColor, float roughness)
{
    return Sphere(pos, normal, view_dir, light.position.xyz, light.position.w, light.color.a, light.color.rgb, diffuseColor, specularColor, roughness);
}

vec3 Quad(const vec3 pos, const vec3 normal, const vec3 view_dir, const QuadLight light, 
          const vec3 diffuseColor, const vec3 specularColor, float roughness)
{
    vec3 light_dir    = light.position.xyz-pos;
    float distance    = length(light_dir);
    light_dir         = light_dir/distance;
    //float radius      = light.position.w;
    //float intensity   = light.color.a;

    // epic falloff
    float att         = 1.0;

    return GGXShading(normal, view_dir, light_dir, light.color.rgb, diffuseColor, specularColor, roughness, att);
}

vec3 ClosestToLine(vec3 pos, vec3 dir, vec3 a, vec3 b)
{
    vec3 pa = a-pos;
    vec3 ab = b-a;
    
    float dotABDir = dot(ab, dir);
    float num = dot(dir, pa)* dotABDir-dot(ab, pa);
    float denom = dot(ab, ab)-dotABDir*dotABDir;
    float t = clamp(num/denom, 0.0f, 1.0f);

    return a+ab*t;
}

vec3 BisectionIntersection(vec3 P, vec3 A, vec3 B)
{
    float a = length(A-P);
    float b = length(B-P);
    vec3 AB = B-A;
    float c = length(AB);

    float x = (c*a)/(b+a);

    return A + (AB)*x;
}

vec3 Tube(const vec3 pos, const vec3 normal, const vec3 view_dir, const TubeLight light, 
          const vec3 diffuseColor, const vec3 specularColor, float roughness)
{

    vec3 reflect_dir = normalize(reflect(-view_dir, normal));
    vec3 closest     = ClosestToLine(pos, reflect_dir, light.p0.xyz, light.p1.xyz);
    vec3 specular    = SphereSpec(pos, normal, view_dir, closest, light.p0.w, light.color.a, light.color.rgb, specularColor, roughness);

    vec3 p0 = light.p0.xyz-pos;
    vec3 p1 = light.p1.xyz-pos;
    vec3 light_to_pos = BisectionIntersection(pos, light.p0.xyz, light.p1.xyz)-pos; 

    vec3 light_dir   = normalize(light_to_pos);
    float distance   = length(light_to_pos);
    float att        = Sq(max(1.0-Sq(Sq(distance/light.color.a)), 0.0))/(Sq(distance)+1);
    vec3 diffuse     = Lambert(normal, light_dir, light.color.rgb, diffuseColor, specularColor, att);

    return diffuse+specular;
}

vec3 ShadingDirectional(in PBR pbr)
{
    vec3 V           = normalize(view_pos.xyz-pbr.position);
    float roughness  = max(Sq(1.0-pbr.smoothness), MIN_ROUGHNESS); 

    vec3 color = Directional(pbr.normal, V, directional, pbr.diffuse, pbr.specular, roughness)*pbr.occlusion*pbr.shadow;

    return color;
}

void parallaxCorrection(const vec3 position, const vec3 probePos, inout vec3 R, const vec3 minBox, const vec3 maxBox)
{
    vec3 first = (maxBox-position)/R;
    vec3 second = (minBox-position)/R;

    vec3 furthest = max(first, second);

    float dist = min(min(furthest.x, furthest.y), furthest.z);

    vec3 intersect = position+R*dist;
    R = intersect-probePos;
}

void getNearestIBL(const vec3 position, out samplerCube diffuse, out samplerCube prefiltered, inout vec3 R)
{
    diffuse = diffuseIBL;
    prefiltered = prefilteredIBL;
    float minDist = 0;

    for(uint i=0; i< num_ibl; ++i)
    {
        IBLLight ibl = ibls[i];
        float dist = distance(ibl.pos.xyz, position);
        if(i == 0 || dist < minDist)
        {
            minDist = dist;
            diffuse = ibl.diffuse;
            prefiltered = ibl.prefiltered;
            parallaxCorrection(position, ibl.pos.xyz, R, ibl.minPoint.xyz, ibl.maxPoint.xyz);
        }
    }
}

vec3 ShadingAmbient(in PBR pbr)
{
    samplerCube difIBL, prefIBL;

    vec3 V           = normalize(view_pos.xyz-pbr.position);
    vec3 R           = reflect(-V, pbr.normal);
    float NdotV      = max(dot(pbr.normal, V), 0.0);
    float roughness  = max(Sq(1.0-pbr.smoothness), MIN_ROUGHNESS); 

    getNearestIBL(pbr.position, difIBL, prefIBL, R);

    vec3 irradiance = texture(difIBL, pbr.normal).rgb;
    vec3 radiance   = textureLod(prefIBL, R, roughness*(prefilteredLevels-1)).rgb;
    vec2 fab        = texture(environmentBRDF, vec2(NdotV, roughness)).rg;
    vec3 indirect   = (pbr.diffuse*(1-pbr.specular))*irradiance+radiance*(pbr.specular*fab.x+fab.y);

    float shadow = min(1.0, pbr.shadow+0.25);

    vec3 color = indirect*pbr.occlusion*shadow;

    //if(found) color = textureLod(prefIBL, R, 0).rgb;

    return color;
}

vec3 ShadingPoint(in PBR pbr, uint index)
{
    vec3 V           = normalize(view_pos.xyz-pbr.position);
    float roughness  = max(Sq(1.0-pbr.smoothness), MIN_ROUGHNESS); 

    float shadow = min(1.0, pbr.shadow+0.5);

    return Point(pbr.position, pbr.normal, V, points[index], pbr.diffuse, pbr.specular, roughness)*pbr.occlusion*shadow;
}

vec3 ShadingPoint(in PBR pbr)
{
    vec3 V           = normalize(view_pos.xyz-pbr.position);
    float roughness  = max(Sq(1.0-pbr.smoothness), MIN_ROUGHNESS); 

    vec3 color = vec3(0.0);

    float shadow = min(1.0, pbr.shadow+0.5);

    for(uint i=0; i< num_point; ++i)
    {
        color += Point(pbr.position, pbr.normal, V, points[i], pbr.diffuse, pbr.specular, roughness)*pbr.occlusion*shadow;
    }

    return color;
}

vec3 ShadingSpot(in PBR pbr)
{
    vec3 V           = normalize(view_pos.xyz-pbr.position);
    float roughness  = max(Sq(1.0-pbr.smoothness), MIN_ROUGHNESS); 

    vec3 color = vec3(0.0);

    float shadow = min(1.0, pbr.shadow+0.5);

    for(uint i=0; i< num_spot; ++i)
    {
        color += Spot(pbr.position, pbr.normal, V, spots[i], pbr.diffuse, pbr.specular, roughness)*pbr.occlusion*shadow;
    }

    return color;
}

vec3 ShadingSphere(in PBR pbr)
{
    vec3 V           = normalize(view_pos.xyz-pbr.position);
    float roughness  = max(Sq(1.0-pbr.smoothness), MIN_ROUGHNESS); 

    vec3 color = vec3(0.0);

    float shadow = min(1.0, pbr.shadow+0.5);

    for(uint i=0; i< num_sphere; ++i)
    {
        color += Sphere(pbr.position, pbr.normal, V, spheres[i], pbr.diffuse, pbr.specular, roughness)*pbr.occlusion*shadow;
    }

    return color;
}

vec3 ShadingQuad(in PBR pbr)
{
    vec3 V           = normalize(view_pos.xyz-pbr.position);
    float roughness  = max(Sq(1.0-pbr.smoothness), MIN_ROUGHNESS); 

    vec3 color = vec3(0.0);

    float shadow = min(1.0, pbr.shadow+0.5);

    for(uint i=0; i< num_quad; ++i)
    {
        color += Quad(pbr.position, pbr.normal, V, quads[i], pbr.diffuse, pbr.specular, roughness)*pbr.occlusion*shadow;
    }

    return color;
}

vec3 ShadingTube(in PBR pbr)
{
    vec3 V           = normalize(view_pos.xyz-pbr.position);
    float roughness  = max(Sq(1.0-pbr.smoothness), MIN_ROUGHNESS); 

    vec3 color = vec3(0.0);

    float shadow = min(1.0, pbr.shadow+0.5);

    for(uint i=0; i< num_tube; ++i)
    {
        color += Tube(pbr.position, pbr.normal, V, tubes[i], pbr.diffuse, pbr.specular, roughness)*pbr.occlusion*shadow;
    }

    return color;
}


vec4 Shading(in PBR pbr)
{
    vec3 color = ShadingAmbient(pbr);
    color += ShadingDirectional(pbr);
    color += ShadingPoint(pbr);
    color += ShadingSpot(pbr);
    color += ShadingSphere(pbr);
    color += ShadingQuad(pbr);
    color += ShadingTube(pbr);

    return vec4(color, pbr.alpha);
    //return vec4(pbr.specular, pbr.alpha);
}

vec4 ShadingNoPoint(in PBR pbr)
{
    vec3 color = ShadingAmbient(pbr);
    color += ShadingDirectional(pbr);
    //color += ShadingSpot(pbr);
    //color += ShadingSphere(pbr);
    //color += ShadingQuad(pbr);
    //color += ShadingTube(pbr);

    return vec4(color, pbr.alpha);
    //return vec4(pbr.normal, pbr.alpha);
    //return vec4(pbr.specular, pbr.alpha);
}


#endif /* _LIGHTING_GLSL_ */