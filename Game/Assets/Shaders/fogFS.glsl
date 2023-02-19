#version 460 
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/lighting.glsl"

layout(binding = GBUFFER_POSITION_TEX_BINDING) uniform sampler2D position;

layout(location = FOG_DENSITY_HEIGHT_FALLOFF_LOCATION) uniform float b;
layout(location = FOG_GLOGAL_DENSITY_LOCATION) uniform float a;
layout(location = FOG_COLOR) uniform vec3 fog_color;
layout(location = FOG_SUN_COLOR) uniform vec3 fog_sun_color;


/* TODO
layout(location = FOG_HEIGHT_OFFSET) uniform float height_offset;
layout(location = FOG_DISTANCE_START) uniform float distance_start;
layout(location = FOG_DISTANCE_END) uniform float distance_end;
layout(location = FOG_MAX_OPACITY) uniform float max_opacity;
*/

in vec2 uv;
out vec4 color;

void main()
{
    vec4 positionSmp = texture(position, uv);
    vec3 rayDir      = view_pos.xyz-positionSmp.xyz;
    float pointDist  = length(rayDir);
    rayDir /= pointDist;

    float fogAmount = (a/b) * exp((view_pos.y)*b)*(1.0-exp(-pointDist*rayDir.y*b))/rayDir.y;
    float sunAmount = max( dot( rayDir, directional.dir.xyz ), 0.0 );
    vec3  fogColor  = mix( vec3(0.5,0.6,0.7), // bluish
                           vec3(1.0,0.9,0.7), // yellowish
                           pow(sunAmount,8.0) );

    color = vec4(fogColor, fogAmount);
}