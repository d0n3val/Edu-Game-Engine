#version 460 
#extension GL_ARB_shading_language_include : require
#extension GL_ARB_bindless_texture : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"

layout(binding = GBUFFER_POSITION_TEX_BINDING) uniform sampler2D position;

layout(std140, binding = DISTANCE_FOG_DATA) uniform FogData
{
    vec4 colour;
    vec2 curve;
    vec2 distRange;
    vec2 pad;
};

in vec2 uv;
out vec4 color;


float evaluate(float t)
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

    //return clamp((t3_1*p0+3.0*t*t2_1*p1+3.0*t2*t_1*p2+t3*p3).y, 0.0, 1.0);
    return clamp(t3_1*p0+3.0*t*t2_1*p1+3.0*t2*t_1*p2+t3*p3, 0.0, 1.0);
}

float getFogFactor(float dist)
{
   return clamp((dist-distRange.x)/(distRange.y-distRange.x), 0.0, 1.0);
}


void main()
{
    vec4 positionSmp = texture(position, uv);
    float dist   = distance(view_pos.xyz, positionSmp.xyz);
    float factor = evaluate(getFogFactor(dist));
    //float factor = getFogFactor(dist);
    
    color = vec4(colour.rgb, factor);
}

