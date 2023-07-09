#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/perlin.glsl"

layout(binding = SPOTCONE_DEPTH_BINDING) uniform sampler2D depth;
layout(binding = SPOTCONE_FOG_BINDING0) uniform sampler2D fog0;
layout(binding = SPOTCONE_FOG_BINDING1) uniform sampler2D fog1;

layout(location = SPOTCONE_FOG_TILING0) uniform vec2 tiling0;
layout(location = SPOTCONE_FOG_OFFSET0) uniform vec2 offset0;
layout(location = SPOTCONE_FOG_SPEED0) uniform vec2 speed0;

layout(location = SPOTCONE_FOG_TILING1) uniform vec2 tiling1;
layout(location = SPOTCONE_FOG_OFFSET1) uniform vec2 offset1;
layout(location = SPOTCONE_FOG_SPEED1) uniform vec2 speed1;

layout(location = SPOTCONE_COLOUR) uniform vec4 colour;

layout(location = SPOTCONE_FOG_TIME) uniform float time;

layout(location = SPOTCONE_FOG_TRANSPARENCY) uniform float transparency;
layout(location = SPOTCONE_FOG_SMOOTH_PARTICLE_AMOUNT) uniform float smoothAmount;
layout(location = SPOTCONE_FOG_SMOOTH_FRESNEL_AMOUNT) uniform float fresnelAmount;


out vec4 outColour;

in vec4 clippingPos;
in vec3 worldNormal;
in vec3 worldPos;
in vec2 texcoord;

float linearizeDepth(float d)
{
    float S = proj[2].z;
    float T = proj[3].z;
    float U = proj[2].w;

    d = d*2-1;

    return T/(S-d*U);
}

void main()
{
    vec2 screenPos = clippingPos.xy/clippingPos.w;
    float coneDepth = clippingPos.z/clippingPos.w;
    vec2 screenUV = screenPos*0.5+0.5;

    float depthValue = texture(depth, screenUV).r;

    // smooth particles
    float alphaMult = pow(linearizeDepth(depthValue)-linearizeDepth(coneDepth), smoothAmount);
    alphaMult = clamp(alphaMult, 0.0, 1.0);

    if(depthValue < coneDepth)
        alphaMult = 0.0;

    // Fresnel
    vec3 V = normalize(view_pos.xyz-worldPos);
    vec3 N = normalize(worldNormal);
    float fr = max(dot(V, N), 0.0);
    fr = pow(fr, fresnelAmount);

    alphaMult *= fr;

    vec2 uv0 = texcoord*tiling0+offset0+time*speed0;
    vec2 uv1 = texcoord*tiling1+offset1+time*speed1;
    vec3 color0 = texture(fog0, uv0).rgb;
    vec3 color1 = texture(fog1, uv1).rgb;

    outColour = vec4((color0+color1)*colour.rgb*colour.a, transparency*clamp(alphaMult, 0.0, 1.0));
}
