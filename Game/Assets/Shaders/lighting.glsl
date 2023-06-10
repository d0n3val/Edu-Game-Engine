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
layout(binding = PLANAR_REFLECTION_BINDING) uniform sampler2D planarReflections;
layout(location = PLANAR_REFLECTION_VIEWPROJ_LOCATION) uniform mat4 planarViewProj;
layout(location = PLANAR_REFLECTION_LOD_LEVELS_LOCATION) uniform int planarLevels;
layout(location = PLANAR_REFLECTION_NORMAL) uniform vec3 planarNormal;
layout(location = PLANAR_REFLECTION_DISTORITION) uniform float planarDistortion;

struct DirLight
{
    vec4 dir;
    vec4 color;
};

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
    mat4 toLocal;
    vec4 position;
    vec4 minParallax;
    vec4 maxParallax;
    vec4 minInfluence;
    vec4 maxInfluence;
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

readonly layout(std430, row_major, binding = IBLLIGHT_SSBO_BINDING) buffer IBLLights
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

vec3 Sphere(const vec3 pos, const vec3 normal, vec3 view_dir, const vec3 light_pos, float sphere_radius, 
            float attenuation_radius, const vec3 light_color, const vec3 diffuseColor, const vec3 specularColor, float roughness)
{
    vec3 reflect_dir  = reflect(-view_dir, normal);
    vec3 light_dir    = light_pos-pos;
    vec3 centerToRay  = pos+reflect_dir*max(0.0, dot(light_dir, reflect_dir))-light_pos;
    vec3 closestPoint = light_pos+centerToRay*min(sphere_radius/length(centerToRay), 1.0);

    float dist        = distance(pos, light_pos);
    light_dir         = normalize(closestPoint-pos);

    // epic falloff
    float att     = Sq(max(1.0-Sq(Sq(dist/attenuation_radius)), 0.0))/(Sq(dist)+1);

    vec3 specular = GGXShadingSpec(normal, view_dir, light_dir, light_color, specularColor, roughness, att);

    light_dir    = normalize(light_pos-pos);
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

vec3 ClosestToLine(vec3 pos, vec3 a, vec3 b, float radius)
{
    vec3 ab  = (b-a);
    float len = length(ab);
    ab = ab/len;
    vec3 pointInLine = a+ab*clamp(dot(pos-a, ab), 0.0, len);

	return pointInLine+normalize(pos-pointInLine)*min(radius, distance(pointInLine, pos));
}

vec3 ClosestToSphere(in vec3 pos, in vec3 dir, in vec3 lightPos, in float radius)
{
    vec3 light_dir    = lightPos-pos;
    vec3 centerToRay  = pos+dir*dot(light_dir, dir)-lightPos;
    return lightPos+(centerToRay)*min(radius/length(centerToRay), 1.0);
}

vec3 BisectionIntersection(vec3 P, vec3 A, vec3 B)
{
    float a = length(A-P);
    float b = length(B-P);
    vec3 AB = B-A;
    float c = length(AB);

    float x = a/(b+a);

    return A + AB*x;
}

vec3 Tube(const vec3 pos, const vec3 normal, const vec3 view_dir, const TubeLight light, 
          const vec3 diffuseColor, const vec3 specularColor, float roughness)
{
    vec3 reflect_dir = normalize(reflect(-view_dir, normal));
    vec3 closest     = ClosestToLine(pos, reflect_dir, light.p0.xyz, light.p1.xyz);
    vec3 closestToSphere = ClosestToSphere(pos, reflect_dir, closest, light.p0.w);
    vec3 closestAtt  = ClosestToLine(pos, light.p0.xyz, light.p1.xyz, light.p0.w);

    float dist        = distance(closestAtt, pos);
    vec3 light_dir    = normalize(closestToSphere-pos);

    // epic falloff
    float att         = Sq(max(1.0-Sq(Sq(dist/light.color.a)), 0.0))/(Sq(dist)+1);

    vec3 specular     = GGXShadingSpec(normal, view_dir, light_dir, light.color.rgb, specularColor, roughness, att);

    vec3 light_to_pos = BisectionIntersection(pos, light.p0.xyz, light.p1.xyz)-pos; 
    light_dir         = normalize(light_to_pos);

    vec3 diffuse      = Lambert(normal, light_dir, light.color.rgb, diffuseColor, specularColor, att);

    return diffuse+specular;
}

vec3 ShadingDirectional(in PBR pbr)
{
    vec3 V           = normalize(view_pos.xyz-pbr.position);
    float roughness  = max(Sq(1.0-pbr.smoothness), MIN_ROUGHNESS); 

    vec3 color = Directional(pbr.normal, V, directional, pbr.diffuse, pbr.specular, roughness)*pbr.occlusion*pbr.shadow;

    return color;
}

vec3 parallaxCorrection(const vec3 localPos, const vec3 localR, const vec3 minBox, const vec3 maxBox)
{
    vec3 first = (maxBox-localPos)/localR;
    vec3 second = (minBox-localPos)/localR;

    vec3 furthest = max(first, second);

    float dist = min(min(furthest.x, furthest.y), furthest.z);

    return localPos+localR*dist;
}

vec3 evaluateIBL(in PBR pbr, in samplerCube difIBL, in samplerCube prefIBL, in float roughness, in float NdotV, in vec3 R, in vec3 coord, in vec4 planarColor)
{
    vec3 irradiance = texture(difIBL, pbr.normal).rgb;
    vec3 radiance   = mix(textureLod(prefIBL, coord, roughness*(prefilteredLevels-1)).rgb, planarColor.rgb, planarColor.a);
    vec2 fab        = texture(environmentBRDF, vec2(NdotV, roughness)).rg;
    vec3 indirect   = (pbr.diffuse*(1-pbr.specular))*irradiance+radiance*(pbr.specular*fab.x+fab.y);

    float shadow = min(1.0, pbr.shadow+0.25);

    return indirect*pbr.occlusion*shadow;
}

bool insideBox(const vec3 localPos, const vec3 minBox, const vec3 maxBox)
{
    return  localPos.x >= minBox.x && localPos.x <= maxBox.x && 
            localPos.y >= minBox.y && localPos.y <= maxBox.y && 
            localPos.z >= minBox.z && localPos.z <= maxBox.z;
}

vec3 ShadingAmbientIBL(in PBR pbr, in vec4 planarColor)
{
    vec3 V           = normalize(view_pos.xyz-pbr.position);
    vec3 R           = reflect(-V, pbr.normal);
    float NdotV      = max(dot(pbr.normal, V), 0.0);
    float roughness  = max(Sq(1.0-pbr.smoothness), MIN_ROUGHNESS); 

    vec3 color = vec3(0.0);

    float totalWeight = 0.0;
    for(uint i=0; i< num_ibl; ++i)
    {
        IBLLight ibl = ibls[i];

        vec3 localPos = (ibl.toLocal*vec4(pbr.position, 1.0)).xyz;

        if(insideBox(localPos, ibl.minInfluence.xyz, ibl.maxInfluence.xyz))
        {
            vec3 closer = min(localPos-ibl.minInfluence.xyz, ibl.maxInfluence.xyz-localPos);
            float weight = min(closer.x, min(closer.y, closer.z));
            weight = weight * weight;

            vec3 localR = mat3(ibl.toLocal)*R;
            vec3 coord = parallaxCorrection(localPos, localR, ibl.minParallax.xyz, ibl.maxParallax.xyz);
            color += evaluateIBL(pbr, ibl.diffuse, ibl.prefiltered, roughness, NdotV, R, coord, planarColor)*weight;
            totalWeight += weight;
        }
    }

    if(totalWeight == 0.0)
    {
        color = evaluateIBL(pbr, diffuseIBL, prefilteredIBL, roughness, NdotV, R, R, planarColor);
    }
    else
    {
        color /= totalWeight;
    }

    return color;
}

mat3 computeTangetSpace(in vec3 normal)
{
    vec3 up    = abs(normal.y) > 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up         = cross(normal, right);

    return mat3(right, up, normal);
}

vec3 ShadingAmbient(in PBR pbr)
{
    vec4 planarColor = vec4(0.0);
    if(pbr.planarReflections != 0)
    {
        vec4 clipPos = planarViewProj*vec4(pbr.position, 1.0);
        vec2 planarUV = (clipPos.xy/clipPos.w)*0.5+0.5;
        float roughness  = max(Sq(1.0-pbr.smoothness), MIN_ROUGHNESS); 

        if(planarUV.x >= 0.0 && planarUV.x <= 1.0 && 
           planarUV.y >= 0.0 && planarUV.y <= 1.0 )
        {
            mat3 tangent = computeTangetSpace(planarNormal);
            vec3 local = transpose(tangent)*pbr.normal;
            planarColor.rgb = textureLod(planarReflections, planarUV+local.xy*planarDistortion, roughness*(planarLevels-1)).rgb;
            planarColor.a = 1.0;
        }
    }

    return ShadingAmbientIBL(pbr, planarColor);
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
}

vec4 ShadingNoPoint(in PBR pbr)
{
    vec3 color = ShadingAmbient(pbr);
    color += ShadingDirectional(pbr);
    color += ShadingSpot(pbr);
    color += ShadingSphere(pbr);
    color += ShadingQuad(pbr);
    color += ShadingTube(pbr);

    return vec4(color, pbr.alpha);
}


#endif /* _LIGHTING_GLSL_ */
