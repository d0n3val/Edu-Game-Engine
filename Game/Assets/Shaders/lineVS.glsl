#version 460 
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"
#include "/shaders/cameraDefs.glsl"

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_texcoord;

void main()
{
    gl_Position = proj*view*vec4(vertex_position, 1.0);
}