#version 460 
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"

uniform sampler2D colorTex;

in vec2 uv;

out vec4 color;

void main()
{
    color = texture(colorTex, uv);
}