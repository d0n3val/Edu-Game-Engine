#version 460 
#extension GL_ARB_shading_language_include : require
#extension GL_ARB_bindless_texture : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"

layout(binding = GBUFFER_POSITION_TEX_BINDING) uniform sampler2D position;
layout(location = DISTANCE_FOG_MIN_DISTANCE) uniform float minDist;
layout(location = DISTANCE_FOG_MAX_DISTANCE) uniform float maxDist;
layout(location = DISTANCE_FOG_COLOUR) uniform vec3 fogColour;
layout(location = DISTANCE_FOG_CURVE) uniform vec4 fogCurve;


in vec2 uv;
out vec4 color;

float evalBezier(in vec4 points, float t)
{
    vec2 p0 = vec2(0.0); 
    vec2 p1 = vec2(0.0); //vec2(points[0], points[1]);
    vec2 p2 = vec2(1.0, 0.0); //vec2(points[2], points[3]);
    vec2 p3 = vec2(1.0); 

    float t2 = t*t;
    float t3 = t2*t;
    float t_1 = (1.0-t);
    float t2_1 = t_1*t_1;
    float t3_1 = t2_1*t_1;

    return clamp((t3_1*p0+3.0*t*t2_1*p1+3.0*t2*t_1*p2+t3*p3).y, 0.0, 1.0);
}

float getFogFactor(float dist)
{
   return clamp((dist-minDist)/(maxDist-minDist), 0.0, 1.0);
}


void main()
{
    vec4 positionSmp = texture(position, uv);
    float dist   = distance(view_pos.xyz, positionSmp.xyz);
    //float factor = evalBezier(fogCurve, getFogFactor(dist));
    float factor = getFogFactor(dist);
    
    color = vec4(fogColour, factor);
}

