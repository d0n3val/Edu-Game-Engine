#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/cameraDefs.glsl"
#include "/shaders/lighting.glsl"

layout(location = POSITION_ATTRIB_LOCATION) in vec3 vertex_position;
layout(location = DRAW_ID_ATTRIB_LOCATION) in int  draw_id_att;

out vec2 uv;
out flat int draw_id;

void main()
{
    PointLight light = points[draw_id_att];

    vec4 clipping = proj*view*vec4(light.position.xyz+vertex_position*light.position.w, 1.0);
    uv = (clipping.xy/clipping.w)*0.5+0.5;

    draw_id = draw_id_att;

    gl_Position = clipping;

}
