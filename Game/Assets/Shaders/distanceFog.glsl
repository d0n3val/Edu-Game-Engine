#version 460 
#extension GL_ARB_shading_language_include : require
#extension GL_ARB_bindless_texture : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/simplexNoise.glsl"

layout(binding = GBUFFER_DEPTH_TEX_BINDING) uniform sampler2D depth;

layout(std140, binding = DISTANCE_FOG_DATA) uniform FogData
{
    vec4 colour;
    vec2 curve;
    vec2 distRange;
    float frame;
    uint pad0;
};

in vec2 uv;
out vec4 color;


float bezier(float t)
{
    float p0 = 0.0;
    float p1 = curve.x;
    float p2 = curve.y;
    float p3 = 1.0;

    float t2 = t*t;
    float t3 = t2*t;
    float t_1 = (1.0-t);
    float t2_1 = t_1*t_1;
    float t3_1 = t_1*t_1*t_1;

    return clamp(t3_1*p0+3.0*t*t2_1*p1+3.0*t2*t_1*p2+t3*p3, 0.0, 1.0);
}

float getDistanceFactor(float dist)
{
   return clamp((dist-distRange.x)/(distRange.y-distRange.x), 0.0, 1.0);
}


void main()
{
    float viewZ = -proj[3][2]/(proj[2][2]+texture(depth, uv).r);
    float viewX = (uv.x*2.0-1.0)*(-viewZ)/proj[0][0];
    float viewY = (uv.y*2.0-1.0)*(-viewZ)/proj[1][1];
    vec3 viewPos = vec3(viewX, viewY, viewZ);

    float dist           = -viewZ;
    float distanceFactor = bezier(getDistanceFactor(dist));

    // Noise

    vec4 v;
    v.xyz = viewPos.xyz*0.5;
    v.w  = frame*0.4;

    //float factor = clamp(distanceFactor+distanceFactor*clamp(fbm(v, 3, 0.5, 0.8), 0.0, 1.0)*0.35, 0.0, 1.0);
    float factor = clamp(distanceFactor, 0.0, 1.0);
    
    color = vec4(colour.rgb*factor, 1.0);
}

