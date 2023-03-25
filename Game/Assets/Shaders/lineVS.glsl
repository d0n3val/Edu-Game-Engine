#version 460 
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"
#include "/shaders/cameraDefs.glsl"

uniform mat4 model;
uniform float time;
uniform vec2 tiling;
uniform vec2 offset;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_texcoord;

out vec2 uv;

void main()
{
    uv = vertex_texcoord*tiling+offset;
    uv.x -= time;
    gl_Position = proj*view*model*vec4(vertex_position, 1.0);
}