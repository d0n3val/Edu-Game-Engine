#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"

in vec2 uv;
in flat int draw_id;
out vec4 color;

void main()
{
    color = vec4(1.0, 1.0, 1.0, 0.1);
}