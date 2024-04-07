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

void main()
{
    float d = texture(depth, uv).r;
    vec3 position = getWorldPos(d, uv);

    //vec4 positionSmp = texture(position, uv);
    vec3 rayDir      = view_pos.xyz-position.xyz;
    float pointDist  = length(rayDir);
    rayDir /= pointDist;

    float fogAmount = min((a/b) * exp((view_pos.y)*b)*(1.0-exp(-pointDist*rayDir.y*b))/rayDir.y, 0.5);

    float sunAmount = max( dot( rayDir, directional.dir.xyz ), 0.0 );
    vec3  fogColor  = mix( vec3(0.5,0.6,0.7), // bluish
                           vec3(1.0,0.9,0.7), // yellowish
                           pow(sunAmount,8.0) );

    color = vec4(fogColor*fogAmout, 1.0);
}
