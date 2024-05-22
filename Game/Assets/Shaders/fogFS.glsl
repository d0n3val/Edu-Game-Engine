#version 460 
#extension GL_ARB_shading_language_include : require
#extension GL_ARB_bindless_texture : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/lighting.glsl"

layout(binding = GBUFFER_DEPTH_TEX_BINDING) uniform sampler2D depth;

layout(location = FOG_DENSITY_HEIGHT_FALLOFF_LOCATION) uniform float b;
layout(location = FOG_GLOGAL_DENSITY_LOCATION) uniform float a;
layout(location = FOG_COLOR) uniform vec3 fog_color;
layout(location = FOG_SUN_COLOR) uniform vec3 fog_sun_color;


in vec2 uv;
out vec4 color;

float applyFog(in float t,    // distance to point
               in vec3  ro,   // camera position
               in vec3  rd )  // camera to point vector
{
    return (a/b) * exp(-ro.y*b) * (1.0-exp(-t*rd.y*b))/rd.y;
}


void main()
{
    float d = texture(depth, uv).r;
    vec3 position = getWorldPos(d, uv);

    vec3 rayDir           = position.xyz-view_pos.xyz;
    float distToPoint     = length(rayDir);
    vec3 rayDirNormalized = rayDir/distToPoint;

    float fogAmount = min(applyFog(distToPoint, view_pos.xyz, rayDirNormalized), 0.25); 

    float sunAmount = 0.0; //max( dot( -rayDirNormalized, directional.dir.xyz ), 0.0 );
    vec3  fogColor  = mix(fog_color, //vec3(0.5,0.6,0.7), // bluish
                          fog_sun_color, //vec3(1.0,0.9,0.7), // yellowish
                          pow(sunAmount,8.0) );

    color = vec4(fogColor*fogAmount, 1.0);
}
