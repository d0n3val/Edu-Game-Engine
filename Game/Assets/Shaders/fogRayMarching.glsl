#version 460
#extension GL_ARB_shading_language_include : require
#extension GL_ARB_bindless_texture : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/lighting.glsl"
#include "/shaders/simplexNoise.glsl"

#define NUM_STEPS 36

layout(binding = GBUFFER_DEPTH_TEX_BINDING) uniform sampler2D depth;
layout(binding = RAYMARCHING_BLUENOISE_TEX_BINDING) uniform sampler2D blueNoise;

uniform layout(binding=RAYMARCHING_FOG_DENSITY_BINDING, rgba32f) writeonly image2D outputImg; 
uniform layout(location=RAYMARCHING_WIDHT_LOCATION) int width;
uniform layout(location=RAYMARCHING_HEIGHT_LOCATION) int height;
uniform layout(location=RAYMACHING_BLUENOISE_UV_TILING_LOCATION) vec2 blueNoiseTiling;

layout(std140, binding = RAYMARCHING_PARAMETERS_LOCATION) uniform Parameters 
{
    vec4  ambientColour;
    float extinctionCoeff;
    float fogIntensity;
    float frame;
    float noiseScale;
    float noiseSpeed;
    float maxDistance;
    int pad0, pad1;
};

float sampleNoise(vec2 uv) // Interleaved gradient
{
    vec2 pixel = uv*vec2(width, height);
    pixel += (float(frame) * 5.588238f);
    return fract(52.9829189f * fract(0.06711056f*float(pixel.x) + 0.00583715f*float(pixel.y)));  

}

float calculateAnisotropy(float k, float cosTheta)
{
    k = max(min(k, 0.999), -0.999);

    return (1.0-Sq(k))/(4.0*PI*Sq(1-k*cosTheta));
}

vec3 calculatePointLight(in vec3 pos, in vec3 V)
{
    vec3 color = vec3(0.0);
    for(uint i=0; i< num_point; ++i)
    {
        PointLight light = points[i];

        float dist   = distance(light.position.xyz, pos);
        float radius = light.position.w;
        float intensity = light.color.a;
        float att    = Sq(max(1.0-Sq(Sq(dist/radius)), 0.0))/(Sq(dist)+1);

        // TODO: phase functions

        color += light.color.rgb*intensity*att;
    }

    return color;
}

vec3 calculateSpotLight(in vec3 pos, in vec3 V)
{
    vec3 color = vec3(0.0);
    for(uint i=0; i< num_spot; ++i)
    {
        SpotLight light = spots[i];

        vec3 light_dir  = light.position.xyz-pos;
        float projDist  = dot(light.direction.xyz, -light_dir);
        light_dir = normalize(light_dir);
        float lightDist = light.dist;
        float intensity = light.intensity;

        float anisotropy = light.position.w;
        float phase = calculateAnisotropy(anisotropy, dot(light_dir, V));

        float inner   = light.inner;
        float outer   = light.outer;
        float cone    = GetCone(-light_dir, normalize(light.direction.xyz), inner, outer);
        float att     = Sq(max(1.0-Sq(Sq(projDist/lightDist)), 0.0))/(Sq(projDist)+1);


        color += light.color.rgb*intensity*cone*att*phase;
    }

    return color;
}

vec3 calculateFogLighting(in vec3 pos, in vec3 V)
{
    vec3 color = ambientColour.rgb*calculateAnisotropy(0.0, 0.0);
    color += calculatePointLight(pos, V);
    color += calculateSpotLight(pos,V);

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

layout(local_size_x = RAYMARCHING_GROUP_SIZE, local_size_y = RAYMARCHING_GROUP_SIZE, local_size_z = 1) in;
void main()
{
    ivec2 index = ivec2(gl_GlobalInvocationID.xy);

    if(index.x < width && index.y < height)
    {
        vec2 uv = vec2(index.x/float(width), index.y/float(height));

        float d = texture(depth, uv).r;
        vec3 position = getWorldPos(d, uv);

        vec3 ray = position-view_pos.xyz;
        float len = length(ray);

        ray = ray*(min(len, max(maxDistance, 0.001))/len); // clamp ray to maxDistance

        vec3 marchingStep = ray/float(NUM_STEPS);
        float stepSize = length(marchingStep);

        vec3 V = normalize(ray);

        vec3 result = vec3(0.0);
        vec3 currentPos = view_pos.xyz+marchingStep*sampleNoise(uv);
        float transmittance = 1.0;

        for(int i=0; i < NUM_STEPS; ++i)
        {
            float density = calculateFogDensity(currentPos);
            transmittance *= calculateTransmittance(density, stepSize);
            vec3 inScattering = calculateFogLighting(currentPos, V);

            result += transmittance*inScattering*stepSize;

            currentPos += marchingStep;
        }

        imageStore(outputImg, index, vec4(result*fogIntensity, 1.0));
    }
}
