#version 460
#extension GL_ARB_shading_language_include : require
#extension GL_ARB_bindless_texture : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/lighting.glsl"
#include "/shaders/simplexNoise.glsl"

#define SCREEN_SCALE 2

#define FLT_MAX 3.402823466e+38

layout(binding = GBUFFER_DEPTH_TEX_BINDING) uniform sampler2D depth;
layout(binding = RAYMARCHING_NOISE_TEXTURE_BINDING) uniform sampler2D noise;
uniform layout(binding=RAYMARCHING_FOG_DENSITY_BINDING, rgba32f) writeonly image2D outputImg; 
uniform layout(location=RAYMARCHING_WIDHT_LOCATION) int width;
uniform layout(location=RAYMARCHING_HEIGHT_LOCATION) int height;

layout(binding=VOLSPOT_LIGHT_LIST_BINDING) uniform isamplerBuffer volSpotLightList;

layout(std140, binding = RAYMARCHING_PARAMETERS_LOCATION) uniform Parameters 
{
    vec4  ambientColour;
    float extinctionCoeff;
    float fogIntensity;
    float frame;
    float noiseScale;
    float noiseSpeed;
    float stepSize; 
    float attCorrection;
    int pad0_, pad1_;
};

float sampleNoise(vec2 uv) // Interleaved gradient
{
    vec2 pixel = uv*vec2(width, height);
    pixel += (float(frame) * 5.588238f );
    return fract(52.9829189f * fract(0.06711056f*float(pixel.x) + 0.00583715f*float(pixel.y)));  
}

float calculateAnisotropy(float k, float cosTheta)
{
    k = max(min(k, 0.999), -0.999);

    return (1.0-Sq(k))/(4.0*PI*Sq(1-k*cosTheta));
}

vec3 calculateSpotLight(in vec3 pos, in vec3 V, int index)
{
    SpotLight light = spots[index];

    vec3 light_dir  = light.transform[3].xyz-pos;
    float projDist  = dot(-light.transform[1].xyz, -light_dir);
    light_dir       = normalize(light_dir);
    float lightDist = light.dist;

    float anisotropy = light.color.w;
    float phase      = calculateAnisotropy(anisotropy, dot(light_dir, V));

    float inner   = light.inner;
    float outer   = light.outer;
    float cone    = GetCone(-light_dir, -normalize(light.transform[1].xyz), inner, outer);
    float att     = Sq(max(1.0-Sq(Sq(projDist/lightDist)), 0.0))/(attCorrection*Sq(projDist)+1);
    float shadow  = 1.0;

    if(light.hasShadow != 0)
    {
        shadow = computeSpotShadow(light.shadowDepth, light.shadowViewProj, pos);
    }

    vec3 color = light.color.rgb*cone*att*shadow*phase;

    color += cone*ambientColour.rgb*calculateAnisotropy(0.0, 0.0);

    return color;
}

vec3 calculateFogLighting(in vec3 pos, in vec3 V, int tileIndex)
{
    uint bufferOffset = tileIndex*num_spot;
    vec3 color  = vec3(0.0);

    for(int i=0; i<num_spot; ++i)
    {
        int lightIndex = texelFetch(volSpotLightList, int(bufferOffset+i)).r;
        if(lightIndex >=0)
        {
            color += calculateSpotLight(pos,V, lightIndex);
        }
        else
        {
            break;
        }
    }

    return color;
}

float calculateFogDensity(vec3 pos)
{
    pos = pos/20;
    vec2 newPos = pos.xz - (vec2(250.0)*floor(pos.xz/vec2(250.0))); // mod(x,y)
    return max(texture(noise, newPos+frame*noiseSpeed).r*noiseScale, 0.0);

    //return max(snoise(vec4(pos, ))*noiseScale, 0.0);
}

float calculateTransmittance(in float density)
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

    float rayConeDot = dot(rayDir, coneDir);
    float rayConeDotSq = Sq(rayConeDot);
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

    return dot(rayOrig+rayDir*t2-coneOrig, coneDir) > 0.0;
}

bool intersectCones(int tileIndex, in vec3 rayPos, in vec3 rayDir, out float minT0, out float maxT1)
{
    bool intersects = false;

    uint bufferOffset = tileIndex*num_spot;

    minT0 = FLT_MAX; 
    maxT1 = 0.0;

    for(int i=0; i<num_spot; ++i)
    {
        int lightIndex = texelFetch(volSpotLightList, int(bufferOffset+i)).r;
        if(lightIndex >=0)
        {
            SpotLight spot = spots[lightIndex];

            float t0, t1;
            if(intersectCone(rayPos, rayDir, spot.transform[3].xyz, -spot.transform[1].xyz, spot.dist, spot.radius, spot.outer, t0, t1))
            {
                intersects = true;
                minT0 = min(t0, minT0);
                maxT1 = max(t1, maxT1);
            }
        }
        else
        {
            break;
        }
    }

    return intersects;

}

int getTileIndex(ivec2 pixel)
{
    ivec2 tile = ivec2(vec2(pixel*SCREEN_SCALE)/vec2(TILE_CULLING_GROUP_SIZE, TILE_CULLING_GROUP_SIZE));
    return TILE_INDEX(tile.x, tile.y, width*SCREEN_SCALE);
}

layout(local_size_x = RAYMARCHING_GROUP_SIZE, local_size_y = RAYMARCHING_GROUP_SIZE, local_size_z = 1) in;
void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

    if(pixel.x < width && pixel.y < height)
    {
        int tileIndex = getTileIndex(pixel);

        vec2 uv = vec2(pixel.x/float(width), pixel.y/float(height));

        vec3 fragmentPos = getWorldPos(texture(depth, uv).r, uv);

        vec3 rayDir     = fragmentPos-view_pos.xyz;
        float fragDist  = length(rayDir);

        // TMP: render always cone 0
        int draw_id = 0;

        // normalize
        rayDir = rayDir/fragDist;
        SpotLight spot  = spots[draw_id];

        float t0, t1;
        //bool intersects = intersectCone(view_pos.xyz, rayDir, spot.transform[3].xyz, -spot.transform[1].xyz, spot.dist, spot.radius, spot.outer, t0, t1);
        bool intersects = intersectCones(tileIndex, view_pos.xyz, rayDir, t0, t1);

        if(intersects)
        {
            t0 = max(t0, 0.0);
            t1 = min(t1, fragDist);

            float nSteps = ceil((t1-t0)/max(stepSize, 0.01));
            vec3 marchingStep = rayDir*stepSize;

            vec3 result = vec3(0.0);
            vec3 currentPos = view_pos.xyz+rayDir*t0;
            vec3 V = rayDir; 

            // dithering
            currentPos += marchingStep*sampleNoise(uv);

            float transmittance = 1.0;

            for(int i=0; i < nSteps; ++i)
            {
                float density = calculateFogDensity(currentPos);
                transmittance *= calculateTransmittance(density);

                vec3 inScattering = calculateFogLighting(currentPos, V, tileIndex);

                result += transmittance*inScattering*stepSize;

                currentPos += marchingStep;
            }

            imageStore(outputImg, pixel, vec4(result*fogIntensity, tileIndex));
        }
    }
}
