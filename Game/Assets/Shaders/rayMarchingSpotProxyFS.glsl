#version 460
#extension GL_ARB_shading_language_include : require
#extension GL_ARB_bindless_texture : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/lighting.glsl"
#include "/shaders/simplexNoise.glsl"

in vec2 uv;
in flat int draw_id;
in vec3 worldPos;
out vec4 color;

#define NUM_STEPS 16 // TODO: Configurable per light

layout(binding = GBUFFER_POSITION_TEX_BINDING) uniform sampler2D position;
uniform layout(location=RAYMARCHING_WIDHT_LOCATION) int width;
uniform layout(location=RAYMARCHING_HEIGHT_LOCATION) int height;

layout(std140, binding = RAYMARCHING_PARAMETERS_LOCATION) uniform Parameters 
{
    vec4  ambientColour;
    float extinctionCoeff;
    float fogIntensity;
    float frame;
    float noiseScale;
    float noiseSpeed;
    float maxDistance;
    int pad0_, pad1_;
};

float sampleNoise(vec2 uv) // Interleaved gradient
{
    vec2 pixel = uv*vec2(width, height);
    pixel += (float(frame) * 5.588238f * 0.01);
    return fract(52.9829189f * fract(0.06711056f*float(pixel.x) + 0.00583715f*float(pixel.y)));  
}

float calculateAnisotropy(float k, float cosTheta)
{
    k = max(min(k, 0.999), -0.999);

    return (1.0-Sq(k))/(4.0*PI*Sq(1-k*cosTheta));
}

vec3 calculateSpotLight(in vec3 pos, in vec3 V)
{
    SpotLight light = spots[draw_id];

    vec3 light_dir  = light.transform[3].xyz-pos;
    float projDist  = dot(-light.transform[1].xyz, -light_dir);
    light_dir       = normalize(light_dir);
    float lightDist = light.dist;

    float anisotropy = light.color.w;
    float phase      = calculateAnisotropy(anisotropy, dot(light_dir, V));

    float inner   = light.inner;
    float outer   = light.outer;
    float cone    = GetCone(-light_dir, -normalize(light.transform[1].xyz), inner, outer);
    float att     = Sq(max(1.0-Sq(Sq(projDist/lightDist)), 0.0))/(Sq(projDist)+1);
    float shadow  = 1.0;

    if(light.hasShadow != 0)
    {
        shadow = computeSpotShadow(light.shadowMap, light.shadowViewProj, pos);
    }

    vec3 color = light.color.rgb*cone*att*shadow*phase;

    color += cone*ambientColour.rgb*calculateAnisotropy(0.0, 0.0);

    return color;
}

vec3 calculateFogLighting(in vec3 pos, in vec3 V)
{
    vec3 color = calculateSpotLight(pos,V);

    return color;
}

float calculateFogDensity(in vec3 pos)
{
    return max(snoise(vec4(pos, frame*noiseSpeed))*noiseScale, 0.0);
}

float calculateTransmittance(in float density, in float stepSize)
{
    return max(exp(-extinctionCoeff*stepSize*density), 0.0);
}

// see https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection.html
bool intersectDisc(in vec3 rayOrig, in vec3 rayDir, in vec3 discOrig, in vec3 discNormal, in float radius, out float t)
{
    float denom = dot(discNormal, rayDir);
    if(denom > 1e-6)
    {
        t = dot(discOrig-rayOrig, discNormal)/denom;

        vec3 pos = rayOrig+rayDir*t;
        vec3 local = pos-discOrig;
        return dot(local , local) < Sq(radius);
    }
    
    return false;
}

// see https://www.shadertoy.com/view/MtcXWr and  https://lousodrome.net/blog/light/2017/01/03/intersection-of-a-ray-and-a-cone/
bool intersectCone(in vec3 rayOrig, in vec3 rayDir, in vec3 coneOrig, in vec3 coneDir, in float coneDist, in float coneRadius, 
                   in float coneCosAngle, out float t1, out float t2)
{
    vec3 co = rayOrig - coneOrig;

    float rayConeDotSq = Sq(dot(rayDir, coneDir));
    float coneCosSq = Sq(coneCosAngle);

    float a = rayConeDotSq - coneCosSq;
    float b = 2. * (dot(rayDir, coneDir)*dot(co, coneDir) - dot(rayDir, co)*coneCosSq);
    float c = Sq(dot(co, coneDir)) - dot(co,co)*coneCosSq;

    float det = b*b - 4.0*a*c;
    if (det < 0.) return false;

    det = sqrt(det);
    t1 = (-b - det) / (2. * a);
    t2 = (-b + det) / (2. * a);

    if(t2 < t1)
    {
        float t = t1;
        t1 = t2;
        t2 = t;
    }

    float t;
    vec3 discPoint = coneOrig+coneDir*coneDist;
    vec3 discNormal = coneDir;
    float discRadius = coneRadius;
    if(intersectDisc(rayOrig, rayDir, discPoint, discNormal, discRadius, t))
    {
        t2 = t; 
    }
    else if(intersectDisc(rayOrig, rayDir, discPoint, -discNormal, discRadius, t))
    {
        t1 = t;
    }

    return true;
}

void main()
{
    vec3 fragmentPos = texture(position, uv).xyz;

    vec3 rayDir     = fragmentPos-view_pos.xyz;
    float fragDist  = length(rayDir);

    // normalize
    rayDir = rayDir/fragDist;

    SpotLight spot  = spots[draw_id];

    float t0, t1;
    bool intersects = intersectCone(view_pos.xyz, rayDir, spot.transform[3].xyz, -spot.transform[1].xyz, spot.dist, spot.radius, spot.outer, t0, t1);

    if(!intersects) discard;

    t0 = max(t0, 0.0);
    t1 = min(t1, fragDist);

    vec3 marchingStep = rayDir*((t1-t0)/float(NUM_STEPS)); 
    float stepSize = length(marchingStep);

    vec3 result = vec3(0.0);
    vec3 currentPos = view_pos.xyz+rayDir*t0;
    vec3 V = rayDir; 

    // dithering
    //currentPos += marchingStep*sampleNoise(uv);

    float transmittance = 1.0;

    for(int i=0; i < NUM_STEPS; ++i)
    {
        float density = calculateFogDensity(currentPos);
        transmittance *= calculateTransmittance(density, stepSize);

        vec3 inScattering = calculateFogLighting(currentPos, V);

        result += transmittance*inScattering*stepSize;

        currentPos += marchingStep;
    }

    color = vec4(result*fogIntensity, 1.0);
}
