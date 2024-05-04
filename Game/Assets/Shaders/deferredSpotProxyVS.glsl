#version 460
#extension GL_ARB_shading_language_include : require
#extension GL_ARB_bindless_texture : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/lighting.glsl"

layout(location = POSITION_ATTRIB_LOCATION) in vec3 vertex_position;

out vec2 uv;
out flat int draw_id;

void main()
{
    SpotLight light = spots[gl_InstanceID];

    vec3 position = vertex_position;
    position.y *= light.dist;
    position.xz *= light.direction.w*2.0; 
    position += light.position.xyz;
    //position.xz *= light.color.w;

    // Rotation ? 

    vec4 clipping = proj*view*vec4(position, 1.0);
    uv = (clipping.xy/clipping.w)*0.5+0.5;

    draw_id = gl_InstanceID;

    gl_Position = clipping;

}